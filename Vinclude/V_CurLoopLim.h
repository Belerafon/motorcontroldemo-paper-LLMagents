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

 \file      V_CurLoopLim.h
 \brief     Динамическое ограничение выхода регуляторов тока (см. TCurLoopLim)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru

 \defgroup  V_CurLoopLim Динамическое ограничение регуляторов тока
 @{
*/

#ifndef V_CUR_LOOP_LIM_H
#define V_CUR_LOOP_LIM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "build.h"
#include "V_pid_reg3.h"

/*! \class TCurLoopLim
  \brief Динамическое ограничение выхода регуляторов тока по оси d и q.

  Класс \a TCurLoopLim, основанный на структуре SCurLoopLim, рассчитывает
  максимально реализуемое задание напряжения на выходе токовых регуляторов
  исходя из текущего напряжения звена постоянного тока (ЗПТ) и возможности
  модуля ШИМ. Без этого модуля пределы \a pid_out_max / \a pid_out_min
  регуляторов \a pid_id и \a pid_iq остаются константами из
  \a PIDREG3_DEFAULTS и не учитывают просадку или всплеск ЗПТ.

  Расчёт ведётся каждый быстрый такт (10 кГц) в физических о.е. напряжения
  (база \a U_nom). Текущий предел:

  \verbatim
   U_dyn = U_lim * (Udc_meas / Udc_nom) * k_dyn
  \endverbatim

  где \a U_lim — настроенный предел вектора напряжения модуля ШИМ при
  номинальном ЗПТ, \a Udc_meas — измеренное напряжение ЗПТ в о.е. от
  \a Udc_nom, \a k_dyn — коэффициент чувствительности к ЗПТ.

  Результат зажимается в диапазон \a [u_dyn_min ; U_lim ], поэтому даже при
  глубокой просадке ЗПТ регуляторы остаются работоспособными, а при всплеске
  ЗПТ — не превышают линейную зону векторной ШИМ.

  Ручки управления для прототипирования выведены через свободные переменные
  словаря объектов CANopen:

  - \a DebugW1 (5174h.09h, IQ24, Editable) — переключатель режима с порогом
    \b 0.5 о.е.: значения ниже 0.5 — старое поведение, пределы регуляторов
    устанавливаются в статическую константу 1.0 о.е.; значения \b >=0.5 —
    включено динамическое ограничение. Порог нужен, чтобы дефолт словаря
    (~0.001 о.е.) надёжно трактовался как ВЫКЛ. Значение по умолчанию
    безопасно для A/B-сравнения.
  - \a DebugW2 (5174h.0Bh, IQ24, Editable) — коэффициент \a k_dyn
    чувствительности к ЗПТ (1.0 — полностью пропорционально, 0 — не зависит
    от ЗПТ). Защита от некорректного значения: вне диапазона (0; 2.0]
    принимается равным 1.0.
  - \a Debug1 (5174h.01h, IQ24, Viewable) — вычисленный предел \a u_dyn
    в о.е., для контроля.

  Возврат к прежнему поведению — установить \a DebugW1 < 0.5 (например, 0):
  на первом же такте пределы обоих регуляторов снова становятся равными
  статической константе 1.0 о.е. из \a PIDREG3_DEFAULTS.
*/

//! см. TCurLoopLim
struct SCurLoopLim {
	long u_dyn;        //!< Вычисленный предел выхода по каждой оси d/q, IQ24 (только чтение)
	long u_dyn_min;    //!< Защитный минимум предела, IQ24, чтобы регуляторы не "умирали" при глубокой просадке ЗПТ
	void (*calc)(volatile struct SCurLoopLim*); //!< Пересчёт u_dyn по текущим U_lim, Udc_meas и k_dyn
};

typedef volatile struct SCurLoopLim TCurLoopLim;


//! Инициализатор по умолчанию.
//! u_dyn_min = 0.05 о.е. — гарантированно ненулевой рабочий диапазон регуляторов.
#define CUR_LOOP_LIM_DEFAULTS { \
    .u_dyn = _IQ(1.0), \
    .u_dyn_min = _IQ(0.05), \
    .calc = CurLoopLim_Calc, \
}


//! \memberof TCurLoopLim
void CurLoopLim_Calc(TCurLoopLim*);
//! \memberof TCurLoopLim
//! Каждый такт устанавливает пределы \a pid_id и \a pid_iq:
//! при \a DebugW1 < 0.5 — статическая константа 1.0 о.е. (прежнее поведение),
//! при \a DebugW1 >= 0.5 — вычисленный динамический предел \a u_dyn.
void CurLoopLim_Apply(TCurLoopLim*, TPidReg3 *pid_id, TPidReg3 *pid_iq);


#ifdef __cplusplus
}
#endif

#endif

/*@}*/
