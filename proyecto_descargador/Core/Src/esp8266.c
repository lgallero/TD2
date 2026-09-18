/*
 * esp8266.c
 *
 * Versión: CON LECTURA DE IP (AT+CIPSTA?).
 * - Muestra la IP asignada por el router antes de abrir el socket.
 * - Mantiene Blind Send y Flush RX.
 */

#define _GNU_SOURCE
#include "esp8266.h"
#include "fsm_monitor.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/* ======================= CONFIGURACIÓN DE RED ======================= */
#define WIFI_SSID       "TeleCentro-038d"
#define WIFI_PASS       "R2NKJMMUWYMM"
#define SERVER_IP       "192.168.0.101"
#define SERVER_PORT     8000
#define LOCAL_PORT      8001

/* ======================= TIMEOUTS Y DEFINES ======================= */
#define RX_BUF_LEN      ESP_RX_PAYLOAD_MAX
#define TASK_DELAY_MS   50

#define TIMEOUT_CMD_TICKS   (3000 / TASK_DELAY_MS)
#define TIMEOUT_WIFI_TICKS  (15000 / TASK_DELAY_MS)

#define RESET_CMD       "AT+RST\r\n"
#define CWMODE_CMD      "AT+CWMODE=1\r\n"
#define CIPSTA_CMD      "AT+CIPSTA?\r\n" // <--- COMANDO NUEVO

#define OK_MSG          "OK"
#define SOK_MSG         "SEND OK"
#define WCONN_MSG       "WIFI CONNECTED"
#define WGIP_MSG        "WIFI GOT IP"
#define DATA_MSG        ">"

/* ======================= VARIABLES ======================= */
extern UART_HandleTypeDef huart1;
#define ESP_UART huart1

static uint8_t rx_buf[RX_BUF_LEN];
static uint8_t tx_auxbuf[RX_BUF_LEN];
static uint32_t recv_total_size;
static size_t pending_length = 0;
static volatile bool tx_done = false;
static volatile bool task_done = false;

static volatile bool flag_envio_pendiente = false;
static volatile FSM_ESP_STATES_T esp_state;
static uint32_t timeout_cnt = 0;

typedef struct { uint8_t data[RX_BUF_LEN]; size_t len; } tx_message_t;
static QueueHandle_t tx_queue;
static StaticQueue_t tx_queue_struct;
static uint8_t tx_queue_storage[sizeof(tx_message_t) * 8];
QueueHandle_t cola_rx = NULL;

/* ======================= FUNCIONES AUXILIARES ======================= */
static void encolar_comando(const char *cmd) {
    tx_message_t msg;
    size_t len = strlen(cmd);
    if (len >= RX_BUF_LEN) return;
    memcpy(msg.data, cmd, len);
    msg.len = len; msg.data[len] = 0;
    xQueueSend(tx_queue, &msg, 0);
}

static void clear_rx_buffer(void) {
    memset(rx_buf, 0, RX_BUF_LEN);
    huart1.RxXferCount = 0;
    huart1.pRxBuffPtr = rx_buf;
}

static void clear_rx_if_full(void) {
    if (memchr(rx_buf, '\0', RX_BUF_LEN) == NULL) clear_rx_buffer();
}

static bool search_str(const char *str) {
    return strstr((char*)rx_buf, str) != NULL;
}

static uint32_t get_rcv_total_size() {
    char atoiBuffer[5]; char *p = strstr((char*)rx_buf, "+IPD,");
    if (!p) return 0; p += 5; int i=0;
    while (p[i] != ':' && i < 4) { atoiBuffer[i] = p[i]; i++; } atoiBuffer[i] = 0;
    return atoi(atoiBuffer);
}
static uint32_t get_rcv_size() { char *p = strstr((char*)rx_buf, ":"); return p ? strlen(p+1) : 0; }

static void forward_payload_to_queue(size_t payload_len) {
    if (!cola_rx || !payload_len) return;
    esp_rx_msg_t msg;
    char *p = strstr((char*)rx_buf, ":"); if(!p) return; p++;
    size_t len = (payload_len >= 255) ? 254 : payload_len;
    memcpy(msg.data, p, len); msg.data[len]=0; msg.len=len;
    xQueueSend(cola_rx, &msg, portMAX_DELAY);
}

