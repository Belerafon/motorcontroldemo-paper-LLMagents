/**
  ******************************************************************************
  * @file    usb_hid_gamepad.c
  *
  * @brief   тЮИК Я ХЯУНДМШЛ ЙНДНЛ ДКЪ ПЮАНРШ Я HID USB Device.
  *
  *          цЕИЛОЮД, ХЛЕЧЫХИ 5 ЙМНОНЙ.
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

/* Includes ------------------------------------------------------------------*/
#include "USBlib/usb_hid_gamepad.h"

/* Private defines -----------------------------------------------------------*/
#define EP_DATA_IN 0x81

/* Private functions prototypes ----------------------------------------------*/
uint32_t USBDev_HID_Init(uint8_t config_num);
uint32_t USBDev_HID_DeInit(uint8_t config_num);
uint32_t USBDev_HID_DataOut(USBDev_EP_TypeDef *ep);
uint32_t USBDev_HID_DataIn(USBDev_EP_TypeDef *ep);
uint32_t USBDev_HID_Setup(USBDev_Setup_TypeDef *pUSB_SETUP_PACKET);
uint32_t USBDev_HID_SOF(void);
uint32_t USBDev_HID_EP0_RxReady(void);
uint32_t USBDev_HID_EP0_TxSent(void);
uint8_t *USBDev_HID_GetConfigDscr(USBDev_Speed_TypeDef dev_speed, uint32_t *len);

/* Private variables ---------------------------------------------------------*/
__ALIGN_BEGIN uint8_t HID_IDLE_State __ALIGN_END = 0;

__ALIGN_BEGIN uint8_t HID_ReportDesc[] __ALIGN_END =
{
    //gamepad 5 buttons
    0x05, 0x01, //USAGE_PAGE (generic desktop)
    0x09, 0x05, //USAGE (gamepad)
    0xA1, 0x01, //COLLECTION (application)
    0xA1, 0x00, // COLLECTION (physical)
    0x05, 0x09, //  USAGE_PAGE (Button)
    0x19, 0x01, //  USAGE_MINIMUM (Button1)
    0x29, 0x05, //  USAGE_MAXIMUM (Button5)
    0x15, 0x00, //  LOGICAL_MINIMUM (0)
    0x25, 0x01, //  LOGICAL_MAXIMUM (1)
    0x95, 0x05, //  REPORT_COUNT(5)
    0x75, 0x01, //  REPORT_SIZE (1)
    0x81, 0x02, //  INPUT(data,var,abs)
    0x95, 0x01, //  REPORT_COUNT(1)
    0x75, 0x03, //  REPORT_SIZE (3)
    0x81, 0x03, //  INPUT(const,var,abs)
    0xC0,       // END_COLLECTION
    0xC0        //END_COLLECTION

};

__ALIGN_BEGIN static uint8_t cfg_dscr [34] __ALIGN_END =
{
    sizeof(CONFIGURATION_DSCR_TYPE),
    USB_DESC_TYPE_CONFIGURATION,
    LOBYTE(sizeof(cfg_dscr)),
    HIBYTE(sizeof(cfg_dscr)),
    0x01, //bNumInterfaces
    0x01, //bConfigurationValue,
    0x00, //iConfiguration
    0xc0, // ???
    0x32, //max 100 mA

    sizeof(INTERFACE_DSCR_TYPE),
    USB_DESC_TYPE_INTERFACE,
    0x00, //bInterfaceNumber
    0x00, //bAlternateSetting
    0x01, //bNumEndpoints
    0x03, //bInterfaceClass (HID)
    0x00, //bInterfaceSubClass (0 = no boot)
    0x00, //bInterfaceProtocol (0 = none, 1 = kbd, 2 = mouse)
    0x00, //iInterface
    //header func descr
    0x09, // HID desc size
    HID_DESCRIPTOR_TYPE, // HID desc type
    LOBYTE(HID_CLASS_SPEC_RELEASE_NUMBER),
    HIBYTE(HID_CLASS_SPEC_RELEASE_NUMBER),
    0x00, //Target country
    0x01, //bNum HID descrs
    0x22, //bDescrType
    LOBYTE(sizeof(HID_ReportDesc)),
    HIBYTE(sizeof(HID_ReportDesc)),
    // mouse EP descr
    0x07,
    USB_DESC_TYPE_ENDPOINT,
    EP_DATA_IN, // EP address (IN)
    0x03,       // interrupt EP
    LOBYTE(HID_IN_PACKET),
    HIBYTE(HID_IN_PACKET),
    0x02 //10 ms interval polling
};

USBDev_ClassCB_TypeDef USBDEV_HID_cb =
{
    USBDev_HID_Init,
    USBDev_HID_DeInit,
    USBDev_HID_DataOut,
    USBDev_HID_DataIn,
    USBDev_HID_Setup,
    USBDev_HID_SOF,
    USBDev_HID_EP0_RxReady,
    USBDev_HID_EP0_TxSent,
    USBDev_HID_GetConfigDscr
};

