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

 \file      V_current_decoupling.h
 \brief     Компенсация перекрестных связей регуляторов тока d, q
*/

#ifndef V_CURRENT_DECOUPLING_H
#define V_CURRENT_DECOUPLING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "V_IQmath.h"

#define CURRENT_DECOUPLING_GAIN_MAX _IQ(1.5)

typedef struct SCurrentDecoupling {
	Uint16 enabled;           //!< Разрешение компенсации
	Uint16 parameters_valid;  //!< Признак корректности базовых параметров
	Uint16 voltage_limited;   //!< Признак ограничения результирующего вектора
	_iq gain;                 //!< Относительное усиление компенсации, Q24
	_iq cross_coefficient;    //!< 2*pi*f_base*Ls*I_base/U_base, Q24
	_iq ud_feedforward;       //!< Компенсация по оси d, Q24 напряжения
	_iq uq_feedforward;       //!< Компенсация по оси q, Q24 напряжения
	_iq ud_output;            //!< Итоговое задание напряжения d, Q24
	_iq uq_output;            //!< Итоговое задание напряжения q, Q24
	_iq id_saturation_error;  //!< Ошибка насыщения для anti-windup Id, Q24
	_iq iq_saturation_error;  //!< Ошибка насыщения для anti-windup Iq, Q24
} TCurrentDecoupling;

#define CURRENT_DECOUPLING_DEFAULTS { \
	.enabled = 1, \
	.gain = _IQ(1.0) \
}

void CurrentDecoupling_Init(TCurrentDecoupling *p);
void CurrentDecoupling_UpdateParameters(TCurrentDecoupling *p,
		_iq inductance_h, int electrical_frequency_hz,
		int current_base_a, int voltage_base_v);
void CurrentDecoupling_Calc(TCurrentDecoupling *p,
		_iq electrical_speed, _iq id, _iq iq,
		_iq ud_controller, _iq uq_controller, _iq voltage_limit);
void CurrentDecoupling_Reset(TCurrentDecoupling *p);

#ifdef __cplusplus
}
#endif

#endif
