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

//File name:       CANBlockTransferInterface.c
//
//Originator:	Aliamkin Dmitry I
//
//Description:   все взаимодействия с блочной передачей по CAN осуществляются
				 в ДА данного файла
//ПРИМЕЧАНИЕ:    
//=====================================================================================
//-------------------------------------------------------------------------------------
		20.09.2012	Release	Rev 1.04
					Добавил автоматическое освобождение инетрпретатора при простое в CANBT_INTERFACE_BUSY более 5 секунд
		20.09.2012	Release	Rev 1.03
					Правильнее сделан выход из состояния CANBT_INTERFACE_DATALOG
		19.09.2012	Release	Rev 1.02
					Добавлена поддержка скачки каналов даталоггера через блочную передачу
// 	26.12.2011	Release	Rev 1.01
//----------------------------------------------------------------------------------*/
#include "DSP.h"
//#include "main.h"
#include "V_data_log.h"
#include "co_ODvars.h"
#include "CANOpen_drv.h"
#include "CANBlockTransferInterface.h"
//#include "X_CANFlashProgStarter.h"
extern TDataLog dlog;

void SMCanBTInterface(volatile struct SCanBTInterface* p, Uint32 time, TCo_OdVars* co_ptr)
{
	//отдельную функцию для инициализации не делаем, т.к. неизвестно, что пользователь раньше вызовет 
	//инит CANOpen-а или данного модуля, что делает поведение автомата непредсказуемым из-за того, что
	//в SPI может быть что-то сохранено (например прожиг по CAN :))
	if(p->alreadyInit == 0) 
	{
		co_ptr->co_blockTransferCommand = CANBT_INTERFACE_FREE;
		p->time_prev = time;
		p->alreadyInit = 1;
	}
	//считаем дискретный автомат
    p->state_shadow = co_ptr->co_blockTransferCommand; //копируем переменную состояния, так как state может быть изменен извне внезапно
    //далее всё анализируем по state_shadow
    if (p->state_prev!=p->state_shadow)
    {
        p->E=1;//флаг первого вхождения (E - ENTRY)
        p->state_time = 0;
    }
    else
        p->E=0;
    p->state_prev=p->state_shadow;

    //время нахождения в текущем состоянии
    p->state_time += p->time_prev - time;
    p->time_prev = time;

    switch (p->state_shadow)//в зависимости от состояния:
    {
		 case CANBT_INTERFACE_FREE:
		 {
			if (p->E)
			  {
					//ENTRY
			  }
			break;
		 }
		 case CANBT_INTERFACE_BUSY:
		 {
			if (p->E)
			  {
					//ENTRY
			  }
			//если находимся в занятом состоянии слишком долго, значит кто то просто забыл очистить инетрпертатор
			if(p->state_time > (150000*5000)) //5 секунд
				co_ptr->co_blockTransferCommand = CANBT_INTERFACE_FREE;
			break;
		 }
		 case CANBT_INTERFACE_FLASH_PROG:
		 {
			if (p->E)
			  {
					//ENTRY
			  }
			/*
			//останавливаем управление ключами
			FP_BLOCK_PWM;
			//попутно непрерывно сбрасываем sm_ctrl и выставляем аварию, чтоб уж наверняка все стояло
			FP_CTRL_STOP;
			FP_FAULT_SET;

			//ждем маленько (100мс), чтоб все ДА одуплились и идем в прожиг
			if(p->state_time > (150000*100))
				StartFlashProgrammer();
			*/
			break;
		 }
		 case CANBT_INTERFACE_DATALOG:
		 {
			if (p->E)
			  {
				//ENTRY

				//DINT;
				//Debug2=CpuTimer2Regs.TIM.all; //150МГЦ таймер, для расчета кол-ва тактов


				//копируем данные из массива осциллограммы в массив для отправки со смещением
				int i;
				Uint32 *CopyPtrFrom = (Uint32 *)(&dlog.buff[dlog.first_point_written+(256*dlog.buff_num)]);
				Uint32 *CopyPtrTo = (Uint32 *)(&dlog.upload_buff[0]);
				for (i=0; i<(128-(dlog.first_point_written>>1)); i++)
					*CopyPtrTo++ = *CopyPtrFrom++;
				CopyPtrFrom = (Uint32 *)(&dlog.buff[256*dlog.buff_num]);	//указатель массива-источника переносим на начало, для массива-приемника он и так, где надо
				for (i=0; i<(dlog.first_point_written>>1); i++)
					*CopyPtrTo++ = *CopyPtrFrom++;


				//Debug2-=CpuTimer2Regs.TIM.all; //150МГЦ таймер, для расчета кол-ва тактов
				//EINT;

				CANBlockTransferInitTX(co_ptr,256,(Uint16*)dlog.dl_upload_buffer_adr);
				

			  }

			//проверяем освободился ли ДА блочной передачи.
			//Если освободился, то освобождаем и интерфейс.
			if(co_ptr->BT->tx_status != CANBT_STATUS_BUSY)
			{
				co_ptr->co_blockTransferCommand = CANBT_INTERFACE_FREE;
			}
			break;
		 }
		 default:
			co_ptr->co_blockTransferCommand = CANBT_INTERFACE_FREE;
    }	
}

//колбэк для блочной передачи описаны тут, чтоб избавить пользователя от них
void callback_BTcmd(Uint16 par, Uint16 tag_CANnum)
{
	#ifdef CAN_1_ENABLE
		if(tag_CANnum == 1)
		{
			if(par==0)
			{
				if(co1_vars.co_blockTransferCommand == 0)
					co1_vars.co_blockTransferCommand = 0x8000;
			}
			if(par==1)
			{

				if(co1_vars.co_blockTransferCommand == 1)
				{	//по 1-му CANу хотят прошиваться, отразим это в настройках прошивальщика
				/*	FPsettings.CANa_enable = 1;
					FPsettings.CANb_enable = 0;
					*/
				}
			}
		}
	#endif

	#ifdef CAN_2_ENABLE
		if(tag_CANnum == 2)
		{
			if(par==0)
			{



				if(co2_vars.co_blockTransferCommand == 0)
					co2_vars.co_blockTransferCommand = 0x8000;
			}
			if(par==1)
			{

				if(co2_vars.co_blockTransferCommand == 1)
				{	//по 2-му CANу хотят прошиваться, отразим это в настройках прошивальщика
				/*	FPsettings.CANb_enable = 1;
					FPsettings.CANa_enable = 0;
					*/
				}
			}
		}
	#endif
}


