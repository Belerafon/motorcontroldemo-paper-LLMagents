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
 
 \file      V_PowerConst.c
 \brief     Ограничитель скорости по мощности P=const
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0

 */

/** \addtogroup V_PowerConst */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif
#include "V_PowerConst.h"
#include "V_IQmath.h"
#ifdef __cplusplus
}
#endif

//! Минимальная скорость и ток для включения ограничения (избегаем деления на малые величины)
#define POWER_CONST_SPEED_MIN _IQ(0.01)
#define POWER_CONST_IQ_MIN   _IQ(0.01)

//! Ограничивает скорость по мощности P = T * omega = const
//! Если текущая мощность P = |Iq| * |speed| превышает PowerMax,
//! пропорционально уменьшает speed_ref: speed_ref *= PowerMax / P_curr.
//! Знак (направление) speed_ref сохраняется.
//! \param[in,out] speed_ref  Задание скорости (формат 8.24), корректируется на месте
//! \param[in]     PowerMax   Ограничение мощности (формат 8.24, положительное)
//! \param[in]     Iq         Текущий ток по оси q (формат 8.24)
//! \param[in]     speed      Текущая скорость (формат 8.24, с учётом знака)
void V_PowerConstLimit(long *speed_ref, long PowerMax, long Iq, long speed) {
    long speed_abs;
    long Iq_abs;
    long P_curr;
    long power_ratio;

    if (PowerMax <= 0)
        return;

    speed_abs = _IQ24abs(speed);
    Iq_abs = _IQ24abs(Iq);

    if (speed_abs < POWER_CONST_SPEED_MIN)
        return;
    if (Iq_abs < POWER_CONST_IQ_MIN)
        return;

    P_curr = _IQ24mpy(Iq_abs, speed_abs);

    if (P_curr > PowerMax) {
        power_ratio = _IQ24div(PowerMax, P_curr);
        *speed_ref = _IQ24mpy(*speed_ref, power_ratio);
    }
}

/*@}*/
