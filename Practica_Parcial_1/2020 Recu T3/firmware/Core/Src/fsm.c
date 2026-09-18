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

/* Umbrales */
#define T_MIN_OK     (36.5f)
#define T_MAX_OK     (37.5f)   /* válida si <= 37.5 */
#define T_FEVER      (37.5f)   /* fiebre si > 37.5 */

/* Tiempos */
#define PERMIT_MS    (2000u)
#define BLOCK60_MS   (60000u)
#define BUZZ30_MS    (30000u)

typedef enum {
    IDLE = 0,
    PERMIT,
    BLOCK_60S,
    PERM_LOCK
} state_t;

static state_t st;

/* eventos */
static volatile bool ev_tick = false;
static volatile bool ev_door_open = false;

/* timer de estado */
static uint32_t t_ms = 0;

/* para buzzer dentro de PERM_LOCK */
static bool buzzer_already_off = false;

static bool temp_valid(float t)
{
    return (t >= T_MIN_OK && t <= T_MAX_OK);
}

static bool temp_fever(float t)
{
    return (t > T_FEVER);
}

void fsm_init(void)
{
    st = IDLE;
    t_ms = 0;
    ev_tick = false;
    ev_door_open = false;
    buzzer_already_off = false;

    lock_access();
    buzzer_off();
}

void fsm_runCycle(void)
{
    /* timebase */
    if (ev_tick)
    {
        ev_tick = false;
        t_ms++;
    }

    /* lectura de temperatura (no bloqueante) */
    float t = temp_get_celsius();

    /* transición global: fiebre => bloqueo permanente + buzzer 30s :contentReference[oaicite:2]{index=2} */
    if (st != PERM_LOCK && temp_fever(t))
    {
        st = PERM_LOCK;
        t_ms = 0;
        buzzer_already_off = false;

        lock_access();
        buzzer_on();
        ev_door_open = false;
        return;
    }

    switch (st)
    {
        case IDLE:
            lock_access();

            /* si temp válida => habilito 2s  */
            if (temp_valid(t))
            {
                unlock_access();
                st = PERMIT;
                t_ms = 0;
                ev_door_open = false;
            }
            break;

        case PERMIT:
            /* si abre puerta dentro de 2s => bloqueo 60s */
            if (ev_door_open)
            {
                ev_door_open = false;
                lock_access();
                st = BLOCK_60S;
                t_ms = 0;
            }
            /* si pasan 2s sin abrir => vuelvo a IDLE */
            else if (t_ms >= PERMIT_MS)
            {
                lock_access();
                st = IDLE;
                t_ms = 0;
            }
            break;

        case BLOCK_60S:
            /* ignora temps válidas durante 60s */
            lock_access();

            if (t_ms >= BLOCK60_MS)
            {
                st = IDLE;
                t_ms = 0;
            }
            break;

        case PERM_LOCK:
            /* siempre bloqueado */
            lock_access();

            /* buzzer solo 30s */
            if (!buzzer_already_off && t_ms >= BUZZ30_MS)
            {
                buzzer_off();
                buzzer_already_off = true;
            }
            break;
    }
}

void fsm_tick(void)
{
    ev_tick = true;
}

void fsm_ev_door_open_irq(void)
{
    ev_door_open = true;
}
