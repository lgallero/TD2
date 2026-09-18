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

#include "fsm_idleblink.h"
#include "user_gpio.h"
#include <stdint.h>

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

static FSM_IDLEBLINK_T idleblink_a;
static FSM_IDLEBLINK_T idleblink_b;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void clearEvents(FSM_IDLEBLINK_T *handle)
{
	handle->evBtnPressed = 0;
	handle->evTickms = 0;
}

static void initFSM(FSM_IDLEBLINK_T *handle)
{
	fsm_idleblink_setLedStatus(handle->id, 0);
	handle->state = IDLE;
	clearEvents(handle);
}

static void runCycle(FSM_IDLEBLINK_T *handle)
{
	switch (handle->state) {
	case IDLE:
		if (handle->evBtnPressed) {
			handle->cont_ms = 0;
			handle->state = LED_OFF;
		}
		break;

	case LED_OFF:
		if (handle->evBtnPressed) {
			handle->state = IDLE;
		}
		else if (handle->evTickms && (handle->cont_ms < handle->delay_ms)) {
			handle->cont_ms++;
		}
		else if (handle->evTickms && (handle->cont_ms == handle->delay_ms)) {
			handle->cont_ms = 0;
			fsm_idleblink_setLedStatus(handle->id, 1);
			handle->state = LED_ON;
		}
		break;

	case LED_ON:
		if (handle->evBtnPressed) {
			fsm_idleblink_setLedStatus(handle->id, 0);
			handle->state = IDLE;
		}
		else if (handle->evTickms && (handle->cont_ms < handle->delay_ms)) {
			handle->cont_ms++;
		}
		else if (handle->evTickms && (handle->cont_ms == handle->delay_ms)) {
			handle->cont_ms = 0;
			fsm_idleblink_setLedStatus(handle->id, 0);
			handle->state = LED_OFF;
		}
		break;
	}

	clearEvents(handle);
}

/*==================[external functions definition]==========================*/

void fsm_idleblink_init(void)
{
	idleblink_a.id = 0;
	idleblink_a.delay_ms = 500;
	initFSM(&idleblink_a);

	idleblink_b.id = 1;
	idleblink_b.delay_ms = 1000;
	initFSM(&idleblink_b);
}

void fsm_idleblink_loop(void)
{
	runCycle(&idleblink_a);
	runCycle(&idleblink_b);
}

void fsm_idleblink_tick(void)
{
	idleblink_a.evTickms = 1;
	idleblink_b.evTickms = 1;
}

void fsm_idleblink_raise_evBtnPressed(uint32_t id)
{
	switch (id) {
	case 0:
		idleblink_a.evBtnPressed = 1;
		break;
	case 1:
		idleblink_b.evBtnPressed = 1;
		break;
	}
}

/*==================[end of file]============================================*/
