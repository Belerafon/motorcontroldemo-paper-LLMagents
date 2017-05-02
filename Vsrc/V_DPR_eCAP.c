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

 \file      V_DPR_eCAP.c
 \brief     Модуль датчика абсолютного положения ротора с использованием модулей CAP TDPReCAP (см. TDPReCAP)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.1 03/01/2013

 */

/** \addtogroup V_DPR_eCAP */
/*@{*/

#include "DSP.h"              // Device Headerfile and Examples Include File
#include "V_IQmath.h"          // библиотека IQmath
#include "V_DPR_eCAP.h"       // заголовочный файл модуля
#include "main.h"

extern TDrvParams drv_params;
//! Инициализация

//!Модули CAP_3, CAP_4, CAP_5 инициализируются
//!для захвата времени между событиями нарастающего и спадающего
//!фронтов, а также генерации прерываний по этим событиям.

//! \memberof TDPReCAP
void DPReCAP_Init(TDPReCAP* p) {

	//Инициализация ECAP1
	NT_CAP0->ECEINT = 0x0000;             	// Disable all capture interrupts
	NT_CAP0->ECCLR = 0xFFFF;              	// Clear all CAP interrupt flags
	NT_CAP0->ECCTL0_bit.CAPLDEN = 0;         // Disable CAP1-CAP4 register loads
	NT_CAP0->ECCTL1_bit.TSCTRSTOP = 0;       // Make sure the counter is stopped
	NT_CAP0->ECCTL0_bit.PRESCALE = 0;			// DIV1

	// Configure peripheral registers
	NT_CAP0->ECCTL1_bit.CONT_ONESHT = 0;      	// continuous mode
	NT_CAP0->ECCTL0_bit.CAP0_POL = 0;          	// rising edge
	NT_CAP0->ECCTL0_bit.CAP1_POL = 1;          	// falling edge
	NT_CAP0->ECCTL0_bit.CTR_RST0 = 0;          	// absolute time stamp
	NT_CAP0->ECCTL0_bit.CTR_RST1 = 0;          	// absolute time stamp
	NT_CAP0->ECCTL1_bit.STOP_WRAP = 1;			// Wrap after Capture Event 2
//	NT_CAP0->ECCTL1_bit.SYNCI_EN = 1;         	// Enable sync in
	NT_CAP0->ECCTL1_bit.SYNCO_SEL = 0;        	// Pass through
	NT_CAP0->ECCTL0_bit.CAPLDEN = 1;          	// Enable capture units



	NT_CAP0->ECCTL1_bit.TSCTRSTOP = 1;        	// Start Counter
	NT_CAP0->ECCTL1_bit.REARM = 0; // Has no effect (Не очень понимаю этот регистр)
	NT_CAP0->ECCTL0_bit.CAPLDEN = 1;          // Enable CAP1-CAP4 register loads
	NT_CAP0->ECEINT_bit.CEVT0 = 1;            	// 1 events = interrupt
	NT_CAP0->ECEINT_bit.CEVT1 = 1;            	// 2 events = interrupt

	//Инициализация ECAP2
	NT_CAP1->ECEINT = 0x0000;             // Disable all capture interrupts
	NT_CAP1->ECCLR = 0xFFFF;              	// Clear all CAP interrupt flags
	NT_CAP1->ECCTL0_bit.CAPLDEN = 0;          // Disable CAP1-CAP4 register loads
	NT_CAP1->ECCTL1_bit.TSCTRSTOP = 0;        // Make sure the counter is stopped
	NT_CAP1->ECCTL0_bit.PRESCALE = 0;			// DIV1

	// Configure peripheral registers
	NT_CAP1->ECCTL1_bit.CONT_ONESHT = 0;      	// continuous mode
	NT_CAP1->ECCTL0_bit.CAP0_POL = 0;          	// rising edge
	NT_CAP1->ECCTL0_bit.CAP1_POL = 1;          	// falling edge
	NT_CAP1->ECCTL0_bit.CTR_RST0 = 0;          	// absolute time stamp
	NT_CAP1->ECCTL0_bit.CTR_RST1 = 0;          	// absolute time stamp
	NT_CAP1->ECCTL1_bit.STOP_WRAP = 1;			// Wrap after Capture Event 2
//	NT_CAP1->ECCTL1_bit.SYNCI_EN = 1;         	// Enable sync in
	NT_CAP1->ECCTL1_bit.SYNCO_SEL = 0;        	// Pass through
	NT_CAP1->ECCTL0_bit.CAPLDEN = 1;          	// Enable capture units

	NT_CAP1->ECCTL1_bit.TSCTRSTOP = 1;        	// Start Counter
	NT_CAP1->ECCTL1_bit.REARM = 0; // Has no effect (Не очень понимаю этот регистр)
	NT_CAP1->ECCTL0_bit.CAPLDEN = 1;          // Enable CAP1-CAP4 register loads
	NT_CAP1->ECEINT_bit.CEVT0 = 1;            	// 1 events = interrupt
	NT_CAP1->ECEINT_bit.CEVT1 = 1;            	// 2 events = interrupt

	//Инициализация ECap3
	NT_CAP2->ECEINT = 0x0000;             // Disable all capture interrupts
	NT_CAP2->ECCLR = 0xFFFF;              	// Clear all CAP interrupt flags
	NT_CAP2->ECCTL0_bit.CAPLDEN = 0;          // Disable CAP1-CAP4 register loads
	NT_CAP2->ECCTL1_bit.TSCTRSTOP = 0;        // Make sure the counter is stopped
	NT_CAP2->ECCTL0_bit.PRESCALE = 0;			// DIV1

	// Configure peripheral registers
	NT_CAP2->ECCTL1_bit.CONT_ONESHT = 0;      	// continuous mode
	NT_CAP2->ECCTL0_bit.CAP0_POL = 0;          	// rising edge
	NT_CAP2->ECCTL0_bit.CAP1_POL = 1;          	// falling edge
	NT_CAP2->ECCTL0_bit.CTR_RST0 = 0;          	// absolute time stamp
	NT_CAP2->ECCTL0_bit.CTR_RST1 = 0;          	// absolute time stamp
	NT_CAP2->ECCTL1_bit.STOP_WRAP = 1;			// Wrap after Capture Event 2
//	NT_CAP2->ECCTL1_bit.SYNCI_EN = 1;         	// Enable sync in
	NT_CAP2->ECCTL1_bit.SYNCO_SEL = 0;        	// Pass through
	NT_CAP2->ECCTL0_bit.CAPLDEN = 1;          	// Enable capture units

	NT_CAP2->ECCTL1_bit.TSCTRSTOP = 1;        	// Start Counter
	NT_CAP2->ECCTL1_bit.REARM = 0; // Has no effect (Не очень понимаю этот регистр)
	NT_CAP2->ECCTL0_bit.CAPLDEN = 1;          // Enable CAP1-CAP4 register loads
	NT_CAP2->ECEINT_bit.CEVT0 = 1;            	// 1 events = interrupt
	NT_CAP2->ECEINT_bit.CEVT1 = 1;            	// 2 events = interrupt

	p->TsNom = ((CORE_CLK / (drv_params.speed_nom * drv_params.p)) * 15*2);
	p->TsNomMilsec = (100000 / (drv_params.speed_nom * drv_params.p));

#if (!defined(HW_VECTORCARD_SIMULATOR)) && (!defined(HW_NIIET_BOARD_SIMULATOR))
	//CAP0, CAP1, CAP2 - E4, E5, E6, соответственно
	//На плате инвертора TI это разъем J10
	NT_GPIOE->ALTFUNCSET=(1<<4)+(1<<5)+(1<<6);
	NT_COMMON_REG->GPIOPCTLE_bit.PIN4 = 1;	//eCAPxPWM[0]
	NT_COMMON_REG->GPIOPCTLE_bit.PIN5 = 1;	//eCAPxPWM[1]
	NT_COMMON_REG->GPIOPCTLE_bit.PIN6 = 1;	//eCAPxPWM[2]
#endif

	DPReCAP_Angle6Calc(p);
	p->CAP_WrongEdgeCnt = 0;
	p->SimulatorOmega2IQ_factor=_IQ(60.0/(2*MOTOR_MODEL_PI*drv_params.speed_nom));

}


