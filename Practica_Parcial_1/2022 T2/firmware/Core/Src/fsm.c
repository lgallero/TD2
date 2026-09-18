/*
 * fsm.c
 *
 *  Created on: Dec 17, 2025
 *      Author: lucas
 */
#include "fsm.h"
#include "user_gpio.h"
#include <stdint.h>
#include <stdbool.h>

/* Enunciado: ya implementada, devuelve ID (4 dígitos) o 0 */
extern uint16_t rfid_get_id(void){
	return 0;
};

/* Tick 100ms generado desde tick 1ms */
#define TICK100_MS            (100U)
#define WAIT_5S_TICKS         (50U)   /* 5s / 0.1s */
#define TIMEOUT_45S_TICKS     (450U)  /* 45s / 0.1s */

typedef enum {
	ST_IDLE = 0,
	ST_A_OPEN_WAIT_S1,
	ST_WAIT_5S,
	ST_B_OPEN_CI,
	ST_B_OPEN_CP,
	ST_EMERGENCY
} fsm_state_t;

static fsm_state_t state;

/* Eventos (flags, estilo simple) */
static volatile bool ev_s1 = false;
static volatile bool ev_s2 = false;
static volatile bool ev_reset = false;
static volatile bool ev_tick100ms = false;

/* timebase */
static uint32_t cont_ms = 0;

/* control */
static uint16_t animal_id = 0;
static uint16_t cont_100ms = 0;

/* ---------------- Helpers ---------------- */
static bool id_impar (uint16_t id)
{
	return ((id & 1U) != 0U);
}

static void clear_events(void)
{
	ev_s1 = false;
	ev_s2 = false;
	ev_reset = false;
	ev_tick100ms = false;
}


/* ---------------- API ---------------- */
void fsm_init(void)
{
	state = ST_IDLE;
	clear_events();
}

void fsm_runCycle(void)
{
	/* Reset: prioridad absoluta */
	if (ev_reset)
	{
		state = ST_IDLE;
		clear_events();
		return;
	}

	switch (state)
	{
	case ST_IDLE:
	{
		uint16_t id = rfid_get_id();
		if (id != 0) // Se detecta un Animal
				{
			animal_id = id;
			doorA_open();
			state = ST_A_OPEN_WAIT_S1;
				}
	} break;

	case ST_A_OPEN_WAIT_S1:
	{
		if (ev_s1) // Pasa por el sensor en el pasillo
		{
			doorA_close();
			cont_100ms = 0;
			state = ST_WAIT_5S;
		}
	} break;

	case ST_WAIT_5S:
	{
		if (ev_tick100ms)
		{
			/* espera 5s */
			if (cont_100ms < WAIT_5S_TICKS){
				cont_100ms++;
			}

			if (cont_100ms == WAIT_5S_TICKS)
			{
				/* abrir puerta B según paridad del ID */
				if (id_impar(animal_id))
				{
					doorB_open_ci();
					state = ST_B_OPEN_CI;
				}
				else
				{
					doorB_open_cp();
					state = ST_B_OPEN_CP;
				}
			}
		}
	} break;

	case ST_B_OPEN_CI:
	case ST_B_OPEN_CP:
	{
		/* timeout 45s sigue corriendo mientras el animal no llega a S2 */
		if (ev_tick100ms)
		{
			if (cont_100ms < TIMEOUT_45S_TICKS){
				cont_100ms++;
			}
			if (cont_100ms == TIMEOUT_45S_TICKS) // Pasaron los 45 segundos
					{
				doorB_close();
				doorA_open();
				state = ST_EMERGENCY;
				break;
					}
		}

		/* Llegó al corral: S2=0 => cierro B y listo para el siguiente */
		if (ev_s2)
		{
			doorB_close();
			state = ST_IDLE;
		}
	} break;

	case ST_EMERGENCY:
	{

		if (ev_reset) // pulsador
		{
			doorA_close();
			state = ST_IDLE;
		}
	} break;
	}

	clear_events();
}

/* tick de 1ms (llamar desde SysTick o HAL_IncTick) */
void fsm_tick(void)
{
	cont_ms++;

	if (cont_ms >= TICK100_MS)
	{
		cont_ms = 0;
		ev_tick100ms = true;
	}
}

/* ------------ IRQ events (desde EXTI) ------------ */
void fsm_raise_s1_irq(void){
	ev_s1 = true;
}
void fsm_raise_s2_irq(void){
	ev_s2 = true;
}
void fsm_raise_reset_irq(void){
	ev_reset = true;
}
