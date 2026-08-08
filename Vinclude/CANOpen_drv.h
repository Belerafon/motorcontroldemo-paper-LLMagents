/*!
     Copyright 2017     АО "НИИЭТ" и ООО "НПФ ВЕКТОР"

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 \file      CANOpen_drv.h
 \brief     Драйвер CANOpen
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 3.19 2019_03_06

 \defgroup  CANOpen_drv Драйвер CANOpen
 @{
*/

#ifndef CAN_OPEN_DRV_H
#define CAN_OPEN_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

//********************************************************************************************
//

//!Включатель CAN 1
#if !defined (HW_VECTORCARD_DRV8301_EVM) && !defined (HW_VECTORCARD_DRV8312_EVM)
#define CAN_1_ENABLE
#endif
//********************************************************************************************
//!Включатель CAN 2
#define CAN_2_ENABLE
//********************************************************************************************
/*
 * Функции инициализации драйвера для CAN1 и CAN2
 * Драйвер использует работу с пользовательской памятью (интерфейс с которой определяется функциями co_UserMemoryRead и co_UserMemoryWrite -
 * см. файл CANOpenUDfuncs.c), поэтому к моменту начала работы с драйвером пользователь должен ГАРАНТИРОВАТЬ,
 * что драйвер пользовательской памяти проинициализирован.
 */
extern Uint16 co1_Init(TCo_OdVars* ppc);//В случае если инициализациЯ не удалась функциЯ возвращает 0, иначе 1.
extern Uint16 co2_Init(TCo_OdVars* ppc);//В случае если инициализациЯ не удалась функциЯ возвращают 0, иначе 1.

//ФункциЯ переинициализации CAN без восстановлениЯ параметров из пользовательской памяти (может быть использована лишь длЯ ПОВТОРНОЙ инициализации)
extern Uint16 co_ReInit(TCo_OdVars* ppc);//В случае если инициализациЯ не удалась функциЯ возвращают 0, иначе 1.
extern void propReset (void);
//Прототипы функций
extern void co_UserMemoryRead (const T_UserMemoryContext *p);
extern void co_UserMemoryWrite (const T_UserMemoryContext *p);

#ifdef CAN_1_ENABLE
extern void co_CAN1GpioInit();
extern void co_CAN1INTEnable();
extern void co_CAN1INTDisable();
#endif

#ifdef CAN_2_ENABLE
extern void co_CAN2GpioInit();
extern void co_CAN2INTDisable();
extern void co_CAN2INTEnable();
#endif


//!функция драйвера CANOpen вызываемая в прерывании 1 мс таймера, тактирующего работу драйвера
extern void co_1ms_calc(TCo_OdVars*);

//!функция обработчика драйвера CANOpen в фоновой программе
extern void co_background_calc(TCo_OdVars*);

//!Функция восстановления значений параметров словаря объектов по умолчанию
/*!
Результатом работы функции является восстановление параметров указанного
типа и диапазона
type -  1 rw-параметры
        2 rwp-параметры
        3 rwps-параметры
range - 1 восстановление индексов 1000h-1FFFh
        2,3,4,5
Функция по времени выполнения относится к длиным (вызывать следует в фоне) */
extern void co_ODexpositor_instaurationDefault(TCo_OdVars* ppc, Uint16 type, Uint16 range);

//!Функция сохранения значений параметров словаря объектов в ЭнОЗУ
/*!Результатом работы функции является сохранение параметров указанного
типа и диапазона
type -  1 rw-параметры
        2 rwp-параметры
        3 rwps-параметры
range - 1 восстановление индексов 1000h-1FFFh
        2,3,4,5
Функция по времени выполнения относится к длиным (вызывать следует в фоне).*/
extern void co_ODexpositor_paramConservation(TCo_OdVars* ppc, Uint16 type, Uint16 range);

