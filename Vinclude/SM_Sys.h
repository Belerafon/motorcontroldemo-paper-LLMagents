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
 
 \file      SMSys.h
 \brief     Модуль-обертка для расчета остальных модулей. (см. TSM_Sys)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \defgroup  SMSys Модуль-обертка для расчета остальных модулей. (см. TSM_Sys)
 @{
*/

#ifndef SM_Sys_H
#define SM_Sys_H

#ifdef __cplusplus
extern "C"
{
#endif
  
  //! Система в инициализации
  #define SYS_INIT 		0
  //! Система проинициализирована
  #define SYS_READY 		1


/*! \class TSM_Sys
      \brief Модуль-обертка для расчета остальных модулей.

      Класс \a TSM_Sys, основанный на структуре SSMSys, является модулем,
      внутри методов которого происходит вызов большинства остальных. Например,
      функция быстрого расчета данного модуля вызывает функции быстрого расчета
      остальных модулей. Кроме того, в инициализации данного модуля настраиваются прерывания
      контроллера.
       */

//! см. TSM_Sys
  struct SSM_Sys
  {
    int state;//!< Состояние
    int state_prev;//!< Состояние предыдущее
    int E;//!<Флаг первого вхождения
    void (*init)(struct SSM_Sys*);     /* Pointer to the init funcion           */
    void (*slow_calc)(struct SSM_Sys*);     /* Pointer to the calc funtion           */
    void (*fast_calc)(struct SSM_Sys*);     /* Pointer to the calc funtion           */
    void (*ms_calc)(struct SSM_Sys*);     /* Pointer to the calc funtion           */
  } ;

  typedef struct SSM_Sys TSM_Sys;
  
//!Инициализатор по умолчанию
#define SM_Sys_DEFAULTS {0,0,0,\
    SM_Sys_Init,\
    SM_Sys_Slow_Calc,\
    SM_Sys_Fast_Calc,\
    SM_Sys_ms_Calc,\
  }

  //! \memberof TSM_Sys
  void SM_Sys_Init(TSM_Sys*);
  //! \memberof TSM_Sys
  void SM_Sys_Slow_Calc(TSM_Sys*);
  //! \memberof TSM_Sys
  void SM_Sys_Fast_Calc(TSM_Sys*);
  //! \memberof TSM_Sys
  void SM_Sys_ms_Calc(TSM_Sys*);

#ifdef __cplusplus
}
#endif

#endif

/*@}*/




