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

/* evento desde IRQ puerta */
void fsm_ev_door_open_irq(void);

#endif /* INC_FSM_H_ */
