/**
  ******************************************************************************
  * @file    usb_otg.h
  *
  * @brief   гЮЦНКНБНВМШИ ТЮИК USB OTG.
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
#ifndef __USB_OTG_H
#define __USB_OTG_H

/* Includes ------------------------------------------------------------------*/
#include <USBlib/usb_conf.h>
#include <USBlib/usb_dev.h>
#include <USBlib/usb_otg_irq.h>
//#include "usb_host.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
    USB_Role_Unknown = 0,
    USB_Role_Device,
    USB_Role_Host
}USB_Role_TypeDef;

/* Exported functions prototypes ---------------------------------------------*/
void USBOTG_Init(USBDev_ClassCB_TypeDef *cb);
void USBOTG_DeInit(void);

bool otg_is_host(void);
bool otg_is_device(void);
void otg_idle(void);
void set_port_change_status (bool status);
void USBOTG_SetRole(USB_Role_TypeDef Role);
USB_Role_TypeDef USBOTG_GetRole(void);

#endif //__USB_OTG_H
