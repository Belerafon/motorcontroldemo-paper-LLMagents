/*!
     Copyright 2017 	Texas Instruments, АО "НИИЭТ" и ООО "НПФ ВЕКТОР"

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 
 \file      V_hzprof.c
 \brief     Кривая U(f) (см. TVhzProf)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \addtogroup vhzprof 
 */

#include "V_IQmath.h"         /* Include header for IQmath library */
#include "V_hzprof.h"
#include <stdlib.h>

//! Функция расчета кривой U(f)

//!На вход принимает частоту freq, и исходя из настроенных значений
//!минимального напряжения на минимальной частоте и максимального
//!напряжения на максимальной частоте, выдает vout - напряжение
//!для частоты freq. Другими словами, модуль представляет собой
//!линейный интерполятор по двум реперным точкам с ограничением максимума и
//!минимума.

//! \memberof TVhzProf
void vhz_prof_calc(TVhzProf *v) {
	_iq vf_slope, freq_abs;

	freq_abs = labs(v->freq);

	if (freq_abs <= v->FL)
		v->vout = v->Vmin; /* Profile #1  */
	else if ((freq_abs > v->FL) & (freq_abs <= v->FH)) {
		vf_slope = _IQdiv((v->Vmax - v->Vmin), (v->FH - v->FL)); /* Computing v/f slope */
		v->vout = v->Vmin + _IQmpy(vf_slope, (freq_abs - v->FL)); /* Profile #2  */
	} else if ((freq_abs > v->FH) & (freq_abs < v->Fmax))
		v->vout = v->Vmax; /* Profile #3  */
}

/*@}*/

