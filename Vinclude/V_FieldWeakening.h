/*!
 * \file V_FieldWeakening.h
 * \brief Automatic field weakening for permanent-magnet synchronous motors.
 */

#ifndef V_FIELD_WEAKENING_H
#define V_FIELD_WEAKENING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "DSP.h"
#include "V_IQmath.h"

#define FIELD_WEAKENING_DEFAULT_ENABLE          1
#define FIELD_WEAKENING_DEFAULT_GAIN            _IQ(0.0005)
#define FIELD_WEAKENING_DEFAULT_ID_MIN          _IQ(-0.5)

#define FIELD_WEAKENING_FLAG_ENABLED            (1U << 0)
#define FIELD_WEAKENING_FLAG_ACTIVE             (1U << 1)
#define FIELD_WEAKENING_FLAG_VOLTAGE_HIGH       (1U << 2)
#define FIELD_WEAKENING_FLAG_CURRENT_LIMITED    (1U << 3)
#define FIELD_WEAKENING_FLAG_ID_LIMITED         (1U << 4)

struct SFieldWeakening {
	int enabled;
	long integral_gain;
	long id_min;

	long id_ref;
	long iq_limit;
	long voltage_demand;
	long voltage_high;
	long voltage_low;
	Uint16 flags;

	void (*init)(struct SFieldWeakening *);
	void (*reset)(struct SFieldWeakening *);
	void (*calc)(struct SFieldWeakening *,
			long speed,
			long voltage_d,
			long voltage_q,
			long torque_current_limit,
			long protection_current_limit,
			long voltage_limit,
			long voltage_correction);
};

typedef struct SFieldWeakening TFieldWeakening;

#define FIELD_WEAKENING_DEFAULTS { \
	.enabled = FIELD_WEAKENING_DEFAULT_ENABLE, \
	.integral_gain = FIELD_WEAKENING_DEFAULT_GAIN, \
	.id_min = FIELD_WEAKENING_DEFAULT_ID_MIN, \
	.init = FieldWeakening_Init, \
	.reset = FieldWeakening_Reset, \
	.calc = FieldWeakening_Calc \
}

void FieldWeakening_Init(TFieldWeakening *p);
void FieldWeakening_Reset(TFieldWeakening *p);
void FieldWeakening_Calc(TFieldWeakening *p,
		long speed,
		long voltage_d,
		long voltage_q,
		long torque_current_limit,
		long protection_current_limit,
		long voltage_limit,
		long voltage_correction);

#ifdef __cplusplus
}
#endif

#endif
