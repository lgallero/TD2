/*
 * user_gpio.c
 *
 *  Created on: Dec 17, 2025
 *      Author: lucas
 */

#include "user_gpio.h"
#include "main.h"
#include "fsm.h"

/* ===================== Puerta A (servo M1) ===================== */
void doorA_open(void)
{
    HAL_GPIO_WritePin(M1_GPIO_Port, M1_Pin, GPIO_PIN_SET);
}

void doorA_close(void)
{
    HAL_GPIO_WritePin(M1_GPIO_Port, M1_Pin, GPIO_PIN_RESET);

}

/* ===================== Puerta B (M2 2 bits) ===================== */
/* Tabla:
   00 = cerrado
   01 = Ci
   11 = Cp
*/
void doorB_close(void){ // 00 = cerrado
	HAL_GPIO_WritePin(M2_BIT0_GPIO_Port, M2_BIT0_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(M2_BIT1_GPIO_Port, M2_BIT1_Pin, GPIO_PIN_RESET);
}

void doorB_open_ci(void){ // 01 = Ci
	HAL_GPIO_WritePin(M2_BIT0_GPIO_Port, M2_BIT0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(M2_BIT1_GPIO_Port, M2_BIT1_Pin, GPIO_PIN_RESET);
}

void doorB_open_cp(void){ // 11 = Cp
	HAL_GPIO_WritePin(M2_BIT0_GPIO_Port, M2_BIT0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(M2_BIT1_GPIO_Port, M2_BIT1_Pin, GPIO_PIN_SET);
}




void user_gpio_init(void)
{
    doorA_close();
    doorB_close();
}

/* ===================== EXTI ===================== */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
        case S1_Pin:
            fsm_raise_s1_irq();
            break;

        case S2_Pin:
            fsm_raise_s2_irq();
            break;

        case PULSADOR_Pin:
            fsm_raise_reset_irq();
            break;

        default:
            break;
    }
}
