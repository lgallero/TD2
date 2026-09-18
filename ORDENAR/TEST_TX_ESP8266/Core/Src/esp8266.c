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

extern UART_HandleTypeDef huart2;
/*==================[macros and definitions]=================================*/

#define RX_BUF_LEN 256
#define TX_BUF_LEN 256
#define USER_BUF_LEN 256
#define ESP8266_QUEUE_LENGTH 4
#define ATOI_BUFFER_SIZE 5

/* Comandos AT*/
#define CWMODE_STR "AT+CWMODE=1\r\n"
#define CWJAP_STR "AT+CWJAP=\"test\",\"12345678\"\r\n"							// CAMBIAR SEGUN LA RED
#define CIPSTART_STR "AT+CIPSTART=\"UDP\",\"10.253.21.169\",8000,8001\r\n"		// CAMBIAR LA IP SEGUN LA RED
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

/*==================[internal data declaration]==============================*/
/*==================[internal functions declaration]=========================*/
/*==================[internal data definition]===============================*/

static volatile bool tx_done = false;

static esp8266_state_t state;

//static uint8_t local_output_buf[RX_BUF_LEN];
static uint8_t rx_buf[RX_BUF_LEN];
static uint8_t tx_buf[TX_BUF_LEN];
static uint8_t tx_auxbuf[USER_BUF_LEN];
static QueueHandle_t tx_queue;
static StaticQueue_t tx_queue_struct;
static uint8_t tx_queue_storage[USER_BUF_LEN * ESP8266_QUEUE_LENGTH];
static bool message_pending;
static size_t pending_length;
static bool wifi_connected_flag;
static bool wifi_got_ip_flag;
/*==================[external data definition]===============================*/
/*==================[internal functions definition]==========================*/

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	(void)huart;
	tx_done = true;
}

static size_t bounded_strlen(const char *str, size_t max_len) {
	size_t len = 0;
	while (len < max_len && str[len] != '\0') {
		len++;
	}
	return len;
}

static void clean_rx_buffer() {
	ESP_UART.pRxBuffPtr = rx_buf;
	ESP_UART.RxXferSize = RX_BUF_LEN;
	ESP_UART.RxXferCount = RX_BUF_LEN;
	memset(rx_buf, 0, RX_BUF_LEN);
}

static bool search_str(char *str) {
	return memmem(rx_buf, RX_BUF_LEN, str, strlen(str)) != NULL;
}

