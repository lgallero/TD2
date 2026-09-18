/*
 * fsm.h
 *
 *  Created on: Jul 13, 2024
 *      Author: federico
 */

#ifndef INC_FSM_H_
#define INC_FSM_H_

/*==================[external functions declaration]=========================*/

typedef enum {
	REPOSO,
	VERIF_LINEA,
	VERIF_BAT,
	WAIT_RECONEXION,
	ALIM_LINEA,
	ALIM_BAT,
	WAIT_PULSADOR,
	WAIT_RECONEXION_LINEA
} FSM_STATES;


void fsm_init(void);

void fsm_runCycle(void);

void fsm_tick(void);

void fsm_ev_pulsador(void);

void obtener_valores_linea(void);
void obtener_valores_bat(void);


#endif /* INC_FSM_H_ */
