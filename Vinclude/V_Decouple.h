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

 \file      V_Decouple.h
 \brief     Декаплинг регуляторов тока - компенсация перекрестных связей (см. TDecouple)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0

 \defgroup  V_Decouple Декаплинг регуляторов тока
 @{
 Компенсация перекрестных связей роторной системы координат (d, q).
 На высокой частоте вращения ЭДС вращения и индуктивные перекрестные
 связи становятся значительными и мешают регуляторам тока быстро
 отрабатывать задание. Модуль формирует компенсирующие добавки к
 выходам ПИ-регуляторов тока осей d и q (feedforward), благодаря
 чему динамика контуров тока сохраняется во всем диапазоне скоростей.

 Математическая модель в относительных единицах:
   Ud_dec = -wr * Kdec_d * iq
   Uq_dec =  wr * Kdec_q * id + wr * Kflux
 где
   wr      - электрическая угловая скорость ротора, о.е.;
   Kdec_d  - коэффициент перекрестной связи оси d (экв. Lq, о.е.);
   Kdec_q  - коэффициент перекрестной связи оси q (экв. Ld, о.е.);
   Kflux   - эквивалентное потокосцепление ПМ (о.е., для АД = 0);
   Kdec_gain - общий коэффициент (0 - компенсация выключена, 1.0 - полная).
*/

#ifndef V_DECOUPLE_H
#define V_DECOUPLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "V_IQmath.h"

/*! \class TDecouple
    \brief Декаплинг регуляторов тока (компенсация перекрестных связей)

    Класс \a TDecouple, основанный на структуре SDecouple, формирует
    компенсирующие напряжения по осям d и q, которые добавляются к
    выходам ПИ-регуляторов тока перед обратным преобразованием координат.
*/

//! см. TDecouple
struct SDecouple{
	_iq id;			//!< Input:  измеренный ток оси d, о.е.
	_iq iq;			//!< Input:  измеренный ток оси q, о.е.
	_iq wr;			//!< Input:  электрическая угловая скорость ротора, о.е.

	_iq Kdec_d;		//!< Parameter: коэф. перекрестной связи оси d (экв. w*Lq), о.е.
	_iq Kdec_q;		//!< Parameter: коэф. перекрестной связи оси q (экв. w*Ld), о.е.
	_iq Kflux;		//!< Parameter: потокосцепление ПМ (о.е., для АД = 0)
	_iq Kdec_gain;	//!< Parameter: общий коэффициент компенсации (0 - выкл., 1.0 - полная)

	_iq ud_dec;		//!< Output: компенсирующая добавка напряжения оси d, о.е.
	_iq uq_dec;		//!< Output: компенсирующая добавка напряжения оси q, о.е.

	void (*calc)(struct SDecouple*);		//!< Pointer to calculation function
	void (*slow_calc)(struct SDecouple*);	//!< Pointer to background calculation function
};

typedef struct SDecouple TDecouple;


//! Инициализатор по-умолчанию.
//! По умолчанию перекрестные связи выключены (Kdec_gain = 0), включение -
//! установка Kdec_gain = 1.0 (или иного значения) после настройки коэффициентов.
#define DECOUPLE_DEFAULTS { \
    .Kdec_d    = _IQ(0.0), \
    .Kdec_q    = _IQ(0.0), \
    .Kflux     = _IQ(0.0), \
    .Kdec_gain = _IQ(0.0), \
    .calc      = Decouple_calc, \
    .slow_calc = Decouple_slow_calc \
}


//! \memberof TDecouple
void Decouple_calc(TDecouple*);
//! \memberof TDecouple
void Decouple_slow_calc(TDecouple*);


#ifdef __cplusplus
}
#endif

#endif
/*@}*/
