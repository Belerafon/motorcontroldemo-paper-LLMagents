#include <assert.h>

#include "V_FieldWeakening.h"

static long abs_long(long value)
{
	return value < 0 ? -value : value;
}

int main(void)
{
	TFieldWeakening fw = FIELD_WEAKENING_DEFAULTS;
	long current_limit = _IQ(0.8);
	long current_sq;
	long vector_sq;
	int i;

	fw.calc(&fw, _IQ(0.5), _IQ(1.0), current_limit);
	assert(fw.id_ref == 0);
	assert(abs_long(fw.iq_limit - current_limit) < _IQ(0.001));

	for (i = 0; i < 200; ++i)
		fw.calc(&fw, _IQ(1.05), _IQ(1.0), current_limit);
	assert(fw.active);
	assert(fw.id_ref < 0);
	assert(fw.iq_limit < current_limit);
	current_sq = _IQmpy(current_limit, current_limit);
	vector_sq = _IQmpy(fw.id_ref, fw.id_ref)
			+ _IQmpy(fw.iq_limit, fw.iq_limit);
	assert(abs_long(vector_sq - current_sq) < _IQ(0.002));

	for (i = 0; i < 4000; ++i)
		fw.calc(&fw, _IQ(0.5), _IQ(1.0), current_limit);
	assert(!fw.active);
	assert(fw.id_ref == 0);

	fw.id_limit = _IQ(0.3);
	for (i = 0; i < 1000; ++i)
		fw.calc(&fw, _IQ(1.2), _IQ(1.0), current_limit);
	assert(abs_long(fw.id_ref + _IQ(0.3)) < _IQ(0.001));

	fw.enabled = 0;
	fw.calc(&fw, _IQ(1.2), _IQ(1.0), current_limit);
	assert(fw.id_ref == 0);
	assert(fw.iq_limit == current_limit);

	return 0;
}
