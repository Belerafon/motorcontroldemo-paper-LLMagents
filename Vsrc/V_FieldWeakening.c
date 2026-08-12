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


//!Инициализация.

//!Обнуление интегратора и выхода. Коэффициенты и ограничения задаются
//!инициализатором FIELD_WEAKENING_DEFAULTS или извне (через словарь CANopen).

//! \memberof TFieldWeakening
void FieldWeakening_init(volatile struct SFieldWeakening *p) {
	p->ui = 0;
	p->uprsat = 0;
	p->Id_ref = 0;
	p->state = 0;
}


//!Сброс (вызывать при перезапуске/останавливе системы управления).

//!Обнуляет интегратор и выход, чтобы при новом запуске не было выброса
//!заданного тока ослабления, накопленного в предыдущем цикле работы.

//! \memberof TFieldWeakening
void FieldWeakening_reset(volatile struct SFieldWeakening *p) {
	p->ui = 0;
	p->uprsat = 0;
	p->Id_ref = 0;
	p->state = 0;
}


//!Расчет.

//!Интегральный (с опциональной пропорциональной частью) регулятор
//!ошибки по напряжению. Вызывать в быстром прерывании (обычно 10 кГц)
//!после того, как стал известен pwm.U_mag с прошлого такта расчёта,
//!и ДО расчёта регуляторов тока. На вход Us подаётся амплитуда
//!требуемого напряжения, на вход Umax - порог (максимально реализуемое
//!напряжение, обычно pwm.U_lim).
//!
//!Если Us < Umax (ниже базовой скорости) - ошибка положительная,
//!интегратор удерживается на уровне Id_max (обычно 0), ослабления нет.
//!Если Us > Umax (превышение напряжения) - ошибка отрицательная,
//!интегратор уходит в минус, формируется отрицательный Id - ослабление поля.
//!Анти-наброс выполнен ограничением интегратора в диапазоне [Id_min, Id_max].

//! \memberof TFieldWeakening
void FieldWeakening_calc(volatile struct SFieldWeakening *p) {
	long error;
	long up;

	if (!p->Enabled) {
		//если ослабление запрещено - Id = 0 (исходное поведение СУ)
		p->Id_ref = 0;
		p->state = 0;
		return;
	}

	//ошибка по напряжению: при Us > Umax становится отрицательной -> Id уходит в минус
	error = p->Umax - p->Us;

	//пропорциональная часть (обычно Kp = 0, чистый интегратор)
	up = _IQmpy(p->Kp, error);

	//интегральная часть
	p->ui = p->ui + _IQmpy(p->Ki, error);

	//предварительный выход
	p->uprsat = up + p->ui;

	//ограничение интегратора (анти-наброс методом фиксации)
	if (p->uprsat > p->Id_max) {
		//насыщение в плюс - ослабления нет, прижимаем интегратор к Id_max
		p->ui = p->Id_max - up;
		if (p->ui > p->Id_max)
			p->ui = p->Id_max;
		p->Id_ref = p->Id_max;
		p->state = 0;
	} else if (p->uprsat < p->Id_min) {
		//насыщение в минус - достигнут предел тока ослабления
		p->ui = p->Id_min - up;
		if (p->ui < p->Id_min)
			p->ui = p->Id_min;
		p->Id_ref = p->Id_min;
		p->state = 1;
	} else {
		//рабочая точка внутри диапазона
		p->Id_ref = p->uprsat;
		//считаем, что ослабление активно, если Id ушёл в минус
		p->state = (p->Id_ref < 0) ? 1 : 0;
	}
}

/*@}*/
