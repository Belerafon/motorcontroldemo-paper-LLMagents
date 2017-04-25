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

 \file      V_ModBus.h
 \brief     Драйвер ModBus (см. TModBus)
 \author	Alecksey Anuchin
 \version   v 1.1 24/03/2017
 \defgroup  V_ModBus Драйвер ModBus (см. TModBus)
 @{
*/

#ifndef V_MODBUS_H
#define V_MODBUS_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "mbod.h"


#define MODBUS_MAX_RX_COUNT 10

/*! \class TModBus
      \brief Драйвер ModBus

      Класс \a TModBus, основанный на структуре SModBus, обрабатывает
      входящие посылки по RS-485 как посылки ModBus (RTU). */

//! см. TModBus


 //! Структура modbus для хранения внутренних данных
 typedef struct {volatile Uint16 In[MODBUS_MAX_RX_COUNT];//!<Массив для принятия данных
 				volatile Uint16 InCount;//!<Счетчик для массива принятых данных
 				volatile MB_Record /*const*/ *Addr;//!<Указатель на словарь
 				volatile Uint16 TimeOut;//!<Текущий таймаут, который надо выдерживать
 				volatile Uint16 TimeOut1_5;//!<Рассчитанный таймаут 1,5 длины переданного байта
 				volatile Uint16 TimeOut2_5;//!<Рассчитанный таймаут 2,5 длины переданного байта
 				volatile Uint16 ToSend;//!<Данные для отправки
 				volatile Uint16 NumOfHRs;//!<Количество Holding Registers
 				volatile Uint16 MiddleOfHRs;//!<Середина Holding Registers
 				volatile Uint16 InitialStepOfHRs;//!<Шаг Holding Registers
 				volatile Uint16 NumOfIRs;//!<Количество Input Registers
 				volatile Uint16 MiddleOfIRs;//!<Середина Input Registers
 				volatile Uint16 InitialStepOfIRs;//!<Шаг Input Registers
 				volatile Uint16 NumOfCs;//!<Количество Coils
 				volatile Uint16 MiddleOfCs;//!<Середина Coils
 				volatile Uint16 InitialStepOfCs;//!<Шаг Coils
 				volatile Uint16 NumOfDIs;//!<Количество Discrete Inputs
 				volatile Uint16 MiddleOfDIs;//!<Середина Discrete Inputs
 				volatile Uint16 InitialStepOfDIs;//!<Шаг Discrete Inputs
 				volatile Uint16 RxCRC;//!<Принимаемая контрольная сумма
 				volatile Uint16 TxCRC;//!<Передаваемая контрольная сумма
 				volatile Uint16 MagicNumber;//!<Эм...
 				volatile int16 TxState;//!<Состояние передачи
 				volatile Uint16 temp;//!<
 				volatile int16 RecievedCounter;//!<Счетчик для отсчиывания таймаута по приему
 				NT_UART_TypeDef *NT_UART;//!Указатель на используемый модуль UART
 				} MODBUS_INT;

 #define MODBUS_INT_DEFAULTS {{0},0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
 				0xFFFF,0xFFFF, 0xA001,-1, 0,0,  \
 				}

struct SModBus{Uint32 BaudRate;//!<Скорость работы по RS
				Uint16 ExecutionFreq;//!< Частота тактирования модуля
				Uint16 RSNodeID; //!< Номер узла в сети
				Uint16 Enabled; //!< Разрешение изменения параметров привода через ModBus
				Uint16 Refresh;	//!< Флаг изменения параметров по Modbus
				int16 error;
				int16 errorCode;
				int16 AutoRestart;
				int16 clear_error;
				Uint16 received_data;
				Uint16 GPIOsValue;
				Uint16 isOnline;//!< В сети мы или нет (идут ли пакеты)
				Uint32 OfflineCounter;//!< Счетчик инкрементируется с частотой вызова процедуры ModBus_Execute и обнуляется, если в сети есть пакеты
				Uint32 OfflineCounterMax;//!< Таймаут отсутствия пакетов в сети (в единицах счёта OfflineCounter)
				Uint16 ReceiveByteCounter;//!< Счетчик принятых байт
                Uint16 ReceivePacketCounter;//!< Счетчик принятых посылок
                MODBUS_INT MBInternals;//!< Служебные переменные для работы драйвера (не для пользователя)
                void (*Init)(volatile struct SModBus*);
                void (*Execute)(volatile struct SModBus*);
                };


typedef volatile struct SModBus TModBus;


#define MODBUS_DEFAULTS {9600,10000, 0x01, 1, 0,\
				0,0,0,0,0,0,0,0,0,0,0,\
				MODBUS_INT_DEFAULTS,\
                ModBus_Init, \
                ModBus_Execute,\
                }


//! \memberof TModBus
void ModBus_Init(TModBus *p);
//! \memberof TModBus
void ModBus_Execute(TModBus *p);
//! \memberof TModBus
int16 ModBus_FindVocAddr(TModBus *p,volatile MB_Record Table[], Uint16 TableSize, int16 Type, Uint16 Index, int16 NumOfIndexs, int16 Mid, int16 Step);



#ifdef __cplusplus
}
#endif

#endif

/*@}*/
