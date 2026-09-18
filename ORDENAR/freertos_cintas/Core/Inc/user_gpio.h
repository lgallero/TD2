/*
 * user_gpio.h
 *
 *  Created on: Sep 24, 2025
 *      Author: lucas
 */

#ifndef INC_USER_GPIO_H_
#define INC_USER_GPIO_H_

#include "main.h"

void prenderMotor(GPIO_TypeDef*MOTOR_GPIO_Port, uint16_t MOTOR_Pin);

void apagarMotor(GPIO_TypeDef*MOTOR_GPIO_Port, uint16_t MOTOR_Pin);

void ponerColectorEnPosicion(uint8_t cinta_id);

uint8_t posicionColector(void);

void user_gpio_init (void);

#endif /* INC_USER_GPIO_H_ */
