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

 \file      V_CurLoopLim.c
 \brief     Динамическое ограничение выхода регуляторов тока (см. TCurLoopLim)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru

 \addtogroup V_CurLoopLim
 @{*/

#include "V_IQmath.h"
#include "V_CurLoopLim.h"
#include "main.h"

//! Пересчёт динамического предела выхода токовых регуляторов.

//! Формула (всё в IQ24, о.е. от U_nom):
//! \verbatim
//!  U_dyn = U_lim * Udc_meas * k_dyn,  затем clamp(u_dyn_min, U_lim)
//! \endverbatim
//! \a U_lim берётся из модуля ШИМ (его способность при номинальном ЗПТ),
//! \a Udc_meas — измеренное напряжение ЗПТ в о.е. от \a Udc_nom.
//! Ручки \a k_dyn и признак расчёта берутся из \a DebugW2 и \a DebugW1,
//! выведенных в словарь объектов CANopen. При \a DebugW1 == 0 предел
//! полагается равным статике (1.0 о.е., как в \a PIDREG3_DEFAULTS) — это
//! значение используется только как диагностическое, в пределы регуляторов
//! оно не записывается (см. \a CurLoopLim_Apply).

//! \memberof TCurLoopLim
void CurLoopLim_Calc(TCurLoopLim *p) {
	long k_dyn = DebugW2;
	long udc;
	long u_lim_cap;
	long u;

	// Защита от некорректного/непроинициализированного k_dyn:
	// допустимый диапазон (0; 2.0], иначе принимаем 1.0 (полный учёт Ud).
	if ((k_dyn <= 0) || (k_dyn > _IQ(2.0)))
		k_dyn = _IQ(1.0);

	udc = adc.Udc_meas;
	if (udc < 0)
		udc = 0;				// защита от отрицательного измерения ЗПТ

	u_lim_cap = pwm.U_lim;		// способность ШИМ при номинальном ЗПТ, о.е. U_nom
	if (u_lim_cap <= 0)
		u_lim_cap = _IQ(1.0);	// защита от нулевой/непроиницализированной настройки

	if (DebugW1 >= _IQ(0.5)) {
		// U_dyn = U_lim * Udc * k_dyn
		u = _IQmpy(u_lim_cap, _IQmpy(udc, k_dyn));
		if (u > u_lim_cap)		// при всплеске ЗПТ не выходим за линейную зону ШИМ
			u = u_lim_cap;
		if (u < p->u_dyn_min)	// при просадке ЗПТ оставляем регуляторам минимум
			u = p->u_dyn_min;
	} else {
		// Старое поведение: предел из PIDREG3_DEFAULTS (1.0 о.е.).
		// В регуляторы это значение не пишется (см. CurLoopLim_Apply),
		// здесь оно нужно только как достоверная диагностика в Debug1.
		u = _IQ(1.0);
	}

	p->u_dyn = u;
	Debug1 = u;
}


//! Применение вычисленного предела к регуляторам тока осей d и q.

//! Поведение детерминировано для прямого A/B-сравнения:
//! - при \a DebugW1 == 0 пределы обоих регуляторов принудительно
//!   устанавливаются в статическую константу \b 1.0 о.е. — то самое
//!   «защёлкнутое» значение из \a PIDREG3_DEFAULTS, которое раньше
//!   не пересчитывалось;
//! - при \a DebugW1 != 0 пределы устанавливаются симметрично в вычисленный
//!   динамический предел \a u_dyn.
//! Запись каждый такт гарантирует мгновенный и точный возврат к прежнему
//! поведению при сбросе \a DebugW1 без остаточных значений.

//! \memberof TCurLoopLim
void CurLoopLim_Apply(TCurLoopLim *p, TPidReg3 *pid_id, TPidReg3 *pid_iq) {
	long u;

	// Включение определяется порогом >= 0.5 о.е., а не «любое ненулевое»:
	// дефолт словаря CANopen для DebugW1 (~0.001 о.е., raw 16777) должен
	// трактоваться как ВЫКЛ, иначе динамическое ограничение ошибочно
	// включается на дефолтных значениях после инициализации CANopen.
	if (DebugW1 >= _IQ(0.5))
		u = p->u_dyn;
	else
		u = _IQ(1.0);		// статика из PIDREG3_DEFAULTS

	pid_id->pid_out_max = u;
	pid_id->pid_out_min = -u;
	pid_iq->pid_out_max = u;
	pid_iq->pid_out_min = -u;
}

/*@}*/
