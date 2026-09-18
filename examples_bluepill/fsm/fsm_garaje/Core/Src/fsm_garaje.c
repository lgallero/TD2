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

#include "fsm_garaje.h"
#include "user_gpio.h"
#include <stdint.h>
#include <stdbool.h>

/*==================[macros and definitions]=================================*/

#define ESPERA_EGRESO_SEG  5

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

static FSM_GARAJE_STATES_T state;

static bool evSensor1_On;
static bool evSensor2_On;
static bool evSensor2_Off;
static bool evTick1seg;

static uint32_t count_ms;
static uint8_t count_seg;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void clearEvents(void)
{
	evSensor1_On = 0;
	evSensor2_On = 0;
	evSensor2_Off = 0;
	evTick1seg = 0;
}

/*==================[external functions definition]==========================*/

void fsm_garaje_init(void)
{
	state = REPOSO;
	clearEvents();
}

void fsm_garaje_runCycle(void)
{
	switch (state) {
	case REPOSO:
		if (evSensor2_On) {
			fsm_garaje_activarAlarma();
			state = ALARMA;
		}
		else if (evSensor1_On) {
			fsm_garaje_abrirBarrera();
			state = INGRESANDO;
		}
		break;

	case INGRESANDO:
		if (evSensor2_On) {
			count_seg = 0;
			state = ESPERANDO_EGRESO;
		}
		break;

	case ESPERANDO_EGRESO:
		if (evSensor1_On) {
			state = INGRESANDO;
		}
		else if (evTick1seg && (count_seg < ESPERA_EGRESO_SEG)) {
			count_seg++;
		}
		else if (evTick1seg && (count_seg == ESPERA_EGRESO_SEG)) {
			fsm_garaje_cerrarBarrera();
			state = REPOSO;
		}
		break;

	case ALARMA:
		if (evSensor2_Off) {
			fsm_garaje_apagarAlarma();
			state = REPOSO;
		}
		break;
	}

	clearEvents();
}

void fsm_garaje_tick(void)
{
	count_ms++;

	if (count_ms >= 1000) {
		count_ms = 0;
		evTick1seg = 1;
	}
}

void fsm_garaje_raise_evSensor1_On(void)
{
	evSensor1_On = 1;
}

void fsm_garaje_raise_evSensor2_On(void)
{
	evSensor2_On = 1;
}

void fsm_garaje_raise_evSensor2_Off(void)
{
	evSensor2_Off = 1;
}

/*==================[end of file]============================================*/
