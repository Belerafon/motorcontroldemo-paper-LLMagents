/*!
    Copyright 2018 	АО "НИИЭТ" и ООО "НПФ ВЕКТОР"

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 \file      startup.h
 \brief     Заголовочный файл с векторами прерываний
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 04/09/2018

 */


#ifndef ASM_STARTUP_H_
#define ASM_STARTUP_H_

#include "K1921BK01T.h"
#include "core_cm4.h"

typedef unsigned char Uint8;
typedef unsigned short int Uint16;
typedef unsigned int Uint32;

// Символы из линкера
extern int _estack;
int main (void);

// Дефолтный обработчик
void defaultISRhandler (void) {

};

// Функции прерываний, объявленные как "weak"
//void _estack(void) __attribute__ ((weak, alias("defaultISRhandler")));
void Reset_Handler(void) __attribute__ ((naked));
void NMI_Handler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void HardFault_Handler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void MemManage_Handler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void BusFault_Handler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void UsageFault_Handler(void) __attribute__ ((weak, alias("defaultISRhandler")));
//void 0(void) __attribute__ ((weak, alias("defaultISRhandler")));
//void 0(void) __attribute__ ((weak, alias("defaultISRhandler")));
//void 0(void) __attribute__ ((weak, alias("defaultISRhandler")));
//void 0(void) __attribute__ ((weak, alias("defaultISRhandler")));
void SVC_Handler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DebugMon_Handler(void) __attribute__ ((weak, alias("defaultISRhandler")));
//void 0(void) __attribute__ ((weak, alias("defaultISRhandler")));
void PendSV_Handler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void SysTick_Handler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void WWDG_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void I2C0_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void I2C1_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void TIM0_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void TIM1_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void TIM2_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream0_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream1_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream2_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream3_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream4_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream5_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream6_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream7_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream8_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream9_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream10_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream11_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream12_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream13_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream14_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream15_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream16_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream17_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream18_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream19_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream20_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream21_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream22_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void DMA1_Stream23_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART0_MX_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART0_RX_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART0_TX_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART0_RT_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART0_E_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART0_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART1_MX_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART1_RX_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART1_TX_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART1_RT_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART1_E_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART1_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART2_MX_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART2_RX_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART2_TX_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART2_RT_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART2_E_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART2_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART3_MX_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART3_RX_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART3_TX_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART3_RT_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART3_E_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USART3_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_0_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_HD_0_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_TZ_0_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_1_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_HD_1_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_TZ_1_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_2_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_HD_2_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_TZ_2_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_3_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_HD_3_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_TZ_3_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_4_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_HD_4_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_TZ_4_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_5_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_HD_5_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_TZ_5_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_6_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_HD_6_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_TZ_6_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_7_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_HD_7_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_TZ_7_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_8_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_HD_8_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EPWM_TZ_8_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void ADC_SEQ0_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void ADC_SEQ1_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void ADC_SEQ2_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void ADC_SEQ3_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void ADC_SEQ4_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void ADC_SEQ5_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void ADC_SEQ6_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void ADC_SEQ7_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void ADC_CompInt_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void ECAP0_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void ECAP1_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void ECAP2_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void ECAP3_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void ECAP4_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void ECAP5_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EQEP0_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void EQEP1_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void BootFlash_IRQ(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CMP1_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CMP2_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CMP3_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void SPI0_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void SPI1_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void SPI2_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void SPI3_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void UserFlash_IRQ(void) __attribute__ ((weak, alias("defaultISRhandler")));
void GPIOA_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void GPIOB_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void GPIOC_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void GPIOD_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void GPIOE_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void GPIOF_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void GPIOG_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void GPIOH_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void Ethernet_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CAN0_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CAN1_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CAN2_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CAN3_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CAN4_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CAN5_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CAN6_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CAN7_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CAN8_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CAN9_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CAN10_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CAN11_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CAN12_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CAN13_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CAN14_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void CAN15_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void RTC_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));
void USBOTG_IRQHandler(void) __attribute__ ((weak, alias("defaultISRhandler")));

struct SisrVectors;
typedef volatile struct SisrVectors TisrVectors;

