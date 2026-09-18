/**
 * @file user_gpio.h
 * @brief Abstracción de hardware para relés, ventilación y RS485.
 */

#ifndef INC_USER_GPIO_H_
#define INC_USER_GPIO_H_

#include "main.h" // Necesario para los defines de pines (LOAD1_Pin, etc)
#include <stdint.h>
#include <stdbool.h>

/* ==============================================================================
 * CONTROL DE COMUNICACIÓN (RS485)
 * ============================================================================== */
/**
 * @brief Habilita la línea de transmisión RS485.
 */
void RS485_TX_ENABLE(void);

/**
 * @brief Habilita la línea de recepción RS485.
 */
void RS485_RX_ENABLE(void);

/* ==============================================================================
 * CONTROL TÉRMICO (Ventilación)
 * ============================================================================== */
/**
 * @brief Activa el ventilador de control térmico.
 */
void activarVentilacion(void);

/**
 * @brief Apaga el ventilador de control térmico.
 */
void desactivarVentilacion(void);

/* ==============================================================================
 * CONTROL DE POTENCIA (Relés de Carga y Tensión)
 * ============================================================================== */
/**
 * @brief Configura el relé selector de tensión base.
 * @param modo_48v 1 = Sistema 48V (Relé ON), 0 = Sistema 24V (Relé OFF).
 */
void configurarTensionDescarga(uint8_t modo_48v);

/**
 * @brief Apaga todas las resistencias de carga.
 */
void desactivarResistencias(void);

/**
 * @brief Activa resistencias según el nivel de corriente deseado.
 * @param nivel 0 = apagado, 1 = carga parcial (LOAD1), 2 = carga total (LOAD1+LOAD2).
 */
void activarResistencias(uint8_t nivel);

#endif /* INC_USER_GPIO_H_ */
