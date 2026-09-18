/*
 * user_gpio.h
 *
 *  Created on: Dec 18, 2025
 *      Author: lucas
 */

#ifndef INC_USER_GPIO_H_
#define INC_USER_GPIO_H_

#include <stdint.h>
#include "main.h"

extern ADC_HandleTypeDef hadc1;

void user_gpio_init(void);
void set_ctrl(uint8_t level);
float get_temperature(void);
uint8_t read_alarm(void);


#endif /* INC_USER_GPIO_H_ */
