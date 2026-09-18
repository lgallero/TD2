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

/* init */
void user_gpio_init(void);

/* cerradura */
void lock_access(void);      /* bloquear ingreso */
void unlock_access(void);    /* permitir ingreso */

/* buzzer */
void buzzer_on(void);
void buzzer_off(void);

/* temperatura (puede ser ADC o ya convertido) */
float temp_get_celsius(void);

#endif /* INC_USER_GPIO_H_ */
