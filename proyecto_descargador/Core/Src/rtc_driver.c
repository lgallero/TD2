/*
 * rtc_driver.c
 * Implementación de manejo de RTC (Real Time Clock) para STM32F1
 */

#include "rtc_driver.h"

// Puntero privado al RTC
static RTC_HandleTypeDef *driver_hrtc = NULL;

// Número mágico para verificar si el Backup Register tiene datos válidos
#define BACKUP_MAGIC_NUMBER 0x32F2

// ==============================================================================
// FUNCIONES PRIVADAS (Acceso a Registros)
// ==============================================================================

static void RTC_WriteCounter(uint32_t counterValue)
{
    // 1. Habilitar acceso al área de Backup (necesario para escribir registros del RTC)
    HAL_PWR_EnableBkUpAccess();

    // 2. Esperar a que el RTC esté sincronizado y entrar en modo configuración
    HAL_RTC_WaitForSynchro(driver_hrtc);
    SET_BIT(driver_hrtc->Instance->CRL, RTC_CRL_CNF);

    // 3. Escribir los 32 bits (Alto y Bajo) en los registros del contador
    WRITE_REG(driver_hrtc->Instance->CNTH, (counterValue >> 16U));
    WRITE_REG(driver_hrtc->Instance->CNTL, (counterValue & 0xFFFFU));

    // 4. Salir de modo configuración
    CLEAR_BIT(driver_hrtc->Instance->CRL, RTC_CRL_CNF);
    HAL_RTC_WaitForSynchro(driver_hrtc);
}

static uint32_t RTC_ReadCounter(void)
{
    // Leer registros alto y bajo del contador
    uint32_t high = READ_REG(driver_hrtc->Instance->CNTH & 0xFFFFU);
    uint32_t low  = READ_REG(driver_hrtc->Instance->CNTL & 0xFFFFU);

    // Combinar en un uint32
    return (high << 16U) | low;
}

// ==============================================================================
// FUNCIONES PÚBLICAS (API del Driver)
// ==============================================================================

uint8_t RTC_Driver_Init(RTC_HandleTypeDef *hrtc)
{
    driver_hrtc = hrtc;

    // Habilitar reloj de Backup y acceso a registros
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_BKP_CLK_ENABLE();

    // Esperar sincronización básica
    HAL_RTC_WaitForSynchro(driver_hrtc);

    // Verificamos el estado usando nuestra función de chequeo
    return RTC_Driver_CheckStatus();
}

uint8_t RTC_Driver_CheckStatus(void)
{
    // Leemos el Registro de Respaldo 1 (BKP_DR1)
    // Si contiene nuestro "Número Mágico", la hora es válida.
    if (HAL_RTCEx_BKUPRead(driver_hrtc, RTC_BKP_DR1) == BACKUP_MAGIC_NUMBER) {
        return 1; // Hora Válida
    }
    return 0; // Batería agotada o primer inicio
}

void RTC_Driver_SetDateTime(RTC_DateTime_t dt)
{
    struct tm timeStruct = {0};

    // Llenar estructura tm (time.h)
    timeStruct.tm_sec  = dt.Seconds;
    timeStruct.tm_min  = dt.Minutes;
    timeStruct.tm_hour = dt.Hours;
    timeStruct.tm_mday = dt.Day;
    timeStruct.tm_mon  = dt.Month - 1;      // 0-11
    timeStruct.tm_year = dt.Year + 100;     // Años desde 1900
    timeStruct.tm_isdst = -1;

    // Convertir a Unix Timestamp (Segundos desde 1970)
    time_t unixTimestamp = mktime(&timeStruct);

    // 1. Guardar en HW (Contador)
    RTC_WriteCounter((uint32_t)unixTimestamp);

    // 2. FIRMAR el registro de respaldo
    HAL_RTCEx_BKUPWrite(driver_hrtc, RTC_BKP_DR1, BACKUP_MAGIC_NUMBER);
}

RTC_DateTime_t RTC_Driver_GetDateTime(void)
{
    RTC_DateTime_t dt = {0};

    // 1. Leer hardware
    time_t unixTimestamp = (time_t)RTC_ReadCounter();

    // 2. Convertir a fecha humana
    struct tm *timeStruct = localtime(&unixTimestamp);

    dt.Seconds = timeStruct->tm_sec;
    dt.Minutes = timeStruct->tm_min;
    dt.Hours   = timeStruct->tm_hour;
    dt.Day     = timeStruct->tm_mday;
    dt.Month   = timeStruct->tm_mon + 1;    // 1-12
    dt.Year    = timeStruct->tm_year - 100; // 20xx

    return dt;
}
