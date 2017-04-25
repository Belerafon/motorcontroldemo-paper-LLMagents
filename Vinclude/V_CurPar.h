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
 
 \file      V_CurPar.h
 \brief     Расчет наблюдаемых текущих параметров  (см. TCurPar)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \defgroup  V_CurPar Расчет наблюдаемых текущих параметров (см. TCurPar)

 @{
*/


#ifndef V_CURPAR_H
#define V_CURPAR_H

#include "filter.h"

#ifdef __cplusplus
extern "C" {
#endif


    /*! \class TCurPar
      \brief Расчет наблюдаемых текущих параметров 

      Класс \a TCurPar, основанный на структуре SCurPar, содержит
      ряд разрозненных вычислений для отображения текущих параметров привода.
      Например, расчет действующих токов фаз, текущей мощности и т.п.
      */

//! см. TCurPar
struct SCurPar{
    long speed; //!< Текущая частота вращения
    long power; //!< Текущая мощность
    long Is; //!< Текущая ток статора амплитудный мгновенный
    long ThetaRefCurr; //!< Заданное угловое положение
    long ThetaCurr; //!< Текущее угловое положение

    long IsRef; //!< Ток статора заданный
    long Ialpha; //!< Ток по оси альфа
    long Ibeta; //!< Ток по оси бетта

    long Ualpha; //!< Напряжение по оси альфа
    long Ubeta; //!< Напряжение по оси бетта
    long PowerK; //!< Служебный параметр для масштабирования мощности

    TFilter fPower; //!< Фильтр мощности

    void (*init)(volatile struct SCurPar*);//!< процедура инициализации
    void (*calc)(volatile struct SCurPar*);//!<расчет
    void (*slow_calc)(volatile struct SCurPar*);//!<медленный расчет
};

typedef volatile struct SCurPar TCurPar;


//!инициализатор по-умолчанию
#define TCUR_PAR_DEFAULTS \
{\
	0,0,0,0,0,\
	0,0,0,\
	0,0,0,\
	FILTER_DEFAULTS,\
	&CurPar_init, \
	&CurPar_calc, \
	&CurPar_slow_calc, \
}


//! \memberof TCurPar
void CurPar_init(TCurPar*);
//! \memberof TCurPar
void CurPar_calc(TCurPar*);
//! \memberof TCurPar
void CurPar_slow_calc(TCurPar*);



#ifdef __cplusplus
}
#endif	// extern "C"

#endif	// V_UF_H

/*@}*/


