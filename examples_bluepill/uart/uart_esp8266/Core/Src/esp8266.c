/*
 * esp8266.c
 *
 *  Created on: May 13, 2023
 *      Author: juan
 */
#define _GNU_SOURCE
#include "esp8266.h"
#include "stdbool.h"
#include "stdlib.h"
#include "main.h"
#include "string.h"

#define ATOI_BUFFER_SIZE 5

static bool evTick1ms;
static uint32_t count_ms;
static esp8266_state_t state;
static uint8_t rxbuf[RXBUF_LEN];
static uint8_t local_output_buf[RXBUF_LEN];
static uint32_t recv_total_size;

void esp8266_tick() {
	evTick1ms = 1;
}

static void clear_events() {
	evTick1ms = 0;
}

static bool search_str(char *str) {
	if (memmem(rxbuf, RXBUF_LEN, str, strlen(str)))
		return 1;
	else
		return 0;
}

static uint32_t get_rcv_total_size() {
	char atoiBuffer[ATOI_BUFFER_SIZE];
	uint8_t *p_size;
	uint32_t i = 0;

	p_size = memmem(rxbuf, RXBUF_LEN, "+IPD,", 5) + 5;
	while ((char) p_size[i] != ':') {
		atoiBuffer[i] = p_size[i];
		i++;
	}
	atoiBuffer[i] = '\0';
	return (uint32_t) atoi(atoiBuffer);
}

static uint32_t get_rcv_size() {
	uint8_t *p_msg = memmem(rxbuf, RXBUF_LEN, ":", 1);
	return strlen((char*) p_msg);
}

static void clean_rx_buffer() {
	huart1.pRxBuffPtr = rxbuf;
	huart1.RxXferSize = RXBUF_LEN;
	huart1.RxXferCount = RXBUF_LEN;
	bzero(rxbuf, RXBUF_LEN);
}

static void copy_to_output_buffer() {
	uint8_t *p_msg = memmem(rxbuf, RXBUF_LEN, ":", 1) + 1;
	memcpy(local_output_buf, p_msg, recv_total_size);
}

void esp8266_init() {
	count_ms = 0;
	state = WAIT_RESET;
	HAL_GPIO_WritePin(ESP_ENABLE_GPIO_Port, ESP_ENABLE_Pin, GPIO_PIN_RESET);
}

void esp8266_loop() {
	HAL_UART_Receive_IT(&huart1, rxbuf, RXBUF_LEN);
	switch (state) {
	case WAIT_RESET:
		if (evTick1ms && count_ms < TIME_INIT) {

			count_ms++;
			state = WAIT_RESET;
		} else if (evTick1ms && count_ms == TIME_INIT) {
			clean_rx_buffer();
			HAL_GPIO_WritePin(ESP_ENABLE_GPIO_Port, ESP_ENABLE_Pin,
					GPIO_PIN_SET);
			state = WAIT_READY_CONNECTED;
		}
		break;
	case WAIT_READY_CONNECTED:
		if (search_str("ready\r\n")
				&& search_str("WIFI CONNECTED\r\nWIFI GOT IP\r\n")) {
			// clean buffer
			clean_rx_buffer();
			HAL_UART_Transmit_IT(&huart1, (uint8_t*) CIPSTART_CMD,
					strlen(CIPSTART_CMD));
			state = WAIT_UDP_READY;
		}
		break;
	case WAIT_UDP_READY:
		if (search_str("CONNECT\r\n")) {
			clean_rx_buffer();
			state = IDLE;
		}
		break;
	case IDLE:
		if (search_str("+IPD") && search_str(":")) {
			recv_total_size = get_rcv_total_size();
			state = RECEIVING;
		}
		break;
	case RECEIVING:
		if (get_rcv_size() >= recv_total_size) {
			copy_to_output_buffer();
			clean_rx_buffer();
			state = IDLE;
		}
		break;
	}
	clear_events();
}

uint32_t esp8266_read(uint8_t *buffer, uint32_t size) {
	uint32_t local_buffer_size = strlen((char *)local_output_buf);
	if (local_buffer_size > 0 && state == IDLE) {
		if (size < local_buffer_size) {
			memcpy(buffer, local_output_buf, size);
		} else {
			memcpy(buffer, local_output_buf, local_buffer_size);
		}

		return local_buffer_size;
	} else {
		return 0;
	}
}

