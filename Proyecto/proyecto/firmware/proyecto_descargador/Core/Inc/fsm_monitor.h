/**
 * @file fsm_monitor.h
 * @brief Máquina de estados para supervisar el descargador y el flujo de preguntas al monitor.
 */

#ifndef CORE_INC_FSM_MONITOR_H_
#define CORE_INC_FSM_MONITOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include <stdbool.h>

/*==================[typedef]================================================*/

/**
 * @brief Estados principales del flujo de monitoreo del descargador.
 */
typedef enum {
    INIT,                  ///< Arranque inicial.
    PEDIR_RTC,             ///< Envío de solicitud de hora vía WiFi.
    WAIT_RTC,              ///< Espera no bloqueante de la respuesta.
    IDLE,                  ///< Espera sin tareas pendientes.
    CONFIGURANDO,          ///< Configurando parámetros en el monitor.
    ESPERANDO_DESCARGA,    ///< Listo para que comience la descarga.
    DESCARGANDO_AUTOMATICO,///< Descarga automática en curso.
    FIN_DESCARGA,          ///< Descarga finalizada.
    MANUAL,                ///< Modo manual activo.
    DESCARGANDO_MANUAL     ///< Descarga manual en curso.
} FSM_MONITOR_STATES_T;

/**
 * @brief Subestados para el flujo de preguntas al monitor.
 */
typedef enum {
    PREGUNTA_TENSION,   ///< Pregunta por la tensión nominal.
    WAIT_1,             ///< Espera de respuesta.
    PREGUNTA_CAPACIDAD, ///< Pregunta por la capacidad de la batería.
    WAIT_2,             ///< Espera de respuesta.
    PREGUNTA_TIPO,      ///< Pregunta por el tipo de batería.
    WAIT_3,             ///< Espera de respuesta.
    ENVIAR_PARAMETROS,  ///< Envía parámetros calculados al monitor.
    CHECK_PARAMETROS    ///< Verifica que la configuración se haya aplicado.
} FSM_PREGUNTAS_STATES_T;

/*==================[external functions declaration]=========================*/
/**
 * @brief Inicializa la FSM del monitor y configura recursos asociados.
 */
void FSM_MONITOR_INIT(void);

/**
 * @brief Señala que el módulo ESP8266 está listo (evento externo).
 */
void ev_esp_ON_raise(void);

/**
 * @brief Señala la pulsación del botón de usuario con anti-rebote.
 */
void ev_pulsador_raise(void);

#ifdef __cplusplus
}
#endif
#endif /* CORE_INC_FSM_MONITOR_H_ */
