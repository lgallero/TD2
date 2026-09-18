/*
 * fsm.c
 *
 *  Created on: Dec 17, 2025
 *      Author: lucas
 */

/*==================[inclusions]=============================================*/
#include "fsm.h"
#include "user_gpio.h"
#include <stdint.h>
#include <stdbool.h>

/*==================[macros and definitions]=================================*/
/* Tiempos en ticks de 100 ms */
#define TICK_100MS_PER_SEC   (10U)
#define STAGE_TOTAL_TICKS    (50U)   /* 5 s */
#define MEASURE_AT_TICKS     (40U)   /* 4 s */
#define ERROR_TOTAL_TICKS    (100U)  /* 10 s */
#define BLINK_TOGGLE_TICKS   (5U)    /* 0.5 s -> 1 Hz (on/off) */
#define ERROR_CHECK_TICKS    (10U)   /* 1 s */

/* Umbrales del enunciado */
#define V_MIN   (5U)
#define V_MAX   (100U)
#define I_MAX   (5U)

/*==================[external functions (I2C) declaration]====================*/
/* Asumimos que ya existen (enunciado) */
extern uint16_t i2c_leer_tension(void){
	return 0;
};
extern uint16_t i2c_leer_corriente(void){
	return 0;
};

/*==================[internal data definition]===============================*/
static FSM_MEAS_STATES_T state;

static volatile bool ev_start_pressed = false;
static volatile bool ev_tick100ms = false;

static uint32_t cont_ms = 0;

/* Contadores */
static uint16_t cont_ticks = 0;       /* cuenta 0..50 en etapa */
static bool stage_measured = false;    /* ya medimos a los 4s */

static uint16_t blink_div = 0;
static uint16_t error_check_div = 0;

/* Últimas mediciones (por si querés guardarlas) */
static uint16_t last_v = 0;
static uint16_t last_i = 0;

/*==================[internal functions definition]==========================*/
static void clearEvents(void)
{
    ev_start_pressed = false;
    ev_tick100ms = false;
}

static bool is_error(uint16_t v, uint16_t i)
{
    if (v < V_MIN) return true;
    if (v > V_MAX) return true;
    if (i > I_MAX) return true;
    return false;
}

/*==================[external functions definition]==========================*/

void fsm_init(void)
{
    state = MEAS_IDLE;
    cont_ms = 0;

    cont_ticks = 0;
    stage_measured = false;

    blink_div = 0;
    error_check_div = 0;

    indicator_off();
    load_set_none();

    clearEvents();
}

void fsm_runCycle(void)
{
    switch (state)
    {
    case MEAS_IDLE:
        if (ev_start_pressed)
        {
            cont_ticks = 0;
            indicator_on();
            load_set_none();
        }
        break;

    case MEAS_NOLOAD:
    	if (ev_tick100ms)
    	{
    	  cont_ticks++;

    	  if(cont_ticks == MEASURE_AT_TICKS ){ // 4segundos
              last_v = i2c_leer_tension();
              last_i = i2c_leer_corriente();

              if (is_error(last_v, last_i))
              {
                  /* Error: desconectar carga y pasar a señalización */
                  load_set_none();
                  state = MEAS_ERROR_BLINK;

                  cont_ticks = 0;
                  blink_div = 0;
                  indicator_off(); /* arranco desde un estado conocido */
                  break;
              }
    	  }


    	  if(cont_ticks == STAGE_TOTAL_TICKS ){ // 5 segundos
    		  load_set_low();
    		  state = MEAS_LOW;
    	  }
    	}

    break;


    case MEAS_LOW:
    	if (ev_tick100ms)
    	{
    	  cont_ticks++;

    	  if(cont_ticks == MEASURE_AT_TICKS ){ // 4segundos
              last_v = i2c_leer_tension();
              last_i = i2c_leer_corriente();

              if (is_error(last_v, last_i))
              {
                  /* Error: desconectar carga y pasar a señalización */
                  load_set_none();
                  state = MEAS_ERROR_BLINK;

                  cont_ticks = 0;
                  blink_div = 0;
                  indicator_off(); /* arranco desde un estado conocido */
                  break;
              }
    	  }


    	  if(cont_ticks == STAGE_TOTAL_TICKS ){ // 5 segundos
    		  load_set_med();
    		  state = MEAS_MED;
    	  }
    	}
    break;

    case MEAS_MED:
    	if (ev_tick100ms)
    	{
    	  cont_ticks++;

    	  if(cont_ticks == MEASURE_AT_TICKS ){ // 4segundos
              last_v = i2c_leer_tension();
              last_i = i2c_leer_corriente();

              if (is_error(last_v, last_i))
              {
                  /* Error: desconectar carga y pasar a señalización */
                  load_set_none();
                  state = MEAS_ERROR_BLINK;

                  cont_ticks = 0;
                  blink_div = 0;
                  indicator_off(); /* arranco desde un estado conocido */
                  break;
              }
    	  }


    	  if(cont_ticks == STAGE_TOTAL_TICKS ){ // 5 segundos
    		  load_set_high();
    		  state = MEAS_HIGH;
    	  }
    	}
    break;

    case MEAS_HIGH:
    	if (ev_tick100ms)
    	{
    	  cont_ticks++;

    	  if(cont_ticks == MEASURE_AT_TICKS ){ // 4segundos
              last_v = i2c_leer_tension();
              last_i = i2c_leer_corriente();

              if (is_error(last_v, last_i))
              {
                  /* Error: desconectar carga y pasar a señalización */
                  load_set_none();
                  state = MEAS_ERROR_BLINK;

                  cont_ticks = 0;
                  blink_div = 0;
                  indicator_off(); /* arranco desde un estado conocido */
                  break;
              }
    	  }


    	  if(cont_ticks == STAGE_TOTAL_TICKS ){ // 5 segundos
              load_set_none();
              indicator_off();
    		  state = MEAS_IDLE;
    	  }
    	}
    break;

    case MEAS_ERROR_BLINK:
        /* 10 segundos destellando 1 Hz (toggle cada 0.5 s) */
        if (ev_tick100ms)
        {
            cont_ticks++;
            blink_div++;
            if (blink_div >= BLINK_TOGGLE_TICKS)
            {
                blink_div = 0;
                indicator_toggle();
            }

            if (cont_ticks >= ERROR_TOTAL_TICKS)
            {
                /* terminó el tiempo */
                indicator_off();
                state = MEAS_ERROR_WAIT_CLEAR;
                cont_ticks = 0;
            }
        }
        break;

    case MEAS_ERROR_WAIT_CLEAR:
        /* No permitir nueva medición hasta que la condición de error NO esté presente */
        if (ev_tick100ms)
        {
        	cont_ticks++;
            if (cont_ticks >= ERROR_CHECK_TICKS)
            {
            	cont_ticks = 0;

                last_v = i2c_leer_tension();
                last_i = i2c_leer_corriente();

                if (!is_error(last_v, last_i))
                {
                    state = MEAS_IDLE;
                }
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
        ev_tick100ms = true;
    }
}

void fsm_raise_start_pressed(void)
{
    ev_start_pressed = true;
}

