/*
 * Dynamic active-power limiter for speed-controlled drive modes.
 */

#ifndef V_POWER_LIMIT_H
#define V_POWER_LIMIT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SPowerLimit {
	long power_ref;       //!< Active-power limit in IQ24 relative to drv_params.power
	long gain;            //!< Integral gain: current pu / power pu / fast calculation
	long current_limit;   //!< Dynamic moment-current limit in IQ24
	long power_error;     //!< power_ref - abs(measured power), IQ24
	int enabled;          //!< Validated enable flag
	int initialized;      //!< Integrator initialization flag
	int limiting;         //!< Dynamic limit is below refs.Iq_ref
	void (*init)(struct SPowerLimit*);
	void (*configure)(struct SPowerLimit*, int, float, float, float);
	long (*calc)(struct SPowerLimit*, long, long);
	void (*reset)(struct SPowerLimit*);
} TPowerLimit;

#define POWER_LIMIT_DEFAULTS { \
	.init = PowerLimit_init, \
	.configure = PowerLimit_configure, \
	.calc = PowerLimit_calc, \
	.reset = PowerLimit_reset \
}

void PowerLimit_init(TPowerLimit *p);
void PowerLimit_configure(TPowerLimit *p, int enable, float limit_w,
		float base_power_w, float gain);
long PowerLimit_calc(TPowerLimit *p, long max_current, long measured_power);
void PowerLimit_reset(TPowerLimit *p);

#ifdef __cplusplus
}
#endif

#endif
