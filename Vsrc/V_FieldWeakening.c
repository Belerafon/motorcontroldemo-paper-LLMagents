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
 \brief     Модуль автоматического ослабления поля (см. TFieldWeakening)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 06/08/2026

 \addtogroup
 @{*/

#include "V_IQmath.h"
#include "V_FieldWeakening.h"


//!Расчет ослабления поля.

//!Вызывать в быстром прерывании (10кГц) один раз за такт системы управления.
//!На вход подаются амплитуда запрошенного напряжения v_mag и ограничение
//!напряжения v_lim (в одних единицах). Амплитуда сглаживается фильтром, чтобы
//!кратковременные броски при разгоне не давали ложных срабатываний. Когда
//!отфильтрованная амплитуда устойчиво превышает ограничение (с учётом зоны
//!нечувствительности), интегратор накапливает размагничивающий ток, а выход
//!id_ref уходит в отрицательную область. Ниже ограничения интегратор плавно
//!спадает, и ослабление выключается.

//! \memberof TFieldWeakening
void FieldWeakening_calc(TFieldWeakening *p) {
	long err_over;
	long err_dz;
	long w_target;
	long id_new;
	long delta;

	//выключено - выдаём нулевое задание по оси d (прежнее поведение)
	if (!p->enabled) {
		p->ui = 0;
		p->err = 0;
		p->out = 0;
		p->id_ref = 0;
		return;
	}

	//сглаживаем амплитуду напряжения
	p->fV.input = p->v_mag;
	p->fV.calc(&p->fV);

	//превышение ограничения напряжения по отфильтрованному значению
	err_over = p->fV.output - p->v_lim;

	//зона нечувствительности: реагируем только на устойчивое превышение
	if (err_over > p->deadband)
		err_dz = err_over - p->deadband;
	else
		err_dz = 0;

	if (err_dz > 0) {
		//накапливаем интегральную составляющую
		p->ui += _IQmpy(p->ki, err_dz);
	} else {
		//ниже ограничения - спадаем к нулю
		p->ui = _IQmpy(p->ui, p->decay);
	}

	//ограничения интегральной составляющей
	if (p->ui < 0)
		p->ui = 0;
	if (p->ui > p->w_max)
		p->ui = p->w_max;

	//требуемое ослабление - сумма пропорциональной и интегральной составляющих
	w_target = _IQmpy(p->kp, err_dz) + p->ui;
	if (w_target < 0)
		w_target = 0;
	if (w_target > p->w_max)
		w_target = p->w_max;

	//ограничение скорости изменения задания, чтобы не было броска тока
	id_new = -w_target;
	delta = id_new - p->id_ref;
	if (delta > p->id_step_max)
		id_new = p->id_ref + p->id_step_max;
	else if (delta < -p->id_step_max)
		id_new = p->id_ref - p->id_step_max;

	//диагностика
	p->err = err_dz;
	p->out = -id_new;

	//задание тока по оси d - отрицательное при ослаблении поля
	p->id_ref = id_new;
}

/*@}*/
