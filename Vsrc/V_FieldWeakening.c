#include "V_FieldWeakening.h"

static long FieldWeakening_Abs(long value)
{
	return value < 0 ? -value : value;
}

void FieldWeakening_Reset(TFieldWeakening *p)
{
	p->active = 0;
	p->id_ref = 0;
	p->iq_limit = 0;
	p->voltage_feedback = 0;
	p->voltage_target = 0;
}

void FieldWeakening_Calc(TFieldWeakening *p, long voltage_mag,
		long voltage_limit, long current_limit)
{
	long current_sq;
	long id_sq;
	long id_limit;
	long release_threshold;
	long correction;

	current_limit = FieldWeakening_Abs(current_limit);
	voltage_mag = FieldWeakening_Abs(voltage_mag);

	if (current_limit <= 0) {
		FieldWeakening_Reset(p);
		return;
	}
	if (!p->enabled || voltage_limit <= 0) {
		FieldWeakening_Reset(p);
		p->iq_limit = current_limit;
		return;
	}

	p->voltage_feedback = voltage_mag;
	p->voltage_target = _IQmpy(voltage_limit, p->voltage_margin);
	release_threshold = p->voltage_target - p->voltage_hysteresis;
	if (release_threshold < 0)
		release_threshold = 0;

	if (voltage_mag > p->voltage_target) {
		correction = _IQmpy(p->attack_gain,
				voltage_mag - p->voltage_target);
		p->id_ref -= correction;
	}
	else if (voltage_mag < release_threshold) {
		correction = _IQmpy(p->release_gain,
				release_threshold - voltage_mag);
		p->id_ref += correction;
	}

	id_limit = FieldWeakening_Abs(p->id_limit);
	if (id_limit == 0 || id_limit > current_limit)
		id_limit = current_limit;

	if (p->id_ref < -id_limit)
		p->id_ref = -id_limit;
	else if (p->id_ref > 0)
		p->id_ref = 0;

	if (p->id_ref == 0) {
		p->iq_limit = current_limit;
	}
	else {
		current_sq = _IQmpy(current_limit, current_limit);
		id_sq = _IQmpy(p->id_ref, p->id_ref);
		if (id_sq >= current_sq)
			p->iq_limit = 0;
		else
			p->iq_limit = _IQsqrt(current_sq - id_sq);
	}

	p->active = (p->id_ref < 0);
}
