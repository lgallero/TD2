#ifndef INC_FSM_H_
#define INC_FSM_H_

/*==================[inclusions]=============================================*/


#include "main.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

typedef enum {
	SLAVE_IDLE,
	SLAVE_WAIT_COMMAND,
	SLAVE_WAIT_END,
	SLAVE_DISABLED
} FSM_SLAVE_STATES_T;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void fsm_init(void);

void fsm_runCycle(void);

void fsm_tick(void);

void fsm_raise_msg_received(uint8_t payload);
void fsm_raise_disable_pressed(void);


/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/


#endif /* INC_FSM_H_ */
