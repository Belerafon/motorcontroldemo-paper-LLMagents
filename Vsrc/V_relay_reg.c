/*!
     Copyright 2017 	ÀÎ "ÍÈÈİÒ" è ÎÎÎ "ÍÏÔ ÂÅÊÒÎĞ"

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 
 \file      V_relay_reg.c
 \brief     Ğåëåéíûé ğåãóëÿòîğ
 \author    ÎÎÎ "ÍÏÔ Âåêòîğ". http://motorcontrol.ru
 \version   v 1.0 23/01/2016

 \addtogroup V_realy_reg
 @{*/

#include "V_IQmath.h"         // Include header for IQmath library
#include "V_relay_reg.h"
//! Ôóíêöèÿ ğàñ÷åòà ğåëåéíîãî ğåãóëÿòîğà

//! \memberof TRelayReg
void relay_reg_calc(TRelayReg *v)
{
	if (v->ref < v->fdb)
		v->out = _IQ(-1.0);
	else
		v->out = _IQ(1.0);
}

/*@}*/