//! Определение углового положения с дискретностью 60 градусов

//!Определение углового положения происходит исходя из опроса трех каналов датчика
//!через GPIO. Результат попадает в переменную Angle6.

//! \memberof TDPReCAP
void DPReCAP_Angle6Calc(TDPReCAP* p) {
	// На момент расчёта, запрещаем прерывания по датчикам.
	//Если этого не сделать, то эта функция может вызваться в 10к,
	//на половне прерваться прерыванием CAP, в котором она вызовется второй раз,
	//потом управление вернется к этой функции в 10к и в p->Angle6 попадет старый результат
	NT_CAP0->ECEINT = 0x0000;
	NT_CAP1->ECEINT = 0x0000;
	NT_CAP2->ECEINT = 0x0000;

	// Формируем код по состоянию ножек.
	p->HallCode = 0;

#if defined(HW_VECTORCARD_SIMULATOR) || defined(HW_NIIET_BOARD_SIMULATOR)
		p->HallCode = model.hallSensor;

#else
	if (NT_GPIOE->DATA & (1 << 4))
		p->HallCode = p->HallCode + 1;
	if (NT_GPIOE->DATA & (1 << 5))
		p->HallCode = p->HallCode + 2;
	if (NT_GPIOE->DATA & (1 << 6))
		p->HallCode = p->HallCode + 4;
#endif


	switch (p->HallCode) {
	case 5: // 0
		p->Angle6 = 0;
		break;
	case 4: // 60
		p->Angle6 = _IQ(1.0 / 6.0);
		break;
	case 6: // 120
		p->Angle6 = _IQ(1.0 / 3.0);
		break;
	case 2: //180
		p->Angle6 = _IQ(1.0 / 2.0);
		break;
	case 3: // 240
		p->Angle6 = _IQ(2.0 / 3.0);
		break;
	case 1: // 300
		p->Angle6 = _IQ(5.0 / 6.0);
		break;
	}

	// Разрешаем прерывания назад.
	NT_CAP0->ECEINT = 6;
	NT_CAP1->ECEINT = 6;
	NT_CAP2->ECEINT = 6;
}

