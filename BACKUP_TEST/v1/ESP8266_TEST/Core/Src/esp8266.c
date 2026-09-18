/*==================[inclusions]=============================================*/
#define _GNU_SOURCE

#include "esp8266.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "queue.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

extern UART_HandleTypeDef huart1;
#define ESP_UART huart1

/*==================[macros and definitions]=================================*/

#define RX_BUF_LEN 256
#define TX_BUF_LEN 256
#define USER_BUF_LEN 256
#define ESP8266_QUEUE_LENGTH 8

/* Comandos AT*/
#define CWMODE_STR "AT+CWMODE=1\r\n"
#define CWJAP_STR "AT+CWJAP=\"test\",\"12345678\"\r\n"
#define CIPSTART_STR "AT+CIPSTART=\"UDP\",\"10.253.21.169\",8000,8001\r\n"
#define RESET_STR "AT+RST\r\n"

/* Strings a esperar */
#define OK_MSG "OK\r\n"
#define WIFI_CON "WIFI CONNECTED\r\n"
#define WIFI_GOT "WIFI GOT IP\r\n"
#define SEND_OK "SEND OK\r\n"
#define SIGNO ">"
#define READY_MSG "ready"

/*==================[estructuras]============================================*/

typedef struct {
    uint8_t data[USER_BUF_LEN];
    size_t len;
} tx_message_t;

/*==================[variables]==============================================*/

static volatile bool tx_done = false; // Solo para tarea_serie_tx
static esp8266_state_t state;

static uint8_t rx_buf[RX_BUF_LEN];
static uint8_t tx_auxbuf[USER_BUF_LEN];

static QueueHandle_t tx_queue;
static StaticQueue_t tx_queue_struct;
static uint8_t tx_queue_storage[sizeof(tx_message_t) * ESP8266_QUEUE_LENGTH];

static bool wifi_connected_flag = false;
static bool wifi_got_ip_flag = false;
static size_t pending_length = 0;

/*==================[tarea TX]================================================*/

static void tarea_serie_tx(void *p)
{
    (void)p;
    tx_message_t msg;

    while (1) {
        if (xQueueReceive(tx_queue, &msg, portMAX_DELAY) == pdPASS) {
            HAL_UART_Transmit_IT(&ESP_UART, msg.data, msg.len);
            while (!tx_done) {
                vTaskDelay(1);
            }
            tx_done = false;
        }
    }
}

/*==================[callbacks]==============================================*/

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    (void)huart;
    tx_done = true;
}

/*==================[helpers]================================================*/

static void clean_rx_buffer(void)
{
    ESP_UART.pRxBuffPtr = rx_buf;
    ESP_UART.RxXferSize = RX_BUF_LEN;
    ESP_UART.RxXferCount = RX_BUF_LEN;
    memset(rx_buf, 0, RX_BUF_LEN);
}

static bool search_str(const char *str)
{
    return memmem(rx_buf, RX_BUF_LEN, str, strlen(str)) != NULL;
}

static void encolar_comando(const char *cmd)
{
    tx_message_t msg;
    size_t len = strlen(cmd);
    if (len >= USER_BUF_LEN) return;
    memcpy(msg.data, cmd, len);
    msg.len = len;
    xQueueSend(tx_queue, &msg, 0);
}

/*==================[task ESP]===============================================*/

