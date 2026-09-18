/*
 * user_gpio.c
 *
 *  Created on: Dec 18, 2025
 *      Author: lucas
 */


#include "user_gpio.h"
#include "main.h"
#include "fsm.h"

/* ====== Ajustá a tus pines de CubeMX ======
   - LOCK_Pin / LOCK_GPIO_Port (salida a cerradura)
   - BUZZ_Pin / BUZZ_GPIO_Port (salida buzzer)
   - DOOR_STATUS_Pin (entrada con EXTI)
*/

/* ADC */
extern ADC_HandleTypeDef hadc1;

void user_gpio_init(void)
{
    lock_access();
    buzzer_off();
}

void lock_access(void)
{
    /* ejemplo: 0 = bloqueado */
    HAL_GPIO_WritePin(LOCK_GPIO_Port, LOCK_Pin, GPIO_PIN_RESET);
}

void unlock_access(void)
{
    /* ejemplo: 1 = desbloqueado */
    HAL_GPIO_WritePin(LOCK_GPIO_Port, LOCK_Pin, GPIO_PIN_SET);
}

void buzzer_on(void)
{
    HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_SET);
}

void buzzer_off(void)
{
    HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_RESET);
}

/* Ejemplo simple: leer ADC y convertir (placeholder) */
float temp_get_celsius(void)
{
    /* Si ya tenés una función del sensor, reemplazá todo esto. */
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint16_t raw = (uint16_t)HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    /* Conversión ficticia: ajustala según tu sensor */
    /* 0..4095 -> 35..42°C */
    float t = 35.0f + ((float)raw * 7.0f) / 4095.0f;
    return t;
}

/* IRQ de puerta: usar switch como venís haciendo */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
        case DOOR_STATUS_Pin:
            fsm_ev_door_open_irq();
            break;
    }
}
