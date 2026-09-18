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

void user_gpio_init(void)
{
	//Como tiene que iniciar los perifericos con los nombres de las acciones
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin)
	{
	case [Nombre de Pin]_Pin:
		
		break;

	case [Nombre de Pin]_Pin:
		
		break;

	case [Nombre de Pin]_Pin:
		
		break;
	}
}

void [Nombre de Acciones](void)
{
	HAL_GPIO_TogglePin();
    HAL_GPIO_WritePin();
}

void [Nombre de Acciones](void)
{
	HAL_GPIO_TogglePin();
    HAL_GPIO_WritePin();
}


/*==================[external functions definition]==========================*/

/*==================[end of file]============================================*/
