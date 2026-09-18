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

#ifndef UART_RB_H
#define UART_RB_H

/** @addtogroup uart_rb UART API with ring buffers
  * @{
  */

/*==================[inclusions]=============================================*/

#include "ring_buffer.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

#define N_UARTS          3   /*!< Number of UARTS available in the device */
#define UART_TX_BUF_LEN  16  /*!< UART Transmission buffer size in bytes */
#define UART_RX_BUF_LEN  1   /*!< UART Reception buffer size in bytes */

/*==================[typedef]================================================*/

/** @brief UART Ring buffer structure */
typedef struct {
	UART_HandleTypeDef *huart; /*!< Pointer to the UART handle */
	RINGBUFF_T *tx_rb;         /*!< Pointer to the Tx ring buffer structure */
	RINGBUFF_T *rx_rb;         /*!< Pointer to the Rx ring buffer structure */
} UART_RB_T;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief   Initialize Tx and Rx ring buffers for a UART
 * @param   huart  : Pointer to the UART handle
 * @param   tx_buf : Pointer to Tx ring buffer
 * @param   rx_buf : Pointer to Rx ring buffer
 * @param   len    : Size of ring buffers in bytes (it is the same for both)
 * @return  ID of the initialized UART (0, 1, ..., N_UARTS-1) or
 *          -1 if there was an error
 */
int32_t uart_rb_init(UART_HandleTypeDef *huart, uint8_t *tx_buf, uint8_t *rx_buf, uint32_t len);

/**
 * @brief   Transmit data via UART using the Tx ring buffer
 * @param   huart   : Pointer to the UART handle
 * @param   data    : Pointer to the data buffer
 * @param   len     : Number of bytes to send
 * @return  Number of bytes that will be sent or
 *          -1 if there was an error
 */
int32_t uart_rb_write(UART_HandleTypeDef *huart, uint8_t *data, uint32_t len);

/**
 * @brief   Receive data via UART using the Rx ring buffer
 * @param   huart   : Pointer to the UART handle
 * @param   data    : Pointer to the data buffer
 * @param   len     : Number of bytes to receive
 * @return  Number of bytes that were read from the Rx ring buffer or
 *          -1 if there was an error
 */
int32_t uart_rb_read(UART_HandleTypeDef *huart, uint8_t *data, uint32_t len);

/**
 * @brief   Return the number of items available in the Rx ring buffer
 * @param   huart   : Pointer to the UART handle
 * @return  Number of items available in the Rx ring buffer or
 *          -1 if there was an error
 */
int32_t uart_rb_rxGetCount(UART_HandleTypeDef *huart);

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/
#endif /* #ifndef UART_RB_H */
