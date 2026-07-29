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
 
 \file      V_SSI_Encoder.c
 \brief     Модуль оценки скорости и положения при помощи цифрового энкодера, работающего по интерфейсу SSI (см. TSSI_Encoder)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 25/04/2016

 \addtogroup V_QEP
 @{*/

#include "DSP.h"
#include "V_IQmath.h"
#include "V_SSI_Encoder.h"
#include "math.h"
#include "stdlib.h"
#include "main.h"

//! Инициализация

//! \memberof TSSI_Encoder
void SSI_Encoder_init(TSSI_Encoder *p) {
	volatile long delay;
	volatile Uint32 tempREG;


	// Настройка ног SPI
#ifdef HW_MOTORCONTROLBOARD
#define SSI_NT_SPI NT_SPI0 //Настройка номера SPI
	NT_GPIOA->ALTFUNCSET = (1 << 6); //SPI_CLK0
	NT_GPIOA->ALTFUNCSET = (1 << 7); //SPI_RXD0
	NT_COMMON_REG->GPIOPCTLA_bit.PIN6 = 0;
	NT_COMMON_REG->GPIOPCTLA_bit.PIN7 = 0;
#else

#define SSI_NT_SPI NT_SPI2 //Настройка номера SPI
	//SPI-SOMI - на VectorCard нога 88, на проце B15, SPI_RXD2
	//SPI-SIMO - на VectorCard нога 38, на проце C6, SPI_TXD2
	//SPI-CLK - на VectorCard нога 39, на проце  B14, SPI_CLK2
	//SPI-STE - на VectorCard нога 89, на проце  B13, SPI_FSS2
	//Плата texas DRV8301-HC-EVM разведена так, что микроконтроллер - это ведомое устройство.
	//Нужно сделать его мастером. Подробнее читайте комментарий в заголовочном файле.

	NT_GPIOB->ALTFUNCSET = (1 << 13);//чип-селект
	NT_GPIOB->ALTFUNCSET = (1 << 14) | (1 << 15);
	NT_GPIOC->ALTFUNCSET = (1 << 6);//SIMO
	NT_COMMON_REG->GPIOPCTLB_bit.PIN13 = 2;//чип-селект
	NT_COMMON_REG->GPIOPCTLB_bit.PIN14 = 2;
	NT_COMMON_REG->GPIOPCTLB_bit.PIN15 = 2;
	NT_COMMON_REG->GPIOPCTLC_bit.PIN6 = 2;//SIMO
#endif

	// Настройка тактирования SSP модуля
	// Всего 4 модуля, на каждый модуль по 8 бит из регистра SSP_CLK_CTRL и по два бита из UART_SSP_CLK_SEL

	// UART_SSP_CLK_SEL
	// Во втором байте слова на каждый из 4-ёх модулей SSP отводится по два бита для выбора источника тактирования 0x0000XX00
	// "00" - в качестве иточника тактирования модуля SSP выбирается системная частота 100 МГц, таким образом f_SSP_IN = SysClk = 100 MHz

	// SSP_CLK_CTRL
	// Младший бит разрешает тактирование (1 - разрешить)
	// Второй бит разрешает деление частоты f_SSP_IN источника тактового сигнала (0 - не делить, 1 - делить)
	// Старшие шесть выбирают делитель частоты этого исотчника:
	//  X - деление по формуле SSPclk = f_SSP_IN /( 2 * (X +1) )
	// Из документации на блок SSP - минимальная частота для работы модуля в режимах и мастер и слейв,
	// f_SSP_IN > 22.12 MHz, поэтому 25 МГц сделаем
#ifdef HW_MOTORCONTROLBOARD
	//Для DRV8301_SPI не делаем, т.к. тоже самое делается в нем самом
	tempREG = NT_COMMON_REG->UART_SPI_CLK_SEL;// Через tempREG, чтобы не задеть другие биты, отвечающие за тактирование UART'a
	tempREG &= 0xFFFF00FF;
	NT_COMMON_REG->UART_SPI_CLK_SEL = tempREG;
	NT_COMMON_REG->SPI_CLK = 0x07070707;// Разрешить тактирование, разрешить деление частоты, делить на 4 - 25 МГц
#endif

	// Настройка самого модуля SPI
	// Продолжение настройки тактирования.
	// Полученная ранее частота f_SSP_IN проходит ещё через два делителя.
	// SSPCPSR - первый делитель, в диапазоне 2 ... 254, может быть только чётным (младший бит всегда хардварно равен 0)
	// SSPCR0.bit.SCR - второй делитель от 0 до 255.
	// Битрейт в итоге BitRate = f_SSP_IN / ( SSPCPSR * (SCR + 1) )

	SSI_NT_SPI->SPI_CR1 = 0;// Режим - мастер, LoopBack отключён, сам модуль SSP тоже отключён
	SSI_NT_SPI->SPI_IMSC = 0x0;		// Запретить все прерывания
	SSI_NT_SPI->SPI_DMACR = 0;		// Запретить DMA
	SSI_NT_SPI->SPI_ICR = 0x3;	// Очистка прерываний ("переполнение FIFO приёма" и "необслуженное FIFO приёма")

	SSI_NT_SPI->SPI_CPSR = 4;			// Деление входной частоты на 4 -> 6,25 MHz
	SSI_NT_SPI->SPI_CR0_bit.DSS = 12;	// Размер данных - 12 бит
	SSI_NT_SPI->SPI_CR0_bit.SCR = 0x3F;	// Второй делитель
	SSI_NT_SPI->SPI_CR0_bit.FRF = 0x0;	// Какая-то "фаза" для протокола Motorola SPI
	SSI_NT_SPI->SPI_CR0_bit.SPH = 0x1;	// Какая-то "полярность" для протокола Motorola SPI
	SSI_NT_SPI->SPI_CR0_bit.SPO = 0x0;	// Выбор формата кадра Motorola/TI/Microwire. "0" - по протоколу Motorola SPI
	SSI_NT_SPI->SPI_CR1_bit.SSE = 1;	// Разрешить работу

	p->resol_inv = 1.0 / ((float) p->resol);

	p->read(p);
}


//! Функция расчёта скорости и положения, вызывается с необходимой дискретностью

//! \memberof TSSI_Encoder
void SSI_Encoder_Calc(TSSI_Encoder *p) {
	p->read(p);
}



void SSI_Encoder_Read(TSSI_Encoder*p) {
	_iq theta_elec_temp;
	Uint16 Data_read=0;
	_iq theta_mech_temp;

	if (SSI_NT_SPI->SPI_SR_bit.BSY == 0){//SPI свободен
		Data_read = SSI_NT_SPI->SPI_DR;//код с датчика (число от 0 до resol)
		SSI_NT_SPI->SPI_DR = 0xff;//отправляем что угодно, главное, чтобы тактирование шло


		if (p->rotation_dir)//обратное направление вращения
			Data_read=(p->resol-1)-Data_read;//период - текущее
		p->Poscnt_res=Data_read;
	}

	//перевод угла в метках на обороте в механический угол
	//Здесь расчет во float - желательно переделать в IQ
	p->theta_mech = _IQ((float )p->Poscnt_res * p->resol_inv); //расчёт механического угла
	p->theta_mech &= 0x00FFFFFF;
	//Фильтр угла
	if (p->theta_mech_filterK!=0){
        p->theta_mech_filtered=p->theta_mech_filtered+_IQmpy(p->theta_mech_filterK,((p->theta_mech-p->theta_mech_filtered+_IQ(0.5))&0x00FFFFFF)-_IQ(0.5));
        p->theta_mech_filtered&=0x00FFFFFF;
	}else
		p->theta_mech_filtered=p->theta_mech;

    // Подсчёт количества полных оборотов.
    if (p->prevThetaMech - p->theta_mech_filtered > _IQ(0.5))
		p->RevolutionCounter++;
	if (p->prevThetaMech - p->theta_mech_filtered < _IQ(-0.5))
		p->RevolutionCounter--;
	p->prevThetaMech=p->theta_mech_filtered;

	//угол в метках без обнуления на обороте, абсолютный
	p->Poscnt_resContinouosLong=p->Poscnt_res+(p->resol*p->RevolutionCounter);
	p->Poscnt_resContinouosInt=p->Poscnt_resContinouosLong;//чтобы было уднобно смотреть в 16ти разрядном осциллографе
	p->Poscnt_resContinouosInt8=p->Poscnt_resContinouosLong&0xF;//чтобы видеть метки в крупном масштабе

	//перевод угла в метках абсолютных (не обнуляемых наобороте) в механический угол
	//на 127 оборотах всё переполнится, но для демо сгодится
	p->theta_mechContinouos = p->theta_mech_filtered + _IQ(p->RevolutionCounter); //расчёт механического угла
	p->theta_elecContinouos = p->theta_mechContinouos*p->pole_pairs+ p->AngleOffset;//электрический угол абсолютный (не обнуляемый)

	//Расчёт электрического положения обнулемого по достижению 360 градусов
	p->theta_el_tmp = p->theta_mech_filtered*p->pole_pairs + p->AngleOffset;
	p->theta_elec = p->theta_el_tmp & 0x00FFFFFF;

}



/*@}*/

