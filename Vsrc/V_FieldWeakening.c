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

 \file      V_FieldWeakening.c
 \brief     Модуль ослабления поля (см. TFieldWeakening)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0

 \addtogroup V_FieldWeakening
 @{*/

#include "V_IQmath.h"
#include "V_FieldWeakening.h"
#include "main.h"


//! Инициализация модуля ослабления поля.

//! Сбрасывает интегральную составляющую регулятора, выход и амплитуду
//! напряжения. Настройки (Enabled, U_lim, Kp, Ki, Id_ref_min) не меняются.
//! \memberof TFieldWeakening
void FieldWeakening_init(TFieldWeakening *p) {
	p->ui = 0;
	p->Id_ref_out = 0;
	p->U_mag = 0;
}


//! Расчёт модуля ослабления поля.

//! Вызывается в быстром прерывании (как правило, 10 кГц) перед расчётом
//! регуляторов тока. На вход подаются напряжения Ud, Uq предыдущего такта
//! (выходы регуляторов тока pid_id, pid_iq). Амплитуда вектора напряжения
//! сравнивается с уставкой U_lim. При превышении уставки ошибка становится
//! отрицательной и интегратор формирует отрицательное задание Id, которое
//! через pid_id уменьшает поток двигателя и, как следствие, противо-ЭДС,
//! что позволяет превысить номинальную скорость вращения.
//! Выход Id_ref_out всегда неположительный и ограничен снизу величиной
//! Id_ref_min. На низкой скорости выход равен нулю - ослабление поля не
//! вмешивается в работу контура тока.
//! \memberof TFieldWeakening
void FieldWeakening_calc(TFieldWeakening *p) {
	long u_err;
	long up;

	if (!p->Enabled) {
		p->Id_ref_out = 0;
		p->ui = 0;
		p->U_mag = 0;
		return;
	}

	//амплитуда вектора напряжения, сформированного регуляторами тока
	p->U_mag = _IQmag(p->Ud_input, p->Uq_input);

	//ошибка по напряжению:
	//  > 0 - запас по напряжению есть, ослабление поля не требуется;
	//  < 0 - напряжения не хватает, нужно "утопить" поток (отрицательный Id)
	u_err = p->U_lim - p->U_mag;

	//пропорциональная часть работает только "в минус" -
	//только тогда, когда действительно нужно ослабление поля
	up = _IQmpy(p->Kp, u_err);
	if (up > 0)
		up = 0;

	//интегральная часть
	p->ui = p->ui + _IQmpy(p->Ki, u_err);

	//анти-windup: интеграл не должен становиться положительным,
	//т.к. положительный Id для ослабления поля не нужен
	if (p->ui > 0)
		p->ui = 0;
	//и не должен уходить глубже допустимого отрицательного Id
	if (p->ui < p->Id_ref_min)
		p->ui = p->Id_ref_min;

	//выход - сумма пропорциональной и интегральной частей
	p->Id_ref_out = up + p->ui;

	//ограничение выхода: только неположительные значения, не ниже Id_ref_min
	if (p->Id_ref_out < p->Id_ref_min)
		p->Id_ref_out = p->Id_ref_min;
	if (p->Id_ref_out > 0)
		p->Id_ref_out = 0;
}

/*@}*/
