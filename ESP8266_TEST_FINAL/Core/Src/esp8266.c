/*==================[inclusions]=============================================*/
#define _GNU_SOURCE

#include "esp8266.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "semphr.h"
#include "queue.h"
#include "stdlib.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
/*==================[macros and definitions]=================================*/
#define RX_BUF_LEN	256              // Buffer circular para recepciones UART
#define ATOI_BUFFER_SIZE 5            // Tamaño maximo para convertir longitud recibida

#define CWJAP_CMD		"AT+CWJAP=\"test\",\"12345678\"\r\n"             // Conecta la WiFi
#define CIPSTART_CMD	"AT+CIPSTART=\"UDP\",\"10.51.190.169\",8000,8001\r\n" // Abre socket UDP remoto
#define CIPSEND_CMD		"AT+CIPSEND="                                     // Prefijo para enviar datos
#define CWMODE_CMD		"AT+CWMODE=1\r\n"                                 // Configura modo Station
#define RESET_CMD 		"AT+RST\r\n"                                      // Reinicia el modulo
#define READY_MSG		"ready"                                           // Mensajes esperados desde el ESP8266
#define SOK_MSG			"SEND OK\r\n"
#define OK_MSG			"OK\r\n"
#define WCONN_MSG		"WIFI CONNECTED\r\n"
#define CONN_MSG		"CONNECT\r\n"
#define WGIP_MSG		"WIFI GOT IP\r\n"
#define DATA_MSG		">"
/*==================[internal data declaration]==============================*/
/*==================[internal functions declaration]=========================*/
/*==================[internal data definition]===============================*/

#define ESP_UART huart1               // UART fisica conectada al modulo ESP8266
extern UART_HandleTypeDef ESP_UART;

//static uint32_t counter_ms;
static uint8_t rx_buf[RX_BUF_LEN];                    // Buffer de recepcion compartido con HAL
static uint8_t local_output_buf[RX_BUF_LEN];          // Copia local de la ultima trama recibida
static uint32_t recv_total_size;                      // Longitud esperada de datos entrantes
static volatile size_t pending_length = 0;            // Bytes por transmitir al modulo
static volatile uint8_t tx_auxbuf[RX_BUF_LEN];        // Buffer temporal para enviar datos usuario
#define ESP8266_QUEUE_LENGTH 8

static volatile bool tx_done = false;                 // Flag levantada en callback de TX

static volatile FSM_ESP_STATES_T state;               // Estado actual de la FSM
static bool evSendData;                               // Reservado para futuros eventos TX

// TX ~~~~~~~~~~~~~~~~~~~~~~~~~

typedef struct {
	uint8_t data[RX_BUF_LEN];
	size_t len;
} tx_message_t;

static uint8_t tx_aux[RX_BUF_LEN];                    // Buffer auxiliar reservado para ampliaciones

static QueueHandle_t tx_queue;                        // Cola para serializar comandos AT
static StaticQueue_t tx_queue_struct;
static uint8_t tx_queue_storage[sizeof(tx_message_t) * ESP8266_QUEUE_LENGTH];

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~


// Mandar msg a la cola ~~~~~~~~~~~~~~~~~~~~

