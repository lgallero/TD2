# Ejemplo de uso de librería fatfs

## Introducción
La librería [`fatfs`](http://elm-chan.org/fsw/ff/00index_e.html) nos provee una interfaz estándar (que cumple con ANSI C) para manejo de archivos en sistemas de archivos FAT. Implementa solamente la capa más alta de acceso a archivos y directorios, lo que le permite ser multiplataforma. Por lo tanto, es necesario implementar la capa de acceso al medio de almacenamiento físico. En este caso, se implementa para el acceso a una memoria SD, utilizando el protocolo de comunicación SPI.

## Implementación de acceso a SD

Para utilizar `fatfs`, se deben implementar las siguientes funciones, cuyo esqueleto se autogenera en el archivo [`user_diskio.c`](FATFS/Target/user_diskio.c).

```c
DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
DRESULT USER_write (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void* buff);
```

Observando el [datasheet](https://www.kingston.com/datasheets/SDCIT-specsheet-8gb-32gb_en.pdf) de una memoria SD, se puede implementar el flujo de comandos que nos permite inicializar, leer y escribir en la memoria.

El código está basado en el de [este blog](https://m.blog.naver.com/PostView.nhn?blogId=eziya76&logNo=221188701172&proxyReferer=https:%2F%2Fgithub.com%2Feziya%2FSTM32_SPI_SDCARD), con algunas modificaciones menores.

## Como utilizar la librería

1. Se debe activar la librería `fatfs` desde la herramienta de configuración del IDE. También se debe activar uno de los SPI disponibles en el micro y un GPIO extra que se utilizará como CS __(definir la macro como SD_CS)__. En el código del ejemplo se utiliza el SPI2 junto con el PB12.

2. Se debe copiar los archivos [`fatfs_sd.h`](Core/Inc/fatfs_sd.h) y [`fatfs_sd.c`](Core/Src/fatfs_sd.c) dentro del proyecto, ya que son los que implementan el código a ejecutar en las funciones antes mencionadas.

3. Agregar las llamadas a las funciones implementadas en [`fatfs_sd.c`](Core/Src/fatfs_sd.c) en el archivo [`user_diskio.c`](FATFS/Target/user_diskio.c) (incluir los headers necesarios).

4. La implementación está hecha para el __SPI 2__ del stm32f103c8. Si se desea utilizar el 1, cambiar la variable definida de forma externa en [`fatfs_sd.c`](Core/Src/fatfs_sd.c) por la correspondiente (y sus llamadas en el código).

5. La función `SD_10ms_Tick_Handler()` se debe llamar cada 10 ms. En este ejemplo se agregó la lógica necesaria en la re-definición de `HAL_IncTick()`.

6. Para información sobre todas las funciones disponibles, dirigirse a la [documentación de FatFs](http://elm-chan.org/fsw/ff/00index_e.html).

## Notas
- Se debe tener en cuenta que las funciones implementadas en `fatfs_sd.c` son bloqueantes. Utilizando el clock del sistema en 8 Mhz, las operaciones de apertura, lectura y escritura de un archivo, consumen aproximadamente 40 mS. 
- Se puede copiar y pegar el archivo `user_diskio.c`, pero como es un archivo autogenerado se debe tener cuidado de que no se borre ninguna parte al volver a generar código.