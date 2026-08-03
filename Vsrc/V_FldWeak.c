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

 \file      V_FldWeak.c
 \brief     Модуль ослабления поля синхронной машины
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 2024

 \addtogroup
 @{*/

#include "main.h"

void FldWeak_init(TFldWeak *p) {
	p->integral = 0;
	p->output = 0;
}

void FldWeak_calc(TFldWeak *p) {
	if (!p->Enabled)
		return;

	if (pwm.ULimitation) {
		p->integral -= p->Ki;
	} else {
		p->integral += p->Ki;
	}

	if (p->integral > 0)
		p->integral = 0;
	if (p->integral < -refs.Iq_ref)
		p->integral = -refs.Iq_ref;

	p->output = p->integral;
}

/*@}*/
