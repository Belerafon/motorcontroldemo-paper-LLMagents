/*
 * Dynamic active-power limiter for speed-controlled drive modes.
 *
 * The limiter is an outer integral loop. It adjusts the speed-controller
 * current ceiling until the filtered active power follows the requested
 * ceiling. refs.Iq_ref remains the absolute current limit.
 */

#include <stdint.h>

#include "V_IQmath.h"
#include "V_power_limit.h"

#define POWER_LIMIT_DEFAULT_GAIN 0.001f
#define POWER_LIMIT_MAX_GAIN     0.05f
#define POWER_LIMIT_CLAMP_GAIN   _IQ(0.01f)

static long PowerLimit_abs(long value) {
	if (value >= 0)
		return value;
	if (value == (long)INT32_MIN)
		return (long)INT32_MAX;
	return -value;
}

void PowerLimit_init(TPowerLimit *p) {
	p->power_ref = 0;
	p->gain = _IQ(POWER_LIMIT_DEFAULT_GAIN);
	p->current_limit = 0;
	p->power_error = 0;
	p->enabled = 0;
	p->initialized = 0;
	p->limiting = 0;
}

void PowerLimit_configure(TPowerLimit *p, int enable, float limit_w,
		float base_power_w, float gain) {
	int was_enabled = p->enabled;

	if ((enable != 1) || !(limit_w > 0.0f) || !(base_power_w > 0.0f)) {
		p->enabled = 0;
		p->initialized = 0;
		return;
	}

	/* drv_params.power is the validated base for cur_par.power. */
	if (limit_w > base_power_w)
		limit_w = base_power_w;
	p->power_ref = _IQ(limit_w / base_power_w);

	if (!(gain > 0.0f))
		gain = POWER_LIMIT_DEFAULT_GAIN;
	if (gain > POWER_LIMIT_MAX_GAIN)
		gain = POWER_LIMIT_MAX_GAIN;
	p->gain = _IQ(gain);
	p->enabled = 1;

	if (!was_enabled)
		p->initialized = 0;
}

long PowerLimit_calc(TPowerLimit *p, long max_current, long measured_power) {
	long current_max;
	long correction;
	long measured_abs;
	long proportional_limit;
	long clamp_correction;
	int64_t candidate;

	if (!p->enabled) {
		p->current_limit = max_current;
		p->power_error = 0;
		p->initialized = 0;
		p->limiting = 0;
		return max_current;
	}

	current_max = PowerLimit_abs(max_current);
	if (current_max == 0) {
		p->current_limit = 0;
		p->power_error = p->power_ref - PowerLimit_abs(measured_power);
		p->initialized = 1;
		p->limiting = 0;
		return 0;
	}

	if (!p->initialized) {
		/* Soft engagement avoids a full-current power overshoot at startup. */
		p->current_limit = 0;
		p->initialized = 1;
	}
	if (p->current_limit > current_max)
		p->current_limit = current_max;

	measured_abs = PowerLimit_abs(measured_power);
	p->power_error = p->power_ref - measured_abs;
	correction = _IQmpy(p->gain, p->power_error);
	candidate = (int64_t)p->current_limit + correction;

	/* Fast multiplicative clamp damps the engagement transient. */
	if ((measured_abs > p->power_ref) && (p->current_limit > 0)) {
		proportional_limit = _IQmpy(p->current_limit,
				_IQdiv(p->power_ref, measured_abs));
		clamp_correction = _IQmpy(POWER_LIMIT_CLAMP_GAIN,
				p->current_limit - proportional_limit);
		if (clamp_correction > 0)
			candidate -= clamp_correction;
	}
	if (candidate < 0)
		p->current_limit = 0;
	else if (candidate > current_max)
		p->current_limit = current_max;
	else
		p->current_limit = (long)candidate;

	p->limiting = p->current_limit < current_max;
	return p->current_limit;
}

void PowerLimit_reset(TPowerLimit *p) {
	p->current_limit = 0;
	p->power_error = 0;
	p->initialized = 0;
	p->limiting = 0;
}
