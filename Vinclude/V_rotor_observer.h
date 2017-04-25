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

 \file      v_rotor_observer.h
 \brief     Наблюдатель ротора АД  (см. TRotorObserver)
 \author	ООО НПФ Вектор
 \version   v 1.0 28/06/2016

 */


/** \addtogroup TRotorObserver */
/*@{*/


#ifndef ROTOR_OBSERVER_H
#define ROTOR_OBSERVER_H

struct SRotorObserver{
			_iq Ks;
			_iq id;
			_iq iq;
			_iq psi_d;
			_iq psi_q;
			_iq theta_psi_elec;		//!< Угол потокосцепления ротора относительно ДПР
			_iq Tr; 				//!< Постоянная времени АД
			_iq FluxCurrentRatio;	//!< Соотношение тока возбуждения оси x к моментообразующему току оси y
			_iq FluxCurrentMin;		//!< Минимум тока возбуждения
			void (*calc)(struct SRotorObserver*);
			void (*slow_calc)(struct SRotorObserver*);
			};

typedef struct SRotorObserver TRotorObserver;

#define ROTOR_OBSERVER_DEFAULTS {\
						0,0,\
						0,0,0,0,\
						0,0,0,\
						RotorObserver_calc,\
						RotorObserver_slow_calc,\
}
						   
void RotorObserver_calc(TRotorObserver *);
void RotorObserver_slow_calc(TRotorObserver *p);

#endif


/*@}*/
