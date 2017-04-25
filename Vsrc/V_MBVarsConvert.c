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

 \file		v_MBVarsConv.c
 \brief		Модуль преобразования данных Modbus
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version	v.1.1. 02/03/2017

 \addtogroup v_ModBus */
/*@{*/

#include "main.h"
#include "mbod.h"
#include "math.h" //для fabs
//! Инициализация не требуется
//! \memberof	TMBVarsConv
void MBVarsConv_init(TMBVarsConv *p) {

}

//! Основная процедура

//! Вход в .input, выход в .output
//! Кривая задается массивом из NumOfPoints точек по x и по y.
//! \memberof	TMBVarsConv
void MBVarsConv_calc(TMBVarsConv *p) {
	float number, scale, scaled;
	long i, index;
	if (ModBus.Enabled == 1){ // Если включен MODBUS
		//Для уменьшения вычислительной нагрузки в случае необходимости можно убрать цикл for и проверять по одному элементу за один заход
		for (i = 0; i < ModBus.MBInternals.NumOfHRs; i++)// Проверяем все элементы словаря HR (редактируемых параметров)
			if (mbodHR[i].type_or_acc == 1){// Если параметр менялся, то анализируем, как его преобразовать

				mbodHR[i].type_or_acc = 0;		// Сбрасываем флаг изменения параметра
				index = mbodHR[i].index;		//какой индекс меняли

				if ((index != 2000) && (index != 2001))	//если не 2000 и не 2001, то нужно сохранить параметры
					p->NeedForSave = 1;//Нужно не злоупотреблять этой командой, иначе вся пользовательская память сотрется до дыр!

				//В зависимости от того, какой индекс словаря поменялся, выполняем нужное преобразование.
				//На каждую проверку индекса тратится 3 такта. Если проверок будет очень много,
				//то логично дополнить драйвер функциями обратного вызова при обращении к параметру
				switch (index){
					case 2000://слово управления
						//если значение поля команда изменилось по сравнению с предыдущим
						if (p->Vars.Command!=p->Vars.CommandPrev){
							if (p->Vars.Command & (1<<0))//команда на запуск
								cmd.bit.start = 1;
							if (p->Vars.Command & (1<<1))//команда на останов
								cmd.bit.stop = 1;
							if (p->Vars.Command & (1<<2))//команда на сброс аварии
								cmd.bit.trip_reset = 1;

						}
						p->Vars.CommandPrev=p->Vars.Command;
						break;

					case  2001:  // текущая частота вращения
						number = (float) p->Vars.speed_ref;
						number = fabs(number);
						scale = (float) co2_vars.co_scaleNum14;	// Частота вращения (об/мин) 16.0
						scaled = number / scale;
						refs.speed_ref = _IQ(scaled);
						break;

					case 43116: //напряжение защиты в вольтах
						number = (float) MBVarsConv.Vars.Umax_protect;
						scale = (float) co2_vars.co_scaleNum7;	//напряжение ЗПТ базовое
						scaled = number / scale;
						sm_prot.Umax_protect = _IQ(scaled);
						break;

					}


			}
	}

	// Для наблюдаемых преобразование запускаем всегда
// 40002
	number = _IQtoF(cur_par.speed);
	scale = (float) co2_vars.co_scaleNum14;	// Частота вращения (об/мин)
	scaled = scale * number;
	p->Vars.CurSpeed = scaled;

// 40007
	number = _IQtoF(cur_par.Is);
	scale = (float) co2_vars.co_scaleNum4;	//Ток фазный базовый
	scaled = scale * number;
	p->Vars.CurIs = scaled;

// 40054
	p->Vars.Main_ErrorCode = sm_prot.Main_ErrorCode; //код текущей аварии (листается, если несколько)
}


void MBVarsConv_slow_calc(TMBVarsConv *p) {
	if (p->NeedForSave) {//команда на сохранение параметров привода через модбас
		p->NeedForSave = 0;

		//сохраняем посредством драйвера CANopen (все его параметры)
		//вызывая функцию сохранения с разными диапазонами, перебирая их все.
		co_ODexpositor_paramConservation(&co2_vars, 1, 1);
		co_ODexpositor_paramConservation(&co2_vars, 2, 1);
		co_ODexpositor_paramConservation(&co2_vars, 3, 1);
		co_ODexpositor_paramConservation(&co2_vars, 1, 2);
		co_ODexpositor_paramConservation(&co2_vars, 2, 2);
		co_ODexpositor_paramConservation(&co2_vars, 3, 2);

		co_ODexpositor_paramConservation(&co2_vars, 1, 3);
		co_ODexpositor_paramConservation(&co2_vars, 2, 3);
		co_ODexpositor_paramConservation(&co2_vars, 3, 3);
		co_ODexpositor_paramConservation(&co2_vars, 1, 4);
		co_ODexpositor_paramConservation(&co2_vars, 2, 4);
		co_ODexpositor_paramConservation(&co2_vars, 3, 4);
		co_ODexpositor_paramConservation(&co2_vars, 1, 5);
		co_ODexpositor_paramConservation(&co2_vars, 2, 5);
		co_ODexpositor_paramConservation(&co2_vars, 3, 5);
	}
}

/*@}*/

