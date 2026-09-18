#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_

/*==================[inclusions]=============================================*/
#include "main.h"
/*==================[macros]=================================================*/
/*==================[typedef]================================================*/
typedef enum {
	WAIT_RESET,         // Espera que el modulo complete el reset
	PREV_SET_MODE,      // Reservado para un modo previo de configuracion
	WAIT_PREV_SET_MODE, // Reservado: espera confirmacion de modo previo
	SET_MODE,           // Configuracion del modo de operacion (station)
	WAIT_SET_MODE,      // Espera confirmacion de modo
	SET_RED,            // Solicita union a la red WiFi
	WAIT_RED,           // Espera eventos de conexion/IP
	SET_CONN,           // Reservado para conexiones adicionales
	SET_UDP,            // Establece socket UDP contra el servidor
	WAIT_UDP,           // Espera que el socket quede listo
	IDLE,               // Reposo: monitorea datos entrantes
	RECEIVING,          // Recibiendo payload +IPD
	SEND_LEN,           // Envio de cabecera AT+CIPSEND
	WAIT_LEN,           // Espera prompt '>' del modulo
	SEND_DATA,          // Transmite el payload al ESP8266
	WAIT_DATA           // Espera confirmacion SEND OK

} FSM_ESP_STATES_T;
/*==================[external data declaration]==============================*/
/*==================[external functions declaration]=========================*/

/** @addtogroup accion_esp Acciones del Modulo ESP
 *
 * @{ */

/**
 * @brief Funcion externa que envia el mensaje desde el ESP
 * @param data Puntero a los datos a enviar
 * @param len Longitud en bytes de los datos a enviar
 */
void esp8266_send_data(const char *data, size_t len);



/** @} */

/** @addtogroup funciones_esp Inicializacion y Funcionamiento del Modulo ESP
 *
 * @{ */

/** @brief Funcion que ejecuta un reset al modulo ESP8266 */
extern void esp8266_init(void);

/** @} */

/*==================[end of file]============================================*/

#endif /* INC_ESP8266_H_ */
