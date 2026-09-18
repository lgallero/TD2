/* USER CODE BEGIN Header */
/**
 **************************
 * @file          esp8266.h
 **************************
 */
/* USER CODE END Header */

#ifndef CORE_INC_ESP8266_H_
#define CORE_INC_ESP8266_H_

/*==================[inclusions]=============================================*/
#include "main.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/
#define ESP_RX_PAYLOAD_MAX 256U

/*==================[typedef]================================================*/
typedef struct {
	size_t len;
	uint8_t data[ESP_RX_PAYLOAD_MAX];
} esp_rx_msg_t;

typedef enum {
	WAIT_RESET,
	SET_MODE,
	WAIT_SET_MODE,
	SET_RED,
	WAIT_RED,
    GET_IP,     // <--- NUEVO
    WAIT_IP,    // <--- NUEVO
	SET_UDP,
	WAIT_UDP,
	IDLE_ESP,
	RECEIVING,
	SEND_LEN,
	WAIT_LEN,
	SEND_DATA,
	WAIT_DATA
} FSM_ESP_STATES_T;

extern QueueHandle_t cola_rx;

/*==================[external data declaration]==============================*/
#define ESP_UART huart1

/*==================[external functions declaration]=========================*/
extern void esp8266_init(void);
extern void esp8266_send_data(const char *data, size_t len);
extern void esp8266_flush_rx(void);

#ifdef __cplusplus
}
#endif
#endif /* CORE_INC_ESP8266_H_ */
