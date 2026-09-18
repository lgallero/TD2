/*
 * rtc_driver.h
 */

#ifndef INC_RTC_DRIVER_H_
#define INC_RTC_DRIVER_H_

#include "main.h"
#include <time.h>

typedef struct {
    uint8_t Hours;
    uint8_t Minutes;
    uint8_t Seconds;
    uint8_t Day;    // 1 a 31
    uint8_t Month;  // 1 a 12
    uint8_t Year;   // 0 a 99 (Representa 2000 a 2099)
} RTC_DateTime_t;

// --- Funciones Públicas ---

// Inicializa el driver
uint8_t RTC_Driver_Init(RTC_HandleTypeDef *hrtc);

// Guarda fecha y FIRMA el backup
void RTC_Driver_SetDateTime(RTC_DateTime_t dt);

// Obtiene fecha
RTC_DateTime_t RTC_Driver_GetDateTime(void);

// Verifica si la hora es válida (1=OK, 0=Fallo)
uint8_t RTC_Driver_CheckStatus(void);

#endif /* INC_RTC_DRIVER_H_ */
