/*
 * fsm.h
 *
 *  Created on: Dec 18, 2025
 *      Author: lucas
 */

#ifndef INC_FSM_H_
#define INC_FSM_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	VENT_OFF,
	VENT_LOW,
	VENT_MED,
	VENT_MAX,
	VENT_REST
} FSM_VENT_STATES_T;

void fsm_init(void);
void fsm_runCycle(void);
void fsm_tick(void);

void fsm_raise_evalarm_on(void);
void fsm_raise_evalarm_off(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_FSM_H_ */
