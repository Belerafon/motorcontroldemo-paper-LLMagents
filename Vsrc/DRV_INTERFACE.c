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

 \file      DRV_INTERFACE.c
 \brief     „ерез данные модуль обеспечивается работа с банком аварий (чтение, очистка).
 //ѕредполагается, что модуль может расширять функциональность (скачивать каие-то другие данные)...
 //ѕ–»ћ≈„јЌ»≈ дл€ программиста:
  	1.дл€ команд работающих по сети с автоматизированными системами (ёникон,, пульты и т.д.) после выполнени€ команды поле ans_data должно быть установлено значением запроса:
  	  	  p->ans_data = temp;
  	  что сообщит внешней системе, что запрос обработан. ¬ этом случае поле p->ans_data обнул€етс€ внешней автоматизированной системой после сбора информации с полей
  	     p->data_Low
		  p->data_High
	2.дл€ внутренних вызовов поле p->ans_data трогать не нужно
 \author    ќќќ "Ќѕ‘ ¬ектор". http://motorcontrol.ru
 \version   v 2.1 2017_07_24

 */


#include "main.h"
void DRV_INTERFACE_Calc(TDrvInterface *p) {
	TEvent event;

	long temp = (p->req_data); //принятие запроса

	p->INTERFACE_delta_time_sec = global_time.relative_time1.second_counter - p->INTERFACE_time_sec;
	// проверка "ухода" ответа
	if ((p->ans_data != 0) && (p->INTERFACE_delta_time_sec < DRV_INTERFACE_RESET_TIME))
		return;

	//раз сюда попали значит либо таймаут для анса истек, либо он нулевой
	//в любом случае его обнуляем
	p->ans_data = 0;
	//запрос обнуляем только после выполнения, чтобы внешние устройства могли контролить выполнение запроса
	switch ((temp) >> 16)
	{
		case DATA_REQUEST_READ_FAULT:  //запрос на чтение ошибки
		{
			FaultLog.read(&FaultLog, (temp & 0x0FFFF), &event);
			//формируем ответ
			p->data_Low = (((Uint32) (event.ev_num)) << 16) + event.ev_code;
			p->data_High = event.ev_time;

			p->ans_data = temp; //говорим ёникону, что команда обработана
			//фиксируем время ответа
			p->INTERFACE_time_sec = global_time.relative_time1.second_counter;
			break;
		}
		case DATA_REQUEST_WRITE_FAULT:  //запрос на запись ошибки
		{
			//!!!
			//пока через интерфейс писать ошибку нельзя
			break;
		}
		case DATA_REQUEST_WRITE_WORK_TIME:  //запрос на запись work_time
		{
			global_time.relative_time1.relative_date.hour = 0;
			global_time.relative_time1.relative_date.minute = 0;
			global_time.WtiteSPI_flag = 1;

			break;
		}
		case DATA_REQUEST_READ_WORK_TIME:  //запрос на чтение work_time
		{
			//Ma][: Ѕу!
			break;
		}
		case DATA_REQUEST_WRITE_OPER_TIME: {  //запрос на запись oper_time
			global_time.relative_time2.relative_date.hour = 0;
			global_time.relative_time2.relative_date.minute = 0;
			global_time.WtiteSPI_flag = 1;

			break;
		}
		case DATA_REQUEST_READ_OPER_TIME:  //запрос на чтение ошибки
		{
			//Ma][: Ѕу!
			break;
		}
		case DATA_REQUEST_CLEAR_ALL:  //запрос на очистку банка аварий
		{
			FaultLog.clear(&FaultLog);
			//формируем ответ
			p->data_High = 0;
			p->data_Low = 0;

			p->ans_data = temp; //говорим ёникону, что команда обработана
			//фиксируем время ответа
			p->INTERFACE_time_sec = global_time.relative_time1.second_counter;
			break;
		}
	}

	p->req_data = 0;//обнуляем поле запроса, чтобы не выполнить его повторно
}

