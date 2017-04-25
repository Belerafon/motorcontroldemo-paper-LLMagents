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
 
 \file      V_CurPar.c
 \brief     Расчет наблюдаемых текущих параметров  (см. TCurPar)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 */

/** \addtogroup V_CurPar */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif
#include "DSP.h"     // Device Headerfile and Examples Include File
#include "V_IQmath.h"          // библиотека IQmath
#include "V_CurPar.h"    // заголовочный файл
#include "main.h"
#ifdef __cplusplus
}
#endif

//! Инициализация модуля

//! Так, всяие обнуления...
//! \memberof TCurPar
void CurPar_init(TCurPar* p) {
	//фильтры
	p->fPower.T = _IQ(0.001);  //фильтр мощности для вывода на экран

}

//!Расчет мощности из данных фаз

//!Расчет происходит в двухфазной системе координат, привязанной к статору
//!Для расчета используются мгнованнеые напряжения фаз UalphaRef, UbetaRef,
//!мгнованные токи фаз Ialpha, Ibeta. Для каждой из осей напряжение умножается на ток,
//!полученная мгновенная мощность суммируется с аналогичной мощностью по другой оси,
//!а затем полученная суммартная мгновенная мощность фильтруется инерционным звеном первого порядка.
//! \memberof TCurPar
void CurPar_PowerCalcUf(TCurPar* p) {
	p->Ualpha = pwm.UalphaRef;  //Напряжение заданное оси альфа
	p->Ubeta = pwm.UbetaRef; //Напряжение заданное оси бета

	//мгновенная мощность по осям альфа и бета подается на вход фильтра
	p->fPower.input = _IQmpy(p->PowerK,
			_IQmpy(p->Ualpha,p->Ialpha)+_IQmpy(p->Ubeta,p->Ibeta));

	//расчет фильтра
	p->fPower.calc(&p->fPower);
	//результат работы фильтра - текущая активная мощность.
	p->power = p->fPower.output;
}

//!Расчет текущих параметров привода

//! \memberof TCurPar
void CurPar_calc(TCurPar* p) {

	CurPar_PowerCalcUf(p); //Расчет мощности
}

//! \memberof TCurPar
//Расчет внутреннего масштабирующего коэффицента мощности
//для приведения различных типов расчетов мощности к базовой мощности drv_params.power
void CurPar_slow_calc(TCurPar* p) {
	float PowerMaxReal;
	float PowerMaxScale;
	PowerMaxReal = (float) drv_params.I_nom * drv_params.U_nom;
	PowerMaxScale = _IQ6toF(drv_params.power);
	p->PowerK = _IQmpy(_IQ(PowerMaxReal / PowerMaxScale), _IQ(3.0/2));//дробь 3/2 нужна для преобразования мощности от 2х фазного представления к 3х фазному
}

/*@}*/

