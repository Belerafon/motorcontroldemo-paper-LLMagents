/*!
 * \file V_PowerLimiter.c
 * \brief Динамическое ограничение активной мощности векторного привода.
 */

#include "DSP.h"
#include "V_IQmath.h"
#include "V_PowerLimiter.h"
#include "main.h"

#define POWER_LIMITER_ENABLE_VALUE       1L
#define POWER_LIMITER_MAX_POWER_PU       _IQ(4.0)
#define POWER_LIMITER_DEFAULT_KP         _IQ(0.5)
#define POWER_LIMITER_MAX_KP             _IQ(4.0)
#define POWER_LIMITER_KI                 _IQ(0.002)
#define POWER_LIMITER_FILTER_K           _IQ(0.2)

static long PowerLimiter_abs(long value) {
	if (value < 0)
		return -value;
	return value;
}

static long PowerLimiter_clamp(long value, long min, long max) {
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

void PowerLimiter_init(TPowerLimiter *p) {
	p->power_limit = 0;
	p->power_fdb = 0;
	p->iq_integrator = 0;
	p->iq_limit = 0;
	p->configured_watts = 0;
	p->base_power_watts = 0;
	p->active = 0;
}

/*
 * DebugW4 задается целым числом ватт. Пересчет в IQ24 выполняется в фоне,
 * чтобы в прерывании 10 кГц не было 64-разрядного деления.
 */
void PowerLimiter_slow_calc(TPowerLimiter *p) {
	long base_power_w = (drv_params.power + 32L) >> 6;
	long requested_w = DebugW4;
	long long limit;

	if ((requested_w <= 0) || (base_power_w <= 0)) {
		p->power_limit = 0;
		p->configured_watts = requested_w;
		p->base_power_watts = base_power_w;
		return;
	}

	if ((requested_w == p->configured_watts)
			&& (base_power_w == p->base_power_watts)
			&& (p->power_limit > 0))
		return;

	limit = ((long long) requested_w << 24) / base_power_w;
	if (limit > POWER_LIMITER_MAX_POWER_PU)
		limit = POWER_LIMITER_MAX_POWER_PU;

	p->power_limit = (long) limit;
	p->configured_watts = requested_w;
	p->base_power_watts = base_power_w;
}

void PowerLimiter_reset(TPowerLimiter *p, long requested_iq_limit) {
	long iq_max = PowerLimiter_abs(requested_iq_limit);

	p->iq_integrator = iq_max;
	p->iq_limit = iq_max;
	p->active = 0;
	Debug1 = p->power_fdb;
	Debug2 = p->iq_limit;
}

/*
 * Регулятор формирует верхнюю границу Iq. При малой мощности граница равна
 * штатному refs.Iq_ref; после достижения задания она уменьшается так, чтобы
 * активная мощность удерживалась около DebugW4.
 */
long PowerLimiter_calc(
		TPowerLimiter *p,
		long requested_iq_limit,
		long instantaneous_power) {
	long iq_max = PowerLimiter_abs(requested_iq_limit);
	long power_sample = PowerLimiter_abs(instantaneous_power);
	long error;
	long kp;
	long proportional;
	long power_ratio;
	long predictive_limit;
	long output;

	p->power_fdb += _IQmpy(
			POWER_LIMITER_FILTER_K,
			power_sample - p->power_fdb);

	if ((DebugW3 != POWER_LIMITER_ENABLE_VALUE) || (p->power_limit <= 0)) {
		PowerLimiter_reset(p, iq_max);
		return iq_max;
	}

	if (p->active == 0)
		p->iq_integrator = iq_max;
	p->active = 1;

	kp = DebugW2;
	if ((kp <= 0) || (kp > POWER_LIMITER_MAX_KP))
		kp = POWER_LIMITER_DEFAULT_KP;

	error = p->power_limit - p->power_fdb;
	p->iq_integrator += _IQmpy(POWER_LIMITER_KI, error);
	p->iq_integrator = PowerLimiter_clamp(p->iq_integrator, 0, iq_max);

	/*
	 * При превышении немедленно масштабируем предыдущую границу отношением
	 * Pзад/Pфакт. Это не дает быстрому разгону уйти далеко выше задания за
	 * время, необходимое интегральной части.
	 */
	if ((p->power_fdb > p->power_limit) && (p->iq_limit > 0)) {
		power_ratio = _IQdiv(p->power_limit, p->power_fdb);
		predictive_limit = _IQmpy(p->iq_limit, power_ratio);
		if (predictive_limit < p->iq_integrator)
			p->iq_integrator = predictive_limit;
	}

	proportional = _IQmpy(kp, error);
	output = PowerLimiter_clamp(
			p->iq_integrator + proportional,
			0,
			iq_max);

	p->iq_limit = output;
	Debug1 = p->power_fdb;
	Debug2 = p->iq_limit;
	return output;
}
