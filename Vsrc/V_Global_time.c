
/*!
     Copyright 2017 	јќ "Ќ»»Ё“" и ќќќ "Ќѕ‘ ¬≈ “ќ–"

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 \file      Global_time.c
 \brief     ћодуль работы со временем
 \author    ќќќ "Ќѕ‘ ¬ектор". http://motorcontrol.ru
 \version   v 1.2 2013_10_16

*/

/** \addtogroup Global_time */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif


#include <main.h>


//!»нициализация модуля

//!≈сли есть часы реального времени, считывает из них время.
//!¬осстанавливает время работы и времени включенного состояния из SPI.

//! \memberof TGlobalTime
void GlobalTime_init(TGlobalTime *p)
{
	p->read_PowerOn_time(p);
	p->read_oper_time(p);


	#ifdef USE_ABSOLUTE_TIME_CLOCK
	//инициализация абсолютного времени
	if (p->time_type == 1)
	{
		//инициализация из часов реального времени
		p->absolute_time.day=spiclock.day;
		p->absolute_time.hour=spiclock.hour;
		p->absolute_time.minute=spiclock.minute;
		p->absolute_time.second=spiclock.second;
		p->absolute_time.year=spiclock.year;
	}
	#endif
	//инициализация относительного времени
	p->relative_time1.ON = 1;//т.к. 1-е время является обязательным

	p->relative_time1.relative_date.hour = (p->PowerOn_time >> 6);
	p->relative_time1.relative_date.minute = (p->PowerOn_time & 0x3f);

	p->relative_time2.relative_date.hour = (p->operational_time >> 6);
	p->relative_time2.relative_date.minute = (p->operational_time & 0x3f);
}




//!–асчет относительного времени.

//!¬ызывает функции расчета относительно времени и, если есть, считывает время
//!с модуля часов реального вермени в абсолютное время.

//! \memberof TGlobalTime
void GlobalTime_calc(TGlobalTime *p)
{
	//счет относительных времен
	GlobalTime_REL_TIME_calc(&p->relative_time1);
	GlobalTime_REL_TIME_calc(&p->relative_time2);
}


//!–асчет относительного времени.

//!¬ызывает функции расчета относительно времени и, если есть, считывает время
//!с модуля часов реального вермени в абсолютное время.

//! \memberof TGlobalTime
void GlobalTime_ms_calc(TGlobalTime *p) {

	//управление счетом operational_time
	if (sm_ctrl.state != 0) //≈сли в каком-то режиме работы
		global_time.relative_time2.ON = TRUE;
	else
		global_time.relative_time2.ON = FALSE;

	//¬ремя запитанного состояния в минутах
	p->PowerOn_time = (global_time.relative_time1.relative_date.hour << 6) + (global_time.relative_time1.relative_date.minute & 0x3f);
	//¬ремя в работе, в минутах
	p->operational_time = (global_time.relative_time2.relative_date.hour << 6) + (global_time.relative_time2.relative_date.minute & 0x3f);

	p->PowerOn_time_min = p->relative_time1.relative_date.hour*60 +p->relative_time1.relative_date.minute;
	p->operational_time_min = p->relative_time2.relative_date.hour*60 + p->relative_time2.relative_date.minute;


#ifdef USE_ABSOLUTE_TIME_CLOCK
	if (global_time.time_type == 1) {
		//время - из часов реального времени
		global_time.absolute_time.day = spiclock.day;
		global_time.absolute_time.hour = spiclock.hour;
		global_time.absolute_time.minute = spiclock.minute;
		global_time.absolute_time.second = spiclock.second;
		global_time.absolute_time.year = spiclock.year;
	}
#endif
}

//!–асчет относительного времени

//!”читывая, что функция вызывается с какой-то определенной дискретизацией,
//!инкрементирует относительное время, переденное ей в структуре типа RELATIVE_TIME
//! \memberof TGlobalTime
void GlobalTime_REL_TIME_calc(RELATIVE_TIME *p) {
	if (p->ON == 0)
		return;
	if (p->tic_isr != 0)
		p->tic_isr--;
	p->delta_millisecond = 0;
	p->delta_second = 0;
	if (p->tic_isr == 0) {
		//миллисекунда
		p->tic_isr = GLOBAL_TIME_CALC_FREQ;
		p->millisecond_counter++;
		p->delta_millisecond = 1;
		if (p->relative_date.millisecond == 999) {
			//секунда
			p->relative_date.millisecond = 0;
			p->second_counter++;
			p->delta_second = 1;
			if (p->relative_date.second == 59) {
				//минута
				p->relative_date.second = 0;
				if (p->relative_date.minute == 59) {
					//час
					p->relative_date.minute = 0;
					p->relative_date.hour++;
				} else
					p->relative_date.minute++;
			} else
				p->relative_date.second++;
		} else
			p->relative_date.millisecond++;
	}
}


//!„итает время наработки.

//! \memberof TCurPar
void GlobalTime_read_PowerOn_time(TGlobalTime *p) { //чтение work_time
	UserMem.MemStartAddr = GLOBAL_TIME_POWER_ON_TIME_ADDR;
	UserMem.MCUStartAddr = (Uint16*) (&(p->PowerOn_time));
	UserMem.data_length = 4;
	UserMem.read(&UserMem);
}


//!записывает время наработки.

//! \memberof TCurPar
void GlobalTime_write_PowerOn_time(TGlobalTime *p) { //запись work_time
	UserMem.MemStartAddr = GLOBAL_TIME_POWER_ON_TIME_ADDR;
	UserMem.MCUStartAddr = (Uint16*) (&(p->PowerOn_time));
	UserMem.data_length = 4;
	UserMem.write(&UserMem);
}

//!„итает время включенного сосотяния.
//! \memberof TCurPar
void GlobalTime_read_oper_time(TGlobalTime *p) { //чтение operational_time
	UserMem.MemStartAddr = GLOBAL_TIME_OPERATIONAL_TIME_ADDR;
	UserMem.MCUStartAddr = (Uint16*) (&(p->operational_time));
	UserMem.data_length = 4;
	UserMem.read(&UserMem);
}

//!записывает время включенного сосотяния.
//! \memberof TCurPar
void GlobalTime_write_oper_time(TGlobalTime *p) { //запись operational_time
	UserMem.MemStartAddr = GLOBAL_TIME_OPERATIONAL_TIME_ADDR;
	UserMem.MCUStartAddr = (Uint16*) (&(p->operational_time));
	UserMem.data_length = 4;
	UserMem.write(&UserMem);
}


void GlobalTime_slow_calc(TGlobalTime*p) {
	//если стоит флаг на сохранение времен в SPI или с момент прошлой записи прошел час
	if ((p->WtiteSPI_flag) || (p->PrevWriteSPIHour != global_time.relative_time1.relative_date.hour)) {

		p->write_PowerOn_time(p); //запись времени вкл. состояния
		p->write_oper_time(p); //запись рабочего времени
		p->PrevWriteSPIHour = global_time.relative_time1.relative_date.hour;
		p->WtiteSPI_flag = 0;
	}
}



#ifdef __cplusplus
} // Extern "C"
#endif
/*@}*/

