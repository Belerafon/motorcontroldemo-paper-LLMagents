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

 \file        filter.h
 \brief  Инерционное звено в IQ математике (см. TFilter)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \defgroup  filter Инерционный фильтр (см. TFilter)

 @{
*/


#ifndef FILTER_H
#define FILTER_H

#ifdef __cplusplus
extern "C"
{
#endif

  
/*! \class TFilter
      \brief Инерционное звено в IQ математике

      Класс \a TFilter, основанный на структуре SFilter, представляет из себя
      инерционное звено в терминах ТАУ, а именно W(p)=1/(Tfiltra*p+1).
      Обычно оно используется в качестве инерционно фильтра, на вход которого
      подается зашумленный сигнал, а на выходе оказывается отфильтрованный.*/

//! см. TFilter
  struct SFilter
  {
    long input;//! Вход
    long output;//! Выход
    long T; //! T=Ts/Tfiltra где - Tfiltra постоянная времени фильтра
    void (*calc)(volatile struct SFilter*);//указатель на функцию расчета
  };

  typedef volatile struct SFilter TFilter;

  //! инициализатор по-умолчанию
#define FILTER_DEFAULTS {0,0,_IQ(0.01), \
	 TFilter_Calc}

 //! \memberof TFilter
void TFilter_Calc(TFilter*);

#ifdef __cplusplus
}
#endif


#endif

/*@}*/





