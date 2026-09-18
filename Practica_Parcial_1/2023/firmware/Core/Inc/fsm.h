/*
 * fsm.h
 *
 *  Created on: Dec 18, 2025
 *      Author: lucas
 */

#ifndef INC_FSM_H_
#define INC_FSM_H_

#include <stdint.h>
#include <stdbool.h>

void fsm_init(void);
void fsm_runCycle(void);
void fsm_tick(void);

/* IRQ desde EXTI (EOC) */
void fsm_raise_eoc1(void);
void fsm_raise_eoc2(void);
void fsm_raise_eoc3(void);


#endif /* INC_FSM_H_ */
