/*
 * user_gpio.c
 *
 *  Created on: Dec 17, 2025
 *      Author: lucas
 */
#include "user_gpio.h"
#include "main.h"
#include "fsm.h"

/* =================== Salidas =================== */

void motor_stop(void)
{
    HAL_GPIO_WritePin(M_SUBIR_GPIO_Port, M_SUBIR_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M_BAJAR_GPIO_Port, M_BAJAR_Pin, GPIO_PIN_RESET);
}

void motor_subir_on(void)
{
    /* Por seguridad: apago el otro */
    HAL_GPIO_WritePin(M_BAJAR_GPIO_Port, M_BAJAR_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M_SUBIR_GPIO_Port, M_SUBIR_Pin, GPIO_PIN_SET);
}

void motor_bajar_on(void)
{
    HAL_GPIO_WritePin(M_SUBIR_GPIO_Port, M_SUBIR_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(M_BAJAR_GPIO_Port, M_BAJAR_Pin, GPIO_PIN_SET);
}

void balizas_off(void)
{
    HAL_GPIO_WritePin(B_SUBIR_GPIO_Port, B_SUBIR_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(B_BAJAR_GPIO_Port, B_BAJAR_Pin, GPIO_PIN_RESET);
}

void baliza_subir_on(void)
{
    HAL_GPIO_WritePin(B_SUBIR_GPIO_Port, B_SUBIR_Pin, GPIO_PIN_SET);
}

void baliza_bajar_on(void)
{
    HAL_GPIO_WritePin(B_BAJAR_GPIO_Port, B_BAJAR_Pin, GPIO_PIN_SET);
}

void balizas_on(void)
{
    HAL_GPIO_WritePin(B_SUBIR_GPIO_Port, B_SUBIR_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(B_BAJAR_GPIO_Port, B_BAJAR_Pin, GPIO_PIN_SET);
}

void user_gpio_init(void)
{
    motor_stop();
    balizas_off();
}

/* =================== EXTI Sensores =================== */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
        case S_IN_SUBIR_Pin:
            fsm_raise_in_subir();
            break;

        case S_OUT_SUBIR_Pin:
            fsm_raise_out_subir();
            break;

        case S_IN_BAJAR_Pin:
            fsm_raise_in_bajar();
            break;

        case S_OUT_BAJAR_Pin:
            fsm_raise_out_bajar();
            break;
    }
}

