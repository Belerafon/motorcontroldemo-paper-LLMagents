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

 \file      V_excitation.h
 \brief     Модуль возбудителя
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 28/03/2017

\defgroup  V_excitation

@{*/



#ifndef V_excitation_H
#define V_excitation_H

#ifdef __cplusplus
extern "C" {
#endif

  /*! \class Texcitation
    \brief Модуль возбудителя

    Класс \a Texcitation, основанный на структуре Sexcitation, представляет
    собой модуль возбудителя.
  
  */

//! см. Texcitation
typedef struct Sexcitation{  long  ref; //!< Задание
		 	 	  void  (*calc)(struct Sexcitation*);//!< Pointer to calculation function
				 }Texcitation;


//! Инициализатор по-умолчанию
#define excitation_DEFAULTS { 0, \
		excitation_calc}



//! \memberof TRelayReg
void excitation_calc(Texcitation*);



#ifdef __cplusplus
}
#endif

#endif

/*@}*/
