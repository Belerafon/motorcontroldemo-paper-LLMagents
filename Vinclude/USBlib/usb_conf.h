/**
  ******************************************************************************
  * @file    usb_conf.h
  *
  * @brief   тЮИК ЙНМТХЦСПЮЖХХ USB.
  *
  *          йНЛОХКЪРНП: GCC ARM 4.9.3
  *          яПЕДЮ: Qt Creator 3.4.2
  *
  * @author  мххщр
  *             - дЛХРПХИ яЕПХЙНБ, lonie@niiet.ru
  *             - аНЦДЮМ йНКАНБ, kolbov@niiet.ru
  * @date    21.09.2013
  *
  ******************************************************************************
  * @attention
  *
  * дюммне опнцпюллмне наеяоевемхе опеднярюбкъеряъ ╚йюй еярэ╩, аег йюйху-кхан
  * цюпюмрхи, ъбмн бшпюфеммшу хкх ондпюгслебюелшу, бйкчвюъ цюпюмрхх рнбюпмни
  * опхцндмнярх, яннрберярбхъ он ецн йнмйпермнлс мюгмювемхч х нрясрярбхъ
  * мюпсьемхи, мн ме нцпюмхвхбюъяэ хлх. дюммне опнцпюллмне наеяоевемхе
  * опедмюгмювемн дкъ нгмюйнлхрекэмшу жекеи х мюопюбкемн рнкэйн мю
  * опеднярюбкемхе днонкмхрекэмни хмтнплюжхх н опндсйре, я жекэч янупюмхрэ бпелъ
  * онрпеахрекч. мх б йюйнл яксвюе юбрнпш хкх опюбннакюдюрекх ме меяср
  * нрберярбеммнярх он йюйхл-кхан хяйюл, гю опълни хкх йнябеммши сыепа, хкх
  * он хмшл рпеанбюмхъл, бнгмхйьхл хг-гю хяонкэгнбюмхъ опнцпюллмнцн наеяоевемхъ
  * хкх хмшу деиярбхи я опнцпюллмшл наеяоевемхел.
  *
  * <h2><center>&copy; 2016 нюн "мххщр"</center></h2>
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

/* бШАНП ПЕФХЛЮ ПЮАНРШ */
//#define USE_HOST_MODE
#define USE_DEVICE_MODE
//#define USE_OTG_MODE

#define USE_USB_DEV_FS
//#define USE_USB_DEV_HS

#ifdef USE_DEVICE_MODE
    #ifndef USE_USB_DEV_FS
        #ifndef USE_USB_DEV_HS
            #error  "USE_USB_DEV_FS ХКХ USE_USB_DEV_HS ДНКФМШ АШРЭ НОПЕДЕКЕМШ"
        #endif
    #endif
#elif defined USE_HOST_MODE

#elif defined USE_OTG_MODE

#else
    #error  "USE_OTG_MODE ХКХ USE_DEVICE_MODE ХКХ USE_HOST_MODE ДНКФМШ АШРЭ НОПЕДЕКЕМШ"
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
