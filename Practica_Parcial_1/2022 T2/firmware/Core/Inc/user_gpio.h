/*
 * user_gpio.h
 *
 *  Created on: Dec 17, 2025
 *      Author: lucas
 */

#ifndef INC_USER_GPIO_H_
#define INC_USER_GPIO_H_

#include <stdbool.h>
#include <stdint.h>

/* Puerta A (servo M1) */
void doorA_open(void);
void doorA_close(void);

/* Puerta B (M2 por 2 bits) */
void doorB_close(void); // 00
void doorB_open_ci(void); // 01
void doorB_open_cp(void); // 11

void user_gpio_init(void);

#endif /* INC_USER_GPIO_H_ */
