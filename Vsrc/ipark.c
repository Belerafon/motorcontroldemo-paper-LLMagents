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
 
 \file    ipark.c
 \brief   Модуль инверсных координатных преобразований координат (см. TIPark)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \addtogroup IPark
 @{
 */

#include "V_IQmath.h"         /* Include header for IQmath library */
#include "ipark.h"

//! Функция инверсных преобразований координат 

//!На входе угол ang, вектор (de,qe).
//!На выходе повернутый на заданный угол вектор (ds,qs)
//! \memberof TIPark
void ipark_calc(TIPark *v) {

	_iq cos_ang, sin_ang;

	sin_ang = _IQsinPU(v->ang);
	cos_ang = _IQcosPU(v->ang);

	v->ds = _IQmpy(v->de,cos_ang) - _IQmpy(v->qe, sin_ang);
	v->qs = _IQmpy(v->qe,cos_ang) + _IQmpy(v->de, sin_ang);
}

/*@}*/

