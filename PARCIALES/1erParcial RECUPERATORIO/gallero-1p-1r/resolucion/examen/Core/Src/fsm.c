/*
 * fsm.c
 *
 *  Created on: Dec 18, 2025
 *      Author: lucas
 */
#include "fsm.h"
#include "user_gpio.h"
#include "main.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

static FSM_VENT_STATES_T state;

static bool ev_alarm_active = false;
static bool ev_tick100ms = false;

static uint32_t cont_ms;
static uint32_t timer_counter;
static float temp;

static void clearEvents(void)
{
	ev_tick100ms = false;
}

void fsm_init(void)
{
	state = VENT_OFF;
	clearEvents();
	cont_ms = 0;
	timer_counter = 0;
	temp = 0;
	set_ctrl(0); // 00
}

void fsm_runCycle(void)
{
	switch (state){

	case VENT_OFF:
		if(ev_tick100ms)
		{
			temp = get_temperature();
		}
		if (ev_alarm_active || (temp > 50.0f)) {
			set_ctrl(1);  // Low: 01
			timer_counter = 50;  // 5s
			state = VENT_LOW;
		}
		break;

	case VENT_LOW:

		if (ev_tick100ms){
			if(timer_counter > 0){
				timer_counter --;
			}
			if(timer_counter == 0) { // Pasaron los 5 segundos
				set_ctrl(2);  // Med: 10
				timer_counter = 100;  // 10s
				state = VENT_MED;
			}
		temp = get_temperature();
		}

		if (!ev_alarm_active && (temp < 40.0f)) { // Bajo la temperatura y no hay alarma
			set_ctrl(0);  // 00
			timer_counter = 600;  // 60s
			state = VENT_REST;
		}
		break;

	case VENT_MED:
		if (ev_tick100ms) {
			if(timer_counter > 0){
				timer_counter --;
			}
			if (timer_counter == 0) { // Pasaron los 10 segundos
				set_ctrl(3);  // Max: 11
				state = VENT_MAX;
			}
			temp = get_temperature();
		}

		if ( !ev_alarm_active && (temp < 40.0f)) { // Bajo la temperatura y no hay alarma
			set_ctrl(0);  // 00
			timer_counter = 600;  // 60s
			state = VENT_REST;
		}
		break;

	case VENT_MAX:

		if (ev_tick100ms){
			temp = get_temperature();
		}

		if ( !ev_alarm_active && (temp < 40.0f)) { // Bajo la temperatura y no hay alarma
			set_ctrl(0);  // 00
			timer_counter = 600;  // 60s
			state = VENT_REST;
		}
		break;

	case VENT_REST:
		if (ev_tick100ms) {
			if(timer_counter > 0){
				timer_counter --;
			}
			if (timer_counter == 0) { // Pasaron los 60segundos = 1 minuto
				state = VENT_OFF;
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
		ev_tick100ms = true;
	}
}

void fsm_raise_evalarm_on()
{
	ev_alarm_active = true;
}

void fsm_raise_evalarm_off()
{
	ev_alarm_active = false;
}
