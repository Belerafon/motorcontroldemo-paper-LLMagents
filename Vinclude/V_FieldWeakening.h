#ifndef V_FIELD_WEAKENING_H
#define V_FIELD_WEAKENING_H

#include "V_IQmath.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Voltage-feedback field weakening. Continuous values use IQ24 per-unit. */
typedef struct SFieldWeakening {
	int enabled;
	int active;
	long voltage_margin;
	long voltage_hysteresis;
	long attack_gain;
	long release_gain;
	long id_limit;
	long id_ref;
	long iq_limit;
	long voltage_feedback;
	long voltage_target;
	void (*calc)(struct SFieldWeakening *, long, long, long);
	void (*reset)(struct SFieldWeakening *);
} TFieldWeakening;

#define FIELD_WEAKENING_DEFAULTS { \
	.enabled = 1, \
	.voltage_margin = _IQ(0.95), \
	.voltage_hysteresis = _IQ(0.03), \
	.attack_gain = _IQ(0.02), \
	.release_gain = _IQ(0.005), \
	.id_limit = _IQ(1.0), \
	.calc = FieldWeakening_Calc, \
	.reset = FieldWeakening_Reset \
}

void FieldWeakening_Calc(TFieldWeakening *p, long voltage_mag,
		long voltage_limit, long current_limit);
void FieldWeakening_Reset(TFieldWeakening *p);

#ifdef __cplusplus
}
#endif

#endif
