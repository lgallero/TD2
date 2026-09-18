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

void fsm_init(void);
void fsm_runCycle(void);
void fsm_tick(void);

/* Eventos por EXTI */
void fsm_raise_s1_irq(void);
void fsm_raise_s2_irq(void);
void fsm_raise_reset_irq(void);

#endif /* INC_FSM_H_ */