void DPReCAP_AngleErrorCalc(TDPReCAP* p) {
	long AngleDiff = 0;
	AngleDiff = (labs(
			((p->Angle6 - p->AnglePrev + _IQ(0.5)) & 0x00FFFFFF) - _IQ(0.5)))
			& 0x00FFFFFF;
	p->AnglePrev = p->Angle6;

	if (AngleDiff > _IQ(61.0 / 360)) { //если угол с прошлого раза изменился больше, чем на 60 градусов, то датчик косячит
		p->WrongCodeCounter++;
		p->WrongCodeCounterPerSec++;

	}
}

//! Определение углового положения с учетом работы интерполятора углового положения

//!Интерполятор углового положения (а вернее даже экстраполятор)
//!использует дискретное угловое положение Angle6, выдаваемое функцией DPReCAP_Angle6Calc.
//!Функция "сглаживает" угловое положение во времени, делая из степенчатой смены угла "лесенкой"
//!непрерывную "пилу" (переменная Angle). Для этого используется сохраненное время между двумя любыми
//!предыдущими фронтами с каналов датчика положения (PrevTs). Считая, что скорость вращения постоянна,
//!следующий фронт (а значит смену углового положения) можно прогнозировать через то же самое время.
//!Таким образом, у текущему дискретному угловому положению Angle6 прибавляется
//!добавка, рассчитываемая по формуле 60*(Текущее время/Период).
//!Так, в момент прихода фронта с датчика переменная Angle равна Angle6. Через некоторый момент времени
//! Angle станет равным Angle6+60*(delta/Ts), где delta - текущее время с момента события последнего фронта с датчика,
//! а Ts - период, время между предыдущими двумя событиями фронров датчика.
//! Функция также учитывает направление вращения, а также добавляет пользовательское смещение
//! AngleOffset к результирующему угловому положению.

