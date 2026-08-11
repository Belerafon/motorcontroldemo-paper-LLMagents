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

 \file      V_Decouple.c
 \brief     Декаплинг регуляторов тока - компенсация перекрестных связей (см. TDecouple)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0

 \addtogroup V_Decouple
 @{*/

#include "V_IQmath.h"
#include "V_Decouple.h"
#include "main.h"

//! Функция расчета компенсирующих добавок напряжения (декаплинг).

//! Формирует feedforward-добавки к выходам ПИ-регуляторов тока осей d и q,
//! компенсируя перекрестные индуктивные связи и ЭДС вращения. С учетом
//! Kdec_gain:
//!   ud_dec = Kdec_gain * (-wr * Kdec_d * iq)
//!   uq_dec = Kdec_gain * ( wr * Kdec_q * id + wr * Kflux)
//! При Kdec_gain = 0 компенсация выключена (поведение прежнее).
//! \memberof TDecouple
void Decouple_calc(TDecouple *p) {
	_iq tmp;

	// Ud_dec = -wr * Kdec_d * iq
	tmp = _IQmpy(p->wr, p->Kdec_d);
	tmp = _IQmpy(tmp, p->iq);
	p->ud_dec = _IQmpy(-tmp, p->Kdec_gain);

	// Uq_dec = wr * Kdec_q * id + wr * Kflux
	tmp = _IQmpy(p->wr, p->Kdec_q);
	tmp = _IQmpy(tmp, p->id);
	tmp += _IQmpy(p->wr, p->Kflux);
	p->uq_dec = _IQmpy(tmp, p->Kdec_gain);
}

//! Медленный (фоновый) расчет коэффициентов.

//! Коэффициенты перекрестных связей Kdec_d и Kdec_q вычисляются из
//! индуктивности фазы статора drv_params.Ls (для двигателей с Ld = Lq).
//! Для асимметричных машин (Ld != Lq) значения можно скорректировать
//! из словаря CANopen после вызова этой функции.
//! Kflux и Kdec_gain функцией не затираются - настраиваются пользователем.
//! \memberof TDecouple
void Decouple_slow_calc(TDecouple *p) {
	p->Kdec_d = drv_params.Ls;
	p->Kdec_q = drv_params.Ls;
}

/*@}*/