static void taskESP(void *a)
{
    (void)a;

    while (1) {
        HAL_UART_Receive_IT(&ESP_UART, rx_buf, RX_BUF_LEN);

        switch (state) {
            case RESET_MODE:
                encolar_comando(RESET_STR);
                vTaskDelay(pdMS_TO_TICKS(1000));
                state = SET_MODE;
                break;

            case SET_MODE:
                encolar_comando(CWMODE_STR);
                vTaskDelay(pdMS_TO_TICKS(50));
                state = SET_RED; // AVANZA DIRECTO DESPUÉS DE ENCOLAR
                break;

            case SET_RED:
                encolar_comando(CWJAP_STR);
                vTaskDelay(pdMS_TO_TICKS(50));
                state = WAIT_READY_CONNECTED;
                wifi_connected_flag = false;
                wifi_got_ip_flag = false;
                break;

            case WAIT_READY_CONNECTED:
                if (search_str(WIFI_CON)) wifi_connected_flag = true;
                if (search_str(WIFI_GOT)) wifi_got_ip_flag = true;
                if (search_str("FAIL")) {
                    wifi_connected_flag = false;
                    wifi_got_ip_flag = false;
                    state = SET_RED;
                    clean_rx_buffer();
                    break;
                }
                if (wifi_connected_flag && wifi_got_ip_flag) {
                    clean_rx_buffer();
                    encolar_comando(CIPSTART_STR);
                    vTaskDelay(pdMS_TO_TICKS(50));
                    state = WAIT_UDP_READY;
                    wifi_connected_flag = false;
                    wifi_got_ip_flag = false;
                }
                break;

            case WAIT_UDP_READY:
                if (search_str(OK_MSG)) {
                    clean_rx_buffer();
                    HAL_GPIO_WritePin(LED_OFF_GPIO_Port, LED_OFF_Pin, SET);
                    HAL_GPIO_WritePin(LED_ON_GPIO_Port, LED_ON_Pin, RESET);
                    state = IDLE;
                }
                break;

            case IDLE:
                HAL_GPIO_TogglePin(LED_ON_GPIO_Port, LED_ON_Pin);
                vTaskDelay(500);
                break;

            case WAIT_SEND_DATA:
                state = SEND_DATA;
                clean_rx_buffer();
                break;

            case SEND_DATA:
                if (search_str(OK_MSG) && search_str(SIGNO)) {
                    vTaskDelay(pdMS_TO_TICKS(5));
                    char cipsend_cmd[32];
                    snprintf(cipsend_cmd, sizeof(cipsend_cmd), "AT+CIPSEND=%d\r\n", (int)pending_length);
                    encolar_comando(cipsend_cmd);
                    state = WAIT_TX_COMPLETE;
                }
                break;

            case WAIT_TX_COMPLETE:
                // Payload se encola en tarea_serie_tx después de CIPSEND
                tx_message_t msg;
                memcpy(msg.data, tx_auxbuf, pending_length);
                msg.len = pending_length;
                xQueueSend(tx_queue, &msg, 0);
                state = WAIT_SEND_CONFIRM;
                clean_rx_buffer();
                break;

            case WAIT_SEND_CONFIRM:
                if (search_str(SEND_OK)) {
                    vTaskDelay(pdMS_TO_TICKS(5));
                    clean_rx_buffer();
                    state = IDLE;
                    pending_length = 0;
                    memset(tx_auxbuf, 0, sizeof(tx_auxbuf));
                }
                break;
        }
    }
}

/*==================[init]===================================================*/

void esp8266_init(void)
{
    tx_queue = xQueueCreateStatic(ESP8266_QUEUE_LENGTH, sizeof(tx_message_t), tx_queue_storage, &tx_queue_struct);

    xTaskCreate(tarea_serie_tx, "txESP", configMINIMAL_STACK_SIZE, NULL, osPriorityAboveNormal, NULL);
    xTaskCreate(taskESP, "taskESP", configMINIMAL_STACK_SIZE * 2, NULL, osPriorityNormal, NULL);

    state = RESET_MODE;
    clean_rx_buffer();
    memset(tx_auxbuf, 0, sizeof(tx_auxbuf));
    pending_length = 0;

    HAL_GPIO_WritePin(LED_OFF_GPIO_Port, LED_OFF_Pin, RESET);
    HAL_GPIO_WritePin(LED_ON_GPIO_Port, LED_ON_Pin, SET);
}

/*==================[envío público]==========================================*/

int32_t esp8266_send_data(const char *data, size_t len)
{
    if (data == NULL || len == 0 || len >= USER_BUF_LEN || tx_queue == NULL) return 0;

    size_t copy_len = (len < USER_BUF_LEN) ? len : USER_BUF_LEN - 1;
    memcpy(tx_auxbuf, data, copy_len);
    pending_length = copy_len;

    state = WAIT_SEND_DATA;

    return (int32_t)copy_len;
}
