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

 \file      V_event_log.c
 \brief     —охранение событий и времени возникновения в Ёнќ«”
			»спользуется драйвер работы с энергонезависимой памятью
			ѕоэтому вызов init(); должен быть после инициализации Ёнќ«”
 \author    ќќќ "Ќѕ‘ ¬ектор". http://motorcontrol.ru
 \version   v 1.0 2009_01_20

*/

/** \addtogroup W_event_log */
/*@{*/
#ifdef __cplusplus
extern "C" {
#endif

#include <main.h>


	//!<ѕротоколирование аварий

//ѕроцедуры записи событий в энергонезависимую флешку очень долгие (св€зано с таймингами I2C-интерфейса
//и большим объемом данных, передаваемым по I2C в процессе записи).
//ѕоэтому алгоритм работы такой:
//≈сли нужно прочитать аварию [Logger_Event_Read] (срочно считывать аварию из банка не требуетс€) -
//она читаетс€ в фоновом цикле пр€миком из Ёнќ«”.
//≈сли нужно записать аварию в банк [Logger_Event_Write] - в прерывании (например, 1мс) данные аварии пишутс€ в FIFO;
//затем в фоновом цикле [Logger_Background_Calc] аварии читаютс€ из FIFO [Event_FIFO_Read]
//(по одно за 1 фоновый цикл) и пишутс€ в Ёнќ«”.


//прочитать событие из FIFO (дл€ дальнейшей записи в Ёнќ«”).
Uint16 Event_FIFO_Read(TEventFIFO* p,TEvent* ev)
{
	if (p->busy_flag > 0) return EVENT_FIFO_BUSY;
	//выставляем флажок работы с FIFO
	p->busy_flag = 1;
	if (p->number_of_msgs == 0)
	{
		//освобождаем флаг работы с FIFO
		p->busy_flag = 0;
		return EVENT_FIFO_EMPTY;
	}
	//читаем данные из FIFO
	ev->ev_code = p->msg_array[(p->read_ptr)].ev_code;
	ev->ev_time = p->msg_array[(p->read_ptr)].ev_time;
	ev->ev_num = p->msg_array[(p->read_ptr)].ev_num;
	//подготовка FIFO к следующему вызову
	p->read_ptr++;
	if (p->read_ptr >= p->size) p->read_ptr = 0;
	p->number_of_msgs--;
	//освобождаем флаг работы с FIFO
	p->busy_flag = 0;
	return EVENT_FIFO_SUCCESSFUL;
}



//«аписать событие в FIFO (вызываетс€ в Logger_Event_Write, в прерывании)
Uint16 Event_FIFO_Write(TEventFIFO* p,TEvent* ev)
{
	if (p->busy_flag > 0) return EVENT_FIFO_BUSY;
	//выставляем флажок работы с FIFO
	p->busy_flag = 1;
	if (p->number_of_msgs == p->size)
	{
		//освобождаем флаг работы с FIFO
		p->busy_flag = 0;
		return EVENT_FIFO_FULL;
	}
	//записываем данные в FIFO
	p->msg_array[(p->write_ptr)].ev_code = ev->ev_code;
	p->msg_array[(p->write_ptr)].ev_time = ev->ev_time;
	p->msg_array[(p->write_ptr)].ev_num = ev->ev_num;
	//подготовка FIFO к следующему вызову
	p->write_ptr++;
	if (p->write_ptr >= p->size)p->write_ptr = 0;
	p->number_of_msgs++;
	//освобождаем флаг работы с FIFO
	p->busy_flag = 0;
	return EVENT_FIFO_SUCCESSFUL;
}

//инициализаци€ (если банк забит мусором - все данные FFFF, очищаем его)
void Logger_Init(TLogger* p,Uint16 BS,Uint16 SA,Uint32* tptr)
{
	TEvent tmp;
	Uint16 i;
	Uint16 last_log_num = 0;
	Uint16 last_log_index = 0;
	//инициализируем внутренние переменные
	p->SPI_buf_size = BS;
	p->SPI_start_addr = SA;
	p->time_ptr = tptr;

	//далее нужно просканировать буфер в Ёнќ«” - найти последнюю
	UserMem.MemStartAddr = p->SPI_start_addr;
	UserMem.data_length = 0;
	for (i=0;i<p->SPI_buf_size;i++)
	{
		//читаем номер
		UserMem.MemStartAddr += UserMem.data_length;
		UserMem.MCUStartAddr = (Uint16*)(&tmp.ev_num);
		UserMem.data_length = LOG_NUM_LENGTH;
		UserMem.read(&UserMem);
		//читаем время
		UserMem.MemStartAddr += UserMem.data_length;
		UserMem.MCUStartAddr = (Uint16*)(&tmp.ev_time);
		UserMem.data_length = LOG_TIME_LENGTH;
		UserMem.read(&UserMem);
		//читаем код
		UserMem.MemStartAddr += UserMem.data_length;
		UserMem.MCUStartAddr = (Uint16*)(&tmp.ev_code);
		UserMem.data_length = LOG_CODE_LENGTH;
		UserMem.read(&UserMem);

		//делаем необходимые проверки
		//если все поля 0xFFFF, то чистим всю память
		if ((tmp.ev_num == 0xFFFF) && (tmp.ev_time == 0xFFFFFFFF) && (tmp.ev_code == 0xFFFF))
		{
			//чистим
			p->clear(p);
			//инициализируем переменные
			p->INTERNAL_last_log_num = 0;
			p->INTERNAL_last_log_index = 0;
			return;
		}
		//ищем последнюю запись
		if (last_log_num < tmp.ev_num)
		{
			last_log_num = tmp.ev_num;
			last_log_index = i;
		}
	}
	p->INTERNAL_last_log_num = last_log_num;
	p->INTERNAL_last_log_index = last_log_index;
}

//функция очистки буфера в Ёнќ«”
void Logger_Clear(TLogger* p)
{
	Uint16 zero = 0;
	UserMem.MemStartAddr = p->SPI_start_addr;
	UserMem.MCUStartAddr = (Uint16*)(&zero);
	UserMem.data_length = 1;
	for (int i=0;i<(p->SPI_buf_size * (LOG_LENGTH));i++)
	{
		UserMem.write(&UserMem);
		UserMem.MemStartAddr++;
	}
	p->INTERNAL_last_log_index = 0;
	p->INTERNAL_last_log_num = 0;
}


//быстрая функция записи события в FIFO
Uint16 Logger_Event_Write(TLogger* p,Uint16 code)
{
	TEvent event;
	//формируем данные
	__disable_irq();
	event.ev_time = *(p->time_ptr);
	event.ev_code = code;
	event.ev_num = ++p->INTERNAL_last_log_num;
	p->INTERNAL_last_log_code = code;
	__enable_irq();
	//пишем в FIFO
	return p->FIFO.write((TEventFIFO*)&(p->FIFO),&event);
}

//прочитать аварию из банка, сложить данные в TEvent* event
void Logger_Event_Read(TLogger* p,Uint16 shift_index,TEvent* event)
{
	Uint16 correction = 0;
	//проверяем правильность запроса
	if (shift_index > (p->SPI_buf_size - 1)) return;

	//ввод коррекции для организации кольца
	if (((int16)p->INTERNAL_last_log_index - (int16)shift_index) < 0) correction = p->SPI_buf_size;
	//чтение номера
	UserMem.MemStartAddr = p->SPI_start_addr + ((p->INTERNAL_last_log_index - shift_index + correction)*(LOG_LENGTH));
	UserMem.MCUStartAddr = (Uint16*)(&(event->ev_num));
	UserMem.data_length = LOG_NUM_LENGTH;
	UserMem.read(&UserMem);
	//чтение времени
	UserMem.MemStartAddr += UserMem.data_length;
	UserMem.MCUStartAddr = (Uint16*)(&(event->ev_time));
	UserMem.data_length = LOG_TIME_LENGTH;
	UserMem.read(&UserMem);
	//чтение кода
	UserMem.MemStartAddr += UserMem.data_length;
	UserMem.MCUStartAddr = (Uint16*)(&(event->ev_code));
	UserMem.data_length = LOG_CODE_LENGTH;
	UserMem.read(&UserMem);
}

//функция обработки FIFO и записи аварий в Ёнќ«”. ≈сли в FIFO что-то клали - запишем во флешку.
//≈сли FIFO зан€та или пуста€ - отдыхаем.
void Logger_Background_Calc(TLogger* p)
{
	Uint16 ret;
	TEvent event;
	//чтобы не сильно затормаживать фоновый цикл будем записывать по 1 записи за раз
	ret = p->FIFO.read((TEventFIFO*)&(p->FIFO),&event);
	if (ret == EVENT_FIFO_SUCCESSFUL)
	{
		//пишем в Ёнќ«”
		if (p->INTERNAL_last_log_index >= (p->SPI_buf_size - 1)) p->INTERNAL_last_log_index = 0;
		else p->INTERNAL_last_log_index++;

		//запись номера
		UserMem.MemStartAddr = p->SPI_start_addr + (p->INTERNAL_last_log_index)*(LOG_LENGTH);
		UserMem.MCUStartAddr = (Uint16*)(&(event.ev_num));
		UserMem.data_length = LOG_NUM_LENGTH;
		UserMem.write(&UserMem);
		//запись времени
		UserMem.MemStartAddr += UserMem.data_length;
		UserMem.MCUStartAddr = (Uint16*)(&(event.ev_time));
		UserMem.data_length = LOG_TIME_LENGTH;
		UserMem.write(&UserMem);
		//запись кода
		UserMem.MemStartAddr += UserMem.data_length;
		UserMem.MCUStartAddr = (Uint16*)(&(event.ev_code));
		UserMem.data_length = LOG_CODE_LENGTH;
		UserMem.write(&UserMem);
	}
}

//функци€ проверки и записи вис€щих в данный момент флагов аварий
//ѕоследовательно смотритс€ каждый бит слов аварий и, если он взведен, но еще не записан, производитс€ запись.
void Logger_ms_Calc(TLogger* p){

}

#ifdef __cplusplus
} // Extern "C"
#endif
/*@}*/
