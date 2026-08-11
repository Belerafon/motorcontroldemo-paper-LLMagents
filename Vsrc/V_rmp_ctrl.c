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
 
 \file      v_rmp_ctrl.c
 \brief     Задатчик интенсивности (см. TRMPCtrl)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 */

/** \addtogroup V_rmp_ctrl */
/*@{*/

#ifdef __cplusplus 
extern "C" {
#endif

#include "v_rmp_ctrl.h"
#include "V_IQmath.h"
#include "main.h"
#include "stdlib.h"
#ifdef __cplusplus 
}
#endif

 //! Функция расчета задатчика интенсивности

 //!Изменяет output с заданным темпом T,
 //!пока output не достигнет input.
 //!Дополнительно реализует ограничение по мощности: если включено (power_ena),
 //!темп разгона динамически снижается при приближении мощности двигателя
 //!к заданному пределу power_limit, поддерживая мощность постоянной на уровне ограничения.

 //! \memberof TRMPCtrl
 void V_RMP_CTRL_calc(TRMPCtrl *p) {

 	long effective_step = p->step;

 	if (p->power_ena && (p->power_K != 0)) {
 		long current_power_frac = _IQ24mpy(model.motorInternals.power, p->power_K);
 		if (current_power_frac >= p->power_limit) {
 			long power_excess = current_power_frac - p->power_limit;
 			long max_step_at_limit = p->power_limit;
 			if (max_step_at_limit > power_excess) {
 				long reduction = _IQ24div(power_excess, max_step_at_limit);
 				effective_step = p->step - _IQ24mpy(p->step, reduction);
 				if (effective_step < _IQ(0.0001))
 					effective_step = _IQ(0.0001);
 			} else {
 				effective_step = _IQ(0.0001);
 			}
 		}
 	}

 	if ((labs(p->output - p->input) <= effective_step) || (p->T==0))
 	{
 		p->output = p->input;
 	}
 	else if (p->output < p->input)
 	{
 		p->output += effective_step;
 	}
 	else
 	{
 		p->output -= effective_step;
 	}

 }

 //! Вспомогательная функция задатчика интенсивности

 //!Пересчитывает темп разгона T в шаг step c учетом частоты
 //!вызова основной функции Ts (дискретизация, обычно 10кГц).

 //! \memberof TRMPCtrl
 void V_RMP_CTRL_slow_calc(TRMPCtrl *p) {
 	p->step = _IQdiv(p->Ts, p->T);//шаг интегратора ЗИ
 }

 //! Функция пересчета коэффициента пересчета мощности из кВт в долю от номинала (IQ24)
 //!
 //! Вычисляет коэффициент power_K на основе параметров двигателя:
 //! power_K = (P_nom / (speed_nom * 2*pi/60)) / 1000.0
 //! где P_nom - номинальная мощность (Вт), speed_nom - номинальная скорость (об/мин)
 //! Модель двигателя выдает мощность в кВт (omega * torque * 0.001), этот коэффициент
 //! приводит её к безразмерной доле от номинала в формате IQ24.
 //!
 //! \memberof TRMPCtrl
  void V_RMP_CTRL_slow_calc_power(TRMPCtrl *p) {
  	float P_nom_W = _IQ6toF(drv_params.power);
  	float speed_nom_rpm = (float)drv_params.speed_nom;
  	float speed_nom_rad_s;
  	if (speed_nom_rpm > 0.0f)
  		speed_nom_rad_s = speed_nom_rpm * 2.0f * 3.1415926535f / 60.0f;
  	else
  		speed_nom_rad_s = 1.0f;
  	float power_K_f = (P_nom_W / speed_nom_rad_s) / 1000.0f;
  	p->power_K = _IQ24(power_K_f);
  }

 /*@}*/

