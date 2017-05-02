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
    void (*init)(volatile struct SSM_CmdLogic*);//!<    Pointer to the init funcion
    void (*slow_calc)(volatile struct SSM_CmdLogic*);//!< Pointer to the calc funtion
    void (*ms_calc)(volatile struct SSM_CmdLogic*); //!< Pointer to the calc funtion
  };

  typedef volatile  struct SSM_CmdLogic TSM_CmdLogic;
  
//! Инициализатр по умолчанию
#define SM_CMD_LOGIC_DEFAULTS {0,0,0,{0},0,0,0,0,0,\
    SM_CmdLogicInit,\
    SM_CmdLogicSlow_Calc,\
    SM_CmdLogicms_Calc}

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