//! \memberof TDPReCAP
void DPReCAP_AngleCalc(TDPReCAP* p) {
	Uint32 delta, Timer;
	Uint32 PrevTs;
	_iq Angle;
	_iq Angle6;

	// Фиксируем значения переменных на момент начала расчёта
	Angle6 = p->Angle6;

#if (!defined(HW_VECTORCARD_SIMULATOR)) && (!defined(HW_NIIET_BOARD_SIMULATOR))

	PrevTs = p->PrevTs;
	// На момент расчёта, запрещаем прерывания по датчикам.
	NT_CAP0->ECEINT = 0x0000;
	NT_CAP1->ECEINT = 0x0000;
	NT_CAP2->ECEINT = 0x0000;



	// Фиксируем значение одного из таймеров на момент начала выполнения модуля.
	switch (p->DPReCAP_FLG1.bit.CAPnumber) {
	case 1:
		Timer = NT_CAP0->TSCTR;
		break;
	case 2:
		Timer = NT_CAP1->TSCTR;
		break;
	case 3:
		Timer = NT_CAP2->TSCTR;
		break;
	}

	// Если скорость равна нулю или выбран соответствующий режим, то угол не интеполируем.
	if ((p->speed == 0) || (p->DPReCAP_FLG1.bit.AngleMode == 0)
			|| (p->Ts == 0)) {
		Angle = _IQ(1.0 / 12);
	} else {
		delta = Timer - PrevTs; // Сколько натикал таймер с момента прошлого обновления периода.
		Angle = _IQdiv(delta, p->Ts); // Интеполируем угол. Отношение прошлого перехода к текущим "тикам".
		if (Angle >= _IQ(1.0 / 6.0)) // ограничиваем угол в приделах 1/6.
			Angle = _IQ(1.0 / 6.0);
	}
#else //симулятор
	Angle=_IQ(model.hallSensorInterpAdd * (1/(2*MOTOR_MODEL_PI)));//приращение угла для интерполятора уже предпосчитано в модели двигателя с учетом дискретности
#endif

	if (p->DPReCAP_FLG1.bit.Dir == 1)
		Angle = _IQ(1.0/6.0) - Angle;

	if (p->DPReCAP_FLG1.bit.Dir) //пользовательская инверсия направления
		Angle = -Angle + _IQ(1.0/6);

	p->Angle = Angle6 + Angle + p->AngleOffset;

	p->Angle &= 0x00FFFFFF;


	// Разрешаем прерывания назад.
	NT_CAP0->ECEINT = 6;
	NT_CAP1->ECEINT = 6;
	NT_CAP2->ECEINT = 6;
}

//! Функция расчета скорости

//!Для расчета скорости используется переменная Tspeed,
//!которая представляет собой время между событиями фронтов датчика положения
//!произошедших по одному и тому же каналу. Так, например, временем между нарастающим и спадающим фронтом
//!канала CAP3, затем между спадающим и нарастающим фронтом CAP4 и т.п.
//!На основе этого времени, направления вращения и предпосчитанной константой TsNom
//!рассчитывается скорость вращения.

