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
 \version   v 1.0

 \addtogroup V_FieldWeakening
 @{*/

#include "V_IQmath.h"
#include "V_FieldWeakening.h"


//! Инициализация модуля ослабления поля.

//! Настраивает подчинённый ПИ-регулятор напряжения: ограничения выхода
//! (от Id_min до 0), коэффициенты по умолчанию и сбрасывает накопители.
//! \memberof TFieldWeakening
void FieldWeakening_Init(TFieldWeakening *p) {
	p->U_mag = 0;
	p->id_ref = 0;

	/* Настройка подчинённого ПИ-регулятора напряжения.
	 * Задание (pid_ref_reg3) - порог U_thresh, обратная связь (pid_fdb_reg3) - U_mag.
	 * Когда U_mag > U_thresh, ошибка ref-fdb отрицательна -> выход уходит в минус,
	 * что и формирует отрицательный Id.
	 * Выход ограничивается сверху нулём (ослабление не делает Id положительным)
	 * и снизу величиной Id_min (максимально допустимый отрицательный ток).
	 */
	p->pid_fw.pid_ref_reg3 = 0;
	p->pid_fw.pid_fdb_reg3 = 0;
	p->pid_fw.pid_out_max = 0;          // выход <= 0
	p->pid_fw.pid_out_min = _IQ(-1.0);  // ограничится значением Id_min в calc()
	p->pid_fw.Kp_reg3 = _IQ(1.0);       // пропорциональный коэффициент по напряжению
	p->pid_fw.Ki_reg3 = _IQ(0.005);     // медленная интегральная часть (ослабление поля - медленный процесс)
	p->pid_fw.Kc_reg3 = _IQ(0.5);       // анти-насыщение (как у остальных регуляторов)
	p->pid_fw.Kd_reg3 = _IQ(0);         // дифференциальная часть не используется
	p->pid_fw.Kf_d = 0;
	p->pid_fw.DiffDelim = 1;
	p->pid_fw.DiffCounter = 0;
	p->pid_fw.DeadZone = 0;
	p->pid_fw.KdFilterInitFlag = 1;
	p->pid_fw.calc = pid_reg3_calc;
	p->pid_fw.reset = pid_reg3_reset;
	p->pid_fw.reset(&p->pid_fw);
}


//! Расчёт задания тока ослабления поля.

//! Вызывается в быстром расчёте (обычно 10 кГц) после измерения
//! текущих величин и до расчёта регуляторов тока. Использует
//! значение U_mag, вычисленное модулем ШИМ на предыдущей итерации,
//! что создаёт задержку в один такт - это допустимо, так как
//! процесс ослабления поля медленный по сравнению с периодом ШИМ.
//! \memberof TFieldWeakening
void FieldWeakening_Calc(TFieldWeakening *p) {
	/* Если функция выключена - ничего не делаем. Интегратор при этом
	 * обнуляем, чтобы при повторном включении не было выброса
	 * накопленного ранее задания Id.
	 */
	if (!p->enabled) {
		p->pid_fw.ui_reg3 = 0;
		p->id_ref = 0;
		return;
	}

	/* Защита от неинициализированного Id_min. По умолчанию допускаем
	 * ослабление до -1.0 о.е. (полный номинальный ток по оси d).
	 */
	if (p->Id_min > 0)
		p->Id_min = _IQ(-1.0);

	/* Ограничение выхода ПИ-регулятора: от Id_min (макс. ослабление) до 0.
	 * Положительный Id ослабление поля никогда не требует, поэтому сверху - 0.
	 */
	p->pid_fw.pid_out_min = p->Id_min;
	p->pid_fw.pid_out_max = 0;

	/* Задание регулятора - порог напряжения, обратная связь - текущее
	 * напряжение. Ошибка e = U_thresh - U_mag:
	 *   - при U_mag < U_thresh: e > 0 -> выход насыщается до 0 (ослабления нет);
	 *   - при U_mag > U_thresh: e < 0 -> выход уходит в отрицательную область,
	 *     формируя отрицательный Id.
	 */
	p->pid_fw.pid_ref_reg3 = p->U_thresh;
	p->pid_fw.pid_fdb_reg3 = p->U_mag;
	p->pid_fw.calc(&p->pid_fw);

	/* Выход регулятора - готовое задание Id для ослабления поля. */
	p->id_ref = p->pid_fw.pid_out_reg3;
}


//! Сброс модуля ослабления поля.

//! Обнуляет интегральную часть регулятора и выход. Должна вызываться
//! при остановке двигателя или смене режима, чтобы предотвратить
//! "всплеск" накопленного задания Id при следующем запуске.
//! \memberof TFieldWeakening
void FieldWeakening_Reset(TFieldWeakening *p) {
	p->pid_fw.reset(&p->pid_fw);
	p->id_ref = 0;
}

/*@}*/
