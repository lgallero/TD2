/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file          esp8266.h
 ** @brief        Header para el archivo esp8266
 *                Este archivo se encarga de las definiciones particulares para el control por WIfi de la erogación
 *
 *
 ******************************************************************************
 */
/* USER CODE END Header */

#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_

/** @addtogroup esp8266 ESP8266
 *			Este módulo se encarga de configurar la conexión del dispositivo Wifi ESP8266,
 *			enviar los mensajes de estado de la cafetera y recibir.
 * @{
 */

/*==================[inclusions]=============================================*/
#include "main.h"
/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
typedef enum{
	WAIT_RESET,				/**< Espera a que se inicialice el dispositivo Wifi*/
	SET_MODE,				/**< Establece el modo de operación, en este caso cliente. */
	WAIT_SET_MODE,			/**< Espera el llamado del Callback de interrupciones para corroborar la conexión.*/
	SET_RED,				/**< Envía la red y la contraseña para establecer el enlace Wifi.*/
	WAIT_RED,				/**< Espera el llamado del Callback de interrupciones para corroborar la conexión.*/
	WAIT_READY_CONNECTED,	/**< Espera el establecimiento de la conexión Wifi y establece la comunicación UDP.*/
	WAIT_UDP_READY,			/**< Se inicia la comunicación UDP*/
	IDLE,					/**< Envía la cantidad de bytes que vamos a transmitir*/
	WAIT_SEND_DATA,			/**< Espera el mensaje a transmitir.*/
	SEND_DATA				/**< Confirma el envío del mensaje y vuelve a esperar la llegada de uno nuevo.*/
}esp8266_state_t;
/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/


/**
 * @brief Incrementa el temporizador del módulo ESP8266 (llamada periódica).
 */
extern void esp8266_tick(void);

/**
 * @brief Inicializa pines, buffers y estado inicial del módulo ESP8266.
 */
extern void esp8266_init(void);

/**
 * @brief Ejecuta la máquina de estados del ESP8266 (no bloqueante).
 */
extern void esp8266_loop(void);

/**
 * @brief Envia un mensaje de texto al ESP8266 de forma no bloqueante.
 * @param data Puntero al mensaje a enviar.
 * @param len Longitud del mensaje.
 * @return Bytes enviados o 0 si no se pudo enviar.
 */
extern int32_t esp8266_send_data(const char *data, size_t len);

/**
 * @brief Activa el evento para iniciar el envío de datos al ESP8266.
 */
void esp8266_raise_evSend(void);



/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/

#endif /* INC_ESP8266_H_ */