//! \memberof TDPReCAP
void DPReCAP_SpeedCalc(TDPReCAP* p) {
#if (!defined(HW_VECTORCARD_SIMULATOR)) && (!defined(HW_NIIET_BOARD_SIMULATOR))

	_iq speed;
	p->milsec++;

	// Считаем скорость в относительных единицах относительно номинальной.
	if ((p->Tspeed != 0) && (p->DPReCAP_FLG1.bit.ZeroFLG == 0)) {

		// Знак скорости опрделяется в зависимости от направления.
		if (p->DPReCAP_FLG1.bit.Dir == 0)
			speed = _IQdiv(p->TsNom, p->Tspeed);
		else
			speed = -_IQdiv(p->TsNom, p->Tspeed);
		if (p->DPReCAP_FLG1.bit.Dir) { //задается пользователем
			speed = -speed;
		}
	} else {
		speed = 0;
	}

	p->speed = speed;

	if (p->milsecREF != p->milsecPrevREF) {
		p->milsecFIX = p->milsecREF * 10; //к 10кГц
		p->speedMinREF = _IQdiv(p->TsNomMilsec, p->milsecFIX);
		p->milsecPrevREF = p->milsecREF;
	}

	DINT;//нужно для потокобезопасного обращения к переменной DPReCAP_FLG1 (чтобы не перетереть присваивание в прерывании захвата)
	// Обнуление скорости, если привысили заданное время между двумя соседними событиями.
	if (p->milsec > p->milsecFIX) {
		p->speed = 0;
		p->DPReCAP_FLG1.bit.ZeroFLG = 1;
//		p->cnt1 = 0; // Надо обнулять при стопе и hold'е. Здесь не надо, только для теста.
	}
	EINT;

	DINT;//нужно для потокобезопасного обращения к переменной DPReCAP_FLG1 (чтобы не перетереть присваивание в прерывании захвата)

	// При скорости меньше заданной, отключаем интерполяцию угла.
	if ((labs(p->speed)) <= (p->speedMIN))
		p->DPReCAP_FLG1.bit.SpeedMinFLG = 0;
	else
		p->DPReCAP_FLG1.bit.SpeedMinFLG = 1;
	EINT;

#else //симулятор
	p->speed=p->SimulatorOmega2IQ_factor*model.hallSensorOmega;//Частота вращения уже предпосчитана в модели двигателя с учетом дискретности
#endif
}

//! Функция, вызываемая в прерывании по нарастающему и спадающему фронту канала датчика 1

//! В функции запоминается время таймера модуля CAP в переменную Timer1.
//! Исходя из этого считаются две переменные - p->Ts, время между двумя ближайшими фронтами,
//!необходимое для интерполятора углового положения, и переменная  p->Tspeed,
//!время между двумя фронтами одного и того же канала, неободимое для расчета скорости.
//! \memberof TDPReCAP
void DPReCAP_CAP1Calc(TDPReCAP* p) {
	Uint32 Timer=0;
	if (p->cnt >= 2) {
		p->HelpCalc(p);

		// В зависимости от номера предудыщего CAP определяем направление вращения.
		if (p->DPReCAP_FLG1.bit.CAPnumber == 3)
			p->DPReCAP_FLG1.bit.Dir = 0;
		if (p->DPReCAP_FLG1.bit.CAPnumber == 2)
			p->DPReCAP_FLG1.bit.Dir = 1;
		if (p->DPReCAP_FLG1.bit.CAPnumber == 1) {
			if (p->DPReCAP_FLG1.bit.PrevDir == 0)
				p->DPReCAP_FLG1.bit.Dir = 1;
			else
				p->DPReCAP_FLG1.bit.Dir = 0;
		}
		if (p->DPReCAP_FLG1.bit.Dir != p->DPReCAP_FLG1.bit.PrevDir) {
			p->DPReCAP_FLG1.bit.PrevDir = p->DPReCAP_FLG1.bit.Dir;
			p->cnt2 = 1;
		}

	    if (NT_CAP0->ECFLG_bit.CEVT0==1)//Фронт вверх
	    {
	            Timer = NT_CAP0->CAP0;
	    }
	    if (NT_CAP0->ECFLG_bit.CEVT1==1)//Фронт вниз
	    {
	            Timer = NT_CAP0->CAP1;
	    }

		// считаем периоды для расчёта угла и скорости.
		p->Ts = (Timer - p->PrevTs) * 6;
		p->PrevTs = Timer;

		p->Tspeed = Timer - p->PrevTspeed1;
		p->PrevTspeed1 = Timer;

	} else {
		p->Ts = 0;
		p->Tspeed = 0;
	}


	p->cnt++;
	p->milsec = 0;
	p->DPReCAP_FLG1.bit.CAPnumber = 1;

}

