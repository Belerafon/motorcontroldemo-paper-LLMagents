/*!
    Copyright 2017 	Texas Instruments, АО "НИИЭТ" , ООО "НПФ ВЕКТОР"

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 \file    clarke.c
 \brief   Модуль фазных преобразований (см. TClarke)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016
 \addtogroup clarke
 @{
 */

#include "V_IQmath.h"         /* Include header for IQmath library */
#include "clarke.h"

//! Функция фазных преобразований 
//! \memberof TClarke
void clarke_calc(TClarke *v) {
	v->ds = v->as;
	v->qs = _IQmpy((v->as + _IQmpy(_IQ(2),v->bs)), _IQ(0.57735026918963));
}

/*@}*/

