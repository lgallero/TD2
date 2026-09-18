/*
 * fsm.h
 *
 *  Created on: Dec 17, 2025
 *      Author: lucas
 */

#ifndef INC_FSM_H_
#define INC_FSM_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void fsm_init(void);
void fsm_runCycle(void);
void fsm_tick(void);

/* Eventos por EXTI (sensores) */
void fsm_raise_in_subir(void);
void fsm_raise_out_subir(void);
void fsm_raise_in_bajar(void);
void fsm_raise_out_bajar(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_FSM_H_ */
