/*
 * user_gpio.h
 *
 *  Created on: Dec 17, 2025
 *      Author: lucas
 */

#ifndef INC_USER_GPIO_H_
#define INC_USER_GPIO_H_

/*==================[inclusions]=============================================*/

#include <stdint.h>

/*==================[cplusplus]==============================================*/

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/*==================[internal functions definition]==========================*/

void user_gpio_init(void);


void set_ctrl(uint8_t level);

float get_temperature(void);


/*==================[end of file]============================================*/


#endif /* INC_USER_GPIO_H_ */
