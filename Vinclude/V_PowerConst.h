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
 
 \file      V_PowerConst.h
 \brief     Ограничитель скорости по мощности P=const
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0

 \defgroup  V_PowerConst Ограничитель скорости по мощности P=const

 @{
 */

#ifndef V_POWERCONST_H
#define V_POWERCONST_H

#ifdef __cplusplus
extern "C" {
#endif

//! Ограничивает скорость по мощности P = T * omega = const
//! Если текущая мощность превышает PowerMax, пропорционально уменьшает speed_ref.
//! Использует обратную связь по току оси q (torque) и скорости.
//! \param[in,out] speed_ref  Задание скорости (формат 8.24), будет скорректировано на месте
//! \param[in]     PowerMax   Ограничение мощности (формат 8.24, положительное)
//! \param[in]     Iq         Текущий ток по оси q (формат 8.24, амплитуда)
//! \param[in]     speed      Текущая скорость (формат 8.24, может быть отрицательной)
void V_PowerConstLimit(long *speed_ref, long PowerMax, long Iq, long speed);

#ifdef __cplusplus
}
#endif

#endif

/*@}*/
