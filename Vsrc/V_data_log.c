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

 \file      v_data_log.c
 \brief     4-х канальный логгер для осциллографирования в реальном времени TDataLog (см. TDataLog)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 */

/** \addtogroup v_data_log */
/*@{*/

#include "DSP.h"
#include "co_ODvars.h"
#include "V_data_log.h"
#include "CANOpen_drv.h"
#include "build.h"

//! Фоновый обработчик.

/*! работает в фоне. Нужно для установки адресов переменных через CAN.
 если нет CAN, можно записать нужные адреса вручную в TDataLog.dlog_iptr  */
//! \memberof TDataLog
void dlog_background_analizer(TDataLog *p) {
	//перевод длины предыстории из % в точки
	p->trig_shift_int = (p->trig_shift&0xFFFFFF)>>16;	//делаем количество точек
	if (p->trig_shift_int > 255)	//ограничим
		p->trig_shift_int = 255;

	TCo_OdVars *pco_vars = &co2_vars; //с каким словарем работать. От какого CAN

	unsigned char mode;

	//если не стоит хоть один из первых шести бит control,
	//то ничего анализировать и делать не надо
	if (!(p->control & 0x3F))
		return;
	//В любом случае перед всеми изменениями
	//тормозим даталоггер. Иначе он может начать записывать данные по прерыванию,
	//пока тут, в фоне, идет, скажем, запись адресов.
	//И тогда в лучшем случае будет каша из данных, в худшем - останов контроллера
	//при обращении по неправильному адресу.
	p->mode_reset = 0;
	p->valid_points_num = 0;	//т.к. даталоггер останавливался (mode = 0), то предыстория неактуальна, т.к.
								//1) за время стопа даталоггера между записанной предысторией и новыми данными будет разрыв по времени и
								//2) выбранные записываемые переменные вообще могли смениться и предыстория останется от старых, а новые данные - от новых
	//спрашиваем адреса у драйвера CAN_OPEN
	if (p->control & 1)
		if (!(p->dlog_iptr1 = (type_data*) co_getAddr(pco_vars, p->ind_subind1))) //если адрес=0
			p->dlog_iptr1 = (type_data*) &p->ind_subind1; //пусть указывает на ind_subind
	if (p->control & 2)
		if (!(p->dlog_iptr2 = (type_data*) co_getAddr(pco_vars, p->ind_subind2)))
			p->dlog_iptr2 = (type_data*) &p->ind_subind2;
	if (p->control & 4)
		if (!(p->dlog_iptr3 = (type_data*) co_getAddr(pco_vars, p->ind_subind3)))
			p->dlog_iptr3 = (type_data*) &p->ind_subind3;
	if (p->control & 8)
		if (!(p->dlog_iptr4 = (type_data*) co_getAddr(pco_vars, p->ind_subind4)))
			p->dlog_iptr4 = (type_data*) &p->ind_subind4;
	p->WriteDelimiter = (p->control >> 16) & 0xFF; //прореживание. Ограничивается 8ю битами
	mode = (p->control >> 4) & 3; //режим - 2 бита
	p->control = 0; //все обработали, обнуляем, чтобы при след. вызове не начать обрабатывать снова
	dlog_set_mode(mode, p); //для установки режима вызывается спец. функция. Напрямую нельзя

}

//! функция для установки mode_reset.

//! предполагается для управления логгером из других модулей
//! проверяет mode_reset на валидность и обнуляет счетчики
//! \memberof TDataLog
void dlog_set_mode(Uint16 mode_reset, TDataLog *p) {
	if (mode_reset > 3)
		return;
	//в 1 переводим только если текущий режим 2, чтоб синхронизироваться
	//по первому событию (очень полезно когда событий много)
	if (p->OneShotOperation == 1) {
		if ((mode_reset == 1)) {
			if (p->mode_reset == 2)
				p->mode_reset = mode_reset;
			else
				return;
		}
	} else
		p->mode_reset = mode_reset;

	if (mode_reset == 3)	//Хотим перейти в режим однократной записи 1024 точек.
		p->Wcounter = 0;	//в этом режиме нужно писать с первой точки первого массива. Сбрасываем счетчик.
	//при смене режима в любом случае сбрасываем флаг "данные готовы"
	p->control = ~((~p->control) | 64);
	p->WriteDelimiterCounter = 0;
}

//! Записывает очередные точки в массивы осциллограммы.

