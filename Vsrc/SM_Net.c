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

 \file      SMNet.c
 \brief     Модуль общего назначения для работы с сетью.  (см. TSM_Net)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 */

/** \addtogroup SMNet */
/*@{*/

#include "DSP.h"
#include "V_IQmath.h"
#include "main.h"

//!Инициализация.

//!Присвоение масштабирующих коэффициентов, инициализация других модулей.
//! \memberof TSM_Net
void SM_Net_Init(TSM_Net *p) {

	p->state = 0;
	p->state_prev = 0xff;

	//Настройка драйвера CANOpen
	co2_vars.settings.LoadParamsFromUserMemory_ena = 1;//Разрешить загружать значения параметров из ЭНОЗУ
	co2_vars.settings.CAN_ISR_priority = IRQ_PRIORITY_CAN;//Присвоение приоритета перывания CAN (обработчик внутри библиотеки)
	co2_vars.settings.MultiPDO_ena = 0;//Отключение обработки PDO без учета номера узла (спец. функция)
	co2_vars.settings.RX_PDO_Callback_ena = 0; //Не вызывать функции обратного вызова при приеме PDO
	co2_vars.settings.AutoBusON_ena = 1; //Автоматически перезапускать CAN при ошибках на линии
	co2_vars.settings.BlockTransfer_ena = 1;//Разрешить блочную передачу (нужна для осциллографа)
	co2_Init(&co2_vars); //Инициализация драйвера CANOpen

	//логгер событий:
	//	размер буфера: 50
	//	стартовый адрес в SPI: 3000
	FaultLog.init(&FaultLog, 50, 3000, &RTCclock.packed_time);



#ifdef CANTORS_ENA
//Работа с CANopen через UART (параллельно с CAN, нет конфликта)
	CANtoRS.nodeID = (Uint16*)&co2_vars.co_nodeID; //Номер узла из CANopen
    CANtoRS.callback = Z_co_receiveSDOrequest;//Указатель на функцию обратного вызова из драйвера CANopen по приходу SDO сообщения
	CANtoRS.init(&CANtoRS);
#endif
#ifdef MODBUS_ENA
	// Настраиваем MODBUS
	ModBus.ExecutionFreq = 10000;//Частота вызова расчета модуля модбас в кГц
	ModBus.Init(&ModBus);
	MBVarsConv.init(&MBVarsConv);
#endif
}

//!Медленный расчет.

//! Присвоение масштабирующих коэффициентов, фоновый расчет некоторых модулей.

//! \memberof TSM_Net
void SM_Net_Slow_Calc(TSM_Net *p) {

	co2_vars.co_scaleNum0 = 1; //без масштабирования
	co2_vars.co_scaleNum1 = 100;	// %
	co2_vars.co_scaleNum2 = drv_params.freq_nom;	//Гц
	co2_vars.co_scaleNum3 = drv_params.U_nom;//напряжение фазное номинальное амплитудное
	co2_vars.co_scaleNum4 = drv_params.I_nom; //Ток фазный базовый
	co2_vars.co_scaleNum5 = 1;
	co2_vars.co_scaleNum6 = 1;
	co2_vars.co_scaleNum7 = drv_params.Udc_nom; //напряжение ЗПТ базовое
	co2_vars.co_scaleNum8 = drv_params.power; //18 в формате 9.6 //Мощность
	co2_vars.co_scaleNum9 = 1;
	co2_vars.co_scaleNumA = 20; //температура
	co2_vars.co_scaleNumB = 1;
	co2_vars.co_scaleNumC = 1;
	co2_vars.co_scaleNumD = 1;
	co2_vars.co_scaleNumE = 1;
	co2_vars.co_scaleNumF = 1;
	co2_vars.co_scaleNum10 = 1; //
	co2_vars.co_scaleNum11 = 1; //
	co2_vars.co_scaleNum12 = drv_params.power; //18 в формате 10.6 //Мощность
	co2_vars.co_scaleNum13 = 360; //
	co2_vars.co_scaleNum14 = drv_params.speed_nom; //номинальная частота вращения
	co2_vars.co_scaleNum15 = 1; //
	co2_vars.co_scaleNum16 = 1; //
	co2_vars.co_scaleNum17 = ((((long) drv_params.Udc_nom) << 6)
			/ (long) drv_params.I_nom); //Rбаз 10.6
	co2_vars.co_scaleNum18 = 1000;
	co2_vars.co_scaleNum19 = 1;
	co2_vars.co_scaleNum1A = 1;
	co2_vars.co_scaleNum1B = 1;
	co2_vars.co_scaleNum1C = 1;
	co2_vars.co_scaleNum1D = 1;
	co2_vars.co_scaleNum1E = 1;
	co2_vars.co_scaleNum1F = 1;

	drv_params.U_nom = 0.5759 * drv_params.Udc_nom;//номинальное фазное напряжение
	drv_params.power = ((long) 300) << 6; //приведем к формату 10.6
	drv_params.freq_nom = ((float) drv_params.speed_nom * drv_params.p) / 60 + 0.5; //номин частота

	co_background_calc(&co2_vars); //Расчет драйвера CANOpen фоновый
	drv_interface.calc(&drv_interface);
	FaultLog.background_calc(&FaultLog);
#ifdef MODBUS_ENA
	MBVarsConv.slow_calc(&MBVarsConv);
#endif

	if (RTCclock.year != 165)    //часы работают?
		FaultLog.time_ptr = &RTCclock.packed_time;
	else
		FaultLog.time_ptr = (Uint32*) &global_time.PowerOn_time;
}

//!Быстрый расчет

//!Быстрый расчет некоторых сетевых модулей, вызов методов "пролистывания" кодов аварий и предупреждений
//! \memberof TSM_Net
void SM_Net_ms_Calc(TSM_Net *p) {
	//Перевод битового состояния системы управления в константу.
	//По drv_status_code отображается статус привода в UniCON
	if (drv_status.bit.ready == 1)
		drv_status_code = DRV_STATUS_READY;
	if (drv_status.bit.running == 1)
		drv_status_code = DRV_STATUS_RUNNING;
	if (drv_status.bit.testing == 1)
		drv_status_code = DRV_STATUS_TESTING;
	if (drv_status.bit.fault == 1)
		drv_status_code = DRV_STATUS_FAULT;
	if (drv_status.bit.alarm == 1)
		drv_status_code |= DRV_STATUS_ALARM;
	else
		drv_status_code &= ~DRV_STATUS_ALARM;
	//функция листания аварий для пульта (UniCON)
	pult_faults_lister.calc(&pult_faults_lister);
	sm_prot.Main_ErrorCode = 0xFF & pult_faults_lister.output;

	co_1ms_calc(&co2_vars);    //Расчет драйвера CANOpen
#ifdef CANTORS_ENA
	CANtoRS.calc(&CANtoRS);
#endif
#ifdef MODBUS_ENA
	MBVarsConv.calc(&MBVarsConv);
#endif

}

void SM_Net_fast_Calc(TSM_Net *p) {

#ifdef MODBUS_ENA
	ModBus.Execute(&ModBus);
#endif
}

/*@}*/
