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

 \defgroup  V_FieldWeakening Модуль ослабления поля (Field Weakening)
 @{
*/

#ifndef V_FIELD_WEAKENING_H
#define V_FIELD_WEAKENING_H

#include "V_pid_reg3.h"

#ifdef __cplusplus
extern "C"
{
#endif


/*! \class TFieldWeakening
  \brief Автоматическое ослабление поля для векторной системы управления.

  Класс \a TFieldWeakening, основанный на структуре SFieldWeakening,
  реализует функцию ослабления поля (Field Weakening) для векторных
  систем управления синхронными машинами (СД). Выходной сигнал -
  задание отрицательного тока по оси d (\a id_ref), которое подаётся
  на вход задания регулятора тока оси d (pid_id.pid_ref_reg3).

  Алгоритм основан на обратной связи по амплитуде вектора напряжения
  статора (U_mag). Когда U_mag превышает порог \a U_thresh, регулятор
  формирует отрицательное задание тока по оси d, что уменьшает
  потокосцепление ротора и, следовательно, противо-ЭДС двигателя.
  Это позволяет вращать двигатель быстрее базовой скорости при
  ограниченном напряжении звена постоянного тока (ЗПТ).

  Регулирование осуществляется ПИ-регулятором с анти-насыщением,
  построенным на базе структуры TPidReg3. Когда напряжение ниже
  порога, выход плавно возвращается к нулю, что соответствует
  номинальному потокосцеплению (максимальному моменту).

  Использование:
  -# В быстром расчёте (10 кГц) заполнить входы: \a U_mag, \a enabled,
     \a Id_min (максимально допустимый отрицательный ток оси d).
  -# Вызвать \a calc.
  -# Подать \a id_ref на вход pid_id.pid_ref_reg3 вместо нуля.

  Включение/выключение функции осуществляется битом
  sw.field_weakening_ena, который дублируется в поле \a enabled.
*/

//! см. TFieldWeakening
struct SFieldWeakening {
	long U_mag;        //!< Input:  Текущая амплитуда вектора напряжения статора (pwm.U_mag), IQ24
	long U_thresh;     //!< Parameter: Порог напряжения для начала ослабления поля (например _IQ(0.85)). При U_mag > U_thresh контроллер вводит отрицательный Id
	long Id_min;       //!< Parameter: Минимально допустимое задание Id (максимальный по модулю отрицательный ток ослабления), IQ24, <= 0
	long id_ref;       //!< Output: Задание тока оси d для регулятора тока (<= 0). Подставляется в pid_id.pid_ref_reg3
	int  enabled;      //!< Input:  Разрешение работы (1 = включено, 0 = выключено, id_ref = 0)
	TPidReg3 pid_fw;   //!< Подчинённый ПИ-регулятор напряжения (осуществляет обратную связь по U_mag)
	void (*init)(struct SFieldWeakening*);  //!< Указатель на функцию инициализации
	void (*calc)(struct SFieldWeakening*);  //!< Указатель на функцию расчёта (вызывается в быстром расчёте)
	void (*reset)(struct SFieldWeakening*); //!< Указатель на функцию сброса (обнуление интегратора)
};

typedef struct SFieldWeakening TFieldWeakening;


//! Инициализатор по умолчанию.
//! Коэффициенты ПИ-регулятора напряжения и порог подобраны как
//! разумные значения по умолчанию для типового СД. При необходимости
//! они могут быть переопределены через словарь объектов после
//! инициализации.
#define FIELD_WEAKENING_DEFAULTS { \
    .U_thresh = _IQ(0.85), \
    .init = FieldWeakening_Init, \
    .calc = FieldWeakening_Calc, \
    .reset = FieldWeakening_Reset \
}


//! \memberof TFieldWeakening
void FieldWeakening_Init(TFieldWeakening*);
//! \memberof TFieldWeakening
void FieldWeakening_Calc(TFieldWeakening*);
//! \memberof TFieldWeakening
void FieldWeakening_Reset(TFieldWeakening*);


#ifdef __cplusplus
}
#endif

#endif
/*@}*/