//!Должна вызываться с заданной дискретизацией и занимается записью массивов,
//! т.е. непосредственно осциллографированием. Умеет записывать данные
//! в массивы по-разному в зависимости от выбранного режима работы осциллографа
//! \memberof TDataLog
void data_log_update(TDataLog *p) {
//везде испоьзуется p-> , а не p* - так быстрее

//прореживание данных. Если не достигли уставки WriteDelimiter, выходим
//WriteDelimiter=1 - не делим
//WriteDelimiter=2 - берем каждую вторую
	if ((p->WriteDelimiterCounter++ + 1) < p->WriteDelimiter)
		return;
	else
		p->WriteDelimiterCounter = 0;

	if (p->mode_reset != p->mode_reset_prev) p->E=1;
    else p->E=0;
	p->mode_reset_prev = p->mode_reset;

	switch (p->mode_reset) {
	case 0: {
		p->Wcounter = 0;
		return;
	}

	case 1:  //однократная запись - дошли до конца и стоп (режим 0)
	{
		//При входе в режим получаем точку срабатывания триггера (это текущий Wcounter, с которым мы сюда зашли) и рассчитываем первую точку, относящуюся к данной осце (исходя из текущей точки и заданной длины предыстории)
        if (p->E==1)
        {
        	p->prehistory_length = p->valid_points_num;		//длина предыстории равна количеству валидных точек
        	if (p->prehistory_length > p->trig_shift_int)	//если предыстория длиннее заданного, ограничим
        		p->prehistory_length = p->trig_shift_int;
        	p->Wcounter &= 0xFE;
        	p->prehistory_length &= 0xFE;
        	p->first_point_written = (p->Wcounter - p->prehistory_length) & 0xFE;        	                            //передачи попарно. +1 - чтобы предыстория получилась на 1 точку короче, а не длиннеее (мало ли что там в этой лишней точке лежит...).
        }
		p->Wcounter &= 0xFF; //защита - если вдруг в counter не валидное значение
		p->dl_buffer1_adr[p->Wcounter] = *p->dlog_iptr1;
		p->dl_buffer2_adr[p->Wcounter] = *p->dlog_iptr2;
		p->dl_buffer3_adr[p->Wcounter] = *p->dlog_iptr3;
		p->dl_buffer4_adr[p->Wcounter] = *p->dlog_iptr4;
		p->Wcounter++;
		p->Wcounter &= 0xFF;  //если прошли 256 точек, обнулится
		//if (p->Wcounter == 0)  //если ноль,
		if (p->Wcounter == p->first_point_written)  //если дошли до последней записываемой точки
		{
			p->mode_reset = 0;                 //режим СТОП

			if (p->BlockTransferSupportFlag)
				p->control |= 192; //флаг "данные готовы"+бит поддержки блочной передачи
			else
				p->control|=64; //флаг "данные готовы"
		}
		return;
	}

	case 2: //записываем лог по кругу
	{
		p->valid_points_num++;	//считаем количество записанных валидных точек предыстории
		if (p->valid_points_num > 256)	//ограничиваем
			p->valid_points_num = 256;
		p->Wcounter &= 0xFF; //защита - если вдруг в counter не валидное значение
		p->dl_buffer1_adr[p->Wcounter] = *p->dlog_iptr1;
		p->dl_buffer2_adr[p->Wcounter] = *p->dlog_iptr2;
		p->dl_buffer3_adr[p->Wcounter] = *p->dlog_iptr3;
		p->dl_buffer4_adr[p->Wcounter] = *p->dlog_iptr4;
		p->Wcounter++;
		p->Wcounter &= 0xFF;  //если прошли 256 точек, обнулится
		return;
	}

	case 3: //режим однократной записи 1024 точек
	{
        if (p->E==1)
        {
        	p->first_point_written = 0;		//пишем с первой точки, предыстория не нужна.
        }
		//с учетом того, что буферы в памяти расположены последовательно,
		//записываем в первый, "заезжая" на остальные три
		p->dl_buffer1_adr[p->Wcounter] = *p->dlog_iptr1;
		p->Wcounter++;
		if (p->Wcounter >= 1024)  //если дошли до конца
		{
			p->mode_reset = 0;                 //режим СТОП
			if (p->BlockTransferSupportFlag)
				p->control |= 192; //флаг "данные готовы"+бит поддержки блочной передачи
			else
				p->control|=64; //флаг "данные готовы"
		}
		return;
	}
	}

}

/*@}*/

