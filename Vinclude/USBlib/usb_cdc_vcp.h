/**
  ******************************************************************************
  * @file    usb_cdc_vcp.h
  *
  * @brief   гЮЦНКНБНВМШИ ТЮИК ЙКЮЯЯЮ CDC USB Device.
  *
  *          Virtual COM-port.
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
#ifndef __USB_CDC_VCP_H
#define __USB_CDC_VCP_H

/* Includes ------------------------------------------------------------------*/
#include <USBlib/usb_dev.h>

/* Exported defines ----------------------------------------------------------*/
#define SEND_ENCAPSULATED_COMMAND               0x00
#define GET_ENCAPSULATED_RESPONSE               0x01
#define SET_COMM_FEATURE                        0x02
#define GET_COMM_FEATURE                        0x03
#define CLEAR_COMM_FEATURE                      0x04
#define SET_LINE_CODING                         0x20
#define GET_LINE_CODING                         0x21
#define SET_CONTROL_LINE_STATE                  0x22
#define SEND_BREAK                              0x23
#define NO_CMD                                  0xFF

/* Exported variables --------------------------------------------------------*/
extern USBDev_ClassCB_TypeDef USBDEV_CDC_VCP_cb;

/* Exported functions prototypes ---------------------------------------------*/
uint32_t VCP_PutChar(uint8_t byte);
uint8_t  VCP_GetChar(void);
uint32_t VCP_IsRxBufEmpty(void);
uint32_t VCP_RxHandlerInit(void (*RxHandler)(void));
uint32_t VCP_TxHandlerInit(void (*TxHandler)(void));

#endif /* __USB_CDC_VCP_H */
