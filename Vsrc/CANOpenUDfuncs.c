/* ==================================================================================
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

//File name:       CANOpenUDfuncs.c

//Description: В файл вынесены функции драйвера CANOpen определяемые пользователем (колбэк функции, функции инициализации и т.п.)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 2017_02_08

//=====================================================================================*/
#include "DSP.h"
#include "co_ODvars.h"
#include "CANOpen_drv.h"
#include "main.h"

//**********************************************************************************************************************************************************************************************************
//функция инициализации Gpio CAN1
#ifdef CAN_1_ENABLE
void co_CAN1GpioInit()
{
	//выбираем альтернативную функцию ГПИО
	NT_COMMON_REG->GPIOPCTLB_bit.PIN14 = 1; //CAN_TX[0]
	NT_COMMON_REG->GPIOPCTLB_bit.PIN15 = 1; //CAN_RX[0]

	//переводим ГПИО на выполнение функции CAN
	NT_GPIOB->ALTFUNCSET = (1 << 14) + (1 << 15); //CAN_TX[0], CAN_RX[0]
}
#endif //CAN_1_ENABLE

//функция инициализации Gpio CAN2
#ifdef CAN_2_ENABLE
void co_CAN2GpioInit()
{
	//выбираем альтернативную функцию ГПИО
	NT_COMMON_REG->GPIOPCTLF_bit.PIN14 = 0; //CAN_TX[1]
	NT_COMMON_REG->GPIOPCTLF_bit.PIN15 = 0; //CAN_RX[1]

	//переводим ГПИО на выполнение функции CAN
	NT_GPIOF->ALTFUNCSET = (1 << 14) + (1 << 15); //CAN_TX[0], CAN_RX[0]
}
#endif //CAN_2_ENABLE
//**********************************************************************************************************************************************************************************************************
//Функции работы с энергонезависимой памятью (в качестве памяти может использоваться как пользовательская флэш на борту MCU,
// так и внешне подключаемая, например через SPI, eeprom).
//Функции используются драйвером CANopen при сохранении и восстановлении параметров словаря объектов
//С точки зрения драйвера CANOpen функции реализуют побайтовое чтение и запись из/в ЭнОЗУ
void co_UserMemoryRead (const T_UserMemoryContext *p)
{
	UserMem.MemStartAddr = p->MemStartAddr;
	UserMem.MCUStartAddr = p->MCUStartAddr;
	UserMem.data_length = p->data_length;
	UserMem.read(&UserMem);
}

void co_UserMemoryWrite (const T_UserMemoryContext *p)
{
	UserMem.MemStartAddr = p->MemStartAddr;
	UserMem.MCUStartAddr = p->MCUStartAddr;
	UserMem.data_length = p->data_length;
	UserMem.write(&UserMem);
}
//**********************************************************************************************************************************************************************************************************

//функция, которая вызывается драйвером CANOpen при необходимости отправки CAN сообщения во внешний интерфейс
//внутрь функции пользователем должен быть вставлен вызов соответствующей функции, осуществляющей передачу CAN пакета
void co_CANToExtInterface_Send(TZCanMsg* MSG, Uint16 tag_CANnum)
{
	CANtoRS.write(MSG,&CANtoRS);
}



//колбэки по приему PDO
void co_RPDO1_Callback(Uint16 nodeID, Uint16 tag_CANnum)
{
	/*
	switch(nodeID)
	{
		case 1:
		{
			//обрабатываем как-нибудь...
			break;
		}
		case 2:
		{
			PDO_cntr2++;
			//обрабатываем как-нибудь...
			break;
		}
		default:
			break;
	}
	*/
}

void co_RPDO2_Callback(Uint16 nodeID, Uint16 tag_CANnum)
{
	/*
	switch(nodeID)
	{
		case 1:
		{
			//обрабатываем как-нибудь...
			break;
		}
		default:
			break;
	}
	*/
}

void co_RPDO3_Callback(Uint16 nodeID, Uint16 tag_CANnum)
{
	/*
	switch(nodeID)
	{
		case 1:
		{
			//обрабатываем как-нибудь...
			break;
		}
		default:
			break;
	}
	*/
}

void co_RPDO4_Callback(Uint16 nodeID, Uint16 tag_CANnum)
{
	/*
	switch(nodeID)
	{
		case 1:
		{
			//обрабатываем как-нибудь...
			break;
		}
		default:
			break;
	}
	*/
}

void co_RPDO5_Callback(Uint16 nodeID, Uint16 tag_CANnum)
{
	/*
	switch(nodeID)
	{
		case 1:
		{
			//обрабатываем как-нибудь...
			break;
		}
		default:
			break;
	}
	*/
}

