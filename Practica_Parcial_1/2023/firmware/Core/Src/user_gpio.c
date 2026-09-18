/*
 * user_gpio.c
 *
 *  Created on: Dec 18, 2025
 *      Author: lucas
 */
#include "user_gpio.h"
#include "main.h"
#include "fsm.h"
#include <string.h>

extern UART_HandleTypeDef huart1;

/* ========== GPIO control ========== */
void convst_set(uint8_t level)  // activado = 0  desactivado = 1
{
	switch(level)
	{
	case 0:
		HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_RESET);
		break;
	case 1:
		HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, GPIO_PIN_SET);
		break;
	}
}

void rd_set(uint8_t level)
{
	switch(level)
	{
	case 0:
		HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_RESET);
		break;
	case 1:
		HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET);
		break;
	}
}

void cs_all_disable(void) // activado = 0  desactivado = 1
{
    HAL_GPIO_WritePin(CS1_GPIO_Port, CS1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CS2_GPIO_Port, CS2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CS3_GPIO_Port, CS3_Pin, GPIO_PIN_SET);
}

void cs1_set(uint8_t level)
{
    switch(level)
    	{
    	case 0:
    		HAL_GPIO_WritePin(CS1_GPIO_Port, CS1_Pin,GPIO_PIN_RESET);
    		break;
    	case 1:
    		HAL_GPIO_WritePin(CS1_GPIO_Port, CS1_Pin,GPIO_PIN_SET);
    		break;
    	}
}

void cs2_set(uint8_t level)
{
    switch(level)
    	{
    	case 0:
    		HAL_GPIO_WritePin(CS2_GPIO_Port, CS2_Pin,GPIO_PIN_RESET);
    		break;
    	case 1:
    		HAL_GPIO_WritePin(CS2_GPIO_Port, CS2_Pin,GPIO_PIN_SET);
    		break;
    	}
}

void cs3_set(uint8_t level)
{
    switch(level)
    	{
    	case 0:
    		HAL_GPIO_WritePin(CS3_GPIO_Port, CS3_Pin,GPIO_PIN_RESET);
    		break;
    	case 1:
    		HAL_GPIO_WritePin(CS3_GPIO_Port, CS3_Pin,GPIO_PIN_SET);
    		break;
    	}
}

void user_gpio_init(void)
{
    convst_set(1);
    rd_set(1);
    cs_all_disable();
}

uint16_t read_d0_d11(void){
	return 0;
}

/* ========== EXTI (EOC) ========== */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
        case EOC1_Pin:
        	fsm_raise_eoc1();
        break;

        case EOC2_Pin:
        	fsm_raise_eoc2();
        break;

        case EOC3_Pin:
        	fsm_raise_eoc3();
        break;
    }
}

/* ========== UART TX por interrupción (cola simple) ========== */
static volatile uint8_t tx_busy = 0;
static char tx_buf[64];

void uart_send_line(const char *s)
{
    if (tx_busy) return; /* ideal: si está ocupado, lo ignoramos */

    strncpy(tx_buf, s, sizeof(tx_buf)-1);
    tx_buf[sizeof(tx_buf)-1] = '\0';

    tx_busy = 1;
    HAL_UART_Transmit_IT(&huart1, (uint8_t*)tx_buf, strlen(tx_buf));
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART1) return;
    tx_busy = 0;
}
