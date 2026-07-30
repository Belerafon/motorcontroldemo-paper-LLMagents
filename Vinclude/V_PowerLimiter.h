/*!
 * \file V_PowerLimiter.h
 * \brief Динамическое ограничение активной мощности векторного привода.
 */

#ifndef V_POWER_LIMITER_H
#define V_POWER_LIMITER_H

#ifdef __cplusplus
extern "C" {
#endif

struct SPowerLimiter {
	long power_limit;       //!< Ограничение мощности, IQ24 от drv_params.power
	long power_fdb;         //!< Фильтрованная активная мощность, IQ24
	long iq_integrator;     //!< Интегральная часть ограничения тока, IQ24
	long iq_limit;          //!< Динамическая граница задания Iq, IQ24
	long configured_watts;  //!< Последнее пересчитанное задание, Вт
	long base_power_watts;  //!< Номинальная мощность при последнем пересчете, Вт
	long active;            //!< Признак активной работы ограничителя

	void (*init)(volatile struct SPowerLimiter*);
	void (*slow_calc)(volatile struct SPowerLimiter*);
	long (*calc)(volatile struct SPowerLimiter*, long, long);
	void (*reset)(volatile struct SPowerLimiter*, long);
};

typedef volatile struct SPowerLimiter TPowerLimiter;

#define POWER_LIMITER_DEFAULTS { \
	.init = PowerLimiter_init, \
	.slow_calc = PowerLimiter_slow_calc, \
	.calc = PowerLimiter_calc, \
	.reset = PowerLimiter_reset \
}

void PowerLimiter_init(TPowerLimiter*);
void PowerLimiter_slow_calc(TPowerLimiter*);
long PowerLimiter_calc(TPowerLimiter*, long, long);
void PowerLimiter_reset(TPowerLimiter*, long);

#ifdef __cplusplus
}
#endif

#endif
