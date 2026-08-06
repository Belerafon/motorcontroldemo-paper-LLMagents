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
 \brief     Модуль автоматического ослабления поля (см. TFieldWeakening)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 06/08/2026

 \defgroup  V_FieldWeakening Автоматическое ослабление поля

 @{*/

#ifndef V_FIELD_WEAKENING_H
#define V_FIELD_WEAKENING_H

#include "V_IQmath.h"
#include "filter.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \class TFieldWeakening
  \brief Автоматическое ослабление поля по напряжению

  Класс \a TFieldWeakening, основанный на структуре SFieldWeakening, следит
  за амплитудой запрашиваемого вектора напряжения (v_mag). Амплитуда сначала
  сглаживается инерционным фильтром fV, чтобы кратковременные броски при
  разгоне не запускали ослабление преждевременно. Когда отфильтрованная
  амплитуда устойчиво превышает ограничение инвертора v_lim (с учётом зоны
  нечувствительности deadband), интегратор формирует размагничивающий ток.
  Выход id_ref (задание тока по оси d) уходит в отрицательную область,
  что позволяет поднять скорость выше базовой при ограниченном напряжении
  звена постоянного тока. Выход ограничен по скорости изменения
  (id_step_max), чтобы не было броска тока. Ниже ограничения интегратор
  спадает с коэффициентом decay и ослабление выключается само.
  */

//! см. TFieldWeakening
struct SFieldWeakening{
	Uint16 enabled;    //!< 1 - ослабление поля включено, 0 - выключено (id_ref=0)
	long   v_mag;      //!< вход: амплитуда запрошенного напряжения (о.е.)
	long   v_lim;      //!< вход: ограничение напряжения инвертора (о.е.), напр. pwm.U_lim
	long   kp;         //!< пропорциональный коэффициент (о.е. напряжения -> о.е. тока)
	long   ki;         //!< интегральный коэффициент (о.е. напряжения -> о.е. тока)
	long   decay;      //!< коэффициент спада интегратора ниже ограничения (чуть меньше 1.0)
	long   deadband;   //!< зона нечувствительности по напряжению (о.е.), меньше не реагируем
	long   w_max;      //!< максимум размагничивающего тока в о.е. (положительный), id_ref_min = -w_max
	long   id_step_max;//!< максимум изменения id_ref за один такт (о.е. тока) - ограничение скорости изменения
	TFilter fV;        //!< инерционный фильтр амплитуды напряжения
	long   ui;         //!< переменная состояния - интегральная составляющая (положительная = ослабление)
	long   id_ref;     //!< выход: задание тока по оси d (отрицательное при ослаблении)
	long   err;        //!< диагностика: отфильтрованное превышение напряжения над ограничением с учётом зоны (о.е.)
	long   out;        //!< диагностика: требуемое ослабление, положительное (о.е. тока)
	void   (*calc)(volatile struct SFieldWeakening*);//!< указатель на функцию расчета
};

typedef volatile struct SFieldWeakening TFieldWeakening;


//! \memberof TFieldWeakening
void FieldWeakening_calc(TFieldWeakening*);


//!инициализатор по-умолчанию
#define FIELD_WEAKENING_DEFAULTS { \
    .enabled = 1, \
    .kp = _IQ(0.1), \
    .ki = _IQ(0.02), \
    .decay = _IQ(0.999), \
    .deadband = _IQ(0.01), \
    .w_max = _IQ(0.9), \
    .id_step_max = _IQ(0.004), \
    .fV = { .T = _IQ(0.02), .calc = TFilter_Calc }, \
    .calc = FieldWeakening_calc \
}


#ifdef __cplusplus
}
#endif

#endif

/*@}*/
