/**
 * @file rtc_driver.h
 * @brief Driver de RTC para STM32F1 con respaldo en Backup Register.
 */

#ifndef INC_RTC_DRIVER_H_
#define INC_RTC_DRIVER_H_

#include "main.h"
#include <time.h>

/**
 * @brief Representa fecha y hora con rango 2000-2099.
 */
typedef struct {
    uint8_t Hours;    ///< Horas 0-23.
    uint8_t Minutes;  ///< Minutos 0-59.
    uint8_t Seconds;  ///< Segundos 0-59.
    uint8_t Day;      ///< Día 1-31.
    uint8_t Month;    ///< Mes 1-12.
    uint8_t Year;     ///< Años desde 2000 (0-99).
} RTC_DateTime_t;

// --- Funciones Públicas ---

/**
 * @brief Inicializa el driver y sincroniza el acceso al RTC.
 * @param hrtc Handle del periférico RTC.
 * @return 1 si la hora almacenada es válida, 0 si debe reconfigurarse.
 */
uint8_t RTC_Driver_Init(RTC_HandleTypeDef *hrtc);

/**
 * @brief Guarda fecha/hora y firma el backup register.
 * @param dt Fecha y hora a almacenar.
 */
void RTC_Driver_SetDateTime(RTC_DateTime_t dt);

/**
 * @brief Obtiene la fecha/hora actual desde hardware.
 * @return Estructura con la fecha/hora leída.
 */
RTC_DateTime_t RTC_Driver_GetDateTime(void);

/**
 * @brief Verifica si la hora guardada es válida.
 * @return 1 si la hora es válida, 0 si falló la batería o es primer inicio.
 */
uint8_t RTC_Driver_CheckStatus(void);

#endif /* INC_RTC_DRIVER_H_ */
