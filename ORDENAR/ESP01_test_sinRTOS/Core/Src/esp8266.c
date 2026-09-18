/*
 * esp8266.c
 *
 *  Created on: Sep 18, 2025
 *      Author: lucas
 */

/*==================[inclusions]=============================================*/
#define _GNU_SOURCE

#include "esp8266.h"
#include <stdint.h>
#include <stdbool.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/*==================[macros and definitions]=================================*/

#define RX_BUF_LEN 256
#define TX_BUF_LEN 256
#define USER_BUF_LEN 256
#define INITIAL_COUNT_MS 100
#define FINAL_COUNT_MS 0
#define DELAY_INIT_MS 200
#define ATOI_BUFFER_SIZE 5

/* Comandos AT*/
#define CWMODE_STR "AT+CWMODE=1\r\n"
#define CWJAP_STR "AT+CWJAP=\"test\",\"12345678\"\r\n"			// CAMBIAR SEGUN LA RED
#define CIPSTART_STR "AT+CIPSTART=\"UDP\",\"10.253.21.169\",8000,8001\r\n"		// CAMBIAR LA IP SEGUN LA RED
#define CIPSEND_STR "AT+CIPSEND="

/* Strings a esperar */
#define OK_MSG "OK\r\n"
#define WIFI_CON "WIFI CONNECTED\r\n"
#define WIFI_GOT "WIFI GOT IP\r\n"
#define CONNECT_MSG "CONNECT\r\n"
#define SEND_OK "SEND OK\r\n"
#define SIGNO ">"
#define LINE_MSG "\r\n"

#define TEST_MSG "HOLA DESDE ESP!\r\n"
#define LEN_TEST_MSG "AT+CIPSEND=17"


/*==================[internal data declaration]==============================*/
/*==================[internal functions declaration]=========================*/
/*==================[internal data definition]===============================*/

static bool evTick1ms;
static bool evSendData;
static bool tx_done = false;

static esp8266_state_t state;
static uint32_t count_ms;

//static uint8_t local_output_buf[RX_BUF_LEN];
static uint8_t rx_buf[RX_BUF_LEN];
static uint8_t tx_buf[TX_BUF_LEN];
static uint8_t tx_auxbuf[USER_BUF_LEN];


/*==================[external data definition]===============================*/



/*==================[internal functions definition]==========================*/

void esp8266_tick() {
	evTick1ms = 1;
}

static void clear_events() {
	evTick1ms = 0;
	evSendData = 0;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *ESP_UART) {
	tx_done = 1;
}

static void clean_rx_buffer() {
	ESP_UART.pRxBuffPtr = rx_buf;
	ESP_UART.RxXferSize = RX_BUF_LEN;
	ESP_UART.RxXferCount = RX_BUF_LEN;
	bzero(rx_buf, RX_BUF_LEN);
}

static bool search_str(char *str) {
	if (memmem(rx_buf, RX_BUF_LEN, str, strlen(str)))
		return 1;
	else
		return 0;
}

void esp8266_init() {
	count_ms = 0;
	state = WAIT_RESET;
	//HAL_GPIO_WritePin(ESP_ENABLE_GPIO_Port, ESP_ENABLE_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	clean_rx_buffer();
	bzero(tx_buf, RX_BUF_LEN);
	bzero(tx_auxbuf, USER_BUF_LEN);
}

void esp8266_loop() {

	HAL_UART_Receive_IT(&ESP_UART, rx_buf, RX_BUF_LEN);

	switch (state){
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ CONEXION DEL MODULO ESP8226 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	case WAIT_RESET:
		if (evTick1ms && count_ms < DELAY_INIT_MS) {
			count_ms++;
			state = WAIT_RESET;
		} else if (evTick1ms && count_ms == DELAY_INIT_MS) {
			clean_rx_buffer();
			//HAL_GPIO_WritePin(ESP_ENABLE_GPIO_Port, ESP_ENABLE_Pin,GPIO_PIN_SET);
			state =SET_MODE;
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
		}
		break;

	case WAIT_READY_CONNECTED:
		if (search_str(WIFI_CON) && search_str(WIFI_GOT) ) {
			clean_rx_buffer();
			HAL_UART_Transmit_IT(&ESP_UART, (uint8_t*) CIPSTART_STR,strlen(CIPSTART_STR));
			state = WAIT_UDP_READY;
			tx_done = false;
		}
		break;

	case WAIT_UDP_READY:
		if (search_str(OK_MSG)) {
			clean_rx_buffer();
			tx_done = false;
			state = IDLE;
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
		}
		break;

		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ CONEXION COMPLETA ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ENVIO DE DATOS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	case IDLE:
		/*if (evSendData) {
			char length_str[8];  // para números hasta 9999999
			uint8_t length = strlen((const char*) tx_auxbuf);

			// Convertir el largo del mensaje en string
			snprintf(length_str, "%d", length);

			// Armar el comando AT+CIPSEND=XX\r\n
			snprintf((char*) tx_buf, TX_BUF_LEN, "%s%s%s", CIPSEND_STR, length_str, LINE_MSG);

			// Enviar el comando al ESP
			if (ESP_UART.gState == HAL_UART_STATE_READY) {
				tx_done = false;
				HAL_UART_Transmit_IT(&ESP_UART, (uint8_t*) tx_buf, strlen((char*) tx_buf));
			}

			// Limpiar el buffer de recepción
			bzero(rx_buf, RX_BUF_LEN);
			clean_rx_buffer();

			// Pasar al siguiente estado
			state = WAIT_SEND_DATA;
		}*/
		if(evTick1ms){
			HAL_UART_Transmit_IT(&ESP_UART, (uint8_t*) LEN_TEST_MSG, strlen((char*) LEN_TEST_MSG));
			state = TEST;
		}


		break;

	case TEST:
		if(evTick1ms)
		{
			HAL_UART_Transmit_IT(&ESP_UART, (uint8_t*) TEST_MSG, strlen((char*) TEST_MSG));
			state = IDLE;

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
			HAL_UART_Transmit_IT(&ESP_UART, (uint8_t*) tx_auxbuf,
					strlen((char*) tx_auxbuf));
			clean_rx_buffer();
			state = IDLE;
		}
		break;
	}
	clear_events();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void esp8266_raise_evSend() {
	evSendData = 1;
}

// Funcion para cargar data para enviar
int32_t esp8266_send_data(const char *data, size_t len) {
	if (state == IDLE && len < USER_BUF_LEN) {
		esp8266_raise_evSend();
		strncpy((char*) tx_auxbuf, data, USER_BUF_LEN - 1);  // copia segura
		tx_auxbuf[USER_BUF_LEN - 1] = '\0';                  // terminador
		return strlen((char*) tx_auxbuf);                    // confirmás lo que copiaste
	} else {
		return 0;  // ESP ocupado o mensaje demasiado largo
	}
}
/*==================[end of file]============================================*/
