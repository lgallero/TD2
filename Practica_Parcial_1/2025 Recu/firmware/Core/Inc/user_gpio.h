/*
 * user_gpio.h
 *
 *  Created on: Dec 18, 2025
 *      Author: lucas
 */

#ifndef INC_USER_GPIO_H_
#define INC_USER_GPIO_H_

#include <stdint.h>
#include <stdbool.h>

/* init de GPIO y UART RX IT */
void user_gpio_init(void);

/* RS485 direction */
void rs485_set_tx(uint8_t tx); /* 0=RX, 1=TX */

/* UART TX no bloqueante (IT) */
bool uart_send_3bytes(uint8_t b0, uint8_t b1, uint8_t b2);

/* Rearmar RX 1 byte (IT) */
void uart_arm_rx_1byte_it(void);


#endif /* INC_USER_GPIO_H_ */
