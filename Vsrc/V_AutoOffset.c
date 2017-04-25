/*!
    Copyright 2017 	АО "НИИЭТ" и ООО "НПФ ВЕКТОР"

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 \file      V_AutoOffset.c
 \brief     Автосмещение АЦП (см. TAutoOffset)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 @{
 */

/** \addtogroup TAutoOffset */
/*@{*/

#include "DSP.h"
#include "v_IQmath.h"
#include "V_adc.h"
#include "V_AutoOffset.h"
#include "V_common.h"

extern TAdcDrv adc;
extern TDrvParams drv_params;

void AutoOffset_init(TAutoOffset *p) {
	p->FilterK = _IQ(0.001);
}

void AutoOffset_calc(TAutoOffset *p) {
	//интеграторы для токов фаз. Когда модуль вызывается, токи должны быть нулевыми.
	//если это не так, то интеграторы пойдут считать
	p->IA_int = p->IA_int + _IQmpy(p->FilterK, adc.Imeas_a);
	p->IB_int = p->IB_int + _IQmpy(p->FilterK, adc.Imeas_b);
	p->IC_int = p->IC_int + _IQmpy(p->FilterK, adc.Imeas_c);
}

void AutoOffset_slow_calc(TAutoOffset *p) {

	long IA_pu;
	long IB_pu;
	long IC_pu;

	if (p->Enabled){//если модуль авторасчета смещения токов включен
		//пересчитываем из о.е. обратно в значения АЦП
		IA_pu = -p->IA_int / (_IQ16mpy(adc.Imeas_a_gain, drv_params._1_I_nom));
		adc.Imeas_a_offset  = IA_pu >> 1;

		IB_pu = -p->IB_int / (_IQ16mpy(adc.Imeas_b_gain, drv_params._1_I_nom));
		adc.Imeas_b_offset = IB_pu >> 1;

		IC_pu = -p->IC_int / (_IQ16mpy(adc.Imeas_c_gain, drv_params._1_I_nom));
		adc.Imeas_c_offset = IC_pu >> 1;
	} else {//модуль выключен, инициализируем интеграторы текущими значенями смещений
		p->IA_int=-(((long)adc.Imeas_a_offset)<<1)*_IQ16mpy(adc.Imeas_a_gain, drv_params._1_I_nom);
		p->IB_int=-(((long)adc.Imeas_b_offset)<<1)*_IQ16mpy(adc.Imeas_b_gain, drv_params._1_I_nom);
		p->IC_int=-(((long)adc.Imeas_c_offset)<<1)*_IQ16mpy(adc.Imeas_c_gain, drv_params._1_I_nom);
	}

}

/*@}*/

