/*==================[inclusions]=============================================*/

#include "user_gpio.h"
#include "main.h"
#include "fsm.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

extern UART_HandleTypeDef huart1;
volatile uint8_t rx_byte;
volatile uint8_t rx_buffer[3];
volatile uint8_t rx_index = 0;

/*==================[external data definition]===============================*/


/*==================[internal functions definition]==========================*/


/*==================[external functions definition]==========================*/

void user_gpio_init(void)
{
	HAL_GPIO_WritePin(RS485_GPIO_Port, RS485_Pin, GPIO_PIN_RESET);
	enable_uart();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin)
	{
	case PULSADOR_Pin:
		fsm_raise_disable_pressed();
		break;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1) {
    rx_buffer[rx_index++] = rx_byte;
    if (rx_index == 3) {
      if (rx_buffer[0] == 0xAA && rx_buffer[2] == 0x55) {
        fsm_raise_msg_received(rx_buffer[1]);
      }
      rx_index = 0;
    }
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
  }
}

void send_ack(void)
{
	uint8_t ack[3] = {0xAA, 0xAC, 0x55};
	HAL_GPIO_WritePin(RS485_GPIO_Port, RS485_Pin, GPIO_PIN_SET);  // Habilitar TX
	HAL_UART_Transmit(&huart1, ack, 3, 100);             // Enviar
	HAL_GPIO_WritePin(RS485_GPIO_Port, RS485_Pin, GPIO_PIN_RESET); // Volver a RX
}

void disable_uart(void)
{
  HAL_UART_AbortReceive(&huart1);
  rx_index = 0;
}

void enable_uart(void)
{
  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

/*==================[end of file]============================================*/
