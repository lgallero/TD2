/*
 * user_gpio.c
 *
 * Description: Implementación de control físico.
 * Mapeo según imagen:
 * - RS485_DE -> PB1
 * - FAN_OUT  -> PA5
 * - LOAD1    -> PA7
 * - LOAD2    -> PA6
 * - OUT_24_48 -> PA4
 */

#include "user_gpio.h"
#include "fsm_monitor.h" // Para llamar a ev_pulsador_raise
#include "FreeRTOS.h"
#include "task.h"

/* ==============================================================================
 * CONTROL RS485
 * ============================================================================== */
void RS485_TX_ENABLE(void)
{
    // Habilita el driver de transmisión (DE = High)
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);
}

void RS485_RX_ENABLE(void)
{
    // Habilita el receptor (DE = Low, /RE = Low)
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
}

/* ==============================================================================
 * CONTROL VENTILACIÓN
 * ============================================================================== */
void activarVentilacion(void)
{
    HAL_GPIO_WritePin(FAN_OUT_GPIO_Port, FAN_OUT_Pin, GPIO_PIN_SET);
}

void desactivarVentilacion(void)
{
    HAL_GPIO_WritePin(FAN_OUT_GPIO_Port, FAN_OUT_Pin, GPIO_PIN_RESET);
}

/* ==============================================================================
 * CONTROL DE CARGAS Y TENSIÓN
 * ============================================================================== */

void configurarTensionDescarga(uint8_t modo_48v)
{
    /* * Lógica del Relé OUT_24_48V:
     * Si es 1 (TRUE) -> Activamos pin (Modo 48V)
     * Si es 0 (FALSE) -> Desactivamos pin (Modo 24V)
     */
    if (modo_48v)
    {
        HAL_GPIO_WritePin(OUT_24_48V_GPIO_Port, OUT_24_48V_Pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(OUT_24_48V_GPIO_Port, OUT_24_48V_Pin, GPIO_PIN_RESET);
    }
}

void desactivarResistencias(void)
{
    HAL_GPIO_WritePin(LOAD1_GPIO_Port, LOAD1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LOAD2_GPIO_Port, LOAD2_Pin, GPIO_PIN_RESET);
}

void activarResistencias(uint8_t nivel)
{
    switch (nivel)
    {
        case 0:
            // Todo OFF
            HAL_GPIO_WritePin(LOAD1_GPIO_Port, LOAD1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LOAD2_GPIO_Port, LOAD2_Pin, GPIO_PIN_RESET);
            break;

        case 1:
            // Nivel 1: Solo LOAD1
            HAL_GPIO_WritePin(LOAD1_GPIO_Port, LOAD1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LOAD2_GPIO_Port, LOAD2_Pin, GPIO_PIN_RESET);
            break;

        default:
            // Nivel 2 (o mayor): LOAD1 + LOAD2 (Potencia Máxima)
            HAL_GPIO_WritePin(LOAD1_GPIO_Port, LOAD1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LOAD2_GPIO_Port, LOAD2_Pin, GPIO_PIN_SET);
            break;
    }
}

/* ==============================================================================
 * INTERRUPCIONES EXTERNAS (Pulsador)
 * ============================================================================== */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    /* Verificamos si la interrupción vino del Pulsador de usuario */
    if (GPIO_Pin == PULSADOR_Pin)
    {
        // Notificamos a la FSM del Monitor
        extern void ev_pulsador_raise(void);
        ev_pulsador_raise();
    }
}
