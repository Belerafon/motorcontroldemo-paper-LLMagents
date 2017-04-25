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
 
 \file    park.c
 \brief   Модуль  координатных преобразований координат (см. TPark)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \addtogroup Park
 @{
 */

#include "V_IQmath.h"         /* Include header for IQmath library */
#include "park.h"

//! Функция  преобразований координат 

//!На входе угол ang, вектор (ds,qs).
//!На выходе повернутый на заданный угол вектор (de,qe)
//! \memberof TPark
void park_calc(TPark *v) {

	_iq cos_ang, sin_ang;

	/* Using look-up IQ sine table */
	sin_ang = _IQsinPU(v->ang);
	cos_ang = _IQcosPU(v->ang);

	v->de = _IQmpy(v->ds,cos_ang) + _IQmpy(v->qs, sin_ang);
	v->qe = _IQmpy(v->qs,cos_ang) - _IQmpy(v->ds, sin_ang);

}

/*@}*/