struct SisrVectors {
	void (*_estack)(void);
	void (*Reset_Handler)(void);
	void (*NMI_Handler)(void);
	void (*HardFault_Handler)(void);
	void (*MemManage_Handler)(void);
	void (*BusFault_Handler)(void);
	void (*UsageFault_Handler)(void);
	void (*reserved1)(void);
	void (*reserved2)(void);
	void (*reserved3)(void);
	void (*reserved4)(void);
	void (*SVC_Handler)(void);
	void (*DebugMon_Handler)(void);
	void (*reserved5)(void);
	void (*PendSV_Handler)(void);
	void (*SysTick_Handler)(void);
	void (*WWDG_IRQHandler)(void);
	void (*I2C0_IRQHandler)(void);
	void (*I2C1_IRQHandler)(void);
	void (*TIM0_IRQHandler)(void);
	void (*TIM1_IRQHandler)(void);
	void (*TIM2_IRQHandler)(void);
	void (*DMA1_Stream0_IRQHandler)(void);
	void (*DMA1_Stream1_IRQHandler)(void);
	void (*DMA1_Stream2_IRQHandler)(void);
	void (*DMA1_Stream3_IRQHandler)(void);
	void (*DMA1_Stream4_IRQHandler)(void);
	void (*DMA1_Stream5_IRQHandler)(void);
	void (*DMA1_Stream6_IRQHandler)(void);
	void (*DMA1_Stream7_IRQHandler)(void);
	void (*DMA1_Stream8_IRQHandler)(void);
	void (*DMA1_Stream9_IRQHandler)(void);
	void (*DMA1_Stream10_IRQHandler)(void);
	void (*DMA1_Stream11_IRQHandler)(void);
	void (*DMA1_Stream12_IRQHandler)(void);
	void (*DMA1_Stream13_IRQHandler)(void);
	void (*DMA1_Stream14_IRQHandler)(void);
	void (*DMA1_Stream15_IRQHandler)(void);
	void (*DMA1_Stream16_IRQHandler)(void);
	void (*DMA1_Stream17_IRQHandler)(void);
	void (*DMA1_Stream18_IRQHandler)(void);
	void (*DMA1_Stream19_IRQHandler)(void);
	void (*DMA1_Stream20_IRQHandler)(void);
	void (*DMA1_Stream21_IRQHandler)(void);
	void (*DMA1_Stream22_IRQHandler)(void);
	void (*DMA1_Stream23_IRQHandler)(void);
	void (*USART0_MX_IRQHandler)(void);
	void (*USART0_RX_IRQHandler)(void);
	void (*USART0_TX_IRQHandler)(void);
	void (*USART0_RT_IRQHandler)(void);
	void (*USART0_E_IRQHandler)(void);
	void (*USART0_IRQHandler)(void);
	void (*USART1_MX_IRQHandler)(void);
	void (*USART1_RX_IRQHandler)(void);
	void (*USART1_TX_IRQHandler)(void);
	void (*USART1_RT_IRQHandler)(void);
	void (*USART1_E_IRQHandler)(void);
	void (*USART1_IRQHandler)(void);
	void (*USART2_MX_IRQHandler)(void);
	void (*USART2_RX_IRQHandler)(void);
	void (*USART2_TX_IRQHandler)(void);
	void (*USART2_RT_IRQHandler)(void);
	void (*USART2_E_IRQHandler)(void);
	void (*USART2_IRQHandler)(void);
	void (*USART3_MX_IRQHandler)(void);
	void (*USART3_RX_IRQHandler)(void);
	void (*USART3_TX_IRQHandler)(void);
	void (*USART3_RT_IRQHandler)(void);
	void (*USART3_E_IRQHandler)(void);
	void (*USART3_IRQHandler)(void);
	void (*EPWM_0_IRQHandler)(void);
	void (*EPWM_HD_0_IRQHandler)(void);
	void (*EPWM_TZ_0_IRQHandler)(void);
	void (*EPWM_1_IRQHandler)(void);
	void (*EPWM_HD_1_IRQHandler)(void);
	void (*EPWM_TZ_1_IRQHandler)(void);
	void (*EPWM_2_IRQHandler)(void);
	void (*EPWM_HD_2_IRQHandler)(void);
	void (*EPWM_TZ_2_IRQHandler)(void);
	void (*EPWM_3_IRQHandler)(void);
	void (*EPWM_HD_3_IRQHandler)(void);
	void (*EPWM_TZ_3_IRQHandler)(void);
	void (*EPWM_4_IRQHandler)(void);
	void (*EPWM_HD_4_IRQHandler)(void);
	void (*EPWM_TZ_4_IRQHandler)(void);
	void (*EPWM_5_IRQHandler)(void);
	void (*EPWM_HD_5_IRQHandler)(void);
	void (*EPWM_TZ_5_IRQHandler)(void);
	void (*EPWM_6_IRQHandler)(void);
	void (*EPWM_HD_6_IRQHandler)(void);
	void (*EPWM_TZ_6_IRQHandler)(void);
	void (*EPWM_7_IRQHandler)(void);
	void (*EPWM_HD_7_IRQHandler)(void);
	void (*EPWM_TZ_7_IRQHandler)(void);
	void (*EPWM_8_IRQHandler)(void);
	void (*EPWM_HD_8_IRQHandler)(void);
	void (*EPWM_TZ_8_IRQHandler)(void);
	void (*ADC_SEQ0_IRQHandler)(void);
	void (*ADC_SEQ1_IRQHandler)(void);
	void (*ADC_SEQ2_IRQHandler)(void);
	void (*ADC_SEQ3_IRQHandler)(void);
	void (*ADC_SEQ4_IRQHandler)(void);
	void (*ADC_SEQ5_IRQHandler)(void);
	void (*ADC_SEQ6_IRQHandler)(void);
	void (*ADC_SEQ7_IRQHandler)(void);
	void (*ADC_CompInt_IRQHandler)(void);
	void (*ECAP0_IRQHandler)(void);
	void (*ECAP1_IRQHandler)(void);
	void (*ECAP2_IRQHandler)(void);
	void (*ECAP3_IRQHandler)(void);
	void (*ECAP4_IRQHandler)(void);
	void (*ECAP5_IRQHandler)(void);
	void (*EQEP0_IRQHandler)(void);
	void (*EQEP1_IRQHandler)(void);
	void (*BootFlash_IRQ)(void);
	void (*CMP1_IRQHandler)(void);
	void (*CMP2_IRQHandler)(void);
	void (*CMP3_IRQHandler)(void);
	void (*SPI0_IRQHandler)(void);
	void (*SPI1_IRQHandler)(void);
	void (*SPI2_IRQHandler)(void);
	void (*SPI3_IRQHandler)(void);
	void (*UserFlash_IRQ)(void);
	void (*GPIOA_IRQHandler)(void);
	void (*GPIOB_IRQHandler)(void);
	void (*GPIOC_IRQHandler)(void);
	void (*GPIOD_IRQHandler)(void);
	void (*GPIOE_IRQHandler)(void);
	void (*GPIOF_IRQHandler)(void);
	void (*GPIOG_IRQHandler)(void);
	void (*GPIOH_IRQHandler)(void);
	void (*Ethernet_IRQHandler)(void);
	void (*CAN0_IRQHandler)(void);
	void (*CAN1_IRQHandler)(void);
	void (*CAN2_IRQHandler)(void);
	void (*CAN3_IRQHandler)(void);
	void (*CAN4_IRQHandler)(void);
	void (*CAN5_IRQHandler)(void);
	void (*CAN6_IRQHandler)(void);
	void (*CAN7_IRQHandler)(void);
	void (*CAN8_IRQHandler)(void);
	void (*CAN9_IRQHandler)(void);
	void (*CAN10_IRQHandler)(void);
	void (*CAN11_IRQHandler)(void);
	void (*CAN12_IRQHandler)(void);
	void (*CAN13_IRQHandler)(void);
	void (*CAN14_IRQHandler)(void);
	void (*CAN15_IRQHandler)(void);
	void (*RTC_IRQHandler)(void);
	void (*USBOTG_IRQHandler)(void);
};

