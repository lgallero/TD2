/* USER CODE BEGIN Header */
/**
 **************************
 * @file          esp8266.h
 ** @brief        Header para el archivo esp8266
 *                Este archivo se encarga de las definiciones particulares para el control por WIfi de la erogación
 *
 *
 **************************
 */
/* USER CODE END Header */

#ifndef CORE_INC_ESP8266_H_
#define CORE_INC_ESP8266_H_

/* USER CODE BEGIN Header */
/**
 **************************
 * @file          esp8266.h
 ** @brief        Header para el archivo esp8266
 *                Este archivo se encarga de las definiciones particulares para el control por WIfi de la erogación
 *
 *
 **************************
 */
/* USER CODE END Header */


/*==================[inclusions]=============================================*/
#include "main.h"
#include <stddef.h>
#include <stdint.h>
/*==================[cplusplus]==============================================*/
#ifdef __cplusplus
extern "C" {
#endif
/*==================[macros]=================================================*/
/*==================[typedef]================================================*/
typedef enum{
	RESET_MODE,					/*< Reinicia el dispositivo Wifi*/
	WAIT_RESET,				/*< Espera a que se inicialice el dispositivo Wifi*/
	SET_MODE,				/*< Establece el modo de operación, en este caso cliente. */
	WAIT_SET_MODE,			/*< Espera el llamado del Callback de interrupciones para corroborar la conexión.*/
	SET_RED,				/*< Envía la red y la contraseña para establecer el enlace Wifi.*/
	WAIT_RED,				/*< Espera el llamado del Callback de interrupciones para corroborar la conexión.*/
	WAIT_READY_CONNECTED,	/*< Espera el establecimiento de la conexión Wifi y establece la comunicación UDP.*/
	WAIT_UDP_READY,			/*< Se inicia la comunicación UDP*/
	IDLE,					/*< Envía la cantidad de bytes que vamos a transmitir*/
	WAIT_SEND_DATA,			/*< Espera el mensaje a transmitir.*/
	SEND_DATA,				/*< Confirma el envío del mensaje y vuelve a esperar la llegada de uno nuevo.*/
	WAIT_TX_COMPLETE,		/*< Aguarda a que finalice el envío en curso.*/
	WAIT_SEND_CONFIRM		/*< Espera la respuesta SEND OK del módulo.*/

}esp8266_state_t;
/*==================[external data declaration]==============================*/
/**
 * @brief UART que esta conectado el ESP8266.
 */
#define ESP_UART huart2
/*==================[external functions declaration]=========================*/
/**
 * @brief Inicializa pines, buffers y estado inicial del módulo ESP8266.
 */
extern void esp8266_init(void);
/**
 * @brief Envia un mensaje de texto al ESP8266 de forma no bloqueante.
 * @param data Puntero al mensaje a enviar.
 * @param len Longitud del mensaje.
 * @return Bytes enviados o 0 si no se pudo enviar.
 */
extern int32_t esp8266_send_data(const char *data, size_t len);
/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif
#endif /* CORE_INC_ESP8266_H_ */
