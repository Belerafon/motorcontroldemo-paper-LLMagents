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

 \file      V_FldWeak.h
 \brief     Модуль ослабления поля синхронной машины
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 2024

 \addtogroup
 @{*/

#ifndef V_FLD_WEAK_H
#define V_FLD_WEAK_H

#include "build.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SFldWeak {
	long Enabled;
	long Ki;
	long Id_max;
	long integral;
	long output;
	void (*init)(volatile struct SFldWeak*);
	void (*calc)(volatile struct SFldWeak*);
};

typedef volatile struct SFldWeak TFldWeak;

void FldWeak_init(TFldWeak*);
void FldWeak_calc(TFldWeak*);

#define FLD_WEAK_DEFAULTS { \
    .Enabled = 1, \
    .Ki = _IQ(0.0002), \
    .Id_max = _IQ(1.0), \
    .integral = 0, \
    .output = 0, \
    .init = FldWeak_init, \
    .calc = FldWeak_calc \
}

#ifdef __cplusplus
}
#endif

#endif

/*@}*/
