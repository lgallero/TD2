/**
 * @file esp8266.h
 * @brief Interfaz del módulo ESP8266 manejado por UART y FreeRTOS.
 */

#ifndef CORE_INC_ESP8266_H_
#define CORE_INC_ESP8266_H_

/*==================[inclusions]=============================================*/
#include "main.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/
/** @brief Tamaño máximo de payload recibido por UART. */
#define ESP_RX_PAYLOAD_MAX 256U

/*==================[typedef]================================================*/
/**
 * @brief Mensaje recibido desde el ESP8266.
 */
typedef struct {
	size_t len;                         ///< Cantidad de bytes válidos en el buffer.
	uint8_t data[ESP_RX_PAYLOAD_MAX];   ///< Buffer con la trama recibida.
} esp_rx_msg_t;

/**
 * @brief Estados de la FSM que configura y usa el ESP8266.
 */
typedef enum {
	WAIT_RESET,   ///< Espera al reinicio del módulo.
	SET_MODE,     ///< Configura modo de operación.
	WAIT_SET_MODE,///< Espera la confirmación de modo.
	SET_RED,      ///< Configura red WiFi.
	WAIT_RED,     ///< Espera asociación a la red.
	GET_IP,       ///< Solicita IP asignada.
	WAIT_IP,      ///< Espera respuesta de IP.
	SET_UDP,      ///< Configura socket UDP.
	WAIT_UDP,     ///< Espera confirmación de socket.
	IDLE_ESP,     ///< En espera de nuevos comandos.
	RECEIVING,    ///< Recibiendo datos.
	SEND_LEN,     ///< Envía longitud de datos.
	WAIT_LEN,     ///< Espera confirmación de longitud.
	SEND_DATA,    ///< Envía datos.
	WAIT_DATA     ///< Espera fin de transmisión.
} FSM_ESP_STATES_T;

extern QueueHandle_t cola_rx; /**< Cola de recepción para tramas UART. */

/*==================[external data declaration]==============================*/
#define ESP_UART huart1 /**< UART asociado al módulo ESP8266. */

/*==================[external functions declaration]=========================*/
/**
 * @brief Inicializa la FSM y los recursos del ESP8266.
 */
extern void esp8266_init(void);

/**
 * @brief Envía datos crudos vía ESP8266.
 * @param data Puntero al buffer a enviar.
 * @param len  Cantidad de bytes a transmitir.
 */
extern void esp8266_send_data(const char *data, size_t len);

/**
 * @brief Limpia la cola de recepción del ESP8266.
 */
extern void esp8266_flush_rx(void);

#ifdef __cplusplus
}
#endif
#endif /* CORE_INC_ESP8266_H_ */
