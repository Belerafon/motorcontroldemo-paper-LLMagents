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
 \brief     Модуль ослабления поля (см. TFieldWeakening)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0

 \defgroup  V_FieldWeakening Ослабление поля (см. TFieldWeakening)
 @{*/


#ifndef V_FIELD_WEAKENING_H
#define V_FIELD_WEAKENING_H

#include "DSP.h"
#include "V_IQmath.h"

#ifdef __cplusplus
extern "C" {
#endif


/*! \class TFieldWeakening
  \brief Модуль автоматического ослабления поля

  Класс \a TFieldWeakening, основанный на структуре SFieldWeakening,
  реализует автоматическое ослабление поля двигателя за счёт задания
  отрицательного тока по оси d (Id). Алгоритм следит за амплитудой
  вектора напряжения, выдаваемого регуляторами тока, и при приближении
  её к пределу выходного напряжения инвертора формирует отрицательную
  добавку к заданию тока оси d. Уменьшение потока снижает противо-ЭДС,
  что позволяет вращать двигатель с частотой выше номинальной (базовой).

  Алгоритм саморегулирующийся: на низкой скорости амплитуда напряжения
  невелика, ошибка по напряжению положительна, интеграл удерживается на
  нуле, поэтому выходной Id равен нулю и контур тока работает как обычно.
  При приближении к пределу напряжения ошибка становится отрицательной
  и интегратор "уходит в минус", задавая отрицательный Id ровно настолько,
  насколько не хватает напряжения. Ограничение Id_ref_min задаёт
  максимальную глубину ослабления поля.
*/

//!см. TFieldWeakening
struct SFieldWeakening {
	long Enabled;		//!< Разрешение работы модуля (1 - разрешён, 0 - выключен)
	long Ud_input;		//!< Вход: напряжение по оси d (Ud) - выход регулятора тока pid_id
	long Uq_input;		//!< Вход: напряжение по оси q (Uq) - выход регулятора тока pid_iq
	long U_mag;			//!< Текущая амплитуда вектора напряжения (для наблюдения)
	long U_lim;			//!< Уставка ограничения напряжения, при превышении которой включается ослабление поля
	long Kp;			//!< Коэффициент пропорциональной части регулятора ослабления поля
	long Ki;			//!< Коэффициент интегральной части регулятора ослабления поля
	long ui;			//!< Интегральная составляющая (служебная)
	long Id_ref_min;	//!< Минимальное (наиболее отрицательное) задание Id при ослаблении поля
	long Id_ref_out;	//!< Выход: добавка к заданию тока оси d (всегда <= 0)
	void  (*init)(struct SFieldWeakening*);	//!< указатель на функцию инициализации
	void  (*calc)(struct SFieldWeakening*);	//!< указатель на функцию расчёта
};

typedef struct SFieldWeakening TFieldWeakening;


//! Инициализатор по-умолчанию
#define FIELD_WEAKENING_DEFAULTS { \
    .Enabled = 1, \
    .U_lim = _IQ(0.8), \
    .Kp = _IQ(0), \
    .Ki = _IQ(0.001), \
    .Id_ref_min = _IQ(-0.8), \
    .init = FieldWeakening_init, \
    .calc = FieldWeakening_calc \
}


//! \memberof TFieldWeakening
void FieldWeakening_init(TFieldWeakening*);
//! \memberof TFieldWeakening
void FieldWeakening_calc(TFieldWeakening*);


#ifdef __cplusplus
}
#endif

#endif


/*@}*/
