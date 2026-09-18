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
/*==================[macros and definitions]=================================*/

#define RX_BUF_LEN 256
#define TX_BUF_LEN 256
#define USER_BUF_LEN 256
#define ATOI_BUFFER_SIZE 5

/* Comandos AT*/
#define CWMODE_STR "AT+CWMODE=1\r\n"
#define CWJAP_STR "AT+CWJAP=\"test\",\"12345678\"\r\n"                     // CAMBIAR SEGUN LA RED
#define CIPSTART_STR "AT+CIPSTART=\"UDP\",\"10.253.21.169\",8000,8001\r\n"   // CAMBIAR LA IP SEGUN LA RED
#define CIPSEND_STR "AT+CIPSEND="
#define AT_STR "AT\r\n"
#define RESET_STR "AT+RST\r\n"

/* Strings a esperar */
#define OK_MSG "OK\r\n"
#define WIFI_CON "WIFI CONNECTED\r\n"
#define WIFI_GOT "WIFI GOT IP\r\n"
#define CONNECT_MSG "CONNECT\r\n"
#define SEND_OK "SEND OK\r\n"
#define SIGNO ">"
#define LINE_MSG "\r\n"
#define READY_MSG "ready"

#define CMD_LED_ON "prender"
#define CMD_LED_OFF "apagar"

/*==================[internal data declaration]==============================*/
/*==================[internal functions declaration]=========================*/
/*==================[internal data definition]===============================*/

#define BUTTON_MESSAGE "Hola desde el ESP!"

typedef enum {
    ESP_EVENT_NONE = 0,
    ESP_EVENT_BUTTON,
} esp8266_event_t;

static QueueHandle_t esp_event_queue;

static volatile bool evSendData;
static volatile bool tx_done = false;

static bool button_request = false;

static esp8266_state_t state;

//static uint8_t local_output_buf[RX_BUF_LEN];
static uint8_t rx_buf[RX_BUF_LEN];
static uint8_t tx_buf[TX_BUF_LEN];
static uint8_t tx_auxbuf[USER_BUF_LEN];
static volatile uint8_t rx_byte;
static volatile size_t rx_len;
/*==================[external data definition]===============================*/
/*==================[internal functions definition]==========================*/

static void clear_events(void)
{
    evSendData = false;
}

static void reset_idle_context(void)
{
    clear_events();
    tx_done = false;
    button_request = false;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &ESP_UART) {
        tx_done = true;
    }
}

static void clean_rx_buffer(void)
{
    taskENTER_CRITICAL();
    rx_len = 0;
    memset(rx_buf, 0, RX_BUF_LEN);
    taskEXIT_CRITICAL();
}

static bool search_str(const char *str)
{
    bool found;

    taskENTER_CRITICAL();
    found = (rx_len > 0U) && (strstr((char *)rx_buf, str) != NULL);
    taskEXIT_CRITICAL();

    return found;
}

static bool handle_incoming_command(void)
{
	bool handled = false;

	if (search_str(CMD_LED_ON)) {
		HAL_GPIO_WritePin(LED_BP_GPIO_Port, LED_BP_Pin, GPIO_PIN_RESET);
		handled = true;
	}
	else if (search_str(CMD_LED_OFF)) {
		HAL_GPIO_WritePin(LED_BP_GPIO_Port, LED_BP_Pin, GPIO_PIN_SET);
		handled = true;
	}

	if (handled) {
		clean_rx_buffer();
		reset_idle_context();
	}

	return handled;
}

