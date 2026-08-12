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

 \file      V_CurDecoupling.c
 \brief     Декаплинг регуляторов тока - компенсация перекрестных связей (см. TCurDecoupling)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru

 \addtogroup V_CurDecoupling
 @{*/

#include "V_IQmath.h"
#include "V_CurDecoupling.h"

//! Функция расчета компенсации перекрестных связей (декаплинга) регуляторов тока

//! На входе:
//!  - omega_e - электрическая угловая скорость в о.е. (1.0 = freq_nom)
//!  - id, iq   - токи осей d, q (обратная связь) в о.е.
//! На выходе:
//!  - ud_decoup - добавка к выходу регулятора оси d на компенсацию перекрестной связи
//!  - uq_decoup - добавка к выходу регулятора оси q на компенсацию перекрестной связи
//!  - uq_emf    - добавка к выходу регулятора оси q на компенсацию ЭДС вращения ПМ
//! При enable == 0 все выходы обнуляются (поведение системы идентично исходной).

//! \memberof TCurDecoupling
void CurDecoupling_calc(TCurDecoupling *p) {
	_iq we;

	if (p->enable == 0) {
		p->ud_decoup = 0;
		p->uq_decoup = 0;
		p->uq_emf = 0;
		return;
	}

	we = p->omega_e;

	// Уравнения СД/АД в осях dq в относительных единицах:
	//   Ud = Rs*Id + (1/w_b)*Lsd*dId/dt - we*Lsq*Iq
	//   Uq = Rs*Iq + (1/w_b)*Lsq*dIq/dt + we*Lsd*Id + we*psi_pm
	// Перекрестные связи и ЭДС вращения компенсируются слагаемыми
	// противоположного знака, чтобы ПИ-регуляторы "видели" только
	// диагональную R-L часть объекта (быстрая реакция на задание на высокой скорости).

	// Компенсация -we*Lsq*Iq в Ud -> добавляем +we*Lsq*Iq
	p->ud_decoup = _IQmpy(_IQmpy(we, p->Lsq), p->iq);

	// Компенсация +we*Lsd*Id в Uq -> добавляем -we*Lsd*Id
	p->uq_decoup = -_IQmpy(_IQmpy(we, p->Lsd), p->id);

	// Компенсация +we*psi_pm в Uq (ЭДС вращения ПМ) -> добавляем -we*psi_pm.
	// При psi_pm = 0 компенсация ЭДС вращения не выполняется.
	p->uq_emf = -_IQmpy(we, p->psi_pm);
}

//! \memberof TCurDecoupling
void CurDecoupling_reset(TCurDecoupling *p) {
	p->omega_e = 0;
	p->id = 0;
	p->iq = 0;
	p->ud_decoup = 0;
	p->uq_decoup = 0;
	p->uq_emf = 0;
}

/*@}*/
