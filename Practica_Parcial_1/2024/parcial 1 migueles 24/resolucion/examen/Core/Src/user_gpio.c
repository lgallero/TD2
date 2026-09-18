/*
 * user_gpio.c
 *
 *  Created on: Jul 13, 2024
 *      Author: federico
 */

#include "user_gpio.h"
#include "main.h"
#include "fsm.h"

void user_gpio_init(void){
	desconectar_carga();
	indicador_off();
}

void user_gpio_loop(void){

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

		fsm_ev_pulsador();
}

void indicador_toggle(void){

	HAL_GPIO_TogglePin(INDICADOR_GPIO_Port,INDICADOR_Pin);
}

void conectar_a_linea(void){ //SE ENVIA 01 A RELE
	HAL_GPIO_WritePin(RELE_MSB_GPIO_Port,RELE_MSB_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RELE_LSB_GPIO_Port,RELE_LSB_Pin, GPIO_PIN_SET);
}

void conectar_a_bateria(void){ //SE ENVIA 10 A RELE
	HAL_GPIO_WritePin(RELE_MSB_GPIO_Port,RELE_MSB_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RELE_LSB_GPIO_Port,RELE_LSB_Pin, GPIO_PIN_RESET);
}

void desconectar_carga(void){ //SE ENVIA 00 A RELE
	HAL_GPIO_WritePin(RELE_MSB_GPIO_Port,RELE_MSB_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RELE_LSB_GPIO_Port,RELE_LSB_Pin, GPIO_PIN_RESET);
}

void indicador_off(void){
	HAL_GPIO_WritePin(INDICADOR_GPIO_Port,INDICADOR_Pin, GPIO_PIN_RESET);
}
void indicador_on(void){
	HAL_GPIO_WritePin(INDICADOR_GPIO_Port,INDICADOR_Pin, GPIO_PIN_SET);
}
