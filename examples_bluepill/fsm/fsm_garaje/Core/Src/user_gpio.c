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

#include "user_gpio.h"
#include "main.h"
#include "fsm_garaje.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

void user_gpio_init(void)
{
	fsm_garaje_cerrarBarrera();
	fsm_garaje_apagarAlarma();
}

void user_gpio_loop(void)
{
	if (HAL_GPIO_ReadPin(SENSOR1_GPIO_Port, SENSOR1_Pin) == GPIO_PIN_RESET) {
		fsm_garaje_raise_evSensor1_On();
	}

	if (HAL_GPIO_ReadPin(SENSOR2_GPIO_Port, SENSOR2_Pin) == GPIO_PIN_RESET) {
		fsm_garaje_raise_evSensor2_On();
	}
	else {
		fsm_garaje_raise_evSensor2_Off();
	}
}

void fsm_garaje_abrirBarrera(void)
{
	HAL_GPIO_WritePin(BARRERA_GPIO_Port, BARRERA_Pin, GPIO_PIN_RESET);
}

void fsm_garaje_cerrarBarrera(void)
{
	HAL_GPIO_WritePin(BARRERA_GPIO_Port, BARRERA_Pin, GPIO_PIN_SET);
}

void fsm_garaje_activarAlarma(void)
{
	HAL_GPIO_WritePin(ALARMA_GPIO_Port, ALARMA_Pin, GPIO_PIN_RESET);
}

void fsm_garaje_apagarAlarma(void)
{
	HAL_GPIO_WritePin(ALARMA_GPIO_Port, ALARMA_Pin, GPIO_PIN_SET);
}

/*==================[end of file]============================================*/
