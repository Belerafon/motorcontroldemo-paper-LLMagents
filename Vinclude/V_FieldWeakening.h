#ifndef V_FIELD_WEAKENING_H
#define V_FIELD_WEAKENING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "DSP.h"

typedef volatile struct SFieldWeakening TFieldWeakening;

struct SFieldWeakening {
	int16 enabled;
	int16 active;
	int32 voltage_target_ratio;
	int32 gain;
	int32 id_limit_ratio;
	int32 id_ref;
	int32 iq_limit;
	int32 voltage_mag;
	int32 voltage_target;
	int32 voltage_error;
	void (*calc)(TFieldWeakening *, int32, int32, int32, int32, int16,
			int32, int32, int32, int32);
	void (*reset)(TFieldWeakening *);
};

#define FIELD_WEAKENING_DEFAULTS { \
	.enabled = 1, \
	.voltage_target_ratio = _IQ(0.97), \
	.gain = _IQ(0.0025), \
	.id_limit_ratio = _IQ(0.90), \
	.calc = FieldWeakening_Calc, \
	.reset = FieldWeakening_Reset \
}

void FieldWeakening_Calc(TFieldWeakening *, int32, int32, int32, int32,
		int16, int32, int32, int32, int32);
void FieldWeakening_Reset(TFieldWeakening *);

#ifdef __cplusplus
}
#endif

#endif
