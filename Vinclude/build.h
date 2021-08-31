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

 \file      build.h
 \brief     Конфигурационный заголовочный файл
 Содержит глобальные макроопределения, используемые во всем проекте.
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 3.0 30/05/2017

 @{
*/


//Выбор типа аппаратной части

//VectorCARD с отладочным комплектом DRV8301
//#define HW_VECTORCARD_DRV8301_EVM

//VectorCARD с отладочным комплектом DRV8312
//#define HW_VECTORCARD_DRV8312_EVM

//VectorCARD с имитационными моделями двигателей
//#define HW_VECTORCARD_SIMULATOR

//Отладочная плата НИИЭТ с имитационными моделями двигателей
//#define HW_NIIET_BOARD_SIMULATOR

//Контроллер МК40.4 НПФ ВЕКТОР
//#define HW_VECTOR_MK_40_4

//Контроллер Motor Control Board
#define HW_MOTORCONTROLBOARD

//Для UART, выбирается чем его занять - MODBUS или CANtoRS драйвером
//#define MODBUS_ENA
#define CANTORS_ENA

// Также можно задействовать USB для CANopen
//#define CANTOUSB_ENA

//!Режимы работы (состояния ДА sm_ctrl)
//! Режим останов
#define CTRL_STOP 							0
//! Режим запуска (включение ШИМ, переход в другой желаемый режим работы)
#define CTRL_RUN 							1
//! Режим намагничивание (постоянный ток), работает контур тока
#define CTRL_FLUXING 				        2
//! Режим скалярного управления по кривой U(f) без контуров тока
#define CTRL_RUN_U2F				        3
//! Режим вращения вектора тока
#define CTRL_RUN_I2F				        4
//! Векторная структура управления двухконтурная с датчиком для синхронной машины
#define CTRL_RUN_VECTOR_SM    				5
//! Векторная структура управления с энкодером и с контуром положения для синхронной машины
#define CTRL_RUN_VECTOR_SM_ENCODER_POS		10

//! Векторное управление для асинхронного двигателя с энкодером с контуром скорости
#define CTRL_RUN_VECTOR_IM_ENCODER			55

//! Векторное управление для асинхронного двигателя с энкодером с контуром скорости и положения
#define CTRL_RUN_VECTOR_IM_ENCODER_POS		60

//! Режим калибровки датчика положения
#define CTRL_AUTO_SENSOR_TUNING		        29

//! Структура управления двигателем постоянного тока без контуров: задание напряжения на обмотки
#define CTRL_DC_DRIVE						35
//! Структура управления двигателем постоянного тока с контуром тока и скорости
#define CTRL_DC_DRIVE_FB					36

//! Режим автокоммутации для SRM
#define CTRL_RUN_SRM_AUTOCOMMUTATION		101




//Флаги аварий для модуля защит

//bit_fault1
#define F_CTRL_LOW_UDC          	0x1
#define F_CTRL_HI_UDC       	 	0x2
#define F_ENCODER_FAULT             0x4
#define F_MODEL_FAULT 	        	0x8
#define F_FLASH_PROGRAM         	0x10
#define F_CTRL_HI_TEMP  		    0x20
#define F_CAN_RESTORATION_ERROR		0x40
//#define F_RESERVED       		    0x80
#define F_PROGRAM_1K	          	0x100
#define F_PROGRAM_10K        		0x200
#define F_PROGRAM_40K	        	0x400
//#define F_RESERVED	        	0x800
#define F_PDPINT		       		0x1000
#define F_RELAY_START			    0x2000
//#define F_RESERVED	       		0x4000
//#define F_RESERVED				0x8000

//bit_fault2
//#define F_RESERVED				0x1
//#define F_RESERVED				0x2
//#define F_RESERVED				0x4
//#define F_RESERVED				0x8
#define F_CTRL_MAX_I_PH_A			0x10                  
#define F_CTRL_MAX_I_PH_B			0x20                  
#define F_CTRL_MAX_I_PH_C		    0x40
//#define F_RESERVED				0x80
//#define F_RESERVED				0x100
#define F_CTRL_SPEED_MAX			0x200  
//#define F_RESERVED				0x400
//#define F_RESERVED				0x800
#define F_PDP_SWITCH_A0				0x1000
#define F_PDP_SWITCH_A1				0x2000
#define F_PDP_SWITCH_A2				0x4000
#define F_PDP_SWITCH_LOWER			0x8000


#define POS_SENSOR_TYPE_NO_SENSOR 0
#define POS_SENSOR_TYPE_ENCODER 1
#define POS_SENSOR_TYPE_HALL 2
#define POS_SENSOR_TYPE_ENC_HALL 3
#define POS_SENSOR_TYPE_SSI 4
#define POS_SENSOR_TYPE_SENSORLESS 5

//!Время дискретизации быстрого расчета (прерывание, которое обычно 10кГц)
#define FAST_CALC_FREQ 10000
#define FAST_CALC_TS _IQ(0.0001)



#if (!defined(HW_VECTORCARD_DRV8301_EVM)) && (!defined(HW_VECTORCARD_DRV8312_EVM)) && (!defined(HW_VECTORCARD_SIMULATOR))\
	&& (!defined(HW_NIIET_BOARD_SIMULATOR)) 	&& (!defined(HW_VECTOR_MK_40_4)) && (!defined(HW_MOTORCONTROLBOARD))
#error необходимо определить хотя бы одну версию аппаратной части
#endif


/*@}*/
