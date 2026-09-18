/*
 * user_gpio.h
 *
 *  Created on: Dec 17, 2025
 *      Author: lucas
 */

#ifndef INC_USER_GPIO_H_
#define INC_USER_GPIO_H_

#include <stdint.h>

/* Motor */
void motor_stop(void);
void motor_subir_on(void);
void motor_bajar_on(void);

/* Balizas */
void balizas_off(void);
void baliza_subir_on(void);
void baliza_bajar_on(void);
void balizas_on(void); /* ambas ON */

void user_gpio_init(void);

#endif /* INC_USER_GPIO_H_ */
