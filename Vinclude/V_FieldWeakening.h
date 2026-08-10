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

 \file      V_FieldWeakening.h
 \brief     Модуль ослабления поля синхронной машины (см. TFieldWeakening)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0

 \addtogroup
 @{*/

#ifndef V_FIELD_WEAKENING_H
#define V_FIELD_WEAKENING_H

#include "build.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \class TFieldWeakening
  \brief Модуль ослабления поля синхронной машины

  Класс \a TFieldWeakening, основанный на структуре SFieldWeakening,
  автоматически формирует отрицательное задание тока по оси d (ослабление поля)
  при превышении выходным напряжением инвертора доступного напряжения.
  Задание вычисляется ПИ-регулятором напряжения: когда измеренная амплитуда
  вектора напряжения \a U_meas превышает допустимую \a U_lim, регулятор
  вырабатывает отрицательный ток \a Id_ref (ток размагничивания). Когда
  напряжение не превышает предела, задание возвращается к нулю.
*/

//! см. TFieldWeakening
struct SFieldWeakening {
	Uint16 Enabled;		//!< разрешение ослабления поля
	long U_meas;		//!< измеренная амплитуда вектора напряжения (о.е.), задается
	long U_lim;		//!< допустимая амплитуда вектора напряжения (о.е.), задается
	long Id_min;		//!< максимально допустимый отрицательный ток оси d (о.е.), задается
	long Kp;		//!< коэффициент П-части регулятора напряжения
	long Ki;		//!< коэффициент И-части регулятора напряжения
	long ui;		//!< интегральная составляющая регулятора (служебная)
	long Id_ref;		//!< задание тока оси d (о.е.), результат
	void   (*calc)(volatile struct SFieldWeakening*);	//!< указатель на функцию расчета
};

typedef volatile struct SFieldWeakening TFieldWeakening;

//! \memberof TFieldWeakening
void FieldWeakening_calc(TFieldWeakening*);

//! Инициализатор по-умолчанию
#define FIELD_WEAKENING_DEFAULTS { \
    .Enabled = 1, \
    .U_lim = _IQ(0.866), \
    .Id_min = _IQ(-1), \
    .Kp = _IQ(1.0), \
    .Ki = _IQ(0.002), \
    .calc = FieldWeakening_calc \
}

#ifdef __cplusplus
}
#endif

#endif

/*@}*/
