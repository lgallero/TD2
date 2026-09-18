/*==================[inclusions]=============================================*/

#include "user_gpio.h"
#include "main.h"
#include "cafetera.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

void user_gpio_init(void)
{
	CafeErogacionOff();
	LecheErogacionOff();
	BuzzerOff();
	ApagarLedTestigo();
}

/*
void user_gpio_loop(void)
{
	if (HAL_GPIO_ReadPin(INGRESO_FICHA_GPIO_Port, INGRESO_FICHA_Pin) == GPIO_PIN_SET)
	{
		cafetera_raise_evFicha_On();
	}

	if (HAL_GPIO_ReadPin(CAFE_NEGRO_GPIO_Port, CAFE_NEGRO_Pin) == GPIO_PIN_SET)
	{
		cafetera_raise_evCafeNegro_On();
	}

	if (HAL_GPIO_ReadPin(CAFE_LECHE_GPIO_Port, CAFE_LECHE_Pin) == GPIO_PIN_SET)
	{
		cafetera_raise_evCafeLeche_On();
	}

}
*/

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin)
	{
	case INGRESO_FICHA_Pin:
		cafetera_raise_evFicha_On();
		break;

	case CAFE_NEGRO_Pin:
		cafetera_raise_evCafeNegro_On();
		break;

	case CAFE_LECHE_Pin:
		cafetera_raise_evCafeLeche_On();
		break;
	}
}

void ToggleLedTestigo(void)
{
	HAL_GPIO_TogglePin(LED_TESTIGO_GPIO_Port, LED_TESTIGO_Pin);
}

void ApagarLedTestigo(void)
{
	HAL_GPIO_WritePin(LED_TESTIGO_GPIO_Port, LED_TESTIGO_Pin, GPIO_PIN_RESET);
}

void PrenderLedTestigo(void)
{
	HAL_GPIO_WritePin(LED_TESTIGO_GPIO_Port, LED_TESTIGO_Pin, GPIO_PIN_SET);
}

void CafeErogacionOn(void)
{
	HAL_GPIO_WritePin(EROGACION_CAFE_GPIO_Port, EROGACION_CAFE_Pin, GPIO_PIN_SET);
}

void CafeErogacionOff(void)
{
	HAL_GPIO_WritePin(EROGACION_CAFE_GPIO_Port, EROGACION_CAFE_Pin, GPIO_PIN_RESET);
}

void LecheErogacionOn(void)
{
	HAL_GPIO_WritePin(EROGACION_LECHE_GPIO_Port, EROGACION_LECHE_Pin, GPIO_PIN_SET);
}

void LecheErogacionOff(void)
{
	HAL_GPIO_WritePin(EROGACION_LECHE_GPIO_Port, EROGACION_LECHE_Pin, GPIO_PIN_RESET);
}

void BuzzerOn(void)
{
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
}

void BuzzerOff(void)
{
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
}

/*==================[external functions definition]==========================*/

/*==================[end of file]============================================*/
