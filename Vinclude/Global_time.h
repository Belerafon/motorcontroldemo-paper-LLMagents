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

 \file      Global_time.h
 \brief     Модуль работы со временем
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.1 2010_02_24

*/

#ifndef GLOBAL_TIME_H
#define GLOBAL_TIME_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "DSP.h"

#define GLOBAL_TIME_CALC_FREQ	10	//!<кГц частота вызова функции calc
//#define USE_ABSOLUTE_TIME_CLOCK	//раскоментировать если используютсЯ часы реального времени



#define GLOBAL_TIME_SPI_START_ADDR 6500						//!<начальный адрес в SPI

#define GLOBAL_TIME_POWER_ON_TIME_ADDR GLOBAL_TIME_SPI_START_ADDR
#define GLOBAL_TIME_OPERATIONAL_TIME_ADDR GLOBAL_TIME_SPI_START_ADDR + 4


//!Абсолютное времЯ
  struct SABSOLUTE_TIME
  {
    int16 second;
    int16 minute;
    int16 hour;
    int16 day;
    int16 year;
  };
  typedef volatile struct SABSOLUTE_TIME ABSOLUTE_TIME;

//!ОтносительнаЯ дата
  typedef struct
  {
    int16 millisecond;
    int16 second;
    int16 minute;
    int32 hour;
  }RELATIVE_DATE;


//!Относительное времЯ
  struct SRELATIVE_TIME
  {
    int		ON;
    Uint32  tic_isr;
    Uint32  millisecond_counter;
    Uint32  second_counter;
    int16 	delta_millisecond;
    int16 	delta_second;
    RELATIVE_DATE	relative_date;
  };
  typedef volatile struct SRELATIVE_TIME RELATIVE_TIME;



  /*! \class TGlobalTime
        \brief Модуль работы со временем.

        Класс \a TGlobalTime, основанный на структуре SGlobalTime, содержит методы
        длЯ работы со временем.Отсчитывает времЯ в:\n
  				  длЯ отсчета времени работы и т.д.:\n
  					милисекунды:	0-999\n
  					секунды:		0-59\n
  					минуты:			0-59\n
  					часы:			0-23\n
  					дни:			0-364\n
  					года:			0-примерно до 160 лет\n
  				  длЯ отсчета дельт (приращений времени):\n
  					милисекунды:	0-0xFFFFFFFF - соответствует примерно 1.5 месЯцам\n
  					секунды:		0-0xFFFFFFFF - соответствует примерно 160 годам\n

      Есть специальный способ, позволЯющий отсчитывать выдержки времени с точностью
  до миллисекунд в  перывании ШИМ. ИспользуетсЯ переменнаЯ delta_millisecond,
  котораЯ возводитсЯ в единицу каждую миллисекунду и на следующем периоде ШИМ
  сбрасываетсЯ. Тогда можно в необходимый счетчик добавить counter+=delta_millisecond и
  получить в counter счетчик миллисекунд. Аналогично delta_second.

  ПРИМЕЧАНИЕ:   1. модуль не требует инициализации
  				2. автоматически перенастраиваетсЯ при изменении несущей частоты

  ДЛЯ КОРРЕКТНОЙ РАБОТЫ МОДУЛЯ ПОЛЬЗОВАТЕЛЬ ДОЛЖЕН:
  1. в макросе GLOBAL_TIME_CALC_FREQ правильно указать  несущую частоту вызова функции расчета.
  Следует отметить, что значение GLOBAL_TIME_CALC_FREQ целочисленное, поэтому
  модуль корректно работает только при целых значениЯх несущей частоты (в кГц).
  Недостаток свЯзан с конкретной реализацией модулЯ.
  */

//! см. TGlobalTime
  struct SGlobalTime
  {
    RELATIVE_TIME relative_time1;//!относительное времЯ 1
    RELATIVE_TIME relative_time2;//!относительное времЯ 2
    ABSOLUTE_TIME absolute_time;//!абсолютное времЯ
    int16 time_type;//!часы реального времени (1 - есть)
	int32 PowerOn_time;				//!<Общее времЯ наработки
	int32 PowerOn_time_min;				//!<Общее времЯ наработки в минутах
	int32 operational_time;	//!<Общее времЯ включенного состоЯниЯ
	int32 operational_time_min;				//!<Общее времЯ включенного состоЯниЯ в минутах
	int16 WtiteSPI_flag;
	int32 PrevWriteSPIHour;
    void	(*init)(volatile struct SGlobalTime*);
    void	(*calc)(volatile struct SGlobalTime*);
    void	(*ms_calc)(volatile struct SGlobalTime*);
    void	(*slow_calc)(volatile struct SGlobalTime*);
	void (*read_PowerOn_time)(volatile struct SGlobalTime*);
	void (*write_PowerOn_time)(volatile struct SGlobalTime*);
	void (*read_oper_time) (volatile struct SGlobalTime*);
	void (*write_oper_time)(volatile struct SGlobalTime*);
  };

  typedef volatile struct SGlobalTime TGlobalTime;

//!Иницализатор относительной даты.
#define RELATIVE_DATE_DEFAULTS { \
    .millisecond = 0, \
    .second = 0, \
    .minute = 0, \
    .hour = 0 \
}

//!Инициализатор относительного времени.
#define RELATIVE_TIME_DEFAULTS { \
    .relative_date = RELATIVE_DATE_DEFAULTS \
  }
//!Инициализатор абсолютного времени.
#define ABSOLUTE_TIME_DEFAULTS { \
    .second = 0, \
    .minute = 0, \
    .hour = 0, \
    .day = 0, \
    .year = 0 \
}

//!Инициализатор TGlobalTime.
#define GLOBAL_TIME_DEFAULTS { \
    .relative_time1 = RELATIVE_TIME_DEFAULTS, \
    .relative_time2 = RELATIVE_TIME_DEFAULTS, \
    .absolute_time = ABSOLUTE_TIME_DEFAULTS, \
    .time_type = 1, \
    .init = GlobalTime_init, \
    .calc = GlobalTime_calc, \
    .ms_calc = GlobalTime_ms_calc, \
    .slow_calc = GlobalTime_slow_calc, \
    .read_PowerOn_time = GlobalTime_read_PowerOn_time, \
    .write_PowerOn_time = GlobalTime_write_PowerOn_time, \
    .read_oper_time = GlobalTime_read_oper_time, \
    .write_oper_time = GlobalTime_write_oper_time \
}

//! \memberof TGlobalTime
  void GlobalTime_init(TGlobalTime*);
//! \memberof TGlobalTime
  void GlobalTime_calc(TGlobalTime*);
  //! \memberof TGlobalTime
  void GlobalTime_ms_calc(TGlobalTime*);
//! \memberof TGlobalTime
  void GlobalTime_slow_calc(TGlobalTime*);

//! \memberof TGlobalTime
  void GlobalTime_REL_TIME_calc(RELATIVE_TIME *p);

//! \memberof TGlobalTime
void GlobalTime_read_PowerOn_time(TGlobalTime*);
//! \memberof TGlobalTime
void GlobalTime_write_PowerOn_time(TGlobalTime*);
//! \memberof TGlobalTime
void GlobalTime_read_oper_time(TGlobalTime*);
//! \memberof TGlobalTime
void GlobalTime_write_oper_time(TGlobalTime*);



#ifdef __cplusplus
}
#endif


#endif



/*@}*/