void co_RPDO6_Callback(Uint16 nodeID, Uint16 tag_CANnum)
{
	/*
	switch(nodeID)
	{
		case 1:
		{
			//обрабатываем как-нибудь...
			break;
		}
		default:
			break;
	}
	*/
}

void co_RPDO7_Callback(Uint16 nodeID, Uint16 tag_CANnum)
{
	/*
	switch(nodeID)
	{
		case 1:
		{
			//обрабатываем как-нибудь...
			break;
		}
		default:
			break;
	}
	*/
}

void co_RPDO8_Callback(Uint16 nodeID, Uint16 tag_CANnum)
{
	/*
	switch(nodeID)
	{
		case 1:
		{
			//обрабатываем как-нибудь...
			break;
		}
		default:
			break;
	}
	*/
}



//!Обращение по CANopen к часам реального времени.

//!Если происходит запись в параметр [5139h.01h] Текущ. знач.Часы реального времени,
//то вызывается callback_RTC - там устанавливается запрос на коррекцию часов.

void callback_RTC(Uint16 par, Uint16 tag_CANnum) {
	if (par == 1) {
		spiclock.setTimeFlag = 1; //произошла запись в часы

	}
}

//!Обращение к переменной dlog.next_value_var с данными цифрового осциллографа.

//!Существует специальный интерфейс, позволяющий автоматизировать считывание оссциллографируемых переменных
//!модуля TDataLog. Эта функция вызывается при обращении к переменной, через которую возможно считать записанные
//!осциллограммы. Запись в эту переменную устанавливает требуемый номер буфера (один и 4х) и номер извлекаемой точки.
//!При чтении же в эту переменную выдвигатются сами отосциллограффированные данные.

void callback_dlog(Uint16 par, Uint16 tag_CANnum) {
	//если запись, то установим буфер на нужную точку
	if (par == 1) {
		//в переменную next_value_var записывается желаемое смещение, номер буфера, из которого хотим читать,
		//а так же разреживание
		dlog.buff_num = (int32) dlog.next_value_var >> 24; //извлекается номер буфера
		dlog.Rcounter = (dlog.next_value_var >> 16) & 0xff; //какую точку надо отдавать

		//проверяем хотят ли использовать блочную передачу
		if ((dlog.next_value_var & 0xffff) == 1)
		{
			//хотят, говорим об этом интерфейсу блочной передачи
			#ifdef CAN_1_ENABLE
				if((tag_CANnum == 1) && (co1_vars.settings.BlockTransfer_ena))
					if (co1_vars.co_blockTransferCommand == CANBT_INTERFACE_FREE)
						co1_vars.co_blockTransferCommand = CANBT_INTERFACE_DATALOG;
			#endif
			#ifdef CAN_2_ENABLE
				if((tag_CANnum == 2) && (co2_vars.settings.BlockTransfer_ena))
					if (co2_vars.co_blockTransferCommand == CANBT_INTERFACE_FREE)
						co2_vars.co_blockTransferCommand = CANBT_INTERFACE_DATALOG;
			#endif
		}
	}

	//выдвигаем данные:
	switch (dlog.buff_num) { //текущий номер буфера
	case 0: {
		dlog.next_value_var = dlog.dl_buffer1_adr[(dlog.Rcounter + dlog.first_point_written) & 0xFF];
		break;
	}
	case 1: {
		dlog.next_value_var = dlog.dl_buffer2_adr[(dlog.Rcounter + dlog.first_point_written) & 0xFF];
		break;
	}
	case 2: {
		dlog.next_value_var = dlog.dl_buffer3_adr[(dlog.Rcounter + dlog.first_point_written) & 0xFF];
		break;
	}
	case 3: {
		dlog.next_value_var = dlog.dl_buffer4_adr[(dlog.Rcounter + dlog.first_point_written) & 0xFF];
		break;
	}
	default: { //если в buff_num что-то не то, по-дефолту пусть первый буфер
		dlog.next_value_var = dlog.dl_buffer1_adr[(dlog.Rcounter + dlog.first_point_written) & 0xFF];
		break;
	}
	}
	//данные только в нижней части
	dlog.next_value_var &= 0xFFFF;
	dlog.next_value_var |= (((Uint32) dlog.Rcounter) << 16); //в верхней части слова Rcounter для контроля
	dlog.next_value_var |= (((Uint32) dlog.buff_num) << 24); //в верхней части слова (еще выше) buff_num для контроля

	dlog.Rcounter++; //какую точку отдаем. инкрементируется само после чтения текущей точки.
	dlog.Rcounter &= 0xFF; //по кругу. ничего страшного, перед чтением обнуляю. см. запись
}