//! Функция, вызываемая в прерывании по нарастающему и спадающему фронту канала датчика 2

//! В функции запоминается время таймера модуля CAP в переменную Timer1.
//! Исходя из этого считаются две переменные - p->Ts, время между двумя ближайшими фронтами,
//!необходимое для интерполятора углового положения, и переменная  p->Tspeed,
//!время между двумя фронтами одного и того же канала, неободимое для расчета скорости.
//! \memberof TDPReCAP
void DPReCAP_CAP2Calc(TDPReCAP* p) {
	Uint32 Timer=0;

	if (p->cnt >= 2) {
		p->HelpCalc(p);

		// В зависимости от номера предудыщего CAP определяем направление вращения.
		if (p->DPReCAP_FLG1.bit.CAPnumber == 1)
			p->DPReCAP_FLG1.bit.Dir = 0;
		if (p->DPReCAP_FLG1.bit.CAPnumber == 3)
			p->DPReCAP_FLG1.bit.Dir = 1;
		if (p->DPReCAP_FLG1.bit.CAPnumber == 2) {
			if (p->DPReCAP_FLG1.bit.PrevDir == 0)
				p->DPReCAP_FLG1.bit.Dir = 1;
			else
				p->DPReCAP_FLG1.bit.Dir = 0;
		}

		if (p->DPReCAP_FLG1.bit.Dir != p->DPReCAP_FLG1.bit.PrevDir) {
			p->DPReCAP_FLG1.bit.PrevDir = p->DPReCAP_FLG1.bit.Dir;
			p->cnt2 = 1;
		}

	    if (NT_CAP1->ECFLG_bit.CEVT0==1)
	    {
	            Timer = NT_CAP1->CAP0;
	    }
	    if (NT_CAP1->ECFLG_bit.CEVT1==1)
	    {
	            Timer = NT_CAP1->CAP1;
	    }

		// считаем периоды для расчёта угла и скорости.
		p->Ts = (Timer - p->PrevTs) * 6;
		p->PrevTs = Timer;

		p->Tspeed = Timer - p->PrevTspeed2;
		p->PrevTspeed2 = Timer;

	} else {
		p->Ts = 0;
		p->Tspeed = 0;
	}


	p->cnt++;
	p->milsec = 0;
	p->DPReCAP_FLG1.bit.CAPnumber = 2;

}

//! Функция, вызываемая в прерывании по нарастающему и спадающему фронту канала датчика 3

