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
 \brief     Модуль ослабления поля синхронной машины (см. TFieldWeakening)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0

 \addtogroup
 @{*/

#include "V_IQmath.h"
#include "V_FieldWeakening.h"

//!Расчет.

//!Автоматическое ослабление поля. Вызывать в быстром прерывании (например
//!10 кГц) перед расчетом регулятора тока оси d векторной системы управления
//!синхронной машины. Когда амплитуда вектора напряжения \a U_meas превышает
//!допустимую \a U_lim, ПИ-регулятор формирует отрицательное задание тока по
//!оси d (ток размагничивания), чем ослабляет поле и позволяет поднять скорость
//!выше базовой. При снижении напряжения ниже предела задание возвращается к нулю.

//! \memberof TFieldWeakening
void FieldWeakening_calc(TFieldWeakening *p) {
	long err;
	long out;

	if (p->Enabled == 0) {	//ослабление поля запрещено
		p->Id_ref = 0;	//задание тока оси d - ноль
		p->ui = 0;	//сброс интегратора
		return;
	}

	//ошибка напряжения: положительная - напряжение ниже предела,
	//отрицательная - напряжение превысило допустимое (вход в ослабление поля)
	err = p->U_lim - p->U_meas;

	//интегральная составляющая с ограничением анти-насыщения
	p->ui += _IQmpy(p->Ki, err);
	if (p->ui > 0)	//выше нуля интегратор не поднимаем
		p->ui = 0;
	if (p->ui < p->Id_min)	//и не допускаем чрезмерного ослабления
		p->ui = p->Id_min;

	//пропорционально-интегральная составляющая
	out = _IQmpy(p->Kp, err) + p->ui;

	//задание может быть только отрицательным (ослабление) и не более Id_min
	if (out > 0)
		out = 0;
	if (out < p->Id_min)
		out = p->Id_min;

	p->Id_ref = out;	//выдача задания тока оси d
}

/*@}*/
