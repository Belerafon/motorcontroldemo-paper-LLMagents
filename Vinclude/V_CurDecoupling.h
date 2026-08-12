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

 \file      V_CurDecoupling.h
 \brief     Декаплинг регуляторов тока - компенсация перекрестных связей (см. TCurDecoupling)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru

 \defgroup  V_CurDecoupling Декаплинг регуляторов тока

 @{
*/

#ifndef V_CUR_DECOUPLING_H
#define V_CUR_DECOUPLING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "V_IQmath.h"

/*! \class TCurDecoupling
  \brief Модуль компенсации перекрестных связей (декаплинга) регуляторов тока

  Реализует feedforward-компенсацию перекрестных связей и ЭДС вращения
  в контурах тока по осям d и q векторной системы управления.

  Уравнения машины в осях dq в относительных единицах:
  \verbatim
  Ud = Rs*Id + (1/w_b)*Lsd*dId/dt - we*Lsq*Iq
  Uq = Rs*Iq + (1/w_b)*Lsq*dIq/dt + we*Lsd*Id + we*psi_pm
  \endverbatim

  Перекрестные связи (-we*Lsq*Iq в Ud и +we*Lsd*Id, +we*psi_pm в Uq)
  ухудшают динамику контуров тока на высокой частоте вращения:
  изменение задания одного регулятора вызывает переходный процесс в другом.

  Чтобы ПИ-регуляторы "видели" только диагональную R-L часть объекта,
  к их выходам добавляются компенсирующие слагаемые, равные по модулю
  и противоположные по знаку перекрестным связям и ЭДС вращения:
  \verbatim
  ipark.de = pid_id.out + ud_decoup
  ipark.qe = pid_iq.out + uq_decoup + uq_emf
  \endverbatim

  Все величины - в относительных единицах:
  - напряжение: 1.0 = U_nom (амплитуда фазного номинального)
  - ток:        1.0 = I_nom (амплитуда фазного номинального)
  - частота:    1.0 = freq_nom (номинальная электрическая частота)
  - индуктивности Lsd, Lsq: в о.е. базового сопротивления Z_b = U_nom/I_nom
    (на номинальной частоте падение напряжения на индуктивности равно we_pu*L_pu*I_pu)
  - потокосцепление psi_pm: в о.е. базового потокосцепления psi_b = U_nom/w_b
    (для СДПМ при номинальной скорости ЭДС вращения равна we_pu*psi_pm_pu)
*/

//! см. TCurDecoupling
typedef struct SCurDecoupling {
	_iq enable;			//!< Включение декаплинга: != 0 - вкл, 0 - выкл. Задается.
	_iq Lsd;			//!< Индуктивность статора по оси d в о.е. (для компенсации перекрестной связи). Задается.
	_iq Lsq;			//!< Индуктивность статора по оси q в о.е. (для компенсации перекрестной связи). Задается.
	_iq psi_pm;			//!< Потокосцепление ПМ ротора в о.е. (для компенсации ЭДС вращения по оси q). 0 - компенсация ЭДС отключена. Задается.

	_iq omega_e;		//!< Input: электрическая угловая скорость в о.е. (1.0 = freq_nom)
	_iq id;				//!< Input: ток оси d (обратная связь) в о.е.
	_iq iq;				//!< Input: ток оси q (обратная связь) в о.е.

	_iq ud_decoup;		//!< Output: добавка к Ud на компенсацию перекрестной связи (-we*Lsq*Iq)
	_iq uq_decoup;		//!< Output: добавка к Uq на компенсацию перекрестной связи (+we*Lsd*Id)
	_iq uq_emf;			//!< Output: добавка к Uq на компенсацию ЭДС вращения ПМ (+we*psi_pm)

	void (*calc)(struct SCurDecoupling*);	//!< Pointer to calculation function
	void (*reset)(struct SCurDecoupling*);	//!< Pointer to reset function
} TCurDecoupling;


//! Инициализатор по-умолчанию.
//! По умолчанию декаплинг ВЫКЛЮЧЕН (enable = 0), параметры равны 0.
//! Для включения необходимо установить enable в _IQ(1) и задать Lsd, Lsq, psi_pm.
#define CUR_DECOUPLING_DEFAULTS { \
    .enable = 0, \
    .Lsd = 0, \
    .Lsq = 0, \
    .psi_pm = 0, \
    .calc = CurDecoupling_calc, \
    .reset = CurDecoupling_reset \
}


//! \memberof TCurDecoupling
void CurDecoupling_calc(TCurDecoupling*);
//! \memberof TCurDecoupling
void CurDecoupling_reset(TCurDecoupling*);


#ifdef __cplusplus
}
#endif

#endif

/*@}*/
