/*==================[inclusions]=============================================*/

#include "fsm.h"
#include "user_gpio.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

static FSM_CAFETERA_STATES_T state;

static bool [Nombre de Evento] = 0;

static uint32_t cont_ms;
static uint32_t cont_100ms;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void clearEvents(void)
{
	[Nombre de Evento] = 0;
	evTick100ms = 0;
}

/*==================[external functions definition]==========================*/

void fsm_init(void)
{
	state = [Nombre primer Estado];
	clearEvents();
}

void cafetera_runCycle(void)
{
	switch (state)
	{
	case [Nombre de Estado]:

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
		evTick100ms = 1;
	}
}

void fsm_raise_[Nombre de Evento](void)
{
	[Nombre de Evento] = 1;
}

/*==================[end of file]============================================*/
