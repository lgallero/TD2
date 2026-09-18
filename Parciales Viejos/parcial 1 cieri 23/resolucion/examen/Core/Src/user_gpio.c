/*==================[inclusions]=============================================*/

#include "user_gpio.h"
#include "main.h"
#include "fsm.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

void user_gpio_init(void) {

}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin){
	case EOC_NEGADO_Pin:
		fsm_ev_transicion_EOC();
		break;

	}
}


void fsm_set_convst_1(void){
	HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, SET);

}
void fsm_set_convst_0(void){
	HAL_GPIO_WritePin(CONVST_GPIO_Port, CONVST_Pin, RESET);

}
void fsm_set_CS1_1(void){
	HAL_GPIO_WritePin(CS1_GPIO_Port, CS1_Pin, SET);
}
void fsm_set_CS1_0(void){
	HAL_GPIO_WritePin(CS1_GPIO_Port, CS1_Pin, RESET);
}
void fsm_set_CS2_1(void){
	HAL_GPIO_WritePin(CS2_GPIO_Port, CS2_Pin, SET);
}
void fsm_set_CS2_0(void){
	HAL_GPIO_WritePin(CS2_GPIO_Port, CS2_Pin, RESET);
}
void fsm_set_CS3_1(void){
	HAL_GPIO_WritePin(CS3_GPIO_Port, CS3_Pin, SET);
}
void fsm_set_CS3_0(void){
	HAL_GPIO_WritePin(CS3_GPIO_Port, CS3_Pin, RESET);
}
void fsm_set_RD_1(void){
	HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, SET);
}
void fsm_set_RD_0(void){
	HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, RESET);
}

/*==================[end of file]============================================*/
