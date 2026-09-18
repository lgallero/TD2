/*
 * fsm.c
 *
 *  Created on: Dec 17, 2025
 *      Author: lucas
 */

#include "fsm.h"
#include "user_gpio.h"
#include <stdint.h>
#include <stdbool.h>

/* Enunciado: ya existe esta función */
extern bool motor_alarma(void) {  /* true si hay alarma */
	return false;
};
/* Tick 100ms */
#define STOP_DELAY_TICKS   (300U) /* 30s */

typedef enum {
    ST_IDLE = 0,
    ST_RUN_SUBIR,
    ST_DELAY_SUBIR,
    ST_RUN_BAJAR,
    ST_DELAY_BAJAR,
    ST_ALARM_LATCH
} fsm_state_t;

static fsm_state_t state;

/* ===== Eventos (contadores para no perder pulsos en EXTI) ===== */

static volatile bool ev_in_subir  = false;

static volatile bool ev_out_subir = false;

static volatile bool ev_in_bajar  = false;

static volatile bool ev_out_bajar = false;

static volatile bool ev_tick100ms = false;

static uint32_t cont_ms = 0;

/* ===== Lógica de personas ===== */
static int32_t persons = 0; // SOLO del sentido activo

/* Timer para “30s después de la última salida” */
static uint16_t stop_timer = 0;

/* ===== Helpers ===== */

static void clear_tick(void)
{
    ev_tick100ms = false;
}


static void go_alarm(void)
{
    motor_stop();
    balizas_on();
    state = ST_ALARM_LATCH;
}

/* ===== API ===== */

void fsm_init(void)
{
    state = ST_IDLE;
    persons = 0;
    stop_timer = 0;

    motor_stop();
    balizas_off();

    ev_in_subir  = false;
    ev_out_subir = false;
    ev_in_bajar  = false;
    ev_out_bajar = false;

    ev_tick100ms = false;
    cont_ms = 0;
}

void fsm_runCycle(void)
{
    /* Alarma tiene prioridad global y es “latched” */
    if (state != ST_ALARM_LATCH)
    {
        if (motor_alarma())
        {
            go_alarm();
            clear_tick();
            return;
        }
    }

    switch (state)
    {
    case ST_IDLE:
        /* Si alguien ingresa por SUBIR -> arranco SUBIR y PARE BAJAR */
        if (ev_in_subir)
        {
        	ev_in_subir = false;
            motor_subir_on();
            balizas_off();
            baliza_bajar_on(); /* PARE sentido opuesto */
            persons = 1;
            state = ST_RUN_SUBIR;
        }
        /* Si alguien ingresa por BAJAR -> arranco BAJAR y PARE SUBIR */
        else if (ev_in_bajar)
        {
        	ev_in_bajar = false;
            motor_bajar_on();
            balizas_off();
            baliza_subir_on();
            persons = 1;
            state = ST_RUN_BAJAR;
        }
        break;

    case ST_RUN_SUBIR: // SUBE

        if (ev_in_subir) // Sube una persona
        {
        	ev_in_subir = false;
            persons++;
        }

        if (ev_out_subir) // Sale una persona
        {
        	ev_out_subir = false;
            if (persons > 0){
            	persons--;
            }
        }

        /* Si quedó en 0: arranco el delay de 30s (motor sigue andando durante el delay) */
        if (persons == 0)
        {
            stop_timer = STOP_DELAY_TICKS;
            state = ST_DELAY_SUBIR;
        }

        /* Sensores del otro sentido se ignoran (PARE encendido) */
        ev_in_bajar  = false;
        ev_out_bajar = false;
        break;

    case ST_DELAY_SUBIR: // Espera el tiempo para parar la subida

        /* Si entra alguien de nuevo en SUBIR, cancelo el apagado */
        if (ev_in_subir)
        {
        	ev_in_subir = false;
            persons++;
            state = ST_RUN_SUBIR;
        }

        /* Cuenta 30s sin bloquear */
        if (ev_tick100ms)
        {
            if (stop_timer > 0){
            	stop_timer--;
            }

            if (stop_timer == 0)
            {
                motor_stop();
                balizas_off();
                state = ST_IDLE;
            }
        }

        ev_in_bajar  = false;
        ev_out_bajar = false;
        break;


    case ST_RUN_BAJAR:

        if (ev_in_bajar) // baja otra persona
        {
        	ev_in_bajar = false;
            persons++;
        }

        if (ev_out_bajar) // Salio una persona
        {
        	ev_out_bajar = false;
            if (persons > 0) {
            	persons--;
            }
        }

        if (persons == 0)
        {
            stop_timer = STOP_DELAY_TICKS;
            state = ST_DELAY_BAJAR;
        }

        ev_in_subir  = false;
        ev_out_subir = false;
        break;

    case ST_DELAY_BAJAR:

        if (ev_in_bajar) // ingresa una persona se cancela
        {
        	ev_in_bajar = false;
            persons++;
            state = ST_RUN_BAJAR;
        }

        if (ev_tick100ms)
        {
            if (stop_timer > 0) {
            	stop_timer--;
            }

            if (stop_timer == 0)
            {
                motor_stop();
                balizas_off();
                state = ST_IDLE;
            }
        }

        ev_in_subir  = false;
        ev_out_subir = false;
        break;

    case ST_ALARM_LATCH:
        /* Permanente */
        motor_stop();
        balizas_on();
        break;

    default:
        state = ST_IDLE;
        break;
    }

    clear_tick();
}

void fsm_tick(void)
{
    cont_ms++;
    if (cont_ms >= 100)
    {
        cont_ms = 0;
        ev_tick100ms = true;
    }
}

/* ===== ISRs (llamadas desde EXTI) ===== */

void fsm_raise_in_subir(void){
	ev_in_subir = true;
}
void fsm_raise_out_subir(void) {
	ev_out_subir = true;
}
void fsm_raise_in_bajar(void)  {
	ev_in_bajar = true;
}
void fsm_raise_out_bajar(void) {
	ev_out_bajar =true;
}

