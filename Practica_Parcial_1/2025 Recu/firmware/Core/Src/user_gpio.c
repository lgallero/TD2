/*
 * user_gpio.c
 *
 *  Created on: Dec 18, 2025
 *      Author: lucas
 */
#include "user_gpio.h"
#include "main.h"
#include "fsm.h"

/* UART handle */
extern UART_HandleTypeDef huart1;

/* buffer RX 1 byte */
static uint8_t rx_byte;

/* TX busy */
static volatile uint8_t tx_busy = 0;


void rs485_set_tx(uint8_t tx)
{
	switch (tx) {
		case 0:
			HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
			break;

		case 1:
			HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);
			break;
	}
}

void uart_arm_rx_1byte_it(void)
{
	HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

bool uart_send_3bytes(uint8_t b0, uint8_t b1, uint8_t b2)
{
    static uint8_t txbuf[3];

    if (tx_busy) {
    	return false;
    }

    txbuf[0] = b0;
    txbuf[1] = b1;
    txbuf[2] = b2;

    tx_busy = 1;

    rs485_set_tx(1); // Habilito Tx

    if (HAL_UART_Transmit_IT(&huart1, txbuf, 3) != HAL_OK)
    {
        rs485_set_tx(0); // Habilito Rx
        return false;
    }
    return true;
}

void user_gpio_init(void)
{
    rs485_set_tx(0);
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

/* ===== Callbacks HAL ===== */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        fsm_uart_rx_byte(rx_byte);
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        rs485_set_tx(0);
        tx_busy = 0;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
        case PULSADOR_Pin:
            fsm_ev_disable_button();
            break;
    }
}