static void taskESP(void *a) {
	while(1){
		HAL_UART_Receive_IT(&ESP_UART, rx_buf, RX_BUF_LEN);
		switch (state){
		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ CONEXION DEL MODULO ESP8226 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		case RESET_MODE:
			tx_done = false;
			clean_rx_buffer();
			HAL_UART_Transmit_IT(&ESP_UART, (uint8_t*) RESET_STR, strlen(RESET_STR));
			vTaskDelay(pdMS_TO_TICKS(10));
			state = WAIT_RESET;
			break;

		case WAIT_RESET:
			if (search_str(READY_MSG)) {
				clean_rx_buffer();
				state = SET_MODE;
			}
			break;

		case SET_MODE:
			state = WAIT_SET_MODE;
			tx_done = false;
			clean_rx_buffer();
			HAL_UART_Transmit_IT(&ESP_UART, (uint8_t*) CWMODE_STR,strlen(CWMODE_STR));
			break;

		case WAIT_SET_MODE:
			if (tx_done){
				state = SET_RED;
				tx_done = false;
			}
			break;

		case SET_RED:
			state = WAIT_RED;
			tx_done = false;
			HAL_UART_Transmit_IT(&ESP_UART, (uint8_t*) CWJAP_STR,strlen(CWJAP_STR));
			break;

		case WAIT_RED:
			if (tx_done){
				state = WAIT_READY_CONNECTED;
				tx_done = false;
				wifi_connected_flag = false;
				wifi_got_ip_flag = false;
			}
			break;

		case WAIT_READY_CONNECTED:
			if (search_str(WIFI_CON)) {
				wifi_connected_flag = true;
			}
			if (search_str(WIFI_GOT)) {
				wifi_got_ip_flag = true;
			}
			if (search_str("FAIL")) {
				wifi_connected_flag = false;
				wifi_got_ip_flag = false;
				state = SET_RED;
				clean_rx_buffer();
				break;
			}
			if (wifi_connected_flag && wifi_got_ip_flag) {
				clean_rx_buffer();
				HAL_UART_Transmit_IT(&ESP_UART, (uint8_t*) CIPSTART_STR,strlen(CIPSTART_STR));
				state = WAIT_UDP_READY;
				tx_done = false;
				wifi_connected_flag = false;
				wifi_got_ip_flag = false;
			}
			break;

		case WAIT_UDP_READY:
			if (search_str(OK_MSG)) {
				clean_rx_buffer();
				tx_done = false;
				state = IDLE;
			}
			break;

			// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ CONEXION COMPLETA ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ENVIO DE DATOS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		case IDLE:
			if (!message_pending && tx_queue != NULL) {
				if (xQueueReceive(tx_queue, tx_auxbuf, 0) == pdPASS) {
					pending_length = bounded_strlen((const char*) tx_auxbuf, USER_BUF_LEN - 1);
					if (pending_length > 0U) {
						tx_auxbuf[pending_length] = '\0';
						message_pending = true;
					} else {
						memset(tx_auxbuf, 0, USER_BUF_LEN);
					}
				}
			}

			if (message_pending) {
				char length_str[8];
				snprintf(length_str, sizeof(length_str), "%u", (unsigned)pending_length);
				snprintf((char*) tx_buf, TX_BUF_LEN, "%s%s%s", CIPSEND_STR, length_str, LINE_MSG);

				if (ESP_UART.gState == HAL_UART_STATE_READY) {
					tx_done = false;
					HAL_UART_Transmit_IT(&ESP_UART, (uint8_t*) tx_buf, strlen((char*) tx_buf));
				}
				memset(rx_buf, 0, RX_BUF_LEN);
				clean_rx_buffer();
				state = WAIT_SEND_DATA;
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
			if (search_str(OK_MSG) && search_str(SIGNO)) {
				vTaskDelay(pdMS_TO_TICKS(5));
				tx_done = false;
				HAL_UART_Transmit_IT(&ESP_UART, (uint8_t*) tx_auxbuf,
						pending_length);
				clean_rx_buffer();
				state = WAIT_TX_COMPLETE;
			}
			break;

		case WAIT_TX_COMPLETE:
			if (tx_done) {
				tx_done = false;
				state = WAIT_SEND_CONFIRM;
				clean_rx_buffer();
			}
			break;

		case WAIT_SEND_CONFIRM:
			if (search_str(SEND_OK)) {
				vTaskDelay(pdMS_TO_TICKS(5));
				clean_rx_buffer();
				state = IDLE;
				message_pending = false;
				pending_length = 0U;
				memset(tx_auxbuf, 0, sizeof(tx_auxbuf));
			}
			break;
		}
	}
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void esp8266_init(void) {
	if (tx_queue == NULL) {
		tx_queue = xQueueCreateStatic(ESP8266_QUEUE_LENGTH, USER_BUF_LEN, tx_queue_storage, &tx_queue_struct);
	}
	message_pending = false;
	pending_length = 0U;
	wifi_connected_flag = false;
	wifi_got_ip_flag = false;
	xTaskCreate(taskESP, "taskESP", configMINIMAL_STACK_SIZE * 2, NULL, osPriorityNormal, NULL);
	state = RESET_MODE;
	clean_rx_buffer();
	memset(tx_buf, 0, sizeof(tx_buf));
	memset(tx_auxbuf, 0, sizeof(tx_auxbuf));
}
// Funcion para cargar data para enviar
int32_t esp8266_send_data(const char *data, size_t len) {
	if (data == NULL) {
		return 0;
	}
	if (len == 0U) {
		len = strlen(data);
	}
	if (len == 0U || len >= USER_BUF_LEN) {
		return 0;
	}
	if (tx_queue == NULL) {
		return 0;
	}
	uint8_t payload[USER_BUF_LEN];
	memset(payload, 0, sizeof(payload));
	size_t copy_len = (len < (USER_BUF_LEN - 1U)) ? len : (USER_BUF_LEN - 1U);
	memcpy(payload, data, copy_len);
	payload[copy_len] = '\0';
	BaseType_t result;
	if (__get_IPSR() != 0U) {
		BaseType_t higher_task_woken = pdFALSE;
		result = xQueueSendFromISR(tx_queue, payload, &higher_task_woken);
		if (higher_task_woken == pdTRUE) {
			portYIELD_FROM_ISR(higher_task_woken);
		}
	} else {
		result = xQueueSend(tx_queue, payload, 0);
	}
	return (result == pdPASS) ? (int32_t)copy_len : 0;
}


/*==================[end of file]============================================*/
