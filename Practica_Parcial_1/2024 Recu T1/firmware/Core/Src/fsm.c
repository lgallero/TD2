/*==================[inclusions]=============================================*/

#include "fsm.h"
#include "user_gpio.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

void run_slave_cmd (uint8_t )
{
	return;
}

uint8_t get_slave_address(void)
{
	return 0;
}

/*==================[internal data definition]===============================*/

static FSM_SLAVE_STATES_T state;

static bool msg_received = 0;
static bool disable_pressed = 0;
static bool ev_tick100ms = 0;

static uint32_t cont_ms;
static uint32_t disable_counter;

static uint8_t my_address;
static uint8_t received_payload;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void clearEvents(void)
{
	msg_received = 0;
	disable_pressed = 0;
	ev_tick100ms = 0;
}

/*==================[external functions definition]==========================*/

void fsm_init(void)
{
	state = SLAVE_IDLE;
	my_address = get_slave_address();
	clearEvents();
	cont_ms = 0;
	disable_counter = 0;
}

void fsm_runCycle(void)
{
	if (disable_pressed) {
		disable_counter = 600;  // 60 seconds / 0.1s
		state = SLAVE_DISABLED;
	}

	switch (state)
	{
	case SLAVE_IDLE:
		if (msg_received) {
			if (received_payload == my_address) {
				send_ack();
				state = SLAVE_WAIT_COMMAND;
			}
			// else ignore
		}
		break;

	case SLAVE_WAIT_COMMAND:
		if (msg_received) {
			run_slave_cmd(received_payload);
			send_ack();
			state = SLAVE_WAIT_END;
		}
		break;

	case SLAVE_WAIT_END:
		if (msg_received) {
			if (received_payload == 0xFF) {
				state = SLAVE_IDLE;
			} else {
				// abort on unexpected
				state = SLAVE_IDLE;
			}
		}
		break;

	case SLAVE_DISABLED:
		if (ev_tick100ms) {
			if (disable_counter > 0) {
				disable_counter--;
			}
			if (disable_counter == 0) {
				state = SLAVE_IDLE;
			}
		}
		break;

	}

	clearEvents();
}


void fsm_tick(void)
{
	cont_ms++;

	if (cont_ms >= 100)
	{
		cont_ms = 0;
		ev_tick100ms = 1;
	}
}

void fsm_raise_msg_received(uint8_t payload)
{
	if (state == SLAVE_DISABLED) { // Mensaje durante
		return;
	}
	received_payload = payload;
	msg_received = 1;
}

void fsm_raise_disable_pressed(void)
{
	disable_pressed = 1;
}

/*==================[end of file]============================================*/
