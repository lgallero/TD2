/*
 * fsm.h
 *
 *  Created on: Dec 17, 2025
 *      Author: lucas
 */

#ifndef INC_FSM_H_
#define INC_FSM_H_

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include <stdbool.h>

/*==================[cplusplus]==============================================*/
#ifdef __cplusplus
extern "C" {
#endif

/*==================[typedef]================================================*/

typedef enum {
    MEAS_IDLE = 0,
    MEAS_NOLOAD,
    MEAS_LOW,
    MEAS_MED,
    MEAS_HIGH,
    MEAS_ERROR_BLINK,
    MEAS_ERROR_WAIT_CLEAR
} FSM_MEAS_STATES_T;

/*==================[external functions declaration]=========================*/

void fsm_init(void);
void fsm_runCycle(void);
void fsm_tick(void);

/* Evento por EXTI del pulsador */
void fsm_raise_start_pressed(void);

/*==================[cplusplus]==============================================*/
#ifdef __cplusplus
}
#endif

#endif /* INC_FSM_H_ */
