/*
 * fsm.c
 *
 *  Created on: Jul 13, 2024
 *      Author: federico
 */

/*==================[inclusions]=============================================*/
#include "main.h"
#include "fsm.h"
#include "user_gpio.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/*==================[macros and definitions]=================================*/

/*==================[internal functions declaration]=========================*/
uint16_t get_i_bateria_max(){
	return 1;
}

uint16_t get_v_bateria_min(){
	return 1;
}

/*==================[internal data definition]===============================*/

static FSM_STATES estado;

static bool ev_pulsador;
static bool evTick1ms;

static uint32_t cont_ms;
static uint32_t cont_500ms;
static uint16_t v_bateria_min;
static uint16_t i_bateria_max;
static uint16_t valor_bat_tension;
static uint16_t valor_bat_corriente;
static uint8_t valores_linea;

static void clearEvents(void){
	ev_pulsador = 0;
	evTick1ms = 0;
}


void fsm_init(void){
	estado = REPOSO;
	cont_ms = 0;
    cont_500ms = 0;
    clearEvents();
}

void fsm_runCycle(void){

	switch (estado) {

	case REPOSO:

		if(evTick1ms && cont_ms < 5000){
			cont_ms ++;
		}

		else if(evTick1ms && cont_ms >= 5000){
			estado = VERIF_LINEA;
			cont_ms = 0;
			obtener_valores_linea();
		}

	break;

	case VERIF_LINEA:

		if(evTick1ms && valores_linea == 0x11){
			estado = ALIM_LINEA;
			conectar_a_linea();
			indicador_off();
		}

		else if(evTick1ms && valores_linea != 0x11){
			estado = VERIF_BAT;
			obtener_valores_bat();
			v_bateria_min = get_v_bateria_min();

		}

	break;

	case VERIF_BAT:

		if(evTick1ms && valor_bat_tension > v_bateria_min){
			estado = ALIM_BAT;
			conectar_a_bateria();
			indicador_on();
		}

		else if(evTick1ms && valor_bat_tension <= v_bateria_min){
			estado = WAIT_RECONEXION;
			cont_500ms = 0;
		}

	break;

	case WAIT_RECONEXION:

		if(evTick1ms && cont_ms >= 5000){
			estado = VERIF_LINEA;
			cont_ms = 0;
			obtener_valores_linea();
		}

		else if(evTick1ms && cont_500ms == 500){
			cont_ms ++;
			cont_500ms = 0;
			indicador_toggle();
		}

		else if(evTick1ms && cont_ms < 5000){
			cont_ms ++;
			cont_500ms ++;
		}



	break;

	case ALIM_LINEA:

		if(evTick1ms && valores_linea  != 0x11){
			estado = VERIF_BAT;
			cont_ms = 0;
			desconectar_carga();
		}

		else if(evTick1ms && cont_ms == 20){
			cont_ms = 0;
			obtener_valores_linea();
			obtener_valores_bat();
			v_bateria_min = get_v_bateria_min();
		}

		else if(evTick1ms && cont_ms < 20){
			cont_ms ++;
		}


	break;

	case ALIM_BAT:

		if(evTick1ms && valor_bat_corriente > i_bateria_max){
			estado = WAIT_PULSADOR;
			cont_ms = 0;
			desconectar_carga();
		}

		else if(valores_linea  == 0x11){
			estado = WAIT_RECONEXION_LINEA;
			cont_ms = 0;
			desconectar_carga();
		}

		else if(evTick1ms && cont_ms == 20){
			cont_ms = 0;
			obtener_valores_linea();
			obtener_valores_bat();
			v_bateria_min = get_v_bateria_min();
			i_bateria_max = get_i_bateria_max();
		}

		else if(evTick1ms && cont_ms < 20){
			cont_ms ++;
		}


	break;

	case WAIT_PULSADOR:

		if(ev_pulsador){
			estado = REPOSO;
			cont_ms = 0;
		}

		else if(evTick1ms && cont_ms < 100){
			cont_ms ++;
		}

		else if(evTick1ms && cont_ms == 100){
			cont_ms = 0;
			indicador_toggle();
		}

	break;

	case WAIT_RECONEXION_LINEA:

		if(evTick1ms && cont_ms < 2000){
			cont_ms ++;
		}

		else if(evTick1ms && cont_ms >= 2000){
			estado = VERIF_LINEA;
			cont_ms = 0;
		}

	break;

	}

	clearEvents();
}

void fsm_tick(){
	evTick1ms = 1;
}

void fsm_ev_pulsador(){
	ev_pulsador = 1;
}

void obtener_valores_linea(){

	bool estado_MSB;
	bool estado_LSB;

	estado_MSB = HAL_GPIO_ReadPin(LINEA_MSB_GPIO_Port,LINEA_MSB_Pin); //devuelve GPIO_PIN_RESET (0) o GPIO_PIN_SET (1)

	estado_LSB = HAL_GPIO_ReadPin(LINEA_LSB_GPIO_Port,LINEA_LSB_Pin); //devuelve GPIO_PIN_RESET (0) o GPIO_PIN_SET (1)

	if(estado_MSB == true && estado_LSB == true){
		valores_linea = 0x11;
	}

	else if(estado_MSB == true && estado_LSB == false){
		valores_linea = 0x10;
	}

	else if(estado_MSB == false && estado_LSB == true){
		valores_linea = 0x01;
	}

	else if(estado_MSB == false && estado_LSB == false){
		valores_linea = 0x00;
	}
}

void obtener_valores_bat(){

	//NO SE LLEGO A IMPLEMENTAR

}


