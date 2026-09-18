/*
 * user_gpio.h
 *
 *  Created on: Dec 17, 2025
 *      Author: lucas
 */

#ifndef INC_USER_GPIO_H_
#define INC_USER_GPIO_H_

/*==================[inclusions]=============================================*/
#include <stdint.h>

/*==================[external functions declaration]=========================*/
void user_gpio_init(void);

/* Acciones (salidas) */
void indicator_on(void);
void indicator_off(void);
void indicator_toggle(void);

void load_set_none(void);
void load_set_low(void);
void load_set_med(void);
void load_set_high(void);

#endif /* INC_USER_GPIO_H_ */
