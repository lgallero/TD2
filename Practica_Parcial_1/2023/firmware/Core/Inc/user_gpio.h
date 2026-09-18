/*
 * user_gpio.h
 *
 *  Created on: Dec 18, 2025
 *      Author: lucas
 */

#ifndef INC_USER_GPIO_H_
#define INC_USER_GPIO_H_


#include <stdint.h>

/* Líneas compartidas */
void convst_set(uint8_t level);  /* 0/1 */
void rd_set(uint8_t level);      /* 0/1 */

/* CS individuales (0 habilita) */
void cs1_set(uint8_t level);     /* 0/1 */
void cs2_set(uint8_t level);
void cs3_set(uint8_t level);

void cs_all_disable(void);

/* Lectura bus 12 bits (la da el enunciado) */
uint16_t read_d0_d11(void);

/* UART por IT (simple) */
void uart_send_line(const char *s);

/* init */
void user_gpio_init(void);


#endif /* INC_USER_GPIO_H_ */
