/*
 * user_gpio.h
 *
 * Description: Abstracción de hardware para relés, ventilación y RS485.
 */

#ifndef INC_USER_GPIO_H_
#define INC_USER_GPIO_H_

#include "main.h" // Necesario para los defines de pines (LOAD1_Pin, etc)
#include <stdint.h>
#include <stdbool.h>

/* ==============================================================================
 * CONTROL DE COMUNICACIÓN (RS485)
 * ============================================================================== */
void RS485_TX_ENABLE(void);
void RS485_RX_ENABLE(void);

/* ==============================================================================
 * CONTROL TÉRMICO (Ventilación)
 * ============================================================================== */
void activarVentilacion(void);
void desactivarVentilacion(void);

/* ==============================================================================
 * CONTROL DE POTENCIA (Relés de Carga y Tensión)
 * ============================================================================== */

/**
 * @brief Configura el relé selector de tensión base.
 * @param modo_48v: 1 = Sistema 48V (Relé ON), 0 = Sistema 24V (Relé OFF).
 */
void configurarTensionDescarga(uint8_t modo_48v);

/**
 * @brief Apaga todas las cargas inmediatamente.
 */
void desactivarResistencias(void);

/**
 * @brief Activa las resistencias según el nivel de corriente deseado.
 * @param nivel:
 * 0 = Todo Apagado.
 * 1 = LOAD1 Encendida (Carga parcial).
 * 2 = LOAD1 + LOAD2 Encendidas (Carga total/Litio).
 */
void activarResistencias(uint8_t nivel);

#endif /* INC_USER_GPIO_H_ */