// Заполнитель массива векторов
#define ISR_VECTORS {\
   (void(*)(void))&_estack,\
   Reset_Handler,\
   NMI_Handler,\
   HardFault_Handler,\
   MemManage_Handler,\
   BusFault_Handler,\
   UsageFault_Handler,\
   0,\
   0,\
   0,\
   0,\
   SVC_Handler,\
   DebugMon_Handler,\
   0,\
   PendSV_Handler,\
   SysTick_Handler,\
   WWDG_IRQHandler,\
   I2C0_IRQHandler,\
   I2C1_IRQHandler,\
   TIM0_IRQHandler,\
   TIM1_IRQHandler,\
   TIM2_IRQHandler,\
   DMA1_Stream0_IRQHandler,\
   DMA1_Stream1_IRQHandler,\
   DMA1_Stream2_IRQHandler,\
   DMA1_Stream3_IRQHandler,\
   DMA1_Stream4_IRQHandler,\
   DMA1_Stream5_IRQHandler,\
   DMA1_Stream6_IRQHandler,\
   DMA1_Stream7_IRQHandler,\
   DMA1_Stream8_IRQHandler,\
   DMA1_Stream9_IRQHandler,\
   DMA1_Stream10_IRQHandler,\
   DMA1_Stream11_IRQHandler,\
   DMA1_Stream12_IRQHandler,\
   DMA1_Stream13_IRQHandler,\
   DMA1_Stream14_IRQHandler,\
   DMA1_Stream15_IRQHandler,\
   DMA1_Stream16_IRQHandler,\
   DMA1_Stream17_IRQHandler,\
   DMA1_Stream18_IRQHandler,\
   DMA1_Stream19_IRQHandler,\
   DMA1_Stream20_IRQHandler,\
   DMA1_Stream21_IRQHandler,\
   DMA1_Stream22_IRQHandler,\
   DMA1_Stream23_IRQHandler,\
   USART0_MX_IRQHandler,\
   USART0_RX_IRQHandler,\
   USART0_TX_IRQHandler,\
   USART0_RT_IRQHandler,\
   USART0_E_IRQHandler,\
   USART0_IRQHandler,\
   USART1_MX_IRQHandler,\
   USART1_RX_IRQHandler,\
   USART1_TX_IRQHandler,\
   USART1_RT_IRQHandler,\
   USART1_E_IRQHandler,\
   USART1_IRQHandler,\
   USART2_MX_IRQHandler,\
   USART2_RX_IRQHandler,\
   USART2_TX_IRQHandler,\
   USART2_RT_IRQHandler,\
   USART2_E_IRQHandler,\
   USART2_IRQHandler,\
   USART3_MX_IRQHandler,\
   USART3_RX_IRQHandler,\
   USART3_TX_IRQHandler,\
   USART3_RT_IRQHandler,\
   USART3_E_IRQHandler,\
   USART3_IRQHandler,\
   EPWM_0_IRQHandler,\
   EPWM_HD_0_IRQHandler,\
   EPWM_TZ_0_IRQHandler,\
   EPWM_1_IRQHandler,\
   EPWM_HD_1_IRQHandler,\
   EPWM_TZ_1_IRQHandler,\
   EPWM_2_IRQHandler,\
   EPWM_HD_2_IRQHandler,\
   EPWM_TZ_2_IRQHandler,\
   EPWM_3_IRQHandler,\
   EPWM_HD_3_IRQHandler,\
   EPWM_TZ_3_IRQHandler,\
   EPWM_4_IRQHandler,\
   EPWM_HD_4_IRQHandler,\
   EPWM_TZ_4_IRQHandler,\
   EPWM_5_IRQHandler,\
   EPWM_HD_5_IRQHandler,\
   EPWM_TZ_5_IRQHandler,\
   EPWM_6_IRQHandler,\
   EPWM_HD_6_IRQHandler,\
   EPWM_TZ_6_IRQHandler,\
   EPWM_7_IRQHandler,\
   EPWM_HD_7_IRQHandler,\
   EPWM_TZ_7_IRQHandler,\
   EPWM_8_IRQHandler,\
   EPWM_HD_8_IRQHandler,\
   EPWM_TZ_8_IRQHandler,\
   ADC_SEQ0_IRQHandler,\
   ADC_SEQ1_IRQHandler,\
   ADC_SEQ2_IRQHandler,\
   ADC_SEQ3_IRQHandler,\
   ADC_SEQ4_IRQHandler,\
   ADC_SEQ5_IRQHandler,\
   ADC_SEQ6_IRQHandler,\
   ADC_SEQ7_IRQHandler,\
   ADC_CompInt_IRQHandler,\
   ECAP0_IRQHandler,\
   ECAP1_IRQHandler,\
   ECAP2_IRQHandler,\
   ECAP3_IRQHandler,\
   ECAP4_IRQHandler,\
   ECAP5_IRQHandler,\
   EQEP0_IRQHandler,\
   EQEP1_IRQHandler,\
   BootFlash_IRQ,\
   CMP1_IRQHandler,\
   CMP2_IRQHandler,\
   CMP3_IRQHandler,\
   SPI0_IRQHandler,\
   SPI1_IRQHandler,\
   SPI2_IRQHandler,\
   SPI3_IRQHandler,\
   UserFlash_IRQ,\
   GPIOA_IRQHandler,\
   GPIOB_IRQHandler,\
   GPIOC_IRQHandler,\
   GPIOD_IRQHandler,\
   GPIOE_IRQHandler,\
   GPIOF_IRQHandler,\
   GPIOG_IRQHandler,\
   GPIOH_IRQHandler,\
   Ethernet_IRQHandler,\
   CAN0_IRQHandler,\
   CAN1_IRQHandler,\
   CAN2_IRQHandler,\
   CAN3_IRQHandler,\
   CAN4_IRQHandler,\
   CAN5_IRQHandler,\
   CAN6_IRQHandler,\
   CAN7_IRQHandler,\
   CAN8_IRQHandler,\
   CAN9_IRQHandler,\
   CAN10_IRQHandler,\
   CAN11_IRQHandler,\
   CAN12_IRQHandler,\
   CAN13_IRQHandler,\
   CAN14_IRQHandler,\
   CAN15_IRQHandler,\
   RTC_IRQHandler,\
   USBOTG_IRQHandler,\
}

#endif

/*@}*/
