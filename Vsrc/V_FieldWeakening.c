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

 \file      V_FieldWeakening.c
 \brief     Модуль ослабления поля (см. TFieldWeakening)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru

 \addtogroup V_FieldWeakening
 @{*/

#include "V_IQmath.h"
#include "V_FieldWeakening.h"
#include "main.h"

//! Инициализация

//! Сбрасывает состояние регулятора и выходные переменные.
//! Начальное состояние - алгоритм выключен, Id-добавка нулевая.
//! Параметр Id_mag_max из инициализатора оставляем как скомпилированный
//! дефолт - он будет регулярно обновляться из DebugW4 в calc.
//! \memberof TFieldWeakening
void FieldWeakening_Init(TFieldWeakening *p) {
	p->enable = 0;
	p->mod_index = 0;
	p->Id_ref = 0;
	p->pid.reset(&p->pid);
}

//! Быстрый расчет (10 кГц)

//! Вызывается из SM_Sys_Fast_Calc один раз за такт расчета.
//! Берёт готовые pwm.U_mag и pwm.U_lim (от предыдущего вызова
//! SM_Ctrl_Fast_Calc), формирует индекс модуляции и отрабатывает
//! его ПИ-регулятором, выход которого ограничен нулем сверху
//! и -Id_mag_max снизу. Готовая добавка Id_ref затем суммируется
//! с заданием регулятора тока оси d внутри соответствующих режимов
//! векторного управления в SM_Ctrl_Fast_Calc.
//!
//! Ручки пользователя (через словарь объектов CANopen):
//! - DebugW3 (>0 - включить, =0 - выключить);
//! - DebugW4 (положительное число - модуль макс. отрицательного Id, о.е.).
//!
//! Диагностика:
//! - Debug1 - mod_index (формат IQ24, безразмерный);
//! - Debug2 - Id_ref (формат IQ24, о.е. тока).
//!
//! При выключении алгоритма интегратор сбрасывается, чтобы при следующем
//! включении не выдавался накопленный ненулевой ток. Безопасное значение
//! по умолчанию (DebugW3=0, DebugW4=0) соответствует полностью выключенному
//! алгоритму, что обеспечивает точный возврат к прежнему поведению.
//! \memberof TFieldWeakening
void FieldWeakening_Calc(TFieldWeakening *p) {
	long mod_index;
	long id_mag_max;

	// Синхронизация пользовательских ручек с CANopen-редактируемыми
	// переменными. Делаем это каждый такт, чтобы изменение на пульте
	// сработало без перезагрузки.
	p->enable = (DebugW3 != 0) ? 1 : 0;

	// DebugW4 - модуль тока (положительное число). Отрицательный или
	// нулевой ввод трактуем как выключение ослабления поля.
	id_mag_max = DebugW4;
	if (id_mag_max < 0)
		id_mag_max = 0;
	p->Id_mag_max = id_mag_max;

	// Текущий индекс модуляции - отношение требуемой амплитуды
	// напряжения к предельно реализуемой. Берём значения из модуля ШИМ
	// после предыдущего такта (pwm.update уже отработал).
	if (pwm.U_lim > 0)
		mod_index = _IQdiv(pwm.U_mag, pwm.U_lim);
	else
		mod_index = 0;

	// Ограничиваем для устойчивости численного расчёта регулятора
	// (на переходных процессах или при аварии ШИМ значения могут
	// кратковременно выходить за единицу).
	if (mod_index < 0)
		mod_index = 0;
	if (mod_index > _IQ(1.5))
		mod_index = _IQ(1.5);

	p->mod_index = mod_index;

	if (p->enable && id_mag_max > 0) {
		// Условное интегрирование: регулятор активен только тогда, когда
		// напряжения действительно не хватает (mod_index > U_setpoint).
		// Пока напряжения хватает, держим интегратор в нуле, чтобы при
		// выходе на ограничение получить чистую отрицательную компенсацию
		// без ложного "набега" от простаивавшего на верхней границе
		// антираскручиваемого интегратора.
		if (mod_index > p->U_setpoint) {
			// ПИ-контур: цель - держать mod_index около U_setpoint.
			// Ошибка (ref - fdb) отрицательна, выход уходит в минус,
			// антираскрутка pid_reg3 удерживает его в диапазоне
			// [-id_mag_max; 0]. Этот отрицательный ток по оси d
			// ослабляет суммарный поток и снижает противо-ЭДС.
			p->pid.pid_ref_reg3 = p->U_setpoint;
			p->pid.pid_fdb_reg3 = mod_index;
			p->pid.pid_out_max = 0;
			p->pid.pid_out_min = -id_mag_max;
			p->pid.calc(&p->pid);
			p->Id_ref = p->pid.pid_out_reg3;
		} else {
			// Напряжения хватает - сбрасываем регулятор, чтобы при
			// переходе через U_setpoint он стартовал с нуля.
			p->pid.ui_reg3 = 0;
			p->pid.pid_out_reg3 = 0;
			p->Id_ref = 0;
		}
	} else {
		// Алгоритм выключен или Id_mag_max=0: сбрасываем интегратор,
		// чтобы при следующем включении регулятор стартовал с нуля.
		p->pid.ui_reg3 = 0;
		p->pid.pid_out_reg3 = 0;
		p->Id_ref = 0;
	}

	// Диагностические переменные для наблюдения в UniCON и в e2e-тестах.
	// Формат IQ24 без инженерного масштаба: mod_index - в о.е.,
	// Id_ref - в о.е. тока (базовый ток drv_params.I_nom).
	Debug1 = p->mod_index;
	Debug2 = p->Id_ref;
}

/*@}*/
