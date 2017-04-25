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
 
 \file      SMCmdLogic.c
 \brief     Обработка команд и заданий, поступающих из различных источников. (см. TSM_CmdLogic)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \addtogroup SMCmdLogic
 @{*/

#include "DSP.h"
#include "V_IQmath.h"
#include "main.h"

//!Инициализация

//!Всякие обнуления...
//! \memberof TSM_CmdLogic
void SM_CmdLogicInit(TSM_CmdLogic *p) {
	p->state = 0;
	p->state_prev = 0xFF;
	cmd.all = 0;

	/* Разрешить ногам работать на вход, это кнопки старт и стоп */
	NT_GPIOA->ALTFUNCCLR = (1 << 13) | (1 << 14);//выключить альт. функцию
	NT_GPIOA->OUTENCLR |= (1 << 14) | (1 << 14);//на вход

	p->StartButton=((NT_GPIOA->DATA & (1 << 14))==0);//обработка ножки старта
	p->StopButton=((NT_GPIOA->DATA & (1 << 13))==0);//обработка ножки стопа
	p->StartButtonPrev=p->StartButton;
	p->StopButtonPrev=p->StopButton;

}

//!Медленный расчет

//! \memberof TSM_CmdLogic
void SM_CmdLogicSlow_Calc(TSM_CmdLogic *p) {

}

//!1кГц расчет

//!Обработка различных источников команд и расчет ДА,
//!который переключает глобально режимы РАБОТА и ОСТАНОВ.
//! \memberof TSM_CmdLogic
void SM_CmdLogicms_Calc(TSM_CmdLogic *p) {

	//Стандартная обработка дискретного автомата
	if (p->state_prev != p->state)
		p->E = 1;
	else
		p->E = 0;
	p->state_prev = p->state;


	p->StartButton=((NT_GPIOA->DATA & (1 << 14))==0);//обработка ножки старта
	p->StopButton=((NT_GPIOA->DATA & (1 << 13))==0);//обработка ножки стопа

	if ((p->StartButton==1) && (p->StartButtonPrev==0)){
		cmd.bit.start=1;//команда на запуск. Она же ставится через CAN
	}
	if ((p->StopButton==1) && (p->StopButtonPrev==0)){
		cmd.bit.stop=1;
	}

	p->StartButtonPrev=p->StartButton;
	p->StopButtonPrev=p->StopButton;

	//определяем режим работы
	switch (p->state) {
	case CMD_LOGIC_TURNED_OFF: {	//состояние 0- ОТКЛЮЧЕН
		if (p->E == 1) {
			/*ENTRY*/
			sm_ctrl.state = CTRL_STOP;
			drv_status.bit.ready = 1;
			drv_status.bit.running = 0;
		}

		if (sm_prot.state == PROT_ON_OK) {//если ДА защит включился, обрабатываем команды включения
			if ((cmd.bit.start == 1) && (sm_sys.state == SYS_READY))//есть битовая команда старта
				p->state = CMD_LOGIC_TURNED_ON;	//поехали
		}

		//дискретный автомат со структурами управления в СТОПе всегда тоже принудительно переводим в стоп
		sm_ctrl.state = CTRL_STOP;
		//всегда сбрасываем битовые команды управления
		cmd.bit.stop = 0;
		cmd.bit.start = 0;
		break;
	}

	case CMD_LOGIC_TURNED_ON: {	//состояние - РАБОТА
		if (p->E == 1) {
			/*ENTRY*/
			drv_status.bit.ready = 1;
			drv_status.bit.running = 1;
			sm_ctrl.state = CTRL_RUN;//поехали
		}
		//если АВАРИя или КОМАНДА СТОП то состояние стоп
		if ((sm_prot.state == PROT_FAIL) || (cmd.bit.stop == 1)
				|| (sm_ctrl.state == CTRL_STOP)){
			p->state = CMD_LOGIC_TURNED_OFF;
		}

		//всегда сбрасываем команду управления
		cmd.bit.stop = 0;
		cmd.bit.start = 0;
		break;
	}

	default:
		break;

	} // конец switch(p->state)
}

/*@}*/
