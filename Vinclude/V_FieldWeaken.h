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

 \file      V_FieldWeaken.h
 \brief     Модуль ослабления поля
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 25/03/2017

 \addtogroup V_FieldWeaken
 @{ */

#ifndef V_FIELD_WEAKEN_H
#define V_FIELD_WEAKEN_H

#include "V_IQmath.h"

#ifdef __cplusplus
extern "C" {
#endif

long FieldWeaken_Calc(long speed, long speed_nom, long Iq_ref, long Id_base);

#ifdef __cplusplus
}
#endif

#endif

/*@}*/
