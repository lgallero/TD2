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
#define RX_LINE_MAX_LEN 128
#define RX_LINE_QUEUE_LENGTH 8
#define USER_BUF_LEN 256
#define ESP8266_QUEUE_LENGTH 8

// TIMEOUTS REALISTAS
#define TIMEOUT_WIFI_LINE pdMS_TO_TICKS(8000) // 8 segundos por línea WiFi
#define TIMEOUT_COMANDO pdMS_TO_TICKS(3000) // 3 segundos para OK
#define TIMEOUT_RESET pdMS_TO_TICKS(2000) // 2 segundos después de RST

/* Comandos AT*/
#define CWMODE_STR "AT+CWMODE=1\r\n"
#define CWJAP_STR "AT+CWJAP=\"test\",\"12345678\"\r\n"
#define CIPSTART_STR "AT+CIPSTART=\"UDP\",\"10.253.21.169\",8000,8001\r\n"
#define RESET_STR "AT+RST\r\n"

/* Strings a esperar */
#define OK_MSG "OK"
#define WIFI_CON "WIFI CONNECTED"
#define WIFI_GOT "WIFI GOT IP"
#define SEND_OK "SEND OK"
#define SIGNO ">"
#define READY_MSG "ready"

/*==================[estructuras]============================================*/

typedef struct {
	uint8_t data[USER_BUF_LEN];
	size_t len;
} tx_message_t;

typedef struct {
	char line[RX_LINE_MAX_LEN];
	size_t len;
} rx_line_t;

/*==================[variables]==============================================*/

static volatile bool tx_done = false;
static esp8266_state_t state;

static uint8_t tx_auxbuf[USER_BUF_LEN];

static QueueHandle_t tx_queue;
static StaticQueue_t tx_queue_struct;
static uint8_t tx_queue_storage[sizeof(tx_message_t) * ESP8266_QUEUE_LENGTH];

static QueueHandle_t rx_line_queue;
static StaticQueue_t rx_line_queue_struct;
static uint8_t rx_line_queue_storage[sizeof(rx_line_t) * RX_LINE_QUEUE_LENGTH];

static uint8_t rx_byte;
static char rx_line_buf[RX_LINE_MAX_LEN];
static size_t rx_line_idx = 0;

static bool wifi_connected_flag = false;
static bool wifi_got_ip_flag = false;
static size_t pending_length = 0;

/*==================[prioridades]============================================*/

#define PRIORIDAD_TX osPriorityAboveNormal
#define PRIORIDAD_ESP osPriorityNormal

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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == ESP_UART.Instance) {
		if (rx_byte == '\n' || rx_byte == '\r') {
			if (rx_line_idx > 0) {
				rx_line_buf[rx_line_idx] = '\0';
				rx_line_t line;
				strncpy(line.line, rx_line_buf, RX_LINE_MAX_LEN - 1);
				line.len = rx_line_idx;
				xQueueSendFromISR(rx_line_queue, &line, NULL);
				rx_line_idx = 0;
			}
		} else if (rx_line_idx < RX_LINE_MAX_LEN - 1) {
			rx_line_buf[rx_line_idx++] = rx_byte;
		}
		HAL_UART_Receive_IT(&ESP_UART, &rx_byte, 1);
	}
}

/*==================[helpers]================================================*/

