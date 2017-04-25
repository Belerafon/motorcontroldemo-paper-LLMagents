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
 
 \file      V_pid_reg3.c
 \brief     ПИД-регулятор (см. TPidReg3)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \addtogroup V_pid_reg3
 @{*/

#include "V_IQmath.h"         // Include header for IQmath library
#include "V_pid_reg3.h"
#include "stdlib.h"
#include "main.h"
//! Функция расчета ПИД регулятора
//! Вообще это чуть доработанная версия тексасовского регулятора
//! Документацию этой магии лучше всего посмотреть, загуглив pid_reg3.pdf

//! На входе pid_ref_reg3 - задание, pid_fdb_reg3 - обратная связь
//! На выходе pid_out_reg3.

//! \memberof TPidReg3
void pid_reg3_calc(TPidReg3 *v) {
	v->e_reg3 = v->pid_ref_reg3 - v->pid_fdb_reg3;//ошибка - задание минус обратная связь
	v->e_reg3Dz=v->e_reg3;//ошибка после коррекции блока мертвой зоны

	if (v->DeadZone!=0){//если есть уставка зоны нечувствительности
		if (v->e_reg3Dz>0){//ошибка в плюс
			v->e_reg3Dz=v->e_reg3Dz-v->DeadZone;//вычитаем мертвую зону
			if (v->e_reg3Dz<0)//но так, чтобы ошибка не стала отрицательной
				v->e_reg3Dz=0;
		}
		if (v->e_reg3Dz<0){
			v->e_reg3Dz=v->e_reg3Dz+v->DeadZone;
			if (v->e_reg3Dz>0)
				v->e_reg3Dz=0;
		}
	}

	v->up_reg3 = _IQmpy(v->Kp_reg3, v->e_reg3Dz);

	v->uprsat_reg3 = v->up_reg3 + v->ui_reg3 + v->ud_reg3;

	if (v->uprsat_reg3 > v->pid_out_max)
		v->pid_out_reg3 = v->pid_out_max;
	else if (v->uprsat_reg3 < v->pid_out_min)
		v->pid_out_reg3 = v->pid_out_min;
	else
		v->pid_out_reg3 = v->uprsat_reg3;

	v->saterr_reg3 = v->pid_out_reg3 - v->uprsat_reg3 + v->saterr_reg3Add;

	//использовать ли фильтр для дифф. части. Если кфильтра ноль, то нет.
	if (v->Kf_d == 0){
		v->e_reg3_filterOut = v->e_reg3;//выход фильтра
	}
	else{//иначе считаем фильтр
		v->e_reg3_filterOut = v->e_reg3_filterOut
				+ _IQmpy(v->Kf_d, (v->e_reg3 - v->e_reg3_filterOut));
	}

	if ((v->DiffCounter++ + 1) >= v->DiffDelim) {//каждые сколько вызовов считать дифф. часть
		if (v->KdFilterInitFlag==1){//это первый такт расчета регулятора
			v->e_reg3_filterOut = v->e_reg3;//выход фильтра ошибки инициализируем ошибкой
			v->up1_reg3 = v->e_reg3_filterOut;//производная ноль
			v->KdFilterInitFlag=0;//инициализация завершена
		}
		v->ud_reg3 = _IQmpy(v->Kd_reg3, (v->e_reg3_filterOut - v->up1_reg3)<<6);//дифференциальная часть
		v->up1_reg3 = v->e_reg3_filterOut;
		v->DiffCounter = 0;
	}

	if (v->Ki_reg3 != 0)//есть интегральная составляющая
		v->ui_reg3 = v->ui_reg3
				+ _IQmpy(v->Ki_reg3,
						v->up_reg3) + _IQmpy(v->Kc_reg3,v->saterr_reg3);
	else
		v->ui_reg3 = 0;

	if (v->Kc_reg3 == 0) {
		if (v->ui_reg3 > v->pid_out_max)
			v->ui_reg3 = v->pid_out_max;
		else if (v->ui_reg3 < v->pid_out_min)
			v->ui_reg3 = v->pid_out_min;
	}

}


//! \memberof TPidReg3
void pid_reg3_reset(TPidReg3 *v) {
	v->pid_fdb_reg3=0;
	v->pid_ref_reg3=0;
	v->ui_reg3=0;
	v->KdFilterInitFlag=1;
}

/*@}*/
