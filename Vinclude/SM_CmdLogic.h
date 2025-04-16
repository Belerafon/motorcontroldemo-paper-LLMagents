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

 \file      SMCmdLogic.h
 \brief     Обработка команд и заданий, поступающих из различных источников. (см. TSM_CmdLogic)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \defgroup  SMCmdLogic Обработка команд и заданий, поступающих из различных источников. (см. TSM_CmdLogic)
 @{
*/


#ifndef SM_CmdLogicH
#define SM_CmdLogicH

#include "V_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


//! Система остановлена, готова к запуску (ГОТОВ)
#define CMD_LOGIC_TURNED_OFF 		0x0
//! Состояние РАБОТА
#define CMD_LOGIC_TURNED_ON			0x2

#if defined (HW_VECTORCARD_DRV8301_EVM) || defined (HW_VECTORCARD_DRV8312_EVM)
#define	STARTBUTTON	((NT_GPIOA->DATA & (1 << 14))==0)	//обработка ножки старта
#define	STOPBUTTON	((NT_GPIOA->DATA & (1 << 13))==0)	//обработка ножки стопа
#define	RESETBUTTON 	0
#endif

#if defined (HW_MOTORCONTROLBOARD)
//Для управления контроллером мы используем внешние дискретные входы с использованием фильтрации (см. V_DIO)
#define	STARTBUTTON (BIT_IS_SET(input_vect, 0) ? 1 : 0)	//Проверка на уставку бита в векторе дискретных входов
#define	STOPBUTTON 	(BIT_IS_SET(input_vect, 1) ? 1 : 0)	//Проверка на уставку бита в векторе дискретных входов
#define	RESETBUTTON 	(BIT_IS_SET(input_vect, 2) ? 1 : 0)	//Проверка на уставку бита в векторе дискретных входов

#endif

#if !defined (HW_MOTORCONTROLBOARD) && !defined (HW_VECTORCARD_DRV8301_EVM) && !defined (HW_VECTORCARD_DRV8312_EVM)
#define	STARTBUTTON		0
#define	STOPBUTTON		0
#define	RESETBUTTON 	0
#endif

/*! \class TSM_CmdLogic
      \brief Обработка команд и заданий, поступающих из различных источников.

      Класс \a TSM_CmdLogic, основанный на структуре SSMCmdLogic, служит для обработки команд
      оперативного управления и задания скорости. Команды могут поступать как по CANOpen,
      так и по другим протоколам (ModBus). Содержит дискретный автомат, который переключает 
      систему между состояниями РАБОТА и ОСТАНОВ (ГОТОВ).
*/

//! см. TSM_CmdLogic
  struct SSM_CmdLogic
  {
    int state;//!<Cостояние
    int state_prev;//!<Предыдущее состояние
    int E;//!<Флаг первого вхождения
    TCmd cmd_by_level_prev;//!<Предыдущее состояние командного слова
    int ReRunCounter;//!< Счетчик времени с момента предыдущего запуска системы
    int StartButtonPrev;
    int StartButton;
    int StopButtonPrev;
    int StopButton;
    int ResetButton;
    int ResetButtonPrev;
    void (*init)(volatile struct SSM_CmdLogic*);//!<    Pointer to the init funcion
    void (*slow_calc)(volatile struct SSM_CmdLogic*);//!< Pointer to the calc funtion
    void (*ms_calc)(volatile struct SSM_CmdLogic*); //!< Pointer to the calc funtion
  };

  typedef volatile  struct SSM_CmdLogic TSM_CmdLogic;
  
//! Инициализатр по умолчанию
#define SM_CMD_LOGIC_DEFAULTS { .cmd_by_level_prev = {0}, .init = SM_CmdLogicInit,\
    .slow_calc = SM_CmdLogicSlow_Calc,\
    .ms_calc = SM_CmdLogicms_Calc }

//! \memberof TSM_CmdLogic
  void SM_CmdLogicInit(TSM_CmdLogic*);
//! \memberof TSM_CmdLogic
  void SM_CmdLogicSlow_Calc(TSM_CmdLogic*);
//! \memberof TSM_CmdLogic
  void SM_CmdLogicms_Calc(TSM_CmdLogic*);


#ifdef __cplusplus
}
#endif

#endif


/*@}*/





