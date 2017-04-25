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
 
 \file      SMCtrl.h
 \brief     Основной ДА переключения систем управления (см. TSM_Ctrl)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \defgroup  SMCtrl Основной ДА переключения систем управления  (см. TSM_Ctrl)
 @{
*/

#ifndef SM_Ctrl_H
#define SM_Ctrl_H

#ifdef __cplusplus
extern "C"
{
#endif


/*! \class TSM_Ctrl
      \brief Основной ДА переключения систем управления

      Класс \a TSM_Ctrl, основанный на структуре SSMCtrl, содержит
      основной дискретный автомат переключения систем управелния. В каждом из
      состояний данного автомата реализована та или иная структура управления.
      Структура собирается из готовых блоков. Обычно используеся модуль ШИМ,
      фазные и координатные преобразования, АЦП, ЗИ и т.п. Структуры управления могут быть:
      скалярное управление, замкнутая по току система управления, векторное управление и т.п.
      В run_mode задается номер желаемой структуры управления.
       */

//! см. TSM_Ctrl

  struct SSM_Ctrl
  {
    long ctrl_counter;//!<Счетчик времени нахождения в одном состоянии
    long run_mode;//!Заданный режим работы (заданное состояние). Векторное управление, скалярное, постоянный ток и т.п.
    int state;//!<Состояние дискретного автомата - оно же текущая структура управления.
    int state_prev;//!<Предыдущее состояние
    int E;//!<Флаг первого вхождения.
    long AnglePrev;//!< Предыдущее значение угла для режима автонастройки ДПР
    long AngleDemo;//!< Угол для демо-режима контура положения
    void (*init)(struct SSM_Ctrl*);     //!< Pointer to the init funcion
    void (*slow_calc)(struct SSM_Ctrl*);//!< Pointer to the calc funtion
    void (*fast_calc)(struct SSM_Ctrl*); //!< Pointer to the calc funtion
    void (*ms_calc)(struct SSM_Ctrl*);  //!< Pointer to the calc funtion
  } ;

  typedef struct SSM_Ctrl TSM_Ctrl;

//!Инициализатор по умолчанию
#define SM_CTRL_DEFAULTS {0,0,0,0,0,0,0,\
    SM_Ctrl_Init,\
    SM_Ctrl_Slow_Calc,\
    SM_Ctrl_Fast_Calc,\
	SM_Ctrl_ms_Calc,\
    }

  //! \memberof TSM_Ctrl
  void SM_Ctrl_Init(TSM_Ctrl*);
  //! \memberof TSM_Ctrl
  void SM_Ctrl_Slow_Calc(TSM_Ctrl*);
  //! \memberof TSM_Ctrl
  void SM_Ctrl_Fast_Calc(TSM_Ctrl*);
  //! \memberof TSM_Ctrl
  void SM_Ctrl_ms_Calc(TSM_Ctrl*);

#ifdef __cplusplus
}
#endif


#endif


/*@}*/




