/*
 * fsm.h
 *
 *  Created on: Dec 17, 2025
 *      Author: lucas
 */

#ifndef INC_FSM_H_
#define INC_FSM_H_

/*==================[inclusions]=============================================*/

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/


typedef enum {
	VENT_OFF,
	VENT_LOW,
	VENT_MED,
	VENT_MAX,
	VENT_REST
} FSM_VENT_STATES_T;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void fsm_init(void);

void fsm_runCycle(void);

void fsm_tick(void);


/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/


#endif /* INC_FSM_H_ */
