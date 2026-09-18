/**
 ******************************************************************************
 * @file           : user_gpio.h
 * @brief          : Header del user_gpio.c
 *                   En este archivo se declaran las funciones que controlarán las diferentes entradas y salidas del
 *                   sistema.
 */
#ifndef INC_USER_GPIO_H_
#define INC_USER_GPIO_H_

/** @addtogroup user_gpio Uso de los GPIOs
 * @{
 */

/*==================[inclusions]=============================================*/

#include <stdint.h>

/*==================[cplusplus]==============================================*/

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/*==================[internal functions definition]==========================*/


/**
 * @brief Inicializa los pines GPIO utilizados.
 */
void user_gpio_init(void);

///**
// * @brief Verifica el estado de los GPIO periódicamente (no bloqueante).
// */
//void user_gpio_loop(void);

/**
 * @brief Callback de interrupción por cambio de estado en un pin.
 * @param GPIO_Pin Pin que disparó la interrupción.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

/**
 * @brief Prende el LED testigo.
 */
void PrenderLedTestigo(void);

/**
 * @brief Apaga el LED testigo.
 */
void ApagarLedTestigo(void);

/**
 * @brief Alterna el estado del LED testigo.
 */
void ToggleLedTestigo(void);

/**
 * @brief Enciende el actuador de café.
 */
void CafeErogacionOn(void);

/**
 * @brief Apaga el actuador de café.
 */
void CafeErogacionOff(void);

/**
 * @brief Enciende el actuador de leche.
 */
void LecheErogacionOn(void);

/**
 * @brief Apaga el actuador de leche.
 */
void LecheErogacionOff(void);

/**
 * @brief Activa el buzzer.
 */
void BuzzerOn(void);

/**
 * @brief Desactiva el buzzer.
 */
void BuzzerOff(void);



/*==================[end of file]============================================*/

#endif /* INC_USER_GPIO_H_ */
