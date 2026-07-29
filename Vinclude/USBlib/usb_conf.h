/**
  ******************************************************************************
  * @file    usb_conf.h
  *
  * @brief   Файл конфигурации USB.
  *
  *          Компилятор: GCC ARM 4.9.3
  *          Среда: Qt Creator 3.4.2
  *
  * @author  НИИЭТ
  *             - Дмитрий Сериков, lonie@niiet.ru
  *             - Богдан Колбов, kolbov@niiet.ru
  * @date    21.09.2013
  *
  ******************************************************************************
  * @attention
  *
  * ДАННОЕ ПРОГРАММНОЕ ОБЕСПЕЧЕНИЕ ПРЕДОСТАВЛЯЕТСЯ «КАК ЕСТЬ», БЕЗ КАКИХ-ЛИБО
  * ГАРАНТИЙ, ЯВНО ВЫРАЖЕННЫХ ИЛИ ПОДРАЗУМЕВАЕМЫХ, ВКЛЮЧАЯ ГАРАНТИИ ТОВАРНОЙ
  * ПРИГОДНОСТИ, СООТВЕТСТВИЯ ПО ЕГО КОНКРЕТНОМУ НАЗНАЧЕНИЮ И ОТСУТСТВИЯ
  * НАРУШЕНИЙ, НО НЕ ОГРАНИЧИВАЯСЬ ИМИ. ДАННОЕ ПРОГРАММНОЕ ОБЕСПЕЧЕНИЕ
  * ПРЕДНАЗНАЧЕНО ДЛЯ ОЗНАКОМИТЕЛЬНЫХ ЦЕЛЕЙ И НАПРАВЛЕНО ТОЛЬКО НА
  * ПРЕДОСТАВЛЕНИЕ ДОПОЛНИТЕЛЬНОЙ ИНФОРМАЦИИ О ПРОДУКТЕ, С ЦЕЛЬЮ СОХРАНИТЬ ВРЕМЯ
  * ПОТРЕБИТЕЛЮ. НИ В КАКОМ СЛУЧАЕ АВТОРЫ ИЛИ ПРАВООБЛАДАТЕЛИ НЕ НЕСУТ
  * ОТВЕТСТВЕННОСТИ ПО КАКИМ-ЛИБО ИСКАМ, ЗА ПРЯМОЙ ИЛИ КОСВЕННЫЙ УЩЕРБ, ИЛИ
  * ПО ИНЫМ ТРЕБОВАНИЯМ, ВОЗНИКШИМ ИЗ-ЗА ИСПОЛЬЗОВАНИЯ ПРОГРАММНОГО ОБЕСПЕЧЕНИЯ
  * ИЛИ ИНЫХ ДЕЙСТВИЙ С ПРОГРАММНЫМ ОБЕСПЕЧЕНИЕМ.
  *
  * <h2><center>&copy; 2016 ОАО "НИИЭТ"</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_CONF_H
#define __USB_CONF_H

/* Includes ------------------------------------------------------------------*/
#include <USBlib/usb_const.h>
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"
#include "K1921BK01T.h"

/* Exported defines ----------------------------------------------------------*/

/* Выбор режима работы */
//#define USE_HOST_MODE
#define USE_DEVICE_MODE
//#define USE_OTG_MODE

#define USE_USB_DEV_FS
//#define USE_USB_DEV_HS

#ifdef USE_DEVICE_MODE
    #ifndef USE_USB_DEV_FS
        #ifndef USE_USB_DEV_HS
            #error  "USE_USB_DEV_FS или USE_USB_DEV_HS должны быть определены"
        #endif
    #endif
#elif defined USE_HOST_MODE

#elif defined USE_OTG_MODE

#else
    #error  "USE_OTG_MODE или USE_DEVICE_MODE или USE_HOST_MODE должны быть определены"
#endif

#if defined (__GNUC__)          /* GNU Compiler */
    #define __ALIGN_END    __attribute__ ((aligned (4)))
    #define __ALIGN_BEGIN
#else
    #define __ALIGN_END
    #if defined   (__CC_ARM)    /* ARM Compiler */
        #define __ALIGN_BEGIN    __align(4)
    #elif defined (__ICCARM__)  /* IAR Compiler */
        #define __ALIGN_BEGIN   #pragma data_alignment=4
    #endif /* __CC_ARM */
#endif /* __GNUC__ */

#define LOBYTE(x)   ((unsigned char)(x & 0x00FF))
#define HIBYTE(x)   ((unsigned char)((x & 0xFF00) >>8))
#define MIN(a, b)   (((a) < (b)) ? (a) : (b))

#endif /* __USB_CONF_H */
