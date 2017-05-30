/*!
     Copyright 2017 	јќ "Ќ»»Ё“" и ќќќ "Ќѕ‘ ¬≈ “ќ–"

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
 \brief     ƒрайвер CANOpen
 \author    ќќќ "Ќѕ‘ ¬ектор". http://motorcontrol.ru
 \version   v 3.14 2017_01_25

 \defgroup  CANOpen_drv ƒрайвер CANOpen
 @{
*/

#ifndef CAN_OPEN_DRV_H
#define CAN_OPEN_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

//********************************************************************************************
//

//!¬ключатель CAN 1
//#define CAN_1_ENABLE
//********************************************************************************************
//!¬ключатель CAN 2
#define CAN_2_ENABLE
//********************************************************************************************
/*
 * ‘ункции инициализации драйвера дл€ CAN1 и CAN2
 * ƒрайвер использует работу с пользовательской пам€тью (интерфейс с которой определ€етс€ функци€ми co_UserMemoryRead и co_UserMemoryWrite -
 * см. файл CANOpenUDfuncs.c), поэтому к моменту начала работы с драйвером пользователь должен √ј–јЌ“»–ќ¬ј“№,
 * что драйвер пользовательской пам€ти проинициализирован.
 */
extern Uint16 co1_Init(TCo_OdVars* ppc);//¬ случае если инициализация не удалась функция возвращает 0, иначе 1.
extern Uint16 co2_Init(TCo_OdVars* ppc);//¬ случае если инициализация не удалась функция возвращают 0, иначе 1.

//‘ункция переинициализации CAN без восстановления параметров из пользовательской пам€ти (может быть использована лишь для ѕќ¬“ќ–Ќќ… инициализации)
extern Uint16 co_ReInit(TCo_OdVars* ppc);//¬ случае если инициализация не удалась функция возвращают 0, иначе 1.
extern void propReset (void);
//ѕрототипы функций
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


//!функци€ драйвера CANOpen вызываема€ в прерывании 1 мс таймера, тактирующего работу драйвера
extern void co_1ms_calc(TCo_OdVars*);

//!функци€ обработчика драйвера CANOpen в фоновой программе
extern void co_background_calc(TCo_OdVars*);

//!‘ункци€ восстановлени€ значений параметров словар€ объектов по умолчанию
/*!
–езультатом работы функции €вл€етс€ восстановление параметров указанного
типа и диапазона
type - 	1 rw-параметры
		2 rwp-параметры
		3 rwps-параметры
range - 1 восстановление индексов 1000h-1FFFh
		2,3,4,5
‘ункци€ по времени выполнени€ относитс€ к длиным (вызывать следует в фоне) */
extern void co_ODexpositor_instaurationDefault(TCo_OdVars* ppc, Uint16 type, Uint16 range);

//!‘ункци€ сохранени€ значений параметров словар€ объектов в Ёнќ«”
/*!–езультатом работы функции €вл€етс€ сохранение параметров указанного
типа и диапазона
type - 	1 rw-параметры
		2 rwp-параметры
		3 rwps-параметры
range - 1 восстановление индексов 1000h-1FFFh
		2,3,4,5
‘ункци€ по времени выполнени€ относитс€ к длиным (вызывать следует в фоне).*/
extern void co_ODexpositor_paramConservation(TCo_OdVars* ppc, Uint16 type, Uint16 range);

//!‘ункци€ восстановлени€ значений параметров словар€ объектов из Ёнќ«”
/*! –езультатом работы функции €вл€етс€ восстановление параметров указанного
типа и диапазона
type - 	1 rw-параметры
		2 rwp-параметры
		3 rwps-параметры
range - 1 восстановление индексов 1000h-1FFFh
		2,3,4,5
¬нимание!!!¬осстановление параметров 2-го и 3-го типов возможно если
переменна€ co_protectBit = 0. ≈сли co_protectBit = 1 , то функци€ ничего
не "делает".
≈сли восстановление параметров заданного типа и диапазона произошло
неудачно(контрольна€ сумма не сошлась), то восстановление заданного
типа и диапазона повтор€етс€.≈сли после трех попыток восстановление
не завершилось успехом, то выполн€етс€ восстановление параметров заданного
типа и диапазона по умолчанию.
‘ункци€ по времени выполнени€ относитс€ к длиным (вызывать следует в фоне).*/
extern void co_ODexpositor_paramInstauration(TCo_OdVars* ppc, Uint16 type, Uint16 range);

// функции сервиса NMT
// драйвер может выступать NMT мастером, либо быть NMT слэйвом
//! функци€ дл€ отправки NMT  (нужна только дл€ NMT мастера)
void co_sendNMT(TCo_OdVars* ppc, Uint16 nmtCommand, Uint16 nmtNODE_ID);

//!функци€ ѕќЋ№«ќ¬ј“≈Ћ€ дл€ отправки SDO запросов на чтение параметров сервера
extern void co_SDOrequestRead(TCo_OdVars*);     
//!функци€ ѕќЋ№«ќ¬ј“≈Ћ€ дл€ отправки SDO запросов на запись параметров сервера
extern void co_SDOrequestWrite(TCo_OdVars*);   

extern void Z_co_receiveSDOrequest(TCo_OdVars*, TZCanMsg*);
extern void co_CANToExtInterface_Send(TZCanMsg* MSG, Uint16 tag_CANnum);

/*!‘ункция возвращает адрес переменной по переданным индексу
 и подындексу. ≈сли "связанная" переменная 32-х разрядная, то
 возвращается указатель на старшую часть.
 ¬Ќ»ћјЌ»≈!!!≈сли запрошенного индекса и подиндекса не существует, то
 функция возвращает 0 (это нужно обязательно отслеживать). */
extern Uint16 * co_getAddr(TCo_OdVars* ppc, Uint32 ind_subind);
// ‘ункци€ получени€ информации об объекте с заданным адресом.
// ¬озвращает 0, если объект не найден.
extern Uint16 co_getObjectInfo(
 TCo_OdVars* ppc,   		// указатель на структуру используемого модул€ CANopen
 Uint32 ind_subind,   		// биты 16-31 - индекс, биты 0-7 - подындекс объекта
 TObjectInfo* pObjectInfo 	// указатель на структуру, куда будет помещена информаци€ об объекте
 );

extern void co_RPDO1_Callback(Uint16 nodeID, Uint16 tag_CANnum);
extern void co_RPDO2_Callback(Uint16 nodeID, Uint16 tag_CANnum);
extern void co_RPDO3_Callback(Uint16 nodeID, Uint16 tag_CANnum);
extern void co_RPDO4_Callback(Uint16 nodeID, Uint16 tag_CANnum);
extern void co_RPDO5_Callback(Uint16 nodeID, Uint16 tag_CANnum);
extern void co_RPDO6_Callback(Uint16 nodeID, Uint16 tag_CANnum);
extern void co_RPDO7_Callback(Uint16 nodeID, Uint16 tag_CANnum);
extern void co_RPDO8_Callback(Uint16 nodeID, Uint16 tag_CANnum);


//функции ѕќЋ№«ќ¬ј“≈Ћ€, инициирующие передачу, или прием блока данных посредством 
//сервиса блочной передачи
extern void CANBlockTransferInitTX(TCo_OdVars* ppc, Uint16 size, Uint16* source_ptr);
extern void CANBlockTransferInitRX(TCo_OdVars* ppc, Uint16 size, Uint16* dest_ptr);


#ifdef __cplusplus
}
#endif


#endif


/*@}*/

