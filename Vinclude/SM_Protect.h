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

 \file      SMProtect.h
 \brief     Модуль защит. (см. TSM_Protect)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \defgroup  SMProtect Модуль защит. (см. TSM_Protect)
 @{
*/
#ifndef SM_Protect_H
#define SM_Protect_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "build.h"

//! Модуль ащит выключен
#define PROT_OFF 			0
//! Модуль защит включен
#define PROT_ON_OK	 		1
//! Модуль защит в состоянии аварии (сработала защита)
#define PROT_FAIL 			2

//Макрос приема аппаратной аварии инвертора: если ноль, то авария
#if defined HW_VECTOR_MK_40_4
#define DRV_FAULT	(NT_GPIOC->DATA & (1 << 8))
#else
#define DRV_FAULT	(NT_GPIOC->DATA & (1 << 4))
#endif

/*! \class TSM_Protect
      \brief Модуль защит.

      Класс \a TSM_Protect, основанный на структуре SSMProtect,
      обрабатывает все возможные аварийные ситуации и возводит соответствующий этой
      аварии бит в одном из слов bit_fault 1-2. далее происходит обработка этих слов, и,
      если эта авария не замаскирована, происходит останов привода.
*/

//! см. TSM_Protect
  struct SSM_Protect
  {
    Uint16 state;//!< Состояние
    Uint16 state_prev;//!< Состояние предыдущее

    Uint16 readDrvSts; //!<Флаг запроса чтения статуса drv8301
    Uint16 clearDrvFault; //!<Флаг запроса сброса аварии drv8301

    Uint16 bit_fault1;//!<Слово аварий 1
    Uint16 bit_fault2;//!<Слово аварий 2

    Uint16 masked_bit_fault1;//!<Слово аварий после маскирования
    Uint16 masked_bit_fault2;//!<Слово аварий после маскирования

    Uint16 mask_fault1;//!<Маска аварий 1
    Uint16 mask_fault2;//!<Маска аварий 2
    
    long Imax_protect; //!<Уставка максимально-токовой защиты
    long Umax_protect; //!<Уставка защиты max напряжения на ЗПТ
    long Umin_protect;//!<Уставка защиты min напряжения на ЗПТ
    long speed_max;//!<Уставка максимальной скорости

    int E;//!<Флаг первого вхождения
    Uint16 Main_ErrorCode;//!< //листающийся код аварии
    Uint32 Main_Flags;//!< //Флаг аварийной блокировки блока
    Uint16 powered_okCounter;//!< Счетчик для ожидания включеня модуля защит после инециализации
    void (*init)(struct SSM_Protect*);     /* Pointer to the init funcion           */
    void (*slow_calc)(struct SSM_Protect*);     /* Pointer to the calc funtion           */
    void (*fast_calc)(struct SSM_Protect*);     /* Pointer to the calc funtion           */
    void (*ms_calc)(struct SSM_Protect*);     /* Pointer to the calc funtion           */
  } ;
  typedef struct SSM_Protect TSM_Protect;

 //!Инициализатор по умолчанию
#define SM_PROTECT_DEFAULTS {0,0,\
    0,0,\
	0,0,\
    0,0,\
    0,0,\
    0,0,0,0,\
    0,0,0,0,\
    SM_Protect_Init,\
    SM_Protect_Slow_Calc,\
    SM_Protect_Fast_Calc,\
    SM_Protect_ms_Calc,\
  }

//! \memberof TSM_Protect
  void SM_Protect_Init(TSM_Protect*);
//! \memberof TSM_Protect
  void SM_Protect_Slow_Calc(TSM_Protect*);
//! \memberof TSM_Protect
  void SM_Protect_Fast_Calc(TSM_Protect*);
//! \memberof TSM_Protect
  void SM_Protect_ms_Calc(TSM_Protect*);

#ifdef __cplusplus
}
#endif

#endif

/*@}*/






