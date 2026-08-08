/**
  ******************************************************************************
  * @file    usb_dev_descr.h
  *
  * @brief   Файл описания дескриптора USB Device.
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
#ifndef __USB_DEV_DESCR_H
#define __USB_DEV_DESCR_H

/* Exported defines ----------------------------------------------------------*/

/* langID */
#define USB_DEVICE_LANGID_STR       0x0409

#define USB_DEVICE_MFC_STR          "NIIET K1921VK01T"
#define USB_DEVICE_PRODUCT_STR      "USB device"
#define USB_DEVICE_SERIAL_STR       "000000001S"
#define USB_DEVICE_CONFIG_STR       "Device"
#define USB_DEVICE_INTERFACE_STR    "USB"

/* USB DEVICE описание дескриптора */
#define USB_DEVICE_DESCRIPTOR_TYPE  0x01
#define USB_BCD                     0x0200
#define USB_DEVICE_CLASS            0xFF
#define USB_DEVICE_SUBCLASS         0x00
#define USB_DEVICE_PROTOCOL         0xFF
#define USB_DEVICE_MAX_EP0_SIZE     64
#define USB_DEVICE_VENDOR_ID        0x1234
#define USB_DEVICE_PRODUCT_ID       0x1234
#define USB_BCD_DEVICE              0x0001
#define USB_DEVICE_IDX_MFC_STR      0x01
#define USB_DEVICE_IDX_PRODUCT_STR  0x02
#define USB_DEVICE_IDX_SERIAL_STR   0x03

#define USB_DEVICE_CFG_MAX_NUM              1
#define USB_DEVICE_CFG_MAX_NUM_QUALIFIER    1
#define USB_DEVICE_ITFACE_MAX_NUM           1

#endif /* __USB_DEV_DESCR_H */