/* Private functions ---------------------------------------------------------*/
uint32_t USBDev_HID_Init(uint8_t config_num)
{
    if (config_num == 1)
    {
        USBDev_EP_Open(EP_DATA_IN,
                       HID_IN_PACKET,
                       0x03); // int
    }
    return 0;
}

uint32_t USBDev_HID_DeInit(uint8_t config_num)
{
    if (config_num == 1)
    {
        USBDev_EP_Close(EP_DATA_IN);
    }
    return 0;
}

char zero_pack_need = 0;

uint32_t USBDev_HID_DataOut(USBDev_EP_TypeDef *ep)
{

    return 0;
}

uint32_t USBDev_HID_DataIn(USBDev_EP_TypeDef *ep)
{

    return 0;
}

uint32_t USBDev_HID_Setup(USBDev_Setup_TypeDef *pUSB_SETUP_PACKET)
{
    uint32_t len = 0;
    uint8_t *pbuf;

    switch(pUSB_SETUP_PACKET->bmRequestType & 0x60)
    {
    //НАПЮАНРЙЮ ЯОЕЖХТХВЕЯЙХУ ДКЪ ЙКЮЯЮ ГЮОПНЯНБ
    case 0x00: //STANDARD_REQUEST
        switch(pUSB_SETUP_PACKET->bRequest)
        {
        case USB_REQ_GET_DESCRIPTOR :
            if((pUSB_SETUP_PACKET->wValue >> 8) == HID_REPORT_DESCRIPTOR)
            {
                len = MIN(sizeof(HID_ReportDesc), pUSB_SETUP_PACKET->wLength);
                pbuf = HID_ReportDesc;
            }
            else if((pUSB_SETUP_PACKET->wValue >> 8) == HID_DESCRIPTOR_TYPE)
            {
                len = MIN( 9, pUSB_SETUP_PACKET->wLength);
                pbuf = cfg_dscr + 18;
            }

            USBDev_EPSendData(pbuf, len, 0);

            break;
        default:
            USBDev_CEPSendResponse(CEP_STALL);
            break;
        }
        break;
    case 0x20: //CLASS_SPECIFIC_REQ
        switch(pUSB_SETUP_PACKET->bRequest)
        {
        case HID_REQ_SET_IDLE:
            HID_IDLE_State = (uint8_t)(pUSB_SETUP_PACKET->wValue);
            USBDev_CEPSendResponse(CEP_ZEROLEN);
            break;
        case HIR_REQ_GET_REPORT:
            USBDev_CEPSendResponse(CEP_STALL);
            break;
        default:
            USBDev_CEPSendResponse(CEP_STALL);
            break;
        }
        break;
    default:
        USBDev_CEPSendResponse(CEP_STALL);
        break;

    }

    if(pUSB_SETUP_PACKET->wLength == 0)
    {
        if(pUSB_SETUP_PACKET->bmRequestType & 0x80) //device to host
        {

        }
        else //host to device
        {
            USBDev_CEPSendResponse(CEP_ZEROLEN);
        }
    }
    
    return 0;
}

uint32_t USBDev_HID_SOF(void)
{
    
    return 0;
}

uint32_t USBDev_HID_EP0_RxReady(void)
{
    return 0;
}

uint32_t USBDev_HID_EP0_TxSent(void)
{
    return 0;
}

uint8_t *USBDev_HID_GetConfigDscr(USBDev_Speed_TypeDef dev_speed, uint32_t *len)
{
    uint8_t *pcfgdscr;
    if(dev_speed == USBDev_Speed_Full)
    {
        *len = sizeof(cfg_dscr);
        pcfgdscr = &cfg_dscr[0];
    }
    else //HIGH
    {
        *len = sizeof(cfg_dscr);
        pcfgdscr = &cfg_dscr[0];
    }
    return pcfgdscr;
}

__ALIGN_BEGIN hid_report_t hid_report __ALIGN_END;

uint32_t HID_SendReport(void)
{
    uint8_t buttons = 0;

    if((NT_GPIOB->DATA & (1<<11)) == 0) buttons |= 1<<0;
    if((NT_GPIOB->DATA & (1<<12)) == 0) buttons |= 1<<1;
    if((NT_GPIOB->DATA & (1<<13)) == 0) buttons |= 1<<2;
    if((NT_GPIOC->DATA & (1<<7))  == 0) buttons |= 1<<3;
    if((NT_GPIOC->DATA & (1<<12)) == 0) buttons |= 1<<4;

    hid_report.buttons1 = buttons;

    if(USBDev_ReadState() == USBDev_Configured)
    {
        if(NT_USBDevice->USB_EP[0].IRQ_STAT & 0x02)
        {
            /*USBDev_DMAOpStart((uint8_t*)&hid_report,
                         1,
                         (EP_DATA_IN & 0x7F),
                         USBDMA_OP_WRITE_TO_EP | USBDMA_OP_START);
      NT_USBDevice->USB_EP[0].RSP_SC = (1<<6); //send short packet*/

            USBDev_WritePacket((uint8_t*)&hid_report,
                               1,
                               EP_DATA_IN & 0x7F);


        }

        NT_GPIOE->DATAOUT ^= 0x000C;

    }

    return 0;
}