static void limpiar_cola_rx(void)
{
	rx_line_t dummy;
	while (xQueueReceive(rx_line_queue, &dummy, 0) == pdPASS) {
		// vaciar cola
	}
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

static bool esperar_respuesta(const char *str, TickType_t timeout)
{
	rx_line_t rx_line;
	TickType_t start = xTaskGetTickCount();

	while ((xTaskGetTickCount() - start) < timeout) {
		if (xQueueReceive(rx_line_queue, &rx_line, pdMS_TO_TICKS(100)) == pdPASS) {
			while (rx_line.len > 0 && (rx_line.line[rx_line.len - 1] == '\r' || rx_line.line[rx_line.len - 1] == '\n')) {
				rx_line.line[--rx_line.len] = '\0';
			}
			if (strstr(rx_line.line, str) != NULL) {
				return true;
			}
		}
	}
	return false;
}

/*==================[task ESP]===============================================*/

static void taskESP(void *a)
{
	(void)a;

	while (1) {
		switch (state) {
		case RESET_MODE:
			encolar_comando(RESET_STR);
			vTaskDelay(TIMEOUT_RESET);
			limpiar_cola_rx();
			state = SET_MODE;
			break;

		case SET_MODE:
			encolar_comando(CWMODE_STR);
			vTaskDelay(2000);
			if (esperar_respuesta(OK_MSG, TIMEOUT_COMANDO)) {
				limpiar_cola_rx();
				state = SET_RED;
			}
			break;

		case SET_RED:
			encolar_comando(CWJAP_STR);
			vTaskDelay(2000);
			state = WAIT_READY_CONNECTED;
			wifi_connected_flag = false;
			wifi_got_ip_flag = false;
			break;

		case WAIT_READY_CONNECTED:
			vTaskDelay(2000);
			if (esperar_respuesta(WIFI_CON, TIMEOUT_WIFI_LINE)) wifi_connected_flag = true;
			if (esperar_respuesta(WIFI_GOT, TIMEOUT_WIFI_LINE)) wifi_got_ip_flag = true;
			if (esperar_respuesta("FAIL", TIMEOUT_WIFI_LINE)) {
				wifi_connected_flag = false;
				wifi_got_ip_flag = false;
				limpiar_cola_rx();
				state = SET_RED;
				break;
			}
			if (wifi_connected_flag && wifi_got_ip_flag) {
				encolar_comando(CIPSTART_STR);
				vTaskDelay(2000);
				if (esperar_respuesta(OK_MSG, TIMEOUT_COMANDO)) {
					limpiar_cola_rx();
					HAL_GPIO_WritePin(LED_OFF_GPIO_Port, LED_OFF_Pin, SET);
					HAL_GPIO_WritePin(LED_ON_GPIO_Port, LED_ON_Pin, RESET);
					state = IDLE;
				}
			}
			break;

		case IDLE:
			HAL_GPIO_TogglePin(LED_ON_GPIO_Port, LED_ON_Pin);
			vTaskDelay(500);
			break;

		case WAIT_SEND_DATA:
			state = SEND_DATA;
			break;

		case SEND_DATA:
			if (esperar_respuesta(SIGNO, pdMS_TO_TICKS(2000))) {
				char cipsend_cmd[32];
				snprintf(cipsend_cmd, sizeof(cipsend_cmd), "AT+CIPSEND=%d\r\n", (int)pending_length);
				encolar_comando(cipsend_cmd);
				state = WAIT_TX_COMPLETE;
			}
			break;

		case WAIT_TX_COMPLETE:
			tx_message_t msg;
			memcpy(msg.data, tx_auxbuf, pending_length);
			msg.len = pending_length;
			xQueueSend(tx_queue, &msg, 0);
			state = WAIT_SEND_CONFIRM;
			break;

		case WAIT_SEND_CONFIRM:
			if (esperar_respuesta(SEND_OK, TIMEOUT_COMANDO)) {
				limpiar_cola_rx();
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
	rx_line_queue = xQueueCreateStatic(RX_LINE_QUEUE_LENGTH, sizeof(rx_line_t), rx_line_queue_storage, &rx_line_queue_struct);

	xTaskCreate(tarea_serie_tx, "txESP", configMINIMAL_STACK_SIZE, NULL, PRIORIDAD_TX, NULL);
	xTaskCreate(taskESP, "taskESP", configMINIMAL_STACK_SIZE * 3, NULL, PRIORIDAD_ESP, NULL);

	state = RESET_MODE;
	rx_line_idx = 0;
	pending_length = 0;
	memset(tx_auxbuf, 0, sizeof(tx_auxbuf));

	HAL_GPIO_WritePin(LED_OFF_GPIO_Port, LED_OFF_Pin, RESET);
	HAL_GPIO_WritePin(LED_ON_GPIO_Port, LED_ON_Pin, SET);

	HAL_UART_Receive_IT(&ESP_UART, &rx_byte, 1);
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