static void taskESP(void *a)
{
    (void)a;

    //HAL_GPIO_WritePin(ESP_ENABLE_GPIO_Port, ESP_ENABLE_Pin, GPIO_PIN_RESET);
    vTaskDelay(5);
    //HAL_GPIO_WritePin(ESP_ENABLE_GPIO_Port, ESP_ENABLE_Pin, GPIO_PIN_SET);

    while (1) {
        if (esp_event_queue != NULL) {
            esp8266_event_t event;
            while (xQueueReceive(esp_event_queue, &event, 0) == pdPASS) {
                if (event == ESP_EVENT_BUTTON) {
                    button_request = true;
                }
            }
        }
        switch (state) {
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ CONEXION DEL MODULO ESP8226 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case WAIT_RESET:
            vTaskDelay(200);
            state = SET_MODE;
            break;


        case SET_MODE:
            state = WAIT_SET_MODE;
            tx_done = false;
            clean_rx_buffer();
            HAL_UART_Transmit_IT(&ESP_UART, (uint8_t *)CWMODE_STR, strlen(CWMODE_STR));
            break;

        case WAIT_SET_MODE:
            if (tx_done) {
                state = SET_RED;
                tx_done = false;
            }
            break;

        case SET_RED:
            state = WAIT_RED;
            tx_done = false;
            HAL_UART_Transmit_IT(&ESP_UART, (uint8_t *)CWJAP_STR, strlen(CWJAP_STR));
            break;

        case WAIT_RED:
            if (tx_done) {
                state = WAIT_READY_CONNECTED;
                tx_done = false;
            }
            break;

        case WAIT_READY_CONNECTED:
            if (search_str(WIFI_CON) && search_str(WIFI_GOT)) {
                clean_rx_buffer();
                HAL_UART_Transmit_IT(&ESP_UART, (uint8_t *)CIPSTART_STR, strlen(CIPSTART_STR));
                state = WAIT_UDP_READY;
                tx_done = false;
            }
            break;

        case WAIT_UDP_READY:
            if (search_str(OK_MSG)) {
                clean_rx_buffer();
                tx_done = false;
                state = IDLE;
                HAL_GPIO_WritePin(LED_ON_GPIO_Port, LED_ON_Pin, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(LED_OFF_GPIO_Port, LED_OFF_Pin, GPIO_PIN_SET);
            }
            break;

            // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ CONEXION COMPLETA ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ENVIO DE DATOS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        case IDLE:
            if (handle_incoming_command()) {
                break;
            }
            if (button_request && !evSendData) {
                strncpy((char *)tx_auxbuf, BUTTON_MESSAGE, USER_BUF_LEN - 1);
                tx_auxbuf[USER_BUF_LEN - 1] = '\0';
                evSendData = true;
                button_request = false;
            }
            if (!evSendData) {
                vTaskDelay(pdMS_TO_TICKS(10));
                break;
            }
            if (evSendData) {
                char length_str[8];  // para números hasta 9999999
                uint8_t length = strlen((const char *)tx_auxbuf);

                // Convertir el largo del mensaje en string
                snprintf(length_str, sizeof(length_str), "%u", (unsigned)length);

                // Armar el comando AT+CIPSEND=XX\r\n
                snprintf((char *)tx_buf, TX_BUF_LEN, "%s%s%s", CIPSEND_STR, length_str, LINE_MSG);

                // Enviar el comando al ESP
                if (ESP_UART.gState == HAL_UART_STATE_READY) {
                    tx_done = false;
                    HAL_UART_Transmit_IT(&ESP_UART, (uint8_t *)tx_buf, strlen((char *)tx_buf));
                }
                // Limpiar el buffer de recepción
                memset(rx_buf, 0, RX_BUF_LEN);
                clean_rx_buffer();

                // Pasar al siguiente estado
                state = WAIT_SEND_DATA;
                clear_events();
            }
            break;

        case WAIT_SEND_DATA:
            if (tx_done) {
                tx_done = false;
                state = SEND_DATA;
                clean_rx_buffer();
            }
            break;

        case SEND_DATA:
            if (search_str(SIGNO)) {
                HAL_UART_Transmit_IT(&ESP_UART, (uint8_t *)tx_auxbuf, strlen((char *)tx_auxbuf));
                clean_rx_buffer();
                state = WAIT_SEND_OK;
            }
            break;

        case WAIT_SEND_OK:
            if (search_str(SEND_OK)) {
                clean_rx_buffer();
                reset_idle_context();
                state = IDLE;
            } else if (search_str("ERROR")) {
                clean_rx_buffer();
                reset_idle_context();
                state = IDLE;
            }
            break;
        }
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void esp8266_init(void)
{
    xTaskCreate(taskESP, "taskESP", configMINIMAL_STACK_SIZE * 2, NULL, osPriorityNormal, NULL);
    state = WAIT_RESET;
    HAL_GPIO_WritePin(LED_ON_GPIO_Port, LED_ON_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_OFF_GPIO_Port, LED_OFF_Pin, GPIO_PIN_RESET);
    //HAL_GPIO_WritePin(ESP_ENABLE_GPIO_Port, ESP_ENABLE_Pin, GPIO_PIN_RESET);
    clean_rx_buffer();
    memset(tx_buf, 0, sizeof(tx_buf));
    memset(tx_auxbuf, 0, sizeof(tx_auxbuf));
    esp_event_queue = xQueueCreate(4, sizeof(esp8266_event_t));
    HAL_UART_Receive_IT(&ESP_UART, (uint8_t *)&rx_byte, 1);
    tx_done = false;
    HAL_UART_Transmit_IT(&ESP_UART, (uint8_t *)RESET_STR, strlen(RESET_STR));
}

void esp8266_raise_evSend(void)
{
    evSendData = true;
}

// Funcion para cargar data para enviar
int32_t esp8266_send_data(const char *data, size_t len)
{
    if (state == IDLE && len < USER_BUF_LEN) {
        strncpy((char *)tx_auxbuf, data, USER_BUF_LEN - 1);  // copia segura
        tx_auxbuf[USER_BUF_LEN - 1] = '\0';                  // terminador
        esp8266_raise_evSend();
        return strlen((char *)tx_auxbuf);                    // confirmás lo que copiaste
    } else {
        return 0;  // ESP ocupado o mensaje demasiado largo
    }
}

void esp8266_notify_button_from_isr(BaseType_t *higher_priority_task_woken)
{
    if (esp_event_queue == NULL) {
        return;
    }

    esp8266_event_t event = ESP_EVENT_BUTTON;
    xQueueSendFromISR(esp_event_queue, &event, higher_priority_task_woken);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &ESP_UART) {
        UBaseType_t saved = taskENTER_CRITICAL_FROM_ISR();
        if (rx_len < RX_BUF_LEN - 1U) {
            rx_buf[rx_len++] = (char)rx_byte;
            rx_buf[rx_len] = '\0';
        } else {
            memmove(rx_buf, rx_buf + 1, RX_BUF_LEN - 2U);
            rx_len = RX_BUF_LEN - 2U;
            rx_buf[rx_len++] = (char)rx_byte;
            rx_buf[rx_len] = '\0';
        }
        taskEXIT_CRITICAL_FROM_ISR(saved);

        HAL_UART_Receive_IT(&ESP_UART, (uint8_t *)&rx_byte, 1);
    }
}


/*==================[end of file]============================================*/
