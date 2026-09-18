/*
 * user_gpio.c
 *
 *  Created on: Dec 18, 2025
 *      Author: lucas
 */

#include "user_gpio.h"
#include "main.h"
#include "fsm.h"

void user_gpio_init(void) // Ventilacion apagada -> 00
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
	}
}

float get_temperature(void)
{
	HAL_ADC_Start(&hadc1);
		if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
			uint32_t adc_val = HAL_ADC_GetValue(&hadc1);
			// Conversión: 0V = -40°C, 3.3V = 150°C (rango 190°C), escala lineal
			return -40.0f + (adc_val * 190.0f / 4095.0f);
		}
		return 0.0f;  // Error case
}

uint8_t read_alarm(void){
	return HAL_GPIO_ReadPin(ALARMA_GPIO_Port, ALARMA_Pin); // SET = NORMAL    RESET = SUPERO EL UMBRAL
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin)
	{
	case ALARMA_Pin:
		{
			if(HAL_GPIO_ReadPin(ALARMA_GPIO_Port, ALARMA_Pin) == GPIO_PIN_RESET){ // Activo en 0 (1 normal, 0 alarma)
				fsm_raise_evalarm_on();
			}
			if(HAL_GPIO_ReadPin(ALARMA_GPIO_Port, ALARMA_Pin) == GPIO_PIN_SET){ // (1 normal, 0 alarma)
				fsm_raise_evalarm_off();
			}
		}
		break;
	}
}
