/*
 * esp8266.c
 *
 *  Created on: May 13, 2023
 *      Author: juan
 */

#ifndef INC_ESP8266_C_
#define INC_ESP8266_C_

#include "stdint.h"

#define CIPSTART_CMD "AT+CIPSTART=\"UDP\",\"192.168.46.34\",8000,8000\r\n"
#define TIME_INIT 200
#define RXBUF_LEN 256
#define TXBUF_LEN 128

typedef enum {
	WAIT_RESET,
	WAIT_READY_CONNECTED,
	WAIT_UDP_READY,
	IDLE,
	RECEIVING
} esp8266_state_t;

void esp8266_loop();
void esp8266_tick();
void esp8266_init();
uint32_t esp8266_read(uint8_t* buffer, uint32_t bufferSize);


#endif /* INC_ESP8266_C_ */
