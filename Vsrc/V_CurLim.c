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

 \file      V_CurLim.c
 \brief     Динамическое ограничение выхода регуляторов тока (см. TCurLim)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru

 \addtogroup V_CurLim
 @{*/

#include "V_IQmath.h"
#include "V_CurLim.h"
#include "main.h"

//! \memberof TCurLim
void CurLim_Init(TCurLim *p) {
	/* Стартовое предположение - номинальное напряжение ЗПТ (1.0 о.е.),
	   чтобы в первые такты работы предел не "давил" ток до нуля, пока
	   фильтр напряжения ЗПТ не сойдётся к реальному значению. */
	p->Udc = _IQ(1.0);
	p->fUdc.input = _IQ(1.0);
	p->fUdc.output = _IQ(1.0);
	p->U_max = p->U_max_cap;
}

//! \memberof TCurLim
void CurLim_Calc(TCurLim *p) {
	long u_max;
	long k_udc;

	/* Инерционный фильтр измеренного напряжения ЗПТ (о.е., 1.0 = Udc_nom). */
	p->fUdc.input = adc.Udc_meas;
	p->fUdc.calc(&p->fUdc);
	p->Udc = p->fUdc.output;

	/* База предела - возможности модуля ШИМ (pwm.U_lim) с настроечным масштабом.
	   Умножать дополнительно на Udc не нужно: модуль ШИМ и так пересчитывает
	   фактическое напряжение как duty*Udc, т.е. шина учитывается на этапе ШИМ. */
	u_max = _IQmpy(pwm.U_lim, p->Scale);

	/* Опциональное снижение предела при сильной просадке напряжения ЗПТ. */
	if (p->UdcDerateEnable) {
		if (p->Udc <= p->UdcDerateZero)
			k_udc = 0;
		else if (p->Udc >= p->UdcDerateFull)
			k_udc = _IQ(1.0);
		else
			k_udc = _IQdiv(p->Udc - p->UdcDerateZero,
					p->UdcDerateFull - p->UdcDerateZero);
		u_max = _IQmpy(u_max, k_udc);
	}

	/* Страховочные ограничения по величине. */
	if (u_max < 0)
		u_max = 0;
	if (u_max > p->U_max_cap)
		u_max = p->U_max_cap;

	p->U_max = u_max;

	/* Диагностика в свободную viewable-переменную словаря объектов. */
	Debug1 = u_max;

	/* При Enable != 0 назначаем динамические пределы выхода регуляторов тока.
	   Переключатель Enable - собственное поле модуля (по умолчанию 1) и НЕ входит
	   в словарь объектов CANopen, поэтому не перезаписывается при восстановлении
	   параметров из EEPROM. При Enable == 0 пределы регуляторов не трогаем: они
	   остаются равными значениям из словаря/по умолчанию (старое поведение с
	   константами +/-1.0), что позволяет делать прямое A/B-сравнение. */
	if (p->Enable) {
		pid_id.pid_out_max = u_max;
		pid_id.pid_out_min = -u_max;
		pid_iq.pid_out_max = u_max;
		pid_iq.pid_out_min = -u_max;
	}
}

/*@}*/
