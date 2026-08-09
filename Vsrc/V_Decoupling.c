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

 \file      V_Decoupling.c
 \brief     Компенсация перекрёстных связей (декаплинг) регуляторов тока (см. TDecoupling)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 2025

 \addtogroup V_Decoupling
 @{ */

#include "V_Decoupling.h"
#include "V_MotorParams.h"
#include "main.h"

//!Расчет компенсации перекрёстных связей

//!Для синхронной машины уравнения по осям d,q:
//!  Ud = R*id + Ld*d(id)/dt - w*Lq*iq,
//!  Uq = R*iq + Lq*d(iq)/dt + w*(Ld*id + psi).
//!Компенсируем перекрёстные члены, пропорциональные скорости w:
//!  ud_ff = -K * w * Lq * iq,
//!  uq_ff =  K * w * (Ld * id + psi).
//!Полученные напряжения добавляются к выходам регуляторов тока.
//! \memberof TDecoupling
void Decoupling_Calc(TDecoupling *p) {
	long tmp;

	if (!p->enabled) { //декаплинг выключен
		p->ud_ff = 0;
		p->uq_ff = 0;
		return;
	}

	//компенсация по оси d: ud_ff = -K * w * Lq * iq
	tmp = _IQmpy(p->speed_elec, _IQmpy(p->Lq, p->iq));
	p->ud_ff = -_IQmpy(p->K, tmp);

	//компенсация по оси q: uq_ff = K * w * (Ld * id + psi)
	tmp = _IQmpy(p->Ld, p->id) + p->psi;
	p->uq_ff = _IQmpy(p->K, _IQmpy(p->speed_elec, tmp));
}

//!Пересчет параметров декаплинга в относительные единицы

//!Берет физические параметры выбранной синхронной машины (l_sd, l_sq в мГн,
//!psi_pm в Вб) из таблицы SMDATA и базовые величины привода (freq_nom - базовая
//!частота в Гц, U_nom - базовое фазное напряжение, I_nom - базовый ток)
//!и пересчитывает их в относительные единицы:
//!  L_pu = L[Гн] * w_base * I_nom / U_nom,
//!  psi_pu = psi[Вб] * w_base / U_nom, где w_base = 2*pi*freq_nom.
//!Вызывается в фоновом расчете после настройки базовых величин.
//! \memberof TDecoupling
void Decoupling_SetParams(TDecoupling *p) {
	float wbase, Ubase, Ibase;
	Tsm_param *sm;

	//базовые величины должны быть настроены
	if ((drv_params.freq_nom <= 0) || (drv_params.U_nom <= 0)
			|| (drv_params.I_nom <= 0))
		return;

	//декаплинг рассчитан для синхронной машины
	if (model.MotorType != MODEL_SYNC_MOTOR)
		return;
	if ((model.MotorParametersNum == 0)
			|| (model.MotorParametersNum > model.MotorParametersNumMax))
		return;

	sm = &SMDATA[model.MotorParametersNum - 1];

	wbase = 6.28318530718f * (float) drv_params.freq_nom;
	Ubase = (float) drv_params.U_nom;
	Ibase = (float) drv_params.I_nom;

	p->Ld = _IQ(sm->l_sd * 0.001f * wbase * Ibase / Ubase);
	p->Lq = _IQ(sm->l_sq * 0.001f * wbase * Ibase / Ubase);
	p->psi = _IQ(sm->psi_pm * wbase / Ubase);
}

/*@}*/
