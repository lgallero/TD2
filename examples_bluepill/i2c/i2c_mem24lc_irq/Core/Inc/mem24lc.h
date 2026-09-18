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

#ifndef MEM24LC_H
#define MEM24LC_H

/** @file 		mem24lc.h
  * @addtogroup mem24lc 24LCxxx I2C memory driver
  * @{
  */

/*==================[inclusions]=============================================*/

#include <stdint.h>

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/** @brief periodic function */
void mem24lc_loop(void);

/** @brief tick function */
void mem24lc_tick(void);

/** @brief  Write I2C memory in non-blocking mode
  * @param  addr Memory address to write
  * @param  data Pointer to data buffer
  * @param  len  Size of data in bytes
  * @retval	-1 if addr/len are invalid or if the memory is busy,
  * 		0 otherwise
  */
int16_t mem24lc_write(uint16_t addr, void *data, uint16_t len);

/** @brief  Read I2C memory in non-blocking mode
  * @param  addr Memory address to read
  * @param  data Pointer to data buffer
  * @param  len Size of data in bytes
  * @retval	-1 if addr/len are invalid or if the memory is busy
  * 		0 otherwise
  */
int16_t mem24lc_read(uint16_t addr, void *data, uint16_t len);

/** @brief This function is called when the read data is available */
void mem24lc_read_callback(void);

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/
#endif /* #ifndef MEM24LC_H */