// Inserta un comando AT en la cola de transmision con copia segura
static void encolar_comando(const char *cmd)
{
	tx_message_t msg;
	size_t len = strlen(cmd);
	if (len >= RX_BUF_LEN) return;
	memcpy(msg.data, cmd, len);
	msg.len = len;
	xQueueSend(tx_queue, &msg, 0);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Reinicia el buffer de recepcion y punteros de HAL
static void clear_rx_buffer(void) {
	huart1.pRxBuffPtr = rx_buf;
	huart1.RxXferSize = RX_BUF_LEN;
	huart1.RxXferCount = RX_BUF_LEN;
	bzero(rx_buf, RX_BUF_LEN);
}

// Evita overflow borrando el buffer si se lleno por completo
static void clear_rx_if_full(void) {
	/* If no zero byte remains, assume the buffer filled up and reset it */
	if (memchr(rx_buf, '\0', RX_BUF_LEN) == NULL) {
		clear_rx_buffer();
	}
}

// Busca una cadena dentro del buffer recibido del ESP8266
static bool search_str(const char *str) {
	size_t len = strlen(str);
	return memmem(rx_buf, RX_BUF_LEN, str, len) != NULL;
}

// Limpia flags de evento locales tras procesar la iteracion
static void clearEvents(void) {
	evSendData = 0;
}

// Obtiene el tamaño total anunciado en la trama +IPD,<len>:<payload>
static uint32_t get_rcv_total_size() {
	char atoiBuffer[ATOI_BUFFER_SIZE];
	uint8_t *p_size;
	uint32_t i = 0;

	p_size = memmem(rx_buf, RX_BUF_LEN, "+IPD,", 5) + 5;
	while ((char) p_size[i] != ':') {
		atoiBuffer[i] = p_size[i];
		i++;
	}
	atoiBuffer[i] = '\0';
	return (uint32_t) atoi(atoiBuffer);
}

// Calcula cuantos bytes de payload se recibieron hasta el momento
static uint32_t get_rcv_size() {
	uint8_t *p_msg = memmem(rx_buf, RX_BUF_LEN, ":", 1);
	return strlen((char*) p_msg);
}

// Copia el payload recibido al buffer de salida para su procesamiento
static void copy_to_output_buffer() {
	uint8_t *p_msg = memmem(rx_buf, RX_BUF_LEN, ":", 1);
	size_t copy_len = recv_total_size;

	p_msg += 1;

	memcpy(local_output_buf, p_msg, copy_len);
	local_output_buf[copy_len] = '\0';
}

// Interpreta comandos remotos para controlar el LED onboard
static void handle_led_command(void) {
	if (strncmp((char*) local_output_buf, "ON", 2) == 0) {
		HAL_GPIO_WritePin(LED_ON_GPIO_Port, LED_ON_Pin, GPIO_PIN_SET);
		clear_rx_buffer();
	} else if (strncmp((char*) local_output_buf, "OFF", 3) == 0) {
		HAL_GPIO_WritePin(LED_ON_GPIO_Port, LED_ON_Pin, GPIO_PIN_RESET);
		clear_rx_buffer();
	}
}

//FUNCIÓN DE CALLBACK TX
// Callback invocado por HAL al finalizar una transmision
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart1) {
	tx_done = 1;
}


//Tarea: Transmision UART_ESP
// Extrae comandos de la cola y los envía por UART de forma no bloqueante
static void task_Tx(void* p) {
	(void)p;
	tx_message_t msg;

	while (1) {
		if (xQueueReceive(tx_queue, &msg, portMAX_DELAY) == pdPASS) {
			HAL_UART_Transmit_IT(&ESP_UART, msg.data, msg.len);
			while (!tx_done) {
				vTaskDelay(1);
			}
			tx_done = false;
		}
	}
}

