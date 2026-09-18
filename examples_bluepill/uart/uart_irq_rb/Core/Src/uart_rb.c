/* Copyright 2021, TD2-FRH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*==================[inclusions]=============================================*/

#include "uart_rb.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

static UART_RB_T uart_rbs[N_UARTS];
static RINGBUFF_T tx_rbs[N_UARTS], rx_rbs[N_UARTS];
static uint8_t uart_rx_buf[UART_RX_BUF_LEN];

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

int32_t uart_rb_init(UART_HandleTypeDef *huart, uint8_t *tx_rb_buf, uint8_t *rx_rb_buf, uint32_t size)
{
	for (uint8_t i = 0; i < N_UARTS; i++) {
		if (uart_rbs[i].huart == NULL) {
			uart_rbs[i].huart = huart;
			uart_rbs[i].tx_rb = &tx_rbs[i];
			uart_rbs[i].rx_rb = &rx_rbs[i];

			/* Initialize the ring buffers with an item size of 1 byte */
			RingBuffer_Init(uart_rbs[i].tx_rb, tx_rb_buf, 1, size);
			RingBuffer_Init(uart_rbs[i].rx_rb, rx_rb_buf, 1, size);

			/* Start a non-blocking UART Rx transfer */
			HAL_UART_Receive_IT(huart, uart_rx_buf, UART_RX_BUF_LEN);

			return i;
		}
	}
	return -1;
}

int32_t uart_rb_write(UART_HandleTypeDef *huart, uint8_t *data, uint32_t len)
{
	for (uint8_t i = 0; i < N_UARTS; i++) {
		if (huart == uart_rbs[i].huart) {
			if (HAL_UART_Transmit_IT(huart, data, len) == HAL_BUSY) {
				len = RingBuffer_InsertMult(uart_rbs[i].tx_rb, data, len);
			}
			return len;
		}
	}
	return -1;
}

int32_t uart_rb_read(UART_HandleTypeDef *huart, uint8_t *data, uint32_t len)
{
	for (uint8_t i = 0; i < N_UARTS; i++) {
		if (huart == uart_rbs[i].huart) {
			return RingBuffer_PopMult(uart_rbs[i].rx_rb, data, len);
		}
	}
	return -1;
}

int32_t uart_rb_rxGetCount(UART_HandleTypeDef *huart)
{
	for (uint8_t i = 0; i < N_UARTS; i++) {
		if (huart == uart_rbs[i].huart) {
			return RingBuffer_GetCount(uart_rbs[i].rx_rb);
		}
	}
	return -1;
}

/**
 * @brief  UART Rx transfer completed callback
 * @param  huart  Pointer to the UART handle
 * @retval Nothing
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	static uint8_t tx_data[UART_TX_BUF_LEN];
	static uint8_t tx_data_len;

	for (uint8_t i = 0; i < N_UARTS; i++) {
		if (huart == uart_rbs[i].huart) {
			/* If this UART's Tx ring buffer has data available, pop
			 * UART_TX_BUF_LEN bytes at most and start a new transmission */
			tx_data_len = RingBuffer_PopMult(uart_rbs[i].tx_rb, tx_data, UART_TX_BUF_LEN);
			HAL_UART_Transmit_IT(uart_rbs[i].huart, tx_data, tx_data_len);
			return;
		}
 	}
}

/**
 * @brief  UART Rx transfer completed callback
 * @param  huart  Pointer to the UART handle
 * @retval Nothing
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	for (uint8_t i = 0; i < N_UARTS; i++) {
		if (huart == uart_rbs[i].huart) {
			/* Insert UART_RX_BUF_LEN received bytes into this UART's Rx ring
			 * buffer and start a new Rx transfer */
			RingBuffer_InsertMult(uart_rbs[i].rx_rb, uart_rx_buf, UART_RX_BUF_LEN);
			HAL_UART_Receive_IT(uart_rbs[i].huart, uart_rx_buf, UART_RX_BUF_LEN);
			return;
		}
	}
}

/*==================[end of file]============================================*/
