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

/*==================[external functions definition]==========================*/

void user_gpio_init(void)
{
    /* Estado inicial */
    indicator_off();
    load_set_none();
}

/*--------- Indicador ---------*/
void indicator_on(void)
{
    HAL_GPIO_WritePin(LED_BP_GPIO_Port, LED_BP_Pin, GPIO_PIN_SET);
}

void indicator_off(void)
{
    HAL_GPIO_WritePin(LED_BP_GPIO_Port, LED_BP_Pin, GPIO_PIN_RESET);
}

void indicator_toggle(void)
{
    HAL_GPIO_TogglePin(LED_BP_GPIO_Port, LED_BP_Pin);
}

/*--------- Cargas (2 bits: 00 none, 01 low, 10 med, 11 high) ---------*/
void load_set_none(void) {
	HAL_GPIO_WritePin(LOAD_BIT0_GPIO_Port, LOAD_BIT0_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LOAD_BIT1_GPIO_Port, LOAD_BIT1_Pin, GPIO_PIN_RESET);
}
void load_set_low(void)  {
	HAL_GPIO_WritePin(LOAD_BIT0_GPIO_Port, LOAD_BIT0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LOAD_BIT1_GPIO_Port, LOAD_BIT1_Pin, GPIO_PIN_RESET);
}
void load_set_med(void)  {
	HAL_GPIO_WritePin(LOAD_BIT0_GPIO_Port, LOAD_BIT0_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LOAD_BIT1_GPIO_Port, LOAD_BIT1_Pin, GPIO_PIN_SET);
}
void load_set_high(void) {
	HAL_GPIO_WritePin(LOAD_BIT0_GPIO_Port, LOAD_BIT0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LOAD_BIT1_GPIO_Port, LOAD_BIT1_Pin, GPIO_PIN_SET);
}

/*--------- EXTI del pulsador ---------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == PULSADOR_Pin)
    {
        fsm_raise_start_pressed();
    }
}
