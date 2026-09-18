/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file          cafetera.h
 ** @brief        Header para el archivo cafetera
 *                Este archivo se encarga de las definiciones particulares para el control de la erogación
 *
 *
 ******************************************************************************
 */
/* USER CODE END Header */




#ifndef INC_CAFETERA_H_
#define INC_CAFETERA_H_



/*==================[inclusions]=============================================*/

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/


typedef enum {
	REPOSO,
	ELEGIR_CAFE,
	SIRVIENDO_CAFE,
	RETIRAR_CAFE
} FSM_CAFETERA_STATES_T;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Inicializa el estado inicial de la cafetera.
 */
void cafetera_init(void);

/**
 * @brief Ejecuta un ciclo de evaluación de eventos y transición de estados.
 */
void cafetera_runCycle(void);

/**
 * @brief Incrementa el temporizador de la cafetera (SysTick).
 */
void cafetera_tick(void);

/**
 * @brief Activa el evento por inserción de ficha.
 */
void cafetera_raise_evFicha_On(void);

/**
 * @brief Activa el evento de selección de café negro.
 */
void cafetera_raise_evCafeNegro_On(void);

/**
 * @brief Activa el evento de selección de café con leche.
 */
void cafetera_raise_evCafeLeche_On(void);


void ToggleLedTestigo(void);
void CafeErogacionOn(void);
void CafeErogacionOff(void);
void LecheErogacionOn(void);
void LecheErogacionOff(void);
void BuzzerOn(void);
void BuzzerOff(void);
void ApagarLedTestigo(void);
void PrenderLedTestigo(void);

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/

#endif /* INC_CAFETERA_H_ */

