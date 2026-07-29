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

 \file      V_current_decoupling.c
 \brief     Компенсация перекрестных связей регуляторов тока d, q
*/

#include "V_current_decoupling.h"

#define CURRENT_DECOUPLING_REQUEST_LIMIT _IQ(2.0)
#define CURRENT_DECOUPLING_TWO_PI 6.28318530718f

static _iq CurrentDecoupling_Clamp(_iq value, _iq minimum, _iq maximum) {
	if (value < minimum)
		return minimum;
	if (value > maximum)
		return maximum;
	return value;
}

static _iq CurrentDecoupling_AddAndClamp(_iq first, _iq second,
		_iq minimum, _iq maximum) {
	int64_t sum = (int64_t) first + (int64_t) second;

	if (sum < minimum)
		return minimum;
	if (sum > maximum)
		return maximum;
	return (_iq) sum;
}

void CurrentDecoupling_Init(TCurrentDecoupling *p) {
	p->enabled = 1;
	p->gain = _IQ(1.0);
	p->cross_coefficient = 0;
	p->parameters_valid = 0;
	CurrentDecoupling_Reset(p);
}

void CurrentDecoupling_UpdateParameters(TCurrentDecoupling *p,
		_iq inductance_h, int electrical_frequency_hz,
		int current_base_a, int voltage_base_v) {
	float coefficient;

	if ((inductance_h <= 0) || (electrical_frequency_hz <= 0)
			|| (current_base_a <= 0) || (voltage_base_v <= 0)) {
		p->cross_coefficient = 0;
		p->parameters_valid = 0;
		return;
	}

	coefficient = _IQtoF(inductance_h) * CURRENT_DECOUPLING_TWO_PI
			* (float) electrical_frequency_hz * (float) current_base_a
			/ (float) voltage_base_v;

	/*
	 * Значения выше 4 о.е. практически означают ошибку базовых параметров.
	 * Ограничение сохраняет безопасный диапазон промежуточных вычислений Q24.
	 */
	if (coefficient > 4.0f)
		coefficient = 4.0f;

	p->cross_coefficient = _IQ(coefficient);
	p->parameters_valid = 1;
}

void CurrentDecoupling_Calc(TCurrentDecoupling *p,
		_iq electrical_speed, _iq id, _iq iq,
		_iq ud_controller, _iq uq_controller, _iq voltage_limit) {
	_iq effective_coefficient;
	_iq voltage_magnitude;
	_iq voltage_scale;
	_iq ud_requested;
	_iq uq_requested;

	p->gain = CurrentDecoupling_Clamp(p->gain, 0,
			CURRENT_DECOUPLING_GAIN_MAX);

	if ((!p->enabled) || (!p->parameters_valid)) {
		p->ud_feedforward = 0;
		p->uq_feedforward = 0;
		p->ud_output = ud_controller;
		p->uq_output = uq_controller;
		p->id_saturation_error = 0;
		p->iq_saturation_error = 0;
		p->voltage_limited = 0;
		return;
	}

	effective_coefficient = _IQmpy(p->cross_coefficient, p->gain);

	/*
	 * Для принятого преобразования Park:
	 *   ud_ff = -omega_e * Ls * iq
	 *   uq_ff = +omega_e * Ls * id
	 * electrical_speed, id и iq заданы в относительных единицах.
	 */
	p->ud_feedforward = -_IQmpy(effective_coefficient,
			_IQmpy(electrical_speed, iq));
	p->uq_feedforward = _IQmpy(effective_coefficient,
			_IQmpy(electrical_speed, id));

	ud_requested = CurrentDecoupling_AddAndClamp(
			ud_controller, p->ud_feedforward,
			-CURRENT_DECOUPLING_REQUEST_LIMIT,
			CURRENT_DECOUPLING_REQUEST_LIMIT);
	uq_requested = CurrentDecoupling_AddAndClamp(
			uq_controller, p->uq_feedforward,
			-CURRENT_DECOUPLING_REQUEST_LIMIT,
			CURRENT_DECOUPLING_REQUEST_LIMIT);

	if (voltage_limit <= 0)
		voltage_limit = _IQ(1.0);

	voltage_magnitude = _IQmag(ud_requested, uq_requested);
	if (voltage_magnitude > voltage_limit) {
		voltage_scale = _IQdiv(voltage_limit, voltage_magnitude);
		p->ud_output = _IQmpy(ud_requested, voltage_scale);
		p->uq_output = _IQmpy(uq_requested, voltage_scale);
		p->voltage_limited = 1;
	} else {
		p->ud_output = ud_requested;
		p->uq_output = uq_requested;
		p->voltage_limited = 0;
	}

	/*
	 * Ошибка между реализуемым и запрошенным напряжением передается
	 * интеграторам регуляторов на следующем такте.
	 */
	p->id_saturation_error = p->ud_output - ud_requested;
	p->iq_saturation_error = p->uq_output - uq_requested;
}

void CurrentDecoupling_Reset(TCurrentDecoupling *p) {
	p->voltage_limited = 0;
	p->ud_feedforward = 0;
	p->uq_feedforward = 0;
	p->ud_output = 0;
	p->uq_output = 0;
	p->id_saturation_error = 0;
	p->iq_saturation_error = 0;
}
