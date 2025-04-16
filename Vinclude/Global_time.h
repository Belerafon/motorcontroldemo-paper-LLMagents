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

 \file      Global_time.h
 \brief     ћодуль работы со временем
 \author    ќќќ "Ќѕ‘ ¬ектор". http://motorcontrol.ru
 \version   v 1.1 2010_02_24

*/

#ifndef GLOBAL_TIME_H
#define GLOBAL_TIME_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "DSP.h"

#define GLOBAL_TIME_CALC_FREQ	10	//!<к√ц частота вызова функции calc
//#define USE_ABSOLUTE_TIME_CLOCK	//раскоментировать если используются часы реального времени



#define GLOBAL_TIME_SPI_START_ADDR 6500						//!<начальный адрес в SPI

#define GLOBAL_TIME_POWER_ON_TIME_ADDR GLOBAL_TIME_SPI_START_ADDR
#define GLOBAL_TIME_OPERATIONAL_TIME_ADDR GLOBAL_TIME_SPI_START_ADDR + 4


//!јбсолютное время
  struct SABSOLUTE_TIME
  {
    int16 second;
    int16 minute;
    int16 hour;
    int16 day;
    int16 year;
  };
  typedef volatile struct SABSOLUTE_TIME ABSOLUTE_TIME;

//!ќтносительная дата
  typedef struct
  {
    int16 millisecond;
    int16 second;
    int16 minute;
    int32 hour;
  }RELATIVE_DATE;


//!ќтносительное время
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
        \brief ћодуль работы со временем.

         ласс \a TGlobalTime, основанный на структуре SGlobalTime, содержит методы
        для работы со временем.ќтсчитывает время в:\n
  				  для отсчета времени работы и т.д.:\n
  					милисекунды:	0-999\n
  					секунды:		0-59\n
  					минуты:			0-59\n
  					часы:			0-23\n
  					дни:			0-364\n
  					года:			0-примерно до 160 лет\n
  				  для отсчета дельт (приращений времени):\n
  					милисекунды:	0-0xFFFFFFFF - соответствует примерно 1.5 месяцам\n
  					секунды:		0-0xFFFFFFFF - соответствует примерно 160 годам\n

      ≈сть специальный способ, позволяющий отсчитывать выдержки времени с точностью
  до миллисекунд в  перывании Ў»ћ. »спользуется переменная delta_millisecond,
  которая возводится в единицу каждую миллисекунду и на следующем периоде Ў»ћ
  сбрасывается. “огда можно в необходимый счетчик добавить counter+=delta_millisecond и
  получить в counter счетчик миллисекунд. јналогично delta_second.

  ѕ–»ћ≈„јЌ»≈:   1. модуль не требует инициализации
  				2. автоматически перенастраивается при изменении несущей частоты

  ƒЋя  ќ––≈ “Ќќ… –јЅќ“џ ћќƒ”Ћя ѕќЋ№«ќ¬ј“≈Ћ№ ƒќЋ∆≈Ќ:
  1. в макросе GLOBAL_TIME_CALC_FREQ правильно указать  несущую частоту вызова функции расчета.
  —ледует отметить, что значение GLOBAL_TIME_CALC_FREQ целочисленное, поэтому
  модуль корректно работает только при целых значениях несущей частоты (в к√ц).
  Ќедостаток связан с конкретной реализацией модуля.
  */

//! см. TGlobalTime
  struct SGlobalTime
  {
    RELATIVE_TIME relative_time1;//!относительное время 1
    RELATIVE_TIME relative_time2;//!относительное время 2
    ABSOLUTE_TIME absolute_time;//!абсолютное время
    int16 time_type;//!часы реального времени (1 - есть)
	int32 PowerOn_time;				//!<ќбщее время наработки
	int32 PowerOn_time_min;				//!<ќбщее время наработки в минутах
	int32 operational_time;	//!<ќбщее время включенного состояния
	int32 operational_time_min;				//!<ќбщее время включенного состояния в минутах
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

//!»ницализатор относительной даты.
#define RELATIVE_DATE_DEFAULTS { \
    .millisecond = 0, \
    .second = 0, \
    .minute = 0, \
    .hour = 0 \
}

//!»нициализатор относительного времени.
#define RELATIVE_TIME_DEFAULTS { \
    .relative_date = RELATIVE_DATE_DEFAULTS \
  }
//!»нициализатор абсолютного времени.
#define ABSOLUTE_TIME_DEFAULTS { \
    .second = 0, \
    .minute = 0, \
    .hour = 0, \
    .day = 0, \
    .year = 0 \
}

//!»нициализатор TGlobalTime.
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

