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
 
 \file      vhzprof.h
 \brief     Кривая U(f) (см. TVhzProf)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \defgroup  vhzprof Модуль кривой U(f) (см. TVhzProf)

*/


#ifndef VHZPOF_H
#define VHZPOF_H

#ifdef __cplusplus
extern "C" {
#endif

  /*! \class TVhzProf
      \brief Кривая U(f)
      
      Класс \a TVhzProf, основанный на структуре SVhzProf, 
      представляет собой двумерную зависимость напряжения U (vout) от 
      частоты f (freq). Модуль нужен при скалярном управлении двигателем,
      без контуров тока - когда напряжение растет вместе с частотой. Данный модуль
      позволяет настроить линейную зависимость напряжения от частоты по двум точкам.  */

//! см. TVhzProf
struct 	SVhzProf{ _iq  freq;//!< Input: Input frequency (pu)
                  _iq  vout;//!< Output: Output voltage (pu) 
                  _iq  FL;//!<  Parameter: Low frequency (pu)			
                  _iq  FH;//!<  Parameter: High frequency at rated voltage  (pu) 
                  _iq  Fmax; //!<Parameter: Maximum frequency (pu)		
                  _iq  Vmax;//!< Parameter: Rated voltage (pu) 					  
                  _iq  Vmin;//!< Parameter: Voltage at low frequency range (pu)
                  void  (*calc)(struct 	SVhzProf*);//!<  Pointer to calculation function
                };	    
				               
typedef struct 	SVhzProf TVhzProf;

//! Инициализатор по-умолчанию
#define VHZPROF_DEFAULTS { 0,0, \
                           0,0,0,0,0, \
                  		   vhz_prof_calc }

//! \memberof TVhzProf
void vhz_prof_calc(TVhzProf *v);

#ifdef __cplusplus
}
#endif

#endif

/*@}*/
