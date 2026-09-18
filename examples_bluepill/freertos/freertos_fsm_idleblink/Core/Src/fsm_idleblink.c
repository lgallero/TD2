/* Copyright 2022, TD2-FRH
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

#define QUEUE_ITEM_COUNT  2
#define QUEUE_ITEM_SIZE   sizeof(FSM_IDLEBLINK_EVENTS_T)

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

static FSM_IDLEBLINK_T idleblink_a;
static FSM_IDLEBLINK_T idleblink_b;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void initFSM(FSM_IDLEBLINK_T *fsm)
{
	fsm_idleblink_setLedStatus(fsm->id, 0);
	fsm->state = IDLE;
	fsm->event_queue = xQueueCreate(QUEUE_ITEM_COUNT, QUEUE_ITEM_SIZE);
}

static void runCycle(FSM_IDLEBLINK_T *fsm, FSM_IDLEBLINK_EVENTS_T event)
{
	switch (fsm->state) {
	case IDLE:
		if (event == EV_BTN_PRESSED) {
			fsm->cont_ms = 0;
			fsm->state = LED_OFF;
		}
		break;

	case LED_OFF:
		if (event == EV_BTN_PRESSED) {
			fsm->state = IDLE;
		}
		else if (event == EV_TICK_MS && (fsm->cont_ms < fsm->delay_ms)) {
			fsm->cont_ms++;
		}
		else if (event == EV_TICK_MS && (fsm->cont_ms == fsm->delay_ms)) {
			fsm->cont_ms = 0;
			fsm_idleblink_setLedStatus(fsm->id, 1);
			fsm->state = LED_ON;
		}
		break;

	case LED_ON:
		if (event == EV_BTN_PRESSED) {
			fsm_idleblink_setLedStatus(fsm->id, 0);
			fsm->state = IDLE;
		}
		else if (event == EV_TICK_MS && (fsm->cont_ms < fsm->delay_ms)) {
			fsm->cont_ms++;
		}
		else if (event == EV_TICK_MS && (fsm->cont_ms == fsm->delay_ms)) {
			fsm->cont_ms = 0;
			fsm_idleblink_setLedStatus(fsm->id, 0);
			fsm->state = LED_OFF;
		}
		break;
	}
}

/*==================[external functions definition]==========================*/

void fsm_idleblink_task(void *arg)
{
	FSM_IDLEBLINK_T *fsm = (FSM_IDLEBLINK_T *)arg;
	FSM_IDLEBLINK_EVENTS_T event;

	while (1) {
		if (xQueueReceive(fsm->event_queue, &event, 100 / portTICK_PERIOD_MS) != pdTRUE) {
			event = EV_NONE;
		}
		runCycle(fsm, event);
	}
}

void fsm_idleblink_init(void)
{
	idleblink_a.id = 0;
	idleblink_a.delay_ms = 500;
	initFSM(&idleblink_a);
	xTaskCreate(fsm_idleblink_task, (const char *)"fsm_idleblink_a",
			configMINIMAL_STACK_SIZE, &idleblink_a, osPriorityNormal, 0);

	idleblink_b.id = 1;
	idleblink_b.delay_ms = 1000;
	initFSM(&idleblink_b);
	xTaskCreate(fsm_idleblink_task, (const char *)"fsm_idleblink_b",
			configMINIMAL_STACK_SIZE, &idleblink_b, osPriorityNormal, 0);
}

void fsm_idleblink_tick(void)
{
	FSM_IDLEBLINK_EVENTS_T event = EV_TICK_MS;

	xQueueSendFromISR(idleblink_a.event_queue, &event, NULL);
	xQueueSendFromISR(idleblink_b.event_queue, &event, NULL);

	/* portEND_SWITCHING_ISR() is not necessary here because this
	 * function is called from vApplicationTickHook() */
}

void fsm_idleblink_raise_evBtnPressed(uint32_t id)
{
	BaseType_t switch_required = pdFALSE;
	FSM_IDLEBLINK_EVENTS_T event = EV_BTN_PRESSED;

	switch (id) {
	case 0:
		xQueueSendFromISR(idleblink_a.event_queue, &event, &switch_required);
		break;
	case 1:
		xQueueSendFromISR(idleblink_b.event_queue, &event, &switch_required);
		break;
	}

	portEND_SWITCHING_ISR(switch_required);
}

/*==================[end of file]============================================*/
