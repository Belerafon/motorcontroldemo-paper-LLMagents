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

//! Инициализация.

//! Сбрасывает состояние регулятора. Параметры (U_ref, Ki, ...)
//! остаются теми, что заданы инициализатором структуры или
//! переприсвоены внешним кодом (например, Debug-переменными).
//! \memberof TFieldWeakening
void FieldWeakening_init(volatile struct SFieldWeakening *p) {
	p->U_mag = 0;
	p->err = 0;
	p->up = 0;
	p->ui = 0;
	p->Id_out = 0;
	p->active = 0;
}

//! Сброс регулятора.

//! Обнуляет интегратор и выход. Вызывается при останове привода
//! и при входе в режим векторного управления, чтобы ослабление поля
//! всегда начиналось с нулевого задания Id.
//! \memberof TFieldWeakening
void FieldWeakening_reset(volatile struct SFieldWeakening *p) {
	p->ui = 0;
	p->up = 0;
	p->Id_out = 0;
	p->err = 0;
	p->active = 0;
}

//! Быстрый расчет ослабления поля.

//! Вызывается в прерывании 10 кГц после pwm.update(), когда уже
//! сформирован pwm.U_mag - амплитуда вектора напряжения статора
//! в относительных единицах IQ24.
//!
//! Алгоритм - ПИ-регулятор с антираскруткой, отслеживающий уставку
//! U_ref:
//! - пока U_mag < U_ref, ошибка положительна, интегратор стремится
//!   вверх и ограничивается нулём, Id_out = 0 (ослабление не нужно,
//!   поведение СУ полностью совпадает со старым кодом);
//! - при U_mag > U_ref ошибка отрицательна, интегратор уходит вниз,
//!   Id_out становится отрицательным - это отрицательный ток оси d,
//!   который компенсирует поток ротора и снижает противо-ЭДС, что
//!   позволяет двигателю разгоняться выше базовой скорости.
//!
//! Выход ограничен диапазоном [Id_min; 0], то есть ослабление поля
//! только уменьшает поток (Id_out <= 0), никогда не намагничивая
//! машину сверх задания базового контура.
//! \memberof TFieldWeakening
void FieldWeakening_calc(volatile struct SFieldWeakening *p) {
	p->U_mag = pwm.U_mag;

	if (!p->Enabled) {
		//Ослабление поля запрещено - прозрачный обход: задаём Id = 0
		//(тот же эффект, что и исходный код режима CTRL_RUN_VECTOR_SM).
		p->Id_out = 0;
		p->ui = 0;
		p->up = 0;
		p->err = 0;
		p->active = 0;
		return;
	}

	//Ошибка регулирования: > 0, если напряжения хватает (ослабление не нужно).
	p->err = p->U_ref - p->U_mag;

	//Пропорциональная и интегральная составляющие.
	p->up = _IQmpy(p->Kp, p->err);
	p->ui = p->ui + _IQmpy(p->Ki, p->err);

	//Преднасыщенный выход.
	long presat = p->up + p->ui;

	//Ограничение выхода диапазоном [Id_min; 0].
	long out;
	if (presat > 0)
		out = 0;
	else if (presat < p->Id_min)
		out = p->Id_min;
	else
		out = presat;

	//Антираскрутка интегратора обратной связью по насыщению.
	p->ui = p->ui + _IQmpy(p->Kc, out - presat);

	//Дополнительная страховка от накапливания интегратора вне диапазона.
	if (p->ui > 0)
		p->ui = 0;
	else if (p->ui < p->Id_min)
		p->ui = p->Id_min;

	p->Id_out = out;
	p->active = (out < 0) ? 1 : 0;
}

/*@}*/
