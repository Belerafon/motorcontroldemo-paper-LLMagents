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

 \file      V_FieldWeaken.c
 \brief     Модуль ослабления поля
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 25/03/2017

 \addtogroup V_FieldWeaken
 @{ */

#include "main.h"

long FieldWeaken_Calc(long speed, long speed_nom, long Iq_ref, long Id_base) {
    long speed_abs = labs(speed);
    long Id_fw = 0;
    long Id_new = Id_base;

    if (speed_abs > speed_nom && speed_nom > 0) {
        long excess = speed_abs - speed_nom;
        Id_fw = -_IQmpy(excess, _IQdiv(Iq_ref, speed_nom));
        if (Id_fw < _IQ(-0.8)) Id_fw = _IQ(-0.8);
        Id_new = Id_base + Id_fw;
        if (Id_new < _IQ(-0.9)) Id_new = _IQ(-0.9);
    }

    return Id_new;
}

/*@}*/
