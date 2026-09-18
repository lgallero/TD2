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

/* eventos */
void fsm_ev_disable_button(void);

/* uart events */
void fsm_uart_rx_byte(uint8_t b);

#endif /* INC_FSM_H_ */
