#ifndef V_DECOUPLE_H
#define V_DECOUPLE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SCurrentDecouple {
	long enable;
	long K_dec;
	long omega_e;
	long id;
	long iq;
	long ud_dec;
	long uq_dec;
	long out_lim;
	void (*calc)(struct SCurrentDecouple*);
	void (*reset)(struct SCurrentDecouple*);
} TCurrentDecouple;

#define CURRENT_DECOUPLE_DEFAULTS { \
    .enable = 0, \
    .K_dec = 0, \
    .out_lim = _IQ(0.5), \
    .calc = CurrentDecouple_calc, \
    .reset = CurrentDecouple_reset, \
}

void CurrentDecouple_calc(TCurrentDecouple*);
void CurrentDecouple_reset(TCurrentDecouple*);

#ifdef __cplusplus
}
#endif

#endif
