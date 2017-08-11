/*!
    Copyright 2017 	АО "НИИЭТ" и ООО "НПФ ВЕКТОР"

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 \file      V_usblib.c
 \brief     Набор функций для инициализации драйвера USB. Основан на примере
 			USB VCP" от НИИЭТ:
 			https://bitbucket.org/niietcm4/
 			---> / chip / K1921VK01T / GCC / USB / CDC_VCP /

 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 08/08/2017

 \addtogroup V_usblib
@{
*/

/* Includes ------------------------------------------------------------------*/
#include "DSP.h"
#include "USBlib/usb_cdc_vcp.h"
#include "USBlib/usb_otg.h"

/* Defines -------------------------------------------------------------------*/
#define EXT_CLK_VALUE 16000000
//#define LDM_SYSTEMS

/* Functions prototypes ------------------------------------------------------*/
void usb_irq_en(void);
void VCP_RxHandler(void);
void VCP_TxHandler(void);

/* Variables -----------------------------------------------------------------*/
volatile uint32_t StartMsgSend = 0;

/* Functions -----------------------------------------------------------------*/

void USB_enable_peripheral (void){
    //init usb block
    NT_COMMON_REG->USB_CTRL = 0x00000006; //clk_en, 12MHz, xtal
    NT_COMMON_REG->PER_RST1 |= (1<<3); //clear reset from usb phy
}

int USB_init_hardware(void)
{
    //NT_COMMON_REG->SYS_CLK = 0x4; //PLL DIV clk set
    USBDev_Init();
    USBDev_ClassCbInit(&USBDEV_CDC_VCP_cb);
    USBOTG_SetRole(USB_Role_Device);

    VCP_RxHandlerInit(&VCP_RxHandler);
    VCP_TxHandlerInit(&VCP_TxHandler);
    NVIC_SetPriority(TIM0_IRQn, IRQ_PRIORITY_CAN);
    usb_irq_en();

    return 0;
}

void usb_irq_en(void)
{
    NVIC_EnableIRQ(USBOTG_IRQn);
}

void usb_irq_dis(void)
{
    NVIC_DisableIRQ(USBOTG_IRQn);
}

void InitPLL (unsigned int PLL_OD, unsigned int PLL_NR, unsigned int PLL_NF)
{
    NT_COMMON_REG->PLL_OD = PLL_OD;
    NT_COMMON_REG->PLL_NR = PLL_NR;
    NT_COMMON_REG->PLL_NF = PLL_NF;
}


/* Функция вызывается по опустошению передающего буфера */
void VCP_TxHandler(void)
{

}

/* Функция вызывается при появлении данных в приемном буфере */
void VCP_RxHandler(void)
{

}


void USBOTG_IRQHandler(void)
{
    USBOTG_IntHandler();
}


/*@}*/
