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

 \file      v_rotor_observer.c
 \brief     Наблюдатель ротора АД  (см. TRotorObserver)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 28/06/2016

 */

/** \addtogroup TRotorObserver */
/*@{*/


#include "main.h"

#ifdef __cplusplus
#pragma CODE_SECTION("secureRamFuncs")
#else
#pragma CODE_SECTION(RotorObserver_calc,"secureRamFuncs");
#endif

void RotorObserver_calc(TRotorObserver *p) {
	//инерционное звено с параметрами ротора
	p->psi_d = p->psi_d + _IQmpy(p->Ks,p->id - p->psi_d);
	p->psi_q = p->psi_q + _IQmpy(p->Ks,p->iq - p->psi_q);
    p->theta_psi_elec = _IQatan2PU(p->psi_q ,p->psi_d);
}

void RotorObserver_slow_calc(TRotorObserver *p) {
	p->Ks=_IQdiv(FAST_CALC_TS,p->Tr);
}

/*@}*/

