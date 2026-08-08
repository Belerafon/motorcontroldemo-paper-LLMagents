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

 \file      V_UdControl.c
 \brief     Модуль плавного заряда ЗПТ
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 25/08/2017

 \addtogroup
 @{*/


#include "main.h"


//!Инициализация.

//!Конфигурируем ножку контроллера, управляющую цепью заряда на вывод.

//! \memberof TUdControl
void UdControl_init(TUdControl *p) {

#ifdef HW_MOTORCONTROLBOARD
	p->Enabled = 1; //Включаем блок заряда ЗПТ

	//ножка, управляющая цепью заряда
	NT_GPIOG->ALTFUNCCLR = (1 << 6); //отключаем альтернативную функцию
	NT_GPIOG->OUTENSET = (1 << 6); //включаем ножку на вывод

	UD_CONTROL_OFF; //сначала выключим
	p->state = UD_CONTROL_STATE_OFF; //и еще в состояние "выключено"
	p->StateOn = 0;
#else
	p->Enabled = 0; //Выключить блок заряда ЗПТ
#endif

}

//!Расчет.

//!Дискретный автомат, управляющий цепью заряда. Вызывать надо в каком-нибудь
//!быстром прерывании (например 10кГц). Может, будет работать и в 1кГц, но
//!не проверялось. Имеет три состояния: выключено, ожидание заряда и включено.
//!все переключения происходят на основе измеряемого напряжения на ЗПТ.

//! \memberof TUdControl
void UdControl_calc(TUdControl *p) {
	if (p->Enabled){//если есть цепь управления зарядом ЗПТ
		p->fUdc.input = adc.Udc_meas; //на вход фильтра - напряжение с ЗПТ
		p->fUdc.calc(&(p->fUdc));
		//считаем дискретный автомат
		p->state_shadow = p->state;
		if (p->state_prev != p->state_shadow)
			p->E = 1; //первое вхождение
		else
			p->E = 0;
		p->state_prev = p->state_shadow;

		switch (p->state_shadow) {
		case UD_CONTROL_STATE_OFF: //ключ выключен
		{
			UD_CONTROL_OFF; //выключаем ключ
			p->StateOn = 0; //состояние выключено
			if (adc.Udc_meas > p->U_on) //напряжение почти высоко
				p->state = UD_CONTROL_STATE_WAIT; //переходим в ожидание
			break;
		}

		case UD_CONTROL_STATE_WAIT: //ключ выключен, готовимся включить
		{
			if (p->E == 1) //первое вхождение
					{
				p->StateCounter = 0;
			}
			p->StateCounter += global_time.relative_time1.delta_millisecond;
			UD_CONTROL_OFF; //выключаем ключ
			p->StateOn = 0; //состояние выключено
			if (adc.Udc_meas < p->U_off) //хотели вроде включать, но напряжение снизилось...
				p->state = UD_CONTROL_STATE_OFF;

			if (global_time.relative_time1.delta_millisecond) { //если стоит бит миллисекунды
				//производная на миллисекунде
				p->deriv_time_ZPT = (labs(p->fUdc.output - p->fUdc_output_prev))<<11; //поднимем значение производной для получения уровня около 1
				p->fUdc_output_prev = p->fUdc.output;
			}
			//таймаут включения и производная
			if ((p->StateCounter >= p->Timeout_on) && (p->deriv_time_ZPT < p->deriv_const)) { //и производная меньше заданного уровня
				p->state = UD_CONTROL_STATE_ON;
				p->StateCounter = 0;
			}
			break;
		}

		case UD_CONTROL_STATE_ON: {
			if (adc.Udc_meas < p->U_off) //работали-работали, а тут напряжение снизилось...
				p->state = UD_CONTROL_STATE_OFF;
			UD_CONTROL_ON; //включем ключ
			p->StateOn = 1; //состояние включено
			break;

		}

		}
	}


}

/*@}*/

