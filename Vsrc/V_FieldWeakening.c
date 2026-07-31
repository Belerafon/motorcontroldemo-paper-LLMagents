#include "V_IQmath.h"
#include "V_FieldWeakening.h"
#include "stdlib.h"

#define FIELD_WEAKENING_GAIN_SCALE_MIN _IQ(0.10)
#define FIELD_WEAKENING_GAIN_SCALE_MAX _IQ(4.00)

void FieldWeakening_Reset(TFieldWeakening *p) {
	p->active = 0;
	p->id_ref = 0;
	p->iq_limit = 0;
	p->voltage_mag = 0;
	p->voltage_target = 0;
	p->voltage_error = 0;
}

void FieldWeakening_Calc(TFieldWeakening *p, int32 ud, int32 uq,
		int32 voltage_limit, int32 voltage_comp, int16 voltage_comp_enabled,
		int32 current_limit, int32 id_feedback, int32 debug_mode,
		int32 debug_gain_scale) {
	int16 enabled = p->enabled;
	int32 gain = p->gain;
	int32 id_limit;
	int32 id_for_current_limit;
	int32 current_square;
	int32 id_square;
	int32 delta;

	if (debug_mode < 0)
		enabled = 0;
	else if (debug_mode > 0)
		enabled = 1;

	if (debug_gain_scale > 0) {
		if (debug_gain_scale < FIELD_WEAKENING_GAIN_SCALE_MIN)
			debug_gain_scale = FIELD_WEAKENING_GAIN_SCALE_MIN;
		if (debug_gain_scale > FIELD_WEAKENING_GAIN_SCALE_MAX)
			debug_gain_scale = FIELD_WEAKENING_GAIN_SCALE_MAX;
		gain = _IQmpy(gain, debug_gain_scale);
	}

	current_limit = labs(current_limit);
	p->voltage_mag = _IQmag(ud, uq);
	/* Some PWM modes do not calculate UdCorTmp. Treat zero as unavailable. */
	if ((voltage_comp_enabled & 1) && voltage_comp > 0)
		p->voltage_mag = _IQmpy(p->voltage_mag, voltage_comp);
	p->voltage_target = _IQmpy(voltage_limit, p->voltage_target_ratio);
	p->voltage_error = p->voltage_mag - p->voltage_target;

	if (!enabled || current_limit == 0 || voltage_limit <= 0) {
		FieldWeakening_Reset(p);
		p->iq_limit = current_limit;
		return;
	}

	id_limit = _IQmpy(current_limit, p->id_limit_ratio);
	delta = _IQmpy(gain, p->voltage_error);
	p->id_ref -= delta;
	if (p->id_ref > 0)
		p->id_ref = 0;
	if (p->id_ref < -id_limit)
		p->id_ref = -id_limit;

	p->active = (p->id_ref < 0);
	id_for_current_limit = labs(p->id_ref);
	if (labs(id_feedback) > id_for_current_limit)
		id_for_current_limit = labs(id_feedback);
	if (id_for_current_limit >= current_limit) {
		p->iq_limit = 0;
		return;
	}

	current_square = _IQmpy(current_limit, current_limit);
	id_square = _IQmpy(id_for_current_limit, id_for_current_limit);
	p->iq_limit = _IQsqrt(current_square - id_square);
}
