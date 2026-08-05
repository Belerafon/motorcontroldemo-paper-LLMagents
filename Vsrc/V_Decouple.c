#include "V_IQmath.h"
#include "V_Decouple.h"
#include "main.h"

void CurrentDecouple_calc(TCurrentDecouple *p) {
	long prod;
	long ud;
	long uq;

	if (p->enable == 0) {
		p->ud_dec = 0;
		p->uq_dec = 0;
		return;
	}

	if (p->K_dec == 0) {
		p->ud_dec = 0;
		p->uq_dec = 0;
		return;
	}

	prod = _IQmpy(p->omega_e, p->iq);
	ud = -_IQmpy(p->K_dec, prod);

	prod = _IQmpy(p->omega_e, p->id);
	uq = _IQmpy(p->K_dec, prod);

	if (ud > p->out_lim)
		ud = p->out_lim;
	else if (ud < -p->out_lim)
		ud = -p->out_lim;

	if (uq > p->out_lim)
		uq = p->out_lim;
	else if (uq < -p->out_lim)
		uq = -p->out_lim;

	p->ud_dec = ud;
	p->uq_dec = uq;
}

void CurrentDecouple_reset(TCurrentDecouple *p) {
	p->omega_e = 0;
	p->id = 0;
	p->iq = 0;
	p->ud_dec = 0;
	p->uq_dec = 0;
}
