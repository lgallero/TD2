/*
 * fsm.c
 *
 *  Created on: Dec 18, 2025
 *      Author: lucas
 */

#include "fsm.h"
#include "user_gpio.h"
#include <stdint.h>
#include <stdbool.h>

/* provista por el enunciado */
extern void get_slave_data(uint8_t *addr, uint8_t *cmd){
}

#define SOF        (0xAAu)
#define EOF_BYTE   (0x55u)
#define MID_ACK    (0xACu)
#define MID_FIN    (0xFFu)

#define PERIOD_5S_MS     (5000u)
#define DISABLE_60S_MS   (60000u)
#define ACK_TIMEOUT_MS   (500u)

typedef enum
{
	IDLE,
	WAIT_ACK1,
	WAIT_ACK2,
	SUPEND,
} fsm_state_t;

static fsm_state_t st;

/* eventos */
static volatile bool ev_tick = false;
static volatile bool ev_btn = false;
static volatile bool ev_ack = false;

/* timers */
static uint32_t cont_ms = 0;

/* datos */
static uint8_t addr = 0;
static uint8_t cmd  = 0;

/* RX parser 3 bytes */
static uint8_t rx0 = 0, rx1 = 0, rx2 = 0;
static uint8_t rx_idx = 0;

static void reset_rx_parser(void)
{
	rx_idx = 0;
	rx0 = rx1 = rx2 = 0;
}

static void enter_disabled(void)
{
	st = SUPEND;
	cont_ms = 0;

	/* suspende TX/RX: no enviamos nada y “ignoramos” rx */
	reset_rx_parser();
	ev_ack = false;
}

static bool send_addr_frame(uint8_t a)
{
	return uart_send_3bytes(SOF, a, EOF_BYTE);
}

static bool send_cmd_frame(uint8_t c)
{
	return uart_send_3bytes(SOF, c, EOF_BYTE);
}

static bool send_fin_frame(void)
{
	return uart_send_3bytes(SOF, MID_FIN, EOF_BYTE);
}

void fsm_init(void)
{
	st = IDLE;
	cont_ms = 0;

	ev_tick= false;
	ev_btn = false;
	ev_ack = false;

	reset_rx_parser();
}

void fsm_runCycle(void)
{
	/* botón: prioridad absoluta */
	if (ev_btn)
	{
		ev_btn = false;
		enter_disabled();
		return;
	}

	switch (st)
	{
	case IDLE:
		if (ev_tick){
			cont_ms ++;

			if (cont_ms >= PERIOD_5S_MS) // 5 segundos
			{
				cont_ms = 0;
				get_slave_data(&addr, &cmd); // tomo la direccion y el comando
				send_addr_frame(addr);
				st = WAIT_ACK1;
			}
		}
			break;

	case WAIT_ACK1:
		if (ev_ack)
		{
			ev_ack = false;
			send_cmd_frame(cmd);
			st = WAIT_ACK2;
		}
		break;

	case WAIT_ACK2:
		if (ev_ack)
		{
			ev_ack = false;
			send_fin_frame();
			cont_ms = 0;
			st = IDLE;
		}
		break;

	case SUPEND:
		if(ev_tick){
			cont_ms++;
			if (cont_ms >= DISABLE_60S_MS)
			{
				/* reanudo */
				st = IDLE;
				cont_ms = 0;
				reset_rx_parser();
			}
		}
		break;
		}
	ev_tick = false;
}
/* ===== tick 1ms ===== */
void fsm_tick(void)
{
	ev_tick = true;
}

/* ===== botón EXTI ===== */
void fsm_ev_disable_button(void)
{
	ev_btn = true;
}

/* parser AA xx 55 */
void fsm_uart_rx_byte(uint8_t b)
{
	if (st == SUPEND){
		return;
	}

	if (rx_idx == 0) // 0xAA
	{
		if (b != SOF) {
			return;
		}
		rx0 = b;
		rx_idx = 1;
		return;
	}
	else if (rx_idx == 1) // ACK
	{
		rx1 = b;
		rx_idx = 2;
		return;
	}
	else if (rx_idx == 2)// 0x55
	{
		rx2 = b;
		rx_idx = 0;

		if (rx0 == SOF && rx2 == EOF_BYTE)
		{
			if (rx1 == MID_ACK)  /* AA-AC-55 */
			{
				ev_ack = true;
			}
		}
	}
}



