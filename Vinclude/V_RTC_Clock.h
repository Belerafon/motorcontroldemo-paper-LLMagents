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

 \file        V_RTC_Clock.h
 \brief   Модуль работы с часами реального времени
 \author    ООО "НПФ Вектор". Все права защищены. http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \defgroup  V_rmp_ctrl Задатчик интенсивности (см. TRMPCtrl)

 @{
*/
#ifndef V_RTC_CLOCK_H_
#define V_RTC_CLOCK_H_

#define RTC_SLAVE_ADDRESS	0x68			// Для RTC DS1340 адрес 1101 000b = 0x68 (хотя в руководстве они его как бы дополняют нулём
											// и получают 1101 0000b = 0xD0)
#define RTC_WRITE_ERROR		1
#define RTC_READ_ERROR		2
#define RTC_TIMEOUT_ERROR	3

#define RTC_TIMEOUT			1000		// Количество ожидания шины
#define RTC_SECOND_WAITING_TIMEOUT			10000		// Время, в течение которого должна смениться секунда (задается в мс). Если не меняется, значит часы не работают.

struct SRTCClock{
	Uint16 ms;			// На всякий
	Uint16 msInDay;		// Милиписюли с начала суток
	Uint16 error;		// Код ошибки
	Uint16 tryCounter;	// Счетчик ожиданий шины

	Uint16 second;
	Uint16 minute;
	Uint16 hour;

	Uint16 DOW;			// DayOfWeek - день недели (пусть будет, что, жалко что ли?)
	Uint16 day;			// Date - это число месяца
	Uint16 month;
	Uint16 year;

	Uint32 packed_time;	// Время, упакованное в слово
	Uint32 timeToSet;	// Время, которое нужно установить на часах, упакованное в слово

	Uint16 setTimeFlag;
	Uint16 readTimeFlag;

	Uint16 secondPrev;
	Uint16 stoppedCounter;
	Uint16 ClockOk;
	void (*init)(volatile struct SRTCClock*);					// Инициализация
	void (*read)(volatile struct SRTCClock*);					// Прочитать время из часиков
	void (*set)(volatile struct SRTCClock*);					// Письнуть время в часики
	void (*msCalc)(volatile struct SRTCClock*);
	void (*slowCalc)(volatile struct SRTCClock*);
};

typedef volatile struct SRTCClock TRTCClock;

#define RTC_CLOCK_DEFAULTS { .init = RTC_Clock_Init, \
                            .read = RTC_Clock_Read_Time, \
                            .set = RTC_Clock_Set_Time, \
                            .msCalc = RTC_Clock_Ms_Calc, \
                            .slowCalc = RTC_Clock_Slow_Calc }

void RTC_Clock_Init (TRTCClock *);
void RTC_Clock_Read_Time (TRTCClock *);
void RTC_Clock_Set_Time (TRTCClock *);
void RTC_Clock_Ms_Calc (TRTCClock *);
void RTC_Clock_Slow_Calc (TRTCClock *);

#endif /* V_I2CCLOCK_H_ */
