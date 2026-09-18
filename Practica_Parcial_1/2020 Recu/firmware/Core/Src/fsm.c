/*==================[inclusions]=============================================*/

#include "fsm.h"
#include "user_gpio.h"
#include "main.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

static FSM_VENT_STATES_T state;

static bool ev_alarm_active = false;
static bool ev_tick100ms = 0;

static uint32_t cont_ms;
static uint32_t timer_counter;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void clearEvents(void)
{
	ev_tick100ms = 0;
}

/*==================[external functions definition]==========================*/

void fsm_init(void)
{
	state = VENT_OFF;
	clearEvents();
	cont_ms = 0;
	timer_counter = 0;
	set_ctrl(0);
}

void fsm_runCycle(void)
{
	float temp;
	if (ev_tick100ms)
	{
	temp = get_temperature();
	}
	uint8_t alarm = HAL_GPIO_ReadPin(ALARMA_GPIO_Port, ALARMA_Pin); // SET = NORMAL    RESET = SUPERO EL UMBRAL

	switch (state)
	{
	case VENT_OFF:
		if (alarm == RESET || (temp > 50.0f)) {
			set_ctrl(1);  // Low: 01
			timer_counter = 50;  // 5s / 0.1s
			state = VENT_LOW;
		}
		break;

	case VENT_LOW:
		if (alarm == SET && (temp < 40.0f)) {
			set_ctrl(0);  // 00
			timer_counter = 600;  // 60s
			state = VENT_REST;
		} else if (ev_tick100ms) {
			if (timer_counter == 0) {
				set_ctrl(2);  // Med: 10
				timer_counter = 100;  // 10s
				state = VENT_MED;
			}
			else{
				timer_counter --;
			}
		}
		break;

	case VENT_MED:
		if (alarm == SET && (temp < 40.0f)) {
			set_ctrl(0);  // 00
			timer_counter = 600;
			state = VENT_REST;
		} else if (ev_tick100ms) {
			if (timer_counter == 0) {
				set_ctrl(3);  // Max: 11
				state = VENT_MAX;
			}
			else{
			timer_counter --;
			}
		}
		break;

	case VENT_MAX:
		if (alarm == SET && (temp < 40.0f)) {
			set_ctrl(0);  // 00
			timer_counter = 600;
			state = VENT_REST;
		}
		break;

	case VENT_REST:
		if (ev_tick100ms) {
			if (timer_counter == 0) {
				state = VENT_OFF;
			}
			else{
				timer_counter -- ;
			}
		}
		break;

	}

	clearEvents();
}


void fsm_tick(void)
{
	cont_ms++;

	if (cont_ms >= 100)
	{
		cont_ms = 0;
		ev_tick100ms = 1;
	}
}

/*==================[end of file]============================================*/
