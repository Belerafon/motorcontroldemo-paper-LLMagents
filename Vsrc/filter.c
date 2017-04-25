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

 \file        filter.c
 \brief Инерционное звено в IQ математике  (см. TFilter)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 */

/** \addtogroup filter */
/*@{*/

#include "DSP.h"
#include "filter.h"
#include "V_IQmath.h"

//! Инерционное звено 1/(Tp+1)
//! Используется как фильтр первого порядка

//!Должна вызываться с заданной дискретизацией. Предварительно необходимо определить
//! переменную, связанную с постоянной времени:
//! T=Ts/Tfiltra где - Tfiltra постоянная времени фильтра, а
//! Ts - время дискретизации вызова фунции
//! \memberof TFilter
void TFilter_Calc(TFilter *p) {
	p->output = _IQmpy(p->T,(p->input-p->output)) + p->output;
}

/*@}*/

