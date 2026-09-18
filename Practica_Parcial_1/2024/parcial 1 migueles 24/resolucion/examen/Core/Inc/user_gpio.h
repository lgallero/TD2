/*
 * user_gpio.h
 *
 *  Created on: Jul 13, 2024
 *      Author: federico
 */

#ifndef INC_USER_GPIO_H_
#define INC_USER_GPIO_H_

void user_gpio_init(void);

void user_gpio_loop(void);

void conectar_a_linea(void);
void conectar_a_bateria(void);
void desconectar_carga(void);
void indicador_off(void);
void indicador_on(void);
void indicador_toggle(void);

#endif /* INC_USER_GPIO_H_ */
