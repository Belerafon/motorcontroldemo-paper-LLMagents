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
 
 \file      V_adc.c
 \brief     Модуль обработки АЦП (см. TAdcDrv)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \addtogroup V_adc 
 @{*/

#include "DSP.h"
#include "V_adc.h"
#include "main.h"

//!Инициализация.

//! Настраиваются секвенсоры и тип оцифровки данных. каналы для оцифровки, частота.
//! В данном случае оцифровка происходит по событию таймера,частота которого
//! равна частоте ШИМ, чтобы измерять ровно посередине периода ШИМ (требуется для шунтовых
//! датчиков тока)
//Запуск оцифровки производится по таймеру NT_PWM3, который синхронизирован с другими таймерами ШИМ.
//Оцифрованные измерения складываются в ФИФО. Каждый канал АЦП настроен на отдельный секвенсор с ФИФО длиной до 16 измерений.
//В прерывании с частотой 10 кГц данные забираются из ФИФО и складываются в кольцевые массивы (старые затираются новыми).
//В том же прерывании из кольцевого массива берется N последних точек и из них вычисляется среднее арифметическое.
//Число выборок для усреднения N зависит от частоты ШИМ и равна Fшим/10кГц. Если частоты ШИМ меньше 10 кГц, то берется одна выборка, если больше 40 кГц - 4 выборки.
//К вычисленному среднему значению добавляется калибровочный сдвиг, и оно умножается на коэффициент измерения, в результате получается окончательный результат измерения.

//! \memberof TAdcDrv
void AdcDrv_init(TAdcDrv *p) {
	// Запрет всех секвенсоров
	NT_ADC->ACTSS = 0;

	// Сброс счётчиков прерываний и измерений
	NT_ADC->SEQ[0].OP = 0;
	NT_ADC->SEQ[1].OP = 0;
	NT_ADC->SEQ[2].OP = 0;
	NT_ADC->SEQ[3].OP = 0;
	NT_ADC->SEQ[4].OP = 0;
	NT_ADC->SEQ[5].OP = 0;
	NT_ADC->SEQ[6].OP = 0;
	NT_ADC->SEQ[7].OP = 0;

	//Включаем 2, 3 и 4 секвенсоры, по одному на измеряемую величину (длина фифо каждого - 16 ячеек)
	//Примечание – Рекомендуется использовать только один секвенсор, либо несколько, но работающих синхронно (одинаковые события запуска, одинаковые задержки перезапуска измерений).
	NT_ADC->ACTSS_bit.ASEN2 = 1;
	NT_ADC->ACTSS_bit.ASEN3 = 1;
	NT_ADC->ACTSS_bit.ASEN4 = 1;
	//Ставим маску прерывания (для 4 секвенсора, нам не надо 3 прерывания, достаточно одного)
	NT_ADC->IM_bit.MASK4 = 1;
	//Настраиваем источник запуска
	NT_ADC->EMUX_bit.EM2 = 8; //запуск производится по команде SocA  Блока ШИМ 3 для vector card
	NT_ADC->EMUX_bit.EM3 = 8; //запуск производится по команде SocA  Блока ШИМ 3 для vector card
	NT_ADC->EMUX_bit.EM4 = 8; //запуск производится по команде SocA  Блока ШИМ 3 для vector card
	NT_ADC->PSSI_bit.SS2 = 1; //разрешить программный запуск секвенсора
	NT_ADC->PSSI_bit.SS3 = 1; //разрешить программный запуск секвенсора
	NT_ADC->PSSI_bit.SS4 = 1; //разрешить программный запуск секвенсора

	//Настраиваем количество перезапусков секвенсора
	NT_ADC->SEQ[2].CTL_bit.RCNT = 0; //Перезапуск не требуется
	NT_ADC->SEQ[3].CTL_bit.RCNT = 0; //Перезапуск не требуется
	NT_ADC->SEQ[4].CTL_bit.RCNT = 0; //Перезапуск не требуется
	//Настраиваем частоту прерываний
	NT_ADC->SEQ[2].CTL_bit.ICNT = 0; //Прерывания после каждого запуска
	NT_ADC->SEQ[3].CTL_bit.ICNT = 0; //Прерывания после каждого запуска
	NT_ADC->SEQ[4].CTL_bit.ICNT = 0; //Прерывания после каждого запуска
	//NT_ADC->SEQUENCER.SEQ0.ADCSSTMR.bit.TMR

	NT_ADC->PP_bit[0].OM = 0x3; //OM2-OM0 = ’011’ All blocks active (ADC plus internal buffers plus internal bandgap).
	NT_ADC->PP_bit[1].OM = 0x3;
	NT_ADC->PP_bit[2].OM = 0x3;
	NT_ADC->PP_bit[3].OM = 0x3;
	NT_ADC->PP_bit[4].OM = 0x3;
	NT_ADC->PP_bit[5].OM = 0x3;
	NT_ADC->PP_bit[6].OM = 0x3;
	NT_ADC->PP_bit[7].OM = 0x3;
	NT_ADC->PP_bit[8].OM = 0x3;
	NT_ADC->PP_bit[9].OM = 0x3;
	NT_ADC->PP_bit[10].OM = 0x3;
	NT_ADC->PP_bit[11].OM = 0x3;

	//Включаем необходимые модули АЦП
	//Настраиваем разрешающую способность модуля АЦП
	NT_ADC->PP_bit[0].ENA = 1;	//Включаем модуль АЦП
	NT_ADC->PP_bit[1].ENA = 1;
	NT_ADC->PP_bit[2].ENA = 1;
	NT_ADC->PP_bit[3].ENA = 1;
	NT_ADC->PP_bit[4].ENA = 1;
	NT_ADC->PP_bit[5].ENA = 1;
	NT_ADC->PP_bit[6].ENA = 1;
	NT_ADC->PP_bit[7].ENA = 1;
	NT_ADC->PP_bit[8].ENA = 1;
	NT_ADC->PP_bit[9].ENA = 1;
	NT_ADC->PP_bit[10].ENA = 1;
	NT_ADC->PP_bit[11].ENA = 1;

#ifdef HW_VECTORCARD_DRV8301_EVM
	// Настраиваем компараторы каждый на свой канал АЦП -
	// из компараторов будем забирать оцифрованные данные, как из ADC result
	//используем аналоговые сигналы токов EXT_IA-FB и EXT_IB-FB (с внешних операционников), так как из-за неудачной разводки
	//кита тексас на канале IA-FB при высокоимпедансном входе (как в VectorCard) есть большая помеха от DCDC, которая на ките делает 5В
	NT_ADC->DCCTL_bit[2].CHNL = 2;	//Udc
	NT_ADC->DCCTL_bit[6].CHNL = 6;	//Ток фазы B
	NT_ADC->DCCTL_bit[14].CHNL = 14;	//Ток фазы A

	//разрешение работы компараторов
	NT_ADC->SEQ[2].DCP_bit.CMP2 = 1;	//Udc
	NT_ADC->SEQ[3].DCP_bit.CMP6 = 1;	//Ток фазы B
	NT_ADC->SEQ[4].DCP_bit.CMP14 = 1;	//Ток фазы A

	//выбор каналов для измерений
	NT_ADC->SEQ[2].MUX_bit.CH2 = 1;	//Udc
	NT_ADC->SEQ[3].MUX_bit.CH6 = 1;	//Ток фазы B
	NT_ADC->SEQ[4].MUX_bit.CH14 = 1;	//Ток фазы A
#endif

#ifdef HW_VECTORCARD_DRV8312_EVM
	// Настраиваем компараторы каждый на свой канал АЦП -
	// из компараторов будем забирать оцифрованные данные, как из ADC result
	//используем аналоговые сигналы токов EXT_IA-FB и EXT_IB-FB (с внешних операционников), так как из-за неудачной разводки
	//кита тексас на канале IA-FB при высокоимпедансном входе (как в VectorCard) есть большая помеха от DCDC, которая на ките делает 5В
	NT_ADC->DCCTL_bit[2].CHNL = 2;	//Udc
	NT_ADC->DCCTL_bit[5].CHNL = 5;	//Ток фазы B
	NT_ADC->DCCTL_bit[9].CHNL = 9;	//Ток фазы A

	//разрешение работы компараторов
	NT_ADC->SEQ[2].DCP_bit.CMP2 = 1;	//Udc
	NT_ADC->SEQ[3].DCP_bit.CMP5 = 1;	//Ток фазы B
	NT_ADC->SEQ[4].DCP_bit.CMP9 = 1;	//Ток фазы A

	//выбор каналов для измерений
	NT_ADC->SEQ[2].MUX_bit.CH2 = 1;	//Udc
	NT_ADC->SEQ[3].MUX_bit.CH5 = 1;	//Ток фазы B
	NT_ADC->SEQ[4].MUX_bit.CH9 = 1;	//Ток фазы A
#endif

//для Контроллера МК40.4
#ifdef HW_VECTOR_MK_40_4
	NT_ADC->DCCTL_bit[1].CHNL = 1;	//Udc
	NT_ADC->DCCTL_bit[17].CHNL = 17;	//Ток фазы B
	NT_ADC->DCCTL_bit[16].CHNL = 16;	//Ток фазы A

	//разрешение работы компараторов
	NT_ADC->SEQ[2].DCP_bit.CMP1 = 1;	//Udc
	NT_ADC->SEQ[3].DCP_bit.CMP17 = 1;	//Ток фазы B
	NT_ADC->SEQ[4].DCP_bit.CMP16 = 1;	//Ток фазы A

	//выбор каналов для измерений
	NT_ADC->SEQ[2].MUX_bit.CH1 = 1;	//Udc
	NT_ADC->SEQ[3].MUX_bit.CH17 = 1;	//Ток фазы B
	NT_ADC->SEQ[4].MUX_bit.CH16 = 1;	//Ток фазы A
#endif
}


//!Расчет АЦП с частотой основного расчета всей системы управления (обычно 10кГц).

//! Занимается обработкой измеренных АЦП значений и преобразует в формат IQ24.
//! Токи фаз для повышения точности усредняются за несколько измерений

//! \memberof TAdcDrv
void AdcDrv_fast_calc(TAdcDrv *p) {

#if (!defined(HW_VECTORCARD_SIMULATOR)) && (!defined(HW_NIIET_BOARD_SIMULATOR))
	while (NT_ADC->SEQ[2].FSTAT_bit.FLOAD > 0)	//пока фифо не опустеет
	{
		//несколько точек из фифо измерений одного и того же
		p->UdcBuf[p->UdcPointer] = NT_ADC->SEQ[2].FIFO_bit.DATA;
		p->UdcPointer = (++p->UdcPointer) & 3;
	}
	while (NT_ADC->SEQ[3].FSTAT_bit.FLOAD > 0)	//пока фифо не опустеет
	{
		p->IBBuf[p->IBPointer] =
				(int16) ((NT_ADC->SEQ[3].FIFO_bit.DATA) << 4) ^ 0x8000;
		p->IBPointer = (++p->IBPointer) & 3;
	}
	while (NT_ADC->SEQ[4].FSTAT_bit.FLOAD > 0)	//пока фифо не опустеет
	{
		p->IABuf[p->IAPointer] =
				(int16) ((NT_ADC->SEQ[4].FIFO_bit.DATA) << 4) ^ 0x8000;
		p->IAPointer = (++p->IAPointer) & 3;
	}

	p->Udc_temp = 0;
	p->IB_temp = 0;
	p->IA_temp = 0;

	//усреднялка
	int i;
	for (i = 0; i < p->IASampleLength; i++) {//берем нужное число точек, начиная с самой новой
		int j = p->IAPointer - i;
		if (j < 0)	//если перешли через 0 - начинаем с 3-й (массив из 4 точек)
			j = 4 + j;
		p->IA_temp += p->IABuf[j];
	}
	//делим на длину выборки (1-4 точки)
	p->IA_temp = p->IA_temp / p->IASampleLength;

	for (i = 0; i < p->IBSampleLength; i++) {//берем нужное число точек, начиная с самой новой
		int j = p->IBPointer - i;
		if (j < 0)	//если перешли через 0 - начинаем с 3-й (массив из 4 точек)
			j = 4 + j;
		p->IB_temp += p->IBBuf[j];
	}
	//делим на длину выборки (1-4 точки)
	p->IB_temp = p->IB_temp / p->IBSampleLength;

	for (i = 0; i < p->UdcSampleLength; i++) {//берем нужное число точек, начиная с самой новой
		int j = p->UdcPointer - i;
		if (j < 0)	//если перешли через 0 - начинаем с 3-й (массив из 4 точек)
			j = 4 + j;
		p->Udc_temp += p->UdcBuf[j];
	}
	//делим на длину выборки (1-4 точки), получая колечную величину аналогового сигнала
	p->Udc_temp = p->Udc_temp / p->UdcSampleLength;
#endif
#ifdef HW_VECTORCARD_DRV8312_EVM
	//комплект HW_VECTORCARD_DRV8312_EVM отличается от HW_VECTORCARD_DRV8301_EVM в 5 раз меньшим коэффициентом усиления и другим знаком
	p->Imeas_a = p->IaGainNom * ((-(p->IA_temp/5) + p->Imeas_a_offset));
	p->Imeas_b = p->IbGainNom * ((-(p->IB_temp/5) + p->Imeas_b_offset));
	p->Udc_meas = p->UdcGainNom * (p->Udc_temp + p->Udc_meas_offset);
	p->Imeas_c = -p->Imeas_a - p->Imeas_b;
#endif
#if defined(HW_VECTORCARD_DRV8301_EVM) || defined(HW_VECTOR_MK_40_4)
	p->Imeas_a = p->IaGainNom * ((p->IA_temp + p->Imeas_a_offset));
	p->Imeas_b = p->IbGainNom * ((p->IB_temp + p->Imeas_b_offset));
	p->Udc_meas = p->UdcGainNom * (p->Udc_temp + p->Udc_meas_offset);
	p->Imeas_c = -p->Imeas_a - p->Imeas_b;
#endif


#if (defined(HW_VECTORCARD_SIMULATOR)) || (defined(HW_NIIET_BOARD_SIMULATOR))
	//Симулятор формирует значения АЦП в диапазоне 0-4095
	p->IA_temp=(int16) ((model.adc_code_iA<<4) ^ 0x8000);
	p->IB_temp=(int16) ((model.adc_code_iB<<4) ^ 0x8000);
	p->IC_temp=(int16) ((model.adc_code_iC<<4) ^ 0x8000);
	p->ID_temp=(int16) ((model.adc_code_iD<<4) ^ 0x8000);
	p->Udc_temp=model.adc_code_Udc;

	p->Imeas_a = p->IaGainNom * ((p->IA_temp + p->Imeas_a_offset));
	p->Imeas_b = p->IbGainNom * ((p->IB_temp + p->Imeas_b_offset));
	if ((model.MotorType==MODEL_INDUCTION_MOTOR) || (model.MotorType==MODEL_SYNC_MOTOR)){//тип двигателя трехфазный синхронный или асинхронный
		p->Imeas_c = -p->Imeas_a - p->Imeas_b;//ток считаем по закону кирхгофа
	}
	else{
		p->Imeas_c = p->IcGainNom * ((p->IC_temp + p->Imeas_c_offset));
	}
	p->Imeas_d = p->IdGainNom * ((p->ID_temp + p->Imeas_d_offset));
	p->Udc_meas = p->UdcGainNom * (p->Udc_temp + p->Udc_meas_offset);

#endif

}

//!Медленный расчет.

//!Занимается пересчетом коэффициентов, используемых в скоростной функции расчета,
//!чтобы не занимать процессорное время там. Находит коэффициенты,
//!на которые надо умножить полученное с АЦП значение, чтобы получить
//!относительные единицы с заданной базой.

//! Пример   p->PvalveGainNom=_IQ16mpy(p->Pvalve_gain,_IQ(1.0/100));
//! Pvalve_gain - значение в формате int. задается пользователем в UniCON.
//!Определяет, скольким процентам соответствует полный диапазон АЦП. В простейшем случае
//!равен 100. Т.е. когда на АЦП максимальный сигнал, это соответствует 100%.

//!_IQ(1.0/100) обратное значение для масштабирующего коэффициента. Так как величину процентов
//!естественнее всего перевести в относительные единицы так, что 100% соответствут 1.0,
//!то масштабирующий коэффициент (база) равен 100. Т.е. UniCON, прочитав из системы управления
//! число 1.0 в формате 8.24 должен умножить его на 100, чтобы отобразились проценты.
//! Здесь коэффициент задан явно как 1.0/100, но для ряда случаев базовое значение нужно менять.
//!Так, для токов фаз используется значение _1_I_nom, в формате 8.24, соответствующее единице деленной на
//базовое значение тока, например, 200 А. Так как в зависимости от мощности преобразователя базовый ток может меняться,
//то это значение, в отличие от процентов, сделано настраиваемым. Расчет _1_I_nom идет в другом месте, так как
//занимает много тактов расчета.

//Для беззнакового значения АЦП измеряет число от 0 до 65535. (16 разрядов, где заполнены верхние).
//Для примера с процентами необходимо сделать так, чтобы получилось результирующее значение в формате 8.24,
//где 1.0 это 65535. Таким образом, нужно сдвинуть число 65535 на 24-16=8 разрядов.
//Сдвиг на 8 разрядов - это умножение на число 255. Число 255 - это 1.0 в формате 24.8.

//Таким образом, PvalveGainNom - это коэффициент в формате 24.8. он получается в результате использования функции
//_IQ16mpy, аргумены которой Pvalve_gain (int) и 1.0/100 в формате 8.24. Функция IQ множения
//по сути представляет собой обычное оуможение в 64 разрядах со сдвигом результата вправо на Q разрядов.
//Т.е. _IQ16mpy умножает число в формате IQ24 9второй аргумент) на целочисленный коэффициент (первый аргумент),
//а потом сдвигате результат на 16 разрядов вправо.
//Так, в результате _IQ16mpy(p->Pvalve_gain,_IQ(1.0/100)); получается целочисленное число 255, являющейся
//1.0 в формате 24.8 из-за сдвига на 16 разрядов вправо.

//Всё вышеприведенное мутево сделано с одной целью - увеличить производитлеьность обработки АЦП.

//! \memberof TAdcDrv
void AdcDrv_slow_calc(TAdcDrv *p) {
	p->IaGainNom = _IQ16mpy(p->Imeas_a_gain, drv_params._1_I_nom) << 1;
	p->IbGainNom = _IQ16mpy(p->Imeas_b_gain, drv_params._1_I_nom) << 1;
	p->IcGainNom = _IQ16mpy(p->Imeas_c_gain, drv_params._1_I_nom) << 1;
	p->IdGainNom = _IQ16mpy(p->Imeas_d_gain, drv_params._1_I_nom) << 1;
	p->UdcGainNom = _IQ16mpy(p->Udc_meas_gain, drv_params._1_Udc_nom) << 4;
}

//! Миллисекундный расчет

//! \memberof TAdcDrv
void AdcDrv_ms_calc(TAdcDrv *p) {

}

/*@}*/

