/*!
 * \file V_FieldWeakening.c
 * \brief Voltage-feedback field weakening for PM synchronous motors.
 */

#include "V_FieldWeakening.h"

#define FIELD_WEAKENING_VOLTAGE_HIGH_RATIO      _IQ(0.95)
#define FIELD_WEAKENING_VOLTAGE_LOW_RATIO       _IQ(0.90)
#define FIELD_WEAKENING_MIN_SPEED               _IQ(0.80)
#define FIELD_WEAKENING_CURRENT_MARGIN           _IQ(0.95)
#define FIELD_WEAKENING_CURRENT_LIMIT_MAX        _IQ(4.0)
#define FIELD_WEAKENING_GAIN_MAX                _IQ(0.01)

static long FieldWeakening_Abs(long value) {
	return value < 0 ? -value : value;
}

static long FieldWeakening_Min(long a, long b) {
	return a < b ? a : b;
}

void FieldWeakening_Init(TFieldWeakening *p) {
	p->enabled = FIELD_WEAKENING_DEFAULT_ENABLE;
	p->integral_gain = FIELD_WEAKENING_DEFAULT_GAIN;
	p->id_min = FIELD_WEAKENING_DEFAULT_ID_MIN;
	FieldWeakening_Reset(p);
}

void FieldWeakening_Reset(TFieldWeakening *p) {
	p->id_ref = 0;
	p->iq_limit = 0;
	p->voltage_demand = 0;
	p->voltage_high = 0;
	p->voltage_low = 0;
	p->flags = 0;
}

void FieldWeakening_Calc(TFieldWeakening *p,
		long speed,
		long voltage_d,
		long voltage_q,
		long torque_current_limit,
		long protection_current_limit,
		long voltage_limit,
		long voltage_correction) {
	long gain;
	long id_min;
	long current_limit;
	long torque_limit;
	long current_square;
	long id_square;
	long iq_from_current;
	long correction;

	torque_limit = FieldWeakening_Abs(torque_current_limit);
	p->flags = 0;
	p->iq_limit = torque_limit;

	/*
	 * The disabled path is the exact legacy current-limit behavior. This is
	 * important for a meaningful runtime A/B comparison.
	 */
	if (!p->enabled) {
		p->id_ref = 0;
		p->voltage_demand = 0;
		return;
	}
	p->flags |= FIELD_WEAKENING_FLAG_ENABLED;

	current_limit = FieldWeakening_Abs(protection_current_limit);
	current_limit = _IQmpy(current_limit, FIELD_WEAKENING_CURRENT_MARGIN);
	if (current_limit > FIELD_WEAKENING_CURRENT_LIMIT_MAX)
		current_limit = FIELD_WEAKENING_CURRENT_LIMIT_MAX;
	torque_limit = FieldWeakening_Min(torque_limit, current_limit);
	p->iq_limit = torque_limit;

	if (current_limit <= 0 || voltage_limit <= 0) {
		p->id_ref = 0;
		p->voltage_demand = 0;
		return;
	}

	gain = p->integral_gain;
	if (gain < 0)
		gain = 0;
	if (gain > FIELD_WEAKENING_GAIN_MAX)
		gain = FIELD_WEAKENING_GAIN_MAX;

	id_min = p->id_min;
	if (id_min > 0)
		id_min = 0;
	if (id_min < -current_limit)
		id_min = -current_limit;

	correction = voltage_correction;
	if (correction <= 0)
		correction = _IQ(1.0);
	p->voltage_demand = _IQmpy(_IQmag(voltage_d, voltage_q), correction);
	p->voltage_high = _IQmpy(voltage_limit,
			FIELD_WEAKENING_VOLTAGE_HIGH_RATIO);
	p->voltage_low = _IQmpy(voltage_limit,
			FIELD_WEAKENING_VOLTAGE_LOW_RATIO);

	if (FieldWeakening_Abs(speed) < FIELD_WEAKENING_MIN_SPEED) {
		p->id_ref += _IQmpy(gain, p->voltage_low);
	} else if (p->voltage_demand > p->voltage_high) {
		p->id_ref -= _IQmpy(gain,
				p->voltage_demand - p->voltage_high);
		p->flags |= FIELD_WEAKENING_FLAG_VOLTAGE_HIGH;
	} else if (p->voltage_demand < p->voltage_low) {
		p->id_ref += _IQmpy(gain,
				p->voltage_low - p->voltage_demand);
	}

	if (p->id_ref > 0)
		p->id_ref = 0;
	if (p->id_ref < id_min)
		p->id_ref = id_min;
	if (p->id_ref < 0)
		p->flags |= FIELD_WEAKENING_FLAG_ACTIVE;
	if (id_min < 0 && p->id_ref <= id_min)
		p->flags |= FIELD_WEAKENING_FLAG_ID_LIMITED;

	current_square = _IQmpy(current_limit, current_limit);
	id_square = _IQmpy(p->id_ref, p->id_ref);
	if (id_square >= current_square)
		iq_from_current = 0;
	else
		iq_from_current = _IQsqrt(current_square - id_square);

	p->iq_limit = FieldWeakening_Min(torque_limit, iq_from_current);
	if (p->iq_limit < torque_limit)
		p->flags |= FIELD_WEAKENING_FLAG_CURRENT_LIMITED;
}
