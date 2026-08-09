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

 \file      V_Decoupling.h
 \brief     Модуль компенсации перекрёстных связей (декаплинг) регуляторов тока (см. TDecoupling)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 2025

 \defgroup  V_Decoupling Модуль компенсации перекрёстных связей (декаплинг) регуляторов тока

 @{*/

#ifndef V_DECOUPLING_H
#define V_DECOUPLING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "DSP.h"
#include "V_IQmath.h"

/*! \class TDecoupling
  \brief Компенсация перекрёстных связей (декаплинг) регуляторов тока

  На высокой частоте вращения между контурами регулирования тока по осям d и q
  возникает перекрёстная связь (члены -w*Lq*iq по оси d и +w*(Ld*id+psi) по оси q),
  которая ухудшает реакцию регуляторов на изменение задания. Данный модуль
  формирует аддитивные компенсирующие напряжения ud_ff, uq_ff, которые
  добавляются к выходам регуляторов тока и развязывают контуры.

  Все величины заданы в относительных единицах (формат 8.24):
  - скорость - в о.е. базовой электрической частоты;
  - токи - в о.е. базового тока;
  - индуктивности и потокосцепление - в о.е.;
  - выходные напряжения - в о.е. базового фазного напряжения.

  Индуктивности Ld, Lq и потокосцепление psi в о.е. пересчитываются из
  физических параметров выбранной синхронной машины функцией set_params.
*/
typedef struct SDecoupling{
	long  Ld;         //!< Параметр: индуктивность статора по оси d, о.е. (8.24)
	long  Lq;         //!< Параметр: индуктивность статора по оси q, о.е. (8.24)
	long  psi;        //!< Параметр: потокосцепление постоянных магнитов, о.е. (8.24)
	long  K;          //!< Параметр: общий коэффициент усиления (0..1), о.е. (8.24)
	long  speed_elec; //!< Вход: электрическая частота вращения, о.е. базовой частоты (8.24)
	long  id;         //!< Вход: ток по оси d, о.е. (8.24)
	long  iq;         //!< Вход: ток по оси q, о.е. (8.24)
	long  ud_ff;      //!< Выход: компенсация по оси d, о.е. (8.24)
	long  uq_ff;      //!< Выход: компенсация по оси q, о.е. (8.24)
	Uint16 enabled;   //!< Разрешение декаплинга
	void (*calc)(struct SDecoupling*);        //!< Указатель на функцию расчета
	void (*set_params)(struct SDecoupling*);  //!< Указатель на функцию пересчета параметров в о.е.
} TDecoupling;

//! Инициализатор по-умолчанию
#define DECOUPLING_DEFAULTS { \
    .Ld = _IQ(0), \
    .Lq = _IQ(0), \
    .psi = _IQ(0), \
    .K = _IQ(1.0), \
    .ud_ff = 0, \
    .uq_ff = 0, \
    .enabled = 1, \
    .calc = Decoupling_Calc, \
    .set_params = Decoupling_SetParams \
}

//! \memberof TDecoupling
void Decoupling_Calc(TDecoupling*);
//! \memberof TDecoupling
void Decoupling_SetParams(TDecoupling*);

#ifdef __cplusplus
}
#endif

#endif

/*@}*/
