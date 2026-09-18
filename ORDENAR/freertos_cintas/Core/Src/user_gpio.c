/*
 * user_gpio.c
 *
 *  Created on: Sep 24, 2025
 *      Author: lucas
 */


#include "user_gpio.h"

void prenderMotor(GPIO_TypeDef*MOTOR_GPIO_Port, uint16_t MOTOR_Pin)
{
	HAL_GPIO_WritePin(MOTOR_GPIO_Port, MOTOR_Pin, GPIO_PIN_RESET);

}

void apagarMotor(GPIO_TypeDef*MOTOR_GPIO_Port, uint16_t MOTOR_Pin)
{
	HAL_GPIO_WritePin(MOTOR_GPIO_Port, MOTOR_Pin, GPIO_PIN_SET);
}

void ponerColectorEnPosicion(uint8_t cinta_id)
{
	switch (cinta_id){

	case 1:
		HAL_GPIO_WritePin(COLECTOR_GPIO_Port, COLECTOR_Pin, GPIO_PIN_RESET);
		break;

	case 2:
		HAL_GPIO_WritePin(COLECTOR_GPIO_Port, COLECTOR_Pin, GPIO_PIN_SET);
		break;

	default:
		break;
	}
}

uint8_t posicionColector(void)
{
	if(HAL_GPIO_ReadPin(COLECTOR_GPIO_Port, COLECTOR_Pin) == GPIO_PIN_RESET){
		return 1;
	}
	else if(HAL_GPIO_ReadPin(COLECTOR_GPIO_Port, COLECTOR_Pin) == GPIO_PIN_SET){
		return 2;
	}
	else {
		return -1;
	}
}

void user_gpio_init (void)
{
	HAL_GPIO_WritePin(COLECTOR_GPIO_Port, COLECTOR_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR1_GPIO_Port, MOTOR1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR2_GPIO_Port, MOTOR2_Pin, GPIO_PIN_SET);
}