// Tarea: FSM del Modulo ESP8266
// Implementa el flujo de configuracion y comunicacion con el modulo ESP8266
static void task_ESP(void* p) {

	//HAL_UART_Transmit_IT(&huart1, RESET_CMD, strlen(RESET_CMD));
	encolar_comando(RESET_CMD);
	vTaskDelay(2000);
	while(1){
		HAL_UART_Receive_IT(&huart1, rx_buf, RX_BUF_LEN);
		clear_rx_if_full();
		switch (state) {
		case WAIT_RESET: // BIEN
			// Tras el reset se pasa a configurar el modo de operacion
			state = SET_MODE;
			clear_rx_buffer();
			break;

		case SET_MODE:// BIEN
			// Configura el modulo en modo station
			//HAL_UART_Transmit_IT(&huart1, CWMODE_CMD, strlen(CWMODE_CMD));
			encolar_comando(CWMODE_CMD);
			state = WAIT_SET_MODE;
			break;

		case WAIT_SET_MODE: // BIEN
			if (search_str(OK_MSG)) {
				clear_rx_buffer();
				state = SET_RED;
			}
			break;

		case SET_RED:// BIEN
			// Solicita conexion a la red WiFi configurada
			//HAL_UART_Transmit_IT(&huart1, CWJAP_CMD, strlen(CWJAP_CMD));
			encolar_comando(CWJAP_CMD);
			state = WAIT_RED;

			break;

		case WAIT_RED:// BIEN
			// Espera confirmaciones de conexion e IP
			if (search_str(OK_MSG) &&
					search_str(WCONN_MSG) &&
					search_str(WGIP_MSG)) {
				clear_rx_buffer();
				state = SET_UDP;
			}
			break;

		case SET_UDP:// BIEN
			// Abre un socket UDP hacia el servidor remoto
			//HAL_UART_Transmit_IT(&huart1, CIPSTART_CMD, strlen(CIPSTART_CMD));
			encolar_comando(CIPSTART_CMD);
			state = WAIT_UDP;
			break;

		case WAIT_UDP:// BIEN
			// Continua cuando el socket quedo listo
			if (search_str(OK_MSG)) {
				state = IDLE;
				clear_rx_buffer();}
			break;

			// Conexion completa:
		case IDLE:// BIEN
			//vTaskDelay(50);
			// Estado de reposo esperando llegada de datos
			if (search_str("+IPD") && search_str(":")) {
				recv_total_size = get_rcv_total_size();
				state = RECEIVING;
			}

			break;

		case RECEIVING:// BIEN
			// Procesa el payload completo una vez recibido
			if (get_rcv_size() >= recv_total_size) {
				copy_to_output_buffer();
				handle_led_command();
				//clear_rx_buffer();
				state = IDLE;
			}
			break;

		case SEND_LEN:
			// Envia el prefijo AT+CIPSEND con la longitud solicitada

			char cmd[128];
			snprintf(cmd, sizeof(cmd),"AT+CIPSEND=%d\r\n",pending_length);
			encolar_comando(cmd);
			state = WAIT_LEN;
			break;

		case WAIT_LEN:// BIEN
			// Espera al prompt '>' que indica listo para recibir datos
			if (search_str(DATA_MSG)) { // busca el >
				clear_rx_buffer();
				state = SEND_DATA;
			}
			break;

		case SEND_DATA:
			// Entrega los datos pendientes al modulo
			encolar_comando(tx_auxbuf);
			state = WAIT_DATA;
			break;

		case WAIT_DATA:// BIEN
			// Finaliza la transmision cuando el modulo confirma envio
			if (search_str(SOK_MSG)) { // busca SEND OK
				clear_rx_buffer();
				state = IDLE;
			}
			break;

		}
		clearEvents();
	}
}

void esp8266_init(void) {

	// Crea la cola de mensajes y tareas de FreeRTOS asociadas al modulo
	tx_queue = xQueueCreateStatic(ESP8266_QUEUE_LENGTH, sizeof(tx_message_t), tx_queue_storage, &tx_queue_struct);

	xTaskCreate(task_ESP, (const char*) "task_ESP", configMINIMAL_STACK_SIZE*2, NULL, osPriorityNormal, NULL);
	xTaskCreate(task_Tx, (const char*) "task_Tx", configMINIMAL_STACK_SIZE , NULL, osPriorityNormal, NULL);
	state = WAIT_RESET;
	clearEvents();
	clear_rx_buffer();
	bzero(rx_buf, RX_BUF_LEN);
}

void esp8266_send_data(const char *data, size_t len)
{
    // Prepara datos a enviar sobre la proxima sesion AT+CIPSEND
    memcpy(tx_auxbuf, data,len);
    pending_length = len;
    state = SEND_LEN;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	BaseType_t switch_required = pdFALSE;

	switch (GPIO_Pin) {

	case BTN_Pin:
		// Cuando se presiona el pulsador se manda un mensaje de prueba
		esp8266_send_data("Hola desde BLUEPILL!!", strlen("Hola desde BLUEPILL!!"));
		break;

	default:
		break;
	}

	portEND_SWITCHING_ISR(switch_required);
}
