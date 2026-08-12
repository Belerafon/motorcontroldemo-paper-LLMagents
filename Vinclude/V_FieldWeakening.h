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

 \defgroup  V_FieldWeakening Модуль ослабления поля
 @{

 Позволяет расширить диапазон скоростей синхронной машины (СДПМ) за счёт
 автоматической выдачи отрицательного задания тока по оси d (Id). При
 приближении амплитуды требуемого вектора напряжения к максимально
 реализуемому значению (ограничение модуля ШИМ) регулятор ослабления поля
 формирует отрицательный Id, который уменьшает потокосцепление от магнитов,
 снижает противо-ЭДС и, тем самым, даёт возможность вращаться быстрее
 номинальной (базовой) скорости.

 Алгоритм: интегральный (с опциональной пропорциональной частью) регулятор
 по сигналу ошибки напряжения (Umax - Us). Если требуемое напряжение Us
 превышает порог Umax, ошибка становится отрицательной и Id_ref уходит в
 отрицательную область (ослабление). Если Us меньше Umax, интегратор
 удерживается на уровне Id_max (обычно 0) и ослабления нет. Анти-наброс
 реализован ограничением интегратора в диапазоне [Id_min, Id_max].
*/

#ifndef V_FIELD_WEAKENING_H
#define V_FIELD_WEAKENING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "V_common.h"


/*! \class TFieldWeakening
  \brief Регулятор ослабления поля

  Класс \a TFieldWeakening, основанный на структуре SFieldWeakening,
  реализует автоматическое ослабление поля для векторной системы
  управления синхронной машиной. Перед вызовом calc необходимо поместить
  в Us амплитуду требуемого напряжения (как правило pwm.U_mag, посчитанную
  на предыдущем такте), а в Umax - максимально реализуемое напряжение
  (как правило pwm.U_lim). Выходом является задание тока по оси d - Id_ref.
*/

//! см. TFieldWeakening
struct SFieldWeakening {
	Uint16 Enabled;		//!< Разрешение работы (1 - включено, 0 - выключено)
	long Us;				//!< Вход: амплитуда требуемого напряжения (pwm.U_mag), о.е. IQ24
	long Umax;			//!< Вход: пороговое напряжение ослабления (pwm.U_lim), о.е. IQ24
	long Kp;				//!< Параметр: коэффициент пропорциональности (обычно 0)
	long Ki;				//!< Параметр: коэффициент интегрирования
	long Kc;				//!< Параметр: коэффициент анти-наброса (обычно _IQ(0.5))
	long ui;				//!< Переменная: интегральная составляющая
	long uprsat;			//!< Переменная: выход до ограничения
	long Id_ref;			//!< Выход: задание тока оси d (отрицательный при ослаблении)
	long Id_min;			//!< Параметр: максимальный ток ослабления (отрицательное, напр. _IQ(-1.0))
	long Id_max;			//!< Параметр: максимальный ток намагничивания (обычно 0)
	Uint16 state;			//!< Флаг: 1 - идёт ослабление поля, 0 - ниже базовой скорости
	void (*init)(volatile struct SFieldWeakening*);	//!< указатель на функцию инициализации
	void (*calc)(volatile struct SFieldWeakening*);	//!< указатель на функцию расчета
	void (*reset)(volatile struct SFieldWeakening*);	//!< указатель на функцию сброса
};

typedef volatile struct SFieldWeakening TFieldWeakening;


//! Инициализатор по умолчанию.
//! Ki подобран для дискретности 10 кГц, Id_min = -1.0 о.е., Id_max = 0.
#define FIELD_WEAKENING_DEFAULTS { \
    .Enabled = 1, \
    .Us = 0, \
    .Umax = _IQ(0.866), \
    .Kp = _IQ(0.0), \
    .Ki = _IQ(0.005), \
    .Kc = _IQ(0.5), \
    .Id_min = _IQ(-1.0), \
    .Id_max = _IQ(0.0), \
    .init = FieldWeakening_init, \
    .calc = FieldWeakening_calc, \
    .reset = FieldWeakening_reset \
}


//! \memberof TFieldWeakening
void FieldWeakening_init(volatile struct SFieldWeakening*);
//! \memberof TFieldWeakening
void FieldWeakening_calc(volatile struct SFieldWeakening*);
//! \memberof TFieldWeakening
void FieldWeakening_reset(volatile struct SFieldWeakening*);


#ifdef __cplusplus
}
#endif

#endif

/*@}*/
