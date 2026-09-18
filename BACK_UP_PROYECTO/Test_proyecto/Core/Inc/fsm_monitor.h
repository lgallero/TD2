/* USER CODE BEGIN Header */
/**
 **************************
 * @file          fsm_monitor.h
 **************************
 */
/* USER CODE END Header */

#ifndef CORE_INC_FSM_MONITOR_H_
#define CORE_INC_FSM_MONITOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include <stdbool.h>

/*==================[typedef]================================================*/

typedef enum {
    INIT,
    PEDIR_RTC,      // Envia la solicitud por WiFi
    WAIT_RTC,       // Espera la respuesta (No bloqueante)
    IDLE,
    CONFIGURANDO,
    ESPERANDO_DESCARGA,
    DESCARGANDO_AUTOMATICO,
    FIN_DESCARGA,
    MANUAL,
    DESCARGANDO_MANUAL
} FSM_MONITOR_STATES_T;

typedef enum {
    PREGUNTA_TENSION,
    WAIT_1,
    PREGUNTA_CAPACIDAD,
    WAIT_2,
    PREGUNTA_TIPO,
    WAIT_3,
    ENVIAR_PARAMETROS,
    CHECK_PARAMETROS
} FSM_PREGUNTAS_STATES_T;

/*==================[external functions declaration]=========================*/
void FSM_MONITOR_INIT(void);
void ev_esp_ON_raise(void);
void ev_pulsador_raise(void);

#ifdef __cplusplus
}
#endif
#endif /* CORE_INC_FSM_MONITOR_H_ */
