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

 \file      V_UdControl.h
 \brief     Модуль плавного заряда ЗПТ
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 25/08/2017

 \addtogroup
 @{*/


#ifndef V_UD_CONTROL_H
#define V_UD_CONTROL_H


#include "build.h"


#ifdef __cplusplus
extern "C" {
#endif

//! состояние выключено
#define UD_CONTROL_STATE_OFF 0
//! состояние ожидания включения
#define UD_CONTROL_STATE_WAIT 1
//! состояние включено
#define UD_CONTROL_STATE_ON 2

#if defined (HW_MOTORCONTROLBOARD)
#define UD_CONTROL_OFF NT_GPIOG->MASKLOWBYTE_bit[64].MASKLB = 0 // операция для выключения
#define UD_CONTROL_ON NT_GPIOG->MASKLOWBYTE_bit[64].MASKLB = 64 // операция для включения
#else
#define UD_CONTROL_OFF	// операция для выключения
#define UD_CONTROL_ON	// операция для включения
#endif



/*! \class TUdControl
  \brief Управление цепью заряда звена постоянного тока

  Класс \a TUdControl, основанный на структуре SUdControl, служит
  для управления дискретным выходом, который замыкает цепь заряда
  звена постоянного тока. Перед включением следуют три проверки:
  выдержка таймаута, превышение напряжения выше заданной уставки и
  выполаживание кривой напряжения (производная стремится к нулю).
*/

//!см. TUdControl
struct SUdControl{
	Uint16 Enabled;
	int state; //!<состояние
	int state_shadow;//!< состояние теневое
	int state_prev;//!<состояние предыдущее
	int E;//!<флаг первого вхождения
	int StateCounter;//!<счетчик времени нахождения в состоянии
	long fUdc_output_prev;//!<значения напряжения предыдущее
	long U_on;//!<напряжение включения
	long U_off;//!<напряжение выключения
	long Timeout_on;//!<таймаут включения в мс.
	long deriv_time_ZPT; //!<производная по напряжению на миллисекунде
	long deriv_const; //!<константа уставки сравнения производной
	int StateOn;//!<двоичное состояние: включен/не включен
	TFilter fUdc;//!<фильтр напряжения на ЗПТ
	Uint16 ExternCheckKey;//!<Вход блок-контакта
	Uint16 ExternCheckKeyEnabled;//!<Надо ли анализировать блок-контакта
	void   (*init)(volatile struct SUdControl*);
	void   (*calc)(volatile struct SUdControl*);
};

typedef volatile struct SUdControl TUdControl;



//! \memberof TUdControl
void UdControl_init(TUdControl*);
//! \memberof TUdControl
void UdControl_calc(TUdControl*);


#define UD_CONTROL_DEFAULTS { \
    .state = UD_CONTROL_STATE_OFF, \
    .state_shadow = UD_CONTROL_STATE_OFF, \
    .state_prev = UD_CONTROL_STATE_OFF, \
    .U_on = _IQ(0.8), \
    .U_off = _IQ(0.5), \
    .Timeout_on = 3000, \
    .fUdc = FILTER_DEFAULTS, \
    .init = UdControl_init, \
    .calc = UdControl_calc \
}


#ifdef __cplusplus
}
#endif

#endif


/*@}*/

