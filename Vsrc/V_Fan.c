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

 \file      V_Fan.c
 \brief     Модуль работы вентилятора
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 25/08/2017

 \addtogroup
 @{*/


#include "main.h"


//! \memberof TFanControl
void FanControl_init(TFanControl *p)
{
#if defined (HW_MOTORCONTROLBOARD)
	p->Enabled = 1; //Включаем блок вентилятора

	//Проинитим выход для управления вентилятором
	NT_GPIOA->ALTFUNCCLR = (1 << 13);		//выключить альт. функцию
	NT_GPIOA->OUTENSET = (1 << 13);			//на выход
#endif
}

//! \memberof TFanControl
void FanControl_slow_calc(TFanControl *p) {
	if (p->Enabled){//если есть вентилятор
		//считаем дискретный автомат
		p->state_shadow = p->state;
		p->state_prev = p->state_shadow;

		switch (p->state_shadow) {


		case FAN_CONTROL_STATE_OFF: //вентилятор не работает, температура инвертора меньше T_on
		{
			FAN_OFF; //
			p->StateOn = 0; //
			if (adc.T_meas > p->T_on) //
				p->state = FAN_CONTROL_STATE_ON; //
			break;
		}

		case FAN_CONTROL_STATE_ON: //включение вентилятора, температура больше T_on и меньше T_alarm
		{
			FAN_ON; //включить вентилятор
			p->StateOn = 1; //состояние работы
			if (adc.T_meas > p->T_alarm) //если температура поднялась еще больше, переходим в режим перегрева
				p->state = FAN_CONTROL_STATE_OVERHEAT;
			if (adc.T_meas < p->T_off) //если температура опустилась ниже заданного уровня, переходим в обычный режим
				p->state = FAN_CONTROL_STATE_OFF;
			break;
		}

		case FAN_CONTROL_STATE_OVERHEAT: //температура больше T_alarm, вентилятор работает, выводится предупреждение о перегреве
		{
			FAN_ON; //включить вентилятор
			p->StateOn = 1; //состояние работы
			drv_status.bit.overheat = 1; //вывод предупреждения о перегреве
			if (adc.T_meas < p->T_alarm) { //если температура опустилась ниже заданного уровня, переходим в обычный режим
				drv_status.bit.overheat = 0; //сброс предупреждения о перегреве
				p->state = FAN_CONTROL_STATE_ON;
			}
			break;

		}

		}
	}


}

/*@}*/