//!Функция восстановления значений параметров словаря объектов из ЭнОЗУ
/*! Результатом работы функции является восстановление параметров указанного
типа и диапазона
type -  1 rw-параметры
        2 rwp-параметры
        3 rwps-параметры
range - 1 восстановление индексов 1000h-1FFFh
        2,3,4,5
Внимание!!!Восстановление параметров 2-го и 3-го типов возможно если
переменная co_protectBit = 0. Если co_protectBit = 1 , то функция ничего
не "делает".
Если восстановление параметров заданного типа и диапазона произошло
неудачно(контрольная сумма не сошлась), то восстановление заданного
типа и диапазона повторяется.Если после трех попыток восстановление
не завершилось успехом, то выполняется восстановление параметров заданного
типа и диапазона по умолчанию.
Функция по времени выполнения относится к длиным (вызывать следует в фоне).*/
extern void co_ODexpositor_paramInstauration(TCo_OdVars* ppc, Uint16 type, Uint16 range);

// функции сервиса NMT
// драйвер может выступать NMT мастером, либо быть NMT слэйвом
//! функция для отправки NMT  (нужна только для NMT мастера)
void co_sendNMT(TCo_OdVars* ppc, Uint16 nmtCommand, Uint16 nmtNODE_ID);

//!функция ПОЛЬЗОВАТЕЛя для отправки SDO запросов на чтение параметров сервера
extern void co_SDOrequestRead(TCo_OdVars*);     
//!функция ПОЛЬЗОВАТЕЛя для отправки SDO запросов на запись параметров сервера
extern void co_SDOrequestWrite(TCo_OdVars*);   

extern void Z_co_receiveSDOrequest(TCo_OdVars*, TZCanMsg*);
extern void co_CANToExtInterface_Send(TZCanMsg* MSG, Uint16 tag_CANnum);

/*!ФункциЯ возвращает адрес переменной по переданным индексу
 и подындексу. Если "свЯзаннаЯ" переменнаЯ 32-х разрЯднаЯ, то
 возвращаетсЯ указатель на старшую часть.
 ВНИМАНИЕ!!!Если запрошенного индекса и подиндекса не существует, то
 функциЯ возвращает 0 (это нужно обЯзательно отслеживать). */
extern Uint16 * co_getAddr(TCo_OdVars* ppc, Uint32 ind_subind);
// Функция получения информации об объекте с заданным адресом.
// Возвращает 0, если объект не найден.
extern Uint16 co_getObjectInfo(
 TCo_OdVars* ppc,           // указатель на структуру используемого модуля CANopen
 Uint32 ind_subind,         // биты 16-31 - индекс, биты 0-7 - подындекс объекта
 TObjectInfo* pObjectInfo   // указатель на структуру, куда будет помещена информация об объекте
 );

extern void co_RPDO1_Callback(Uint16 nodeID, Uint16 tag_CANnum);
extern void co_RPDO2_Callback(Uint16 nodeID, Uint16 tag_CANnum);
extern void co_RPDO3_Callback(Uint16 nodeID, Uint16 tag_CANnum);
extern void co_RPDO4_Callback(Uint16 nodeID, Uint16 tag_CANnum);
extern void co_RPDO5_Callback(Uint16 nodeID, Uint16 tag_CANnum);
extern void co_RPDO6_Callback(Uint16 nodeID, Uint16 tag_CANnum);
extern void co_RPDO7_Callback(Uint16 nodeID, Uint16 tag_CANnum);
extern void co_RPDO8_Callback(Uint16 nodeID, Uint16 tag_CANnum);


//функции ПОЛЬЗОВАТЕЛя, инициирующие передачу, или прием блока данных посредством 
//сервиса блочной передачи
extern void CANBlockTransferInitTX(TCo_OdVars* ppc, Uint16 size, Uint16* source_ptr);
extern void CANBlockTransferInitRX(TCo_OdVars* ppc, Uint16 size, Uint16* dest_ptr);


#ifdef __cplusplus
}
#endif


#endif


/*@}*/