//! В функции запоминается время таймера модуля CAP в переменную Timer1.
//! Исходя из этого считаются две переменные - p->Ts, время между двумя ближайшими фронтами,
//!необходимое для интерполятора углового положения, и переменная  p->Tspeed,
//!время между двумя фронтами одного и того же канала, неободимое для расчета скорости.
//! \memberof TDPReCAP
void DPReCAP_CAP3Calc(TDPReCAP* p) {
	Uint32 Timer;
	if (p->cnt >= 2) {
		p->HelpCalc(p);

		// В зависимости от номера предудыщего CAP определяем направление вращения.
		if (p->DPReCAP_FLG1.bit.CAPnumber == 2)
			p->DPReCAP_FLG1.bit.Dir = 0;
		if (p->DPReCAP_FLG1.bit.CAPnumber == 1)
			p->DPReCAP_FLG1.bit.Dir = 1;
		if (p->DPReCAP_FLG1.bit.CAPnumber == 3) {
			if (p->DPReCAP_FLG1.bit.PrevDir == 0)
				p->DPReCAP_FLG1.bit.Dir = 1;
			else
				p->DPReCAP_FLG1.bit.Dir = 0;
		}

		if (p->DPReCAP_FLG1.bit.Dir != p->DPReCAP_FLG1.bit.PrevDir) {
			p->DPReCAP_FLG1.bit.PrevDir = p->DPReCAP_FLG1.bit.Dir;
			p->cnt2 = 1;
		}

	    if (NT_CAP2->ECFLG_bit.CEVT0==1)
	    {
	            Timer = NT_CAP2->CAP0;
	    }
	    if (NT_CAP2->ECFLG_bit.CEVT1==1)
	    {
	            Timer = NT_CAP2->CAP1;
	    }


		// считаем периоды для расчёта угла и скорости.
		p->Ts = (Timer - p->PrevTs) * 6;
		p->PrevTs = Timer;

		p->Tspeed = Timer - p->PrevTspeed3;
		p->PrevTspeed3 = Timer;

	} else {
		p->Ts = 0;
		p->Tspeed = 0;
	}


	p->cnt++;
	p->milsec = 0;
	p->DPReCAP_FLG1.bit.CAPnumber = 3;
}

void DPReCAP_calc_10k(TDPReCAP* p) {
	p->milsec++;
	DINT;
	if (p->CAPCalcEna1==0){
		p->CAPCalcEna1=1;
		NT_CAP0->ECCLR_bit.CEVT0 = 1;
		NT_CAP0->ECCLR_bit.CEVT1 = 1;
		NT_CAP0->ECCLR_bit.INT = 1;
	}
	if (p->CAPCalcEna2==0){
		p->CAPCalcEna2=1;
		NT_CAP1->ECCLR_bit.CEVT0 = 1;
		NT_CAP1->ECCLR_bit.CEVT1 = 1;
		NT_CAP1->ECCLR_bit.INT = 1;
	}
	if (p->CAPCalcEna3==0){
		p->CAPCalcEna3=1;
		NT_CAP2->ECCLR_bit.CEVT0 = 1;
		NT_CAP2->ECCLR_bit.CEVT1 = 1;
		NT_CAP2->ECCLR_bit.INT = 1;
	}
	EINT;

}


void DPReCAP_HelpCalc(TDPReCAP* p) {
	// Обнуляем скорость и выставляем флаг,
	// если время между двумя событиями больше заданного.
	if (p->milsec > p->milsecFIX) {
		p->speed = 0;
		p->DPReCAP_FLG1.bit.ZeroFLG = 1;
	} else {
		p->DPReCAP_FLG1.bit.ZeroFLG = 0;
	}
}

void DPReCAP_SlowCalc(TDPReCAP* p) {
//формула для расчета T=Ts/Tfiltra где - Tfiltra постоянная времени фильтра
//	p->AngleFilter_1_T=_IQdiv(FAST_CALC_TS,AngleFilterT);
}


void DPReCAP_msCalc(TDPReCAP* p) {
	p->ErrorLevelTimeCounterBig++;
	if (p->ErrorLevelTimeCounterBig > 10000) {
		p->ErrorLevel = p->ErrorLevelCounter;
		p->ErrorLevelCounter = 0;
		p->ErrorLevelTimeCounterBig = 0;
	}

	p->ErrorLevelTimeCounter++;
	if (p->ErrorLevelTimeCounter > 1000) {
		if (p->WrongCodeCounterPerSec > p->WrongCodeCounterLimitPerSec){
			p->SensorFault = 1;
		} else
			p->SensorFault = 0;
		p->WrongCodeCounterPerSec=0;
		p->ErrorLevelTimeCounter = 0;
	}


	if (p->CAP_WrongEdgeCntPrev != p->CAP_WrongEdgeCnt)
		p->ErrorLevelCounter++;
	p->CAP_WrongEdgeCntPrev = p->CAP_WrongEdgeCnt;

}

/*@}*/

