/* Copyright 2023, TD2-FRH
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

#include "main.h"
#include <mem24lc.h>
#include <stdbool.h>
#include <string.h>

/*==================[macros and definitions]=================================*/

#define SLAVE_ADDR        0xA0
#define MAX_WR_DATA_LEN   32          /* 24lc64: 32 byte page write */
#define MAX_ADDR          (8292 - 1)  /* 24lc64: 8k addresses */
#define T_WRITE_CYCLE_MS  5           /* 24lc64: 5 ms max page write time */
#define TIMEOUT_MS        100

/*==================[internal data declaration]==============================*/

typedef enum {
	IDLE,
	WRITING,
	READING_TX_ADDR,
	READING_RX_DATA
} MEM_STATE_T;

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

static uint8_t wr_data[MAX_WR_DATA_LEN];
static uint8_t wr_data_len;
static uint8_t *rd_data;
static uint8_t rd_data_len;
static HAL_StatusTypeDef i2c_status;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

int16_t mem24lc_write(uint16_t addr, void *data, uint16_t len)
{
	if ((addr > MAX_ADDR) || (len > MAX_WR_DATA_LEN)) {
		return -1;
	}

	wr_data[0] = addr >> 8;
	wr_data[1] = addr;
	memcpy(wr_data + 2, (uint8_t *)data, len);
	wr_data_len = len + 2;

	i2c_status = HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDR, wr_data, wr_data_len, TIMEOUT_MS);

	return 0;
}

int16_t mem24lc_read(uint16_t addr, void *data, uint16_t len)
{
	if ((addr > MAX_ADDR) || (len > MAX_ADDR)) {
		return -1;
	}

	wr_data[0] = addr >> 8;
	wr_data[1] = addr;
	wr_data_len = 2;
	rd_data = data;
	rd_data_len = len;

	i2c_status = HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDR, wr_data, wr_data_len, TIMEOUT_MS);
	i2c_status = HAL_I2C_Master_Receive(&hi2c1, SLAVE_ADDR, rd_data, rd_data_len, TIMEOUT_MS);

	return 0;
}

/*==================[end of file]============================================*/

