/*
 * user_gpio.c
 *
 *  Created on: Dec 17, 2025
 *      Author: lucas
 */
/*==================[inclusions]=============================================*/

#include "user_gpio.h"
#include "main.h"
#include "fsm.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

extern ADC_HandleTypeDef hadc1;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

void user_gpio_init(void)
{
	HAL_GPIO_WritePin(VENT_BIT_0_GPIO_Port, VENT_BIT_0_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(VENT_BIT_1_GPIO_Port, VENT_BIT_1_Pin, GPIO_PIN_RESET);
}

void set_ctrl(uint8_t level)
{
	switch (level) {
		case 0:  // 00: Off
			HAL_GPIO_WritePin(VENT_BIT_0_GPIO_Port, VENT_BIT_0_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(VENT_BIT_1_GPIO_Port, VENT_BIT_1_Pin, GPIO_PIN_RESET);
			break;
		case 1:  // 01: Low
			HAL_GPIO_WritePin(VENT_BIT_0_GPIO_Port, VENT_BIT_0_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(VENT_BIT_1_GPIO_Port, VENT_BIT_1_Pin, GPIO_PIN_RESET);
			break;
		case 2:  // 10: Med
			HAL_GPIO_WritePin(VENT_BIT_0_GPIO_Port, VENT_BIT_0_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(VENT_BIT_1_GPIO_Port, VENT_BIT_1_Pin, GPIO_PIN_SET);
			break;
		case 3:  // 11: Max
			HAL_GPIO_WritePin(VENT_BIT_0_GPIO_Port, VENT_BIT_0_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(VENT_BIT_1_GPIO_Port, VENT_BIT_1_Pin, GPIO_PIN_SET);
			break;
		default:
			// Error: default to off
			HAL_GPIO_WritePin(VENT_BIT_0_GPIO_Port, VENT_BIT_0_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(VENT_BIT_1_GPIO_Port, VENT_BIT_1_Pin, GPIO_PIN_RESET);
			break;
	}
}

float get_temperature(void)
{
	HAL_ADC_Start(&hadc1);
	if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
		uint32_t adc_val = HAL_ADC_GetValue(&hadc1);
		// Asumir conversión lineal: 0V=-40C, 3.3V=150C (rango 190C)
		return -40.0f + (adc_val * 190.0f / 4095.0f);
	}
	return 0.0f;  // Error case
}

/*==================[end of file]============================================*/

