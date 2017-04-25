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
 
 \file      v_rmp_ctrl.c
 \brief     Задатчик интенсивности (см. TRMPCtrl)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 */

/** \addtogroup V_rmp_ctrl */
/*@{*/

#ifdef __cplusplus 
extern "C" {
#endif

#include "v_rmp_ctrl.h"
#include "V_IQmath.h"
#include "stdlib.h"
#ifdef __cplusplus 
}
#endif

//! Функция расчета задатчика интенсивности

//!Изменяет output с заданным темпом T,
//!пока output не достигнет input.

//! \memberof TRMPCtrl
void V_RMP_CTRL_calc(TRMPCtrl *p) {
	//выход меньше входа?
	if (p->output < p->input)
		p->output += p->step;//увелчичиваем с заданным темпом выход
	if (p->output > p->input)
		p->output -= p->step;

	if ((labs(p->output - p->input) <= p->step) || (p->T==0)) {//вошли в задание с точность до шага или ЗИ вообще отключен
		p->output = p->input;//выход равен входу
	}
}

//! Вспомогательная функция задатчика интенсивности

//!Пересчитывает темп разгона T в шаг step c учетом частоты 
//!вызова основной функции Ts (дискретизация, обычно 10кГц).

//! \memberof TRMPCtrl
void V_RMP_CTRL_slow_calc(TRMPCtrl *p) {
	p->step = _IQdiv(p->Ts, p->T);//шаг интегратора ЗИ
}

/*@}*/

