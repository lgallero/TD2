/*==================[inclusions]=============================================*/

#include "cafetera.h"
#include "user_gpio.h"
#include "esp8266.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/*==================[macros and definitions]=================================*/

#define CAFE_NEGRO 0
#define CAFE_LECHE 1

#define ESPERAR_MSG "Esperando ficha"
#define SELECCIONAR_MSG "Seleccionar cafe"
#define CAFE_LECHE_MSG "Sirviendo: CAFE CON LECHE"
#define CAFE_NEGRO_MSG "Sirviendo: CAFE NEGRO"
#define RETIRAR_MSG "Retirar cafe"

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

static FSM_CAFETERA_STATES_T state;

static bool evFicha_On = 0;
static bool evCafeNegro_On = 0;
static bool evCafeLeche_On = 0;
static bool evTick100ms = 0;

static uint32_t cont_ms;
static uint32_t cont_100ms;
static uint8_t Cafe;



/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void clearEvents(void)
{
	evFicha_On = 0;
	evCafeNegro_On = 0;
	evCafeLeche_On = 0;
	evTick100ms = 0;
}

/*==================[external functions definition]==========================*/

void cafetera_init(void)
{
	state = REPOSO;
	clearEvents();
	esp8266_send_data(ESPERAR_MSG, strlen(ESPERAR_MSG));
}

void cafetera_runCycle(void)
{
	switch (state)
	{
	case REPOSO:

		if (evFicha_On)
		{
			state = ELEGIR_CAFE;
			esp8266_send_data(SELECCIONAR_MSG, strlen(SELECCIONAR_MSG));
			cont_100ms = 0;
			PrenderLedTestigo();
		}
		else if (evTick100ms && (cont_100ms < 5))
		{
			cont_100ms++;
		}
		else if (evTick100ms && (cont_100ms == 5))
		{
			ToggleLedTestigo();
			cont_100ms = 0;
		}
		break;

	case ELEGIR_CAFE:

		if (evCafeNegro_On)
		{

			state = SIRVIENDO_CAFE;
			esp8266_send_data(CAFE_NEGRO_MSG, strlen(CAFE_NEGRO_MSG));
			CafeErogacionOn();
			cont_100ms = 0;
			Cafe = CAFE_NEGRO;
		}
		else if (evCafeLeche_On)
		{
			state = SIRVIENDO_CAFE;
			esp8266_send_data(CAFE_LECHE_MSG, strlen(CAFE_LECHE_MSG));
			CafeErogacionOn();
			cont_100ms = 0;
			Cafe = CAFE_LECHE;
		}
		else if (evTick100ms && (cont_100ms == 50))
		{
			cont_100ms = 0;
			ApagarLedTestigo();
			state = REPOSO;
		}
		else if (evTick100ms && (cont_100ms < 50))
		{
			cont_100ms++;
		}
		break;

	case SIRVIENDO_CAFE:

		if (evTick100ms && (cont_100ms == 40) && (Cafe == CAFE_LECHE))
		{
			CafeErogacionOff();
			LecheErogacionOn();
			ToggleLedTestigo();
			cont_100ms++;
		}
		else if (evTick100ms && (cont_100ms < 80))
		{
			ToggleLedTestigo();
			cont_100ms++;
		}
		else if (evTick100ms && (cont_100ms == 80) && (Cafe == CAFE_LECHE))
		{
			LecheErogacionOff();
			cont_100ms = 0;
			state = RETIRAR_CAFE;
			esp8266_send_data(RETIRAR_MSG, strlen(RETIRAR_MSG));
			BuzzerOn();
			ApagarLedTestigo();
		}
		else if (evTick100ms && (cont_100ms == 80) && (Cafe == CAFE_NEGRO))
		{
			CafeErogacionOff();
			cont_100ms = 0;
			state = RETIRAR_CAFE;
			esp8266_send_data(RETIRAR_MSG, strlen(RETIRAR_MSG));
			BuzzerOn();
			ApagarLedTestigo();
		}
		break;

	case RETIRAR_CAFE:
		if (evTick100ms && (cont_100ms == 20))
		{
			cont_100ms = 0;
			BuzzerOff();
			PrenderLedTestigo();
			state = REPOSO;
			esp8266_send_data(ESPERAR_MSG, strlen(ESPERAR_MSG));
		}
		else if (evTick100ms && (cont_100ms < 20))
		{
			cont_100ms++;
		}
		break;
	}

	clearEvents();
}


void cafetera_tick(void)
{
	cont_ms++;

	if (cont_ms >= 100)
	{
		cont_ms = 0;
		evTick100ms = 1;
	}
}

void cafetera_raise_evFicha_On(void)
{
	evFicha_On = 1;
}

void cafetera_raise_evCafeNegro_On(void)
{
	evCafeNegro_On = 1;
}

void cafetera_raise_evCafeLeche_On(void)
{
	evCafeLeche_On = 1;
}
/*==================[end of file]============================================*/
