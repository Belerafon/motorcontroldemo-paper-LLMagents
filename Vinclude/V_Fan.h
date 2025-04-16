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

 \file      V_Fan.h
 \brief     Модуль работы вентилятора
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 25/08/2017

 \addtogroup
 @{*/


#ifndef V_FAN_H
#define V_FAN_H


//! вентилятор выключен
#define FAN_CONTROL_STATE_OFF 		0
//! вентилятор включен
#define FAN_CONTROL_STATE_ON 		1
//! состояние перегрева
#define FAN_CONTROL_STATE_OVERHEAT 	2

/* Включение и выключение вентилятора */
#if defined (HW_MOTORCONTROLBOARD)
#define FAN_ON			NT_GPIOA->MASKHIGHBYTE_bit[32].MASKHB = 32			//NT_GPIOA->DATA |= (1 << 13)
#define FAN_OFF			NT_GPIOA->MASKHIGHBYTE_bit[32].MASKHB = 0			//NT_GPIOA->DATA &= ~(1 << 13)
#endif

#if !defined (HW_MOTORCONTROLBOARD)
/* Дефайны для других контроллеров */
#define FAN_ON
#define FAN_OFF
#endif


struct SFanControl{
	Uint16 Enabled;

	int state; //!<состояние
	int state_shadow;//!< состояние теневое
	int state_prev;//!<состояние предыдущее
	int StateOn;//!<двоичное состояние: включен/не включен

	long T_on;//!<температура включения
	long T_off;//!<температура выключения
	long T_alarm;//!температура вывода предупреждения
	void   (*init)(volatile struct SFanControl*);
	void   (*slow_calc)(volatile struct SFanControl*);
};

typedef volatile struct SFanControl TFanControl;



//! \memberof TFanControl
void FanControl_init(TFanControl*);
//! \memberof TFanControl
void FanControl_slow_calc(TFanControl*);	//расчет в фоновом режиме


#define FAN_CONTROL_DEFAULTS { \
    .state = FAN_CONTROL_STATE_OFF, \
    .state_shadow = FAN_CONTROL_STATE_OFF, \
    .state_prev = FAN_CONTROL_STATE_OFF, \
    .T_on = _IQ(2.5), \
    .T_off = _IQ(2), \
    .T_alarm = _IQ(4), \
    .init = FanControl_init, \
    .slow_calc = FanControl_slow_calc \
}



#endif

/*@}*/