/* ======================= TAREAS ======================= */

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) { if(huart==&ESP_UART) tx_done = true; }

static void task_Tx(void* p) {
    tx_message_t msg;
    while (1) {
        if (xQueueReceive(tx_queue, &msg, portMAX_DELAY) == pdPASS) {
            tx_done = false; HAL_UART_Transmit_IT(&ESP_UART, msg.data, msg.len);
            while (!tx_done) vTaskDelay(1);
        }
    }
}

static void task_ESP(void* p) {
    char cmd_buffer[128]; bool iniciado = false;

    HAL_GPIO_WritePin(WIFI_STATE_GPIO_Port, WIFI_STATE_Pin, GPIO_PIN_RESET);

    printf("[ESP] Iniciando...\r\n");
    encolar_comando(RESET_CMD);
    vTaskDelay(pdMS_TO_TICKS(2000));
    clear_rx_buffer();

    while(1) {
        HAL_UART_Receive_IT(&ESP_UART, rx_buf, RX_BUF_LEN);
        clear_rx_if_full();

        switch (esp_state) {

        case WAIT_RESET:
            HAL_GPIO_WritePin(WIFI_STATE_GPIO_Port, WIFI_STATE_Pin, GPIO_PIN_RESET);
            esp_state = SET_MODE; timeout_cnt=0; clear_rx_buffer();
            break;

        case SET_MODE:
            encolar_comando(CWMODE_CMD);
            esp_state=WAIT_SET_MODE; timeout_cnt=0;
            break;

        case WAIT_SET_MODE:
            if (search_str(OK_MSG)) { clear_rx_buffer(); esp_state=SET_RED; }
            else if (++timeout_cnt > TIMEOUT_CMD_TICKS) esp_state=SET_MODE;
            break;

        case SET_RED:
            printf("[ESP] Conectando a WiFi %s...\r\n", WIFI_SSID);
            snprintf(cmd_buffer, 128, "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASS);
            encolar_comando(cmd_buffer);
            esp_state=WAIT_RED; timeout_cnt=0;
            break;

        case WAIT_RED:
            if (search_str(WCONN_MSG) && search_str(WGIP_MSG)) {
                printf("[ESP] WiFi CONECTADO. Obteniendo IP...\r\n");
                // Ahora vamos a pedir la IP en lugar de saltar directo al UDP
                esp_state=GET_IP;
            }
            else if (search_str("FAIL")) {
                printf("[ESP] WiFi FAIL. Reintentando...\r\n");
                vTaskDelay(1000); esp_state=SET_RED; clear_rx_buffer();
            }
            else if (++timeout_cnt > TIMEOUT_WIFI_TICKS) {
                esp_state=SET_RED; clear_rx_buffer();
            }
            break;

        // --- ESTADO NUEVO: PEDIR IP ---
        case GET_IP:
            vTaskDelay(pdMS_TO_TICKS(500)); // Esperar un poquito tras el GOT IP
            clear_rx_buffer();
            encolar_comando(CIPSTA_CMD);
            esp_state=WAIT_IP; timeout_cnt=0;
            break;

        // --- ESTADO NUEVO: LEER IP ---
        case WAIT_IP:
            if (search_str(OK_MSG)) {
                // Buscamos la respuesta: +CIPSTA:ip:"192.168.1.XX"
                char *p = strstr((char*)rx_buf, "+CIPSTA:ip:\"");
                if (p) {
                    p += 12; // Saltamos +CIPSTA:ip:"
                    char *q = strchr(p, '"'); // Buscamos la comilla de cierre
                    if (q) {
                        *q = 0; // Cortamos el string temporalmente
                        printf("[ESP] IP Asignada: %s\r\n", p);
                    }
                } else {
                    printf("[ESP] IP no parseada (Raw OK).\r\n");
                }
                clear_rx_buffer();
                esp_state=SET_UDP; // Seguimos flujo normal
            }
            else if (++timeout_cnt > TIMEOUT_CMD_TICKS) {
                printf("[ESP] Timeout pidiendo IP. Continuando...\r\n");
                esp_state=SET_UDP; // Seguimos igual aunque falle
            }
            break;
        // ------------------------------

        case SET_UDP:
            vTaskDelay(pdMS_TO_TICKS(500)); clear_rx_buffer();
            printf("[ESP] Abriendo UDP...\r\n");
            snprintf(cmd_buffer, 128, "AT+CIPSTART=\"UDP\",\"%s\",%d,%d\r\n", SERVER_IP, SERVER_PORT, LOCAL_PORT);
            encolar_comando(cmd_buffer);
            esp_state=WAIT_UDP; timeout_cnt=0;
            break;

        case WAIT_UDP:
            if (search_str(OK_MSG) || search_str("ALREADY") || search_str("CONNECT")) {
                printf("[ESP] ONLINE. Link Listo.\r\n");
                HAL_GPIO_WritePin(WIFI_STATE_GPIO_Port, WIFI_STATE_Pin, GPIO_PIN_SET);
                esp_state=IDLE_ESP; clear_rx_buffer();
            }
            else if (++timeout_cnt > TIMEOUT_CMD_TICKS) esp_state=SET_UDP;
            break;

        case IDLE_ESP:
            if (!iniciado) {
                extern void ev_esp_ON_raise(void); ev_esp_ON_raise(); iniciado=true;
            }

            if (!task_done && flag_envio_pendiente) {
                flag_envio_pendiente = false;
                task_done=true;
                clear_rx_buffer();
                esp_state=SEND_LEN;
            }

            if (search_str("+IPD") && search_str(":")) {
                recv_total_size=get_rcv_total_size();
                esp_state=RECEIVING; timeout_cnt=0;
            }

            if (search_str("CLOSED") || search_str("link is not")) {
                printf("[ESP] ERROR: Link perdido.\r\n");
                esp_state=SET_UDP;
            }
            break;

        case RECEIVING:
            if (get_rcv_size() >= recv_total_size) {
                forward_payload_to_queue(get_rcv_size());
                esp_state=IDLE_ESP; clear_rx_buffer();
            }
            else if (++timeout_cnt > TIMEOUT_CMD_TICKS) {
                esp_state=IDLE_ESP; clear_rx_buffer();
            }
            break;

        case SEND_LEN:
            snprintf(cmd_buffer, 128, "AT+CIPSEND=%d\r\n", (int)pending_length);
            encolar_comando(cmd_buffer);
            vTaskDelay(pdMS_TO_TICKS(200));
            clear_rx_buffer(); esp_state=SEND_DATA;
            break;

        case SEND_DATA:
            encolar_comando((const char *)tx_auxbuf);
            vTaskDelay(pdMS_TO_TICKS(500));
            task_done=false;
            clear_rx_buffer(); esp_state=IDLE_ESP;
            break;

        case WAIT_LEN: case WAIT_DATA: esp_state=IDLE_ESP; break;

        default: break;
        }

        vTaskDelay(pdMS_TO_TICKS(TASK_DELAY_MS));
    }
}

void esp8266_init(void) {
    tx_queue = xQueueCreateStatic(8, sizeof(tx_message_t), tx_queue_storage, &tx_queue_struct);
    HAL_GPIO_WritePin(WIFI_STATE_GPIO_Port, WIFI_STATE_Pin, GPIO_PIN_RESET);
    esp_state = WAIT_RESET;
    xTaskCreate(task_ESP, "task_ESP", 512, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(task_Tx, "task_Tx", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
    clear_rx_buffer();
}

void esp8266_send_data(const char *data, size_t len) {
    if (len >= RX_BUF_LEN) len=RX_BUF_LEN-1;
    memcpy(tx_auxbuf, data, len); tx_auxbuf[len]=0;
    pending_length = len;

    if (esp_state == IDLE_ESP || esp_state == RECEIVING) {
        flag_envio_pendiente = true;
    }
}

void esp8266_flush_rx(void) {
    clear_rx_buffer();
    if (cola_rx != NULL) xQueueReset(cola_rx);
}
