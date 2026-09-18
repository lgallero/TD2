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
#include <stdio.h>

#define PULSE_1MS_TICKS   (1U)
#define WAIT_100MS_TICKS  (100U)

typedef enum
{
    INICIAR_CONV_LOW = 0,
    INICIAR_CONV_HIGH,
    LEER_EOC,
    LEER_S1,
    LEER_S2,
    LEER_S3,
    ESPERAR_100MS
} fsm_state_t;

static fsm_state_t state;

/* eventos */
static volatile bool ev_tick1ms = false;
static volatile bool ev_eoc1 = false;
static volatile bool ev_eoc2 = false;
static volatile bool ev_eoc3 = false;

/* contador ms */
static uint16_t count_ms = 0;

/* mediciones */
static uint16_t s1_val = 0;
static uint16_t s2_val = 0;
static uint16_t s3_val = 0;

static void clear_tick(void){
	ev_tick1ms = false;
}

static void start_conversion_pulse(void)
{
    /* armamos un nuevo ciclo */
    ev_eoc1 = false;
    ev_eoc2 = false;
    ev_eoc3 = false;

    cs_all_disable();
    rd_set(1);

    /* CONVST pulso: LOW 1ms */
    convst_set(0);
    count_ms = 0;
    state = INICIAR_CONV_LOW;
}

void fsm_init(void)
{
    state = INICIAR_CONV_LOW;
    start_conversion_pulse();
}

void fsm_runCycle(void)
{
    switch (state)
    {
        case INICIAR_CONV_LOW:
            if (ev_tick1ms)
            {
                if (count_ms < PULSE_1MS_TICKS) {
                	count_ms++;
                }

                if (count_ms >= PULSE_1MS_TICKS)
                {
                    /* vuelve a HIGH: arranca conversión */
                    convst_set(1);
                    state = INICIAR_CONV_HIGH;
                }
            }
            break;

        case INICIAR_CONV_HIGH:
            /* apenas sube CONVST, pasamos a esperar EOC */
            state = LEER_EOC;
            break;

        case LEER_EOC:
            if (ev_eoc1 && ev_eoc2 && ev_eoc3) // Espero los 3 valores esten disponibles
            {
                cs_all_disable();
                cs1_set(0);
                rd_set(0);
                count_ms = 0;
                state = LEER_S1;
            }
            break;

        case LEER_S1:
            if (ev_tick1ms)
            {
                if (count_ms < PULSE_1MS_TICKS) count_ms++;

                if (count_ms >= PULSE_1MS_TICKS)
                {
                    rd_set(1);
                    s1_val = read_d0_d11();
                    cs1_set(1);

                    /* mensaje UART */
                    char msg[32];
                    (void)snprintf(msg, sizeof(msg), "D1: %u\r\n", (unsigned)s1_val);
                    uart_send_line(msg);

                    /* preparar ADC2 */
                    cs2_set(0);
                    rd_set(0);
                    count_ms = 0;
                    state = LEER_S2;
                }
            }
            break;

        case LEER_S2:
            if (ev_tick1ms)
            {
                if (count_ms < PULSE_1MS_TICKS) count_ms++;

                if (count_ms >= PULSE_1MS_TICKS)
                {
                    rd_set(1);
                    s2_val = read_d0_d11();
                    cs2_set(1);

                    char msg[32];
                    (void)snprintf(msg, sizeof(msg), "D2: %u\r\n", (unsigned)s2_val);
                    uart_send_line(msg);

                    /* preparar ADC3 */
                    cs3_set(0);
                    rd_set(0);
                    count_ms = 0;
                    state = LEER_S3;
                }
            }
            break;

        case LEER_S3:
            if (ev_tick1ms)
            {
                if (count_ms < PULSE_1MS_TICKS) count_ms++;

                if (count_ms >= PULSE_1MS_TICKS)
                {
                    rd_set(1);
                    s3_val = read_d0_d11();
                    cs3_set(1);

                    char msg[32];
                    (void)snprintf(msg, sizeof(msg), "D3: %u\r\n", (unsigned)s3_val);
                    uart_send_line(msg);

                    count_ms = 0;
                    state = ESPERAR_100MS;
                }
            }
            break;

        case ESPERAR_100MS:
            if (ev_tick1ms)
            {
                if (count_ms < WAIT_100MS_TICKS){
                	count_ms++;
                }

                if (count_ms >= WAIT_100MS_TICKS)
                {
                    start_conversion_pulse();
                }
            }
            break;
    }

    clear_tick();
}

/* tick 1ms */
void fsm_tick(void)
{
    ev_tick1ms = true;
}

/* IRQ EOC (falling) */
void fsm_raise_eoc1_irq(void){
	ev_eoc1 = true;
}
void fsm_raise_eoc2_irq(void){
	ev_eoc2 = true;
}
void fsm_raise_eoc3_irq(void){
	ev_eoc3 = true;
}
