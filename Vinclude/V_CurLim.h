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

 \file      V_CurLim.h
 \brief     Динамическое ограничение выхода регуляторов тока (см. TCurLim)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru

 \defgroup  V_CurLim Динамическое ограничение регуляторов тока

 @{*/

#ifndef V_CUR_LIM_H
#define V_CUR_LIM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "V_IQmath.h"
#include "filter.h"

/*! \class TCurLim
    \brief Динамическое ограничение выхода токовых регуляторов pid_id/pid_iq.

    Ранее пределы выхода регуляторов тока (pid_out_max/pid_out_min) задавались
    фиксированными константами +/-1.0 (о.е.) и не были связаны ни с реальными
    возможностями модуля ШИМ, ни с напряжением звена постоянного тока (ЗПТ).
    Из-за этого интегральные каналы накапливались выше реально реализуемого
    напряжения, а сами пределы оказывались неуниверсальными.

    Модуль каждый такт быстрого расчёта пересчитывает доступную амплитуду
    напряжения (о.е., базис - амплитуда фазного напряжения):

        U_max = clamp( pwm.U_lim * Scale * Kudc, 0, U_max_cap )

    где:
      - pwm.U_lim  - предел модуляции модуля ШИМ (радиус вписанной в шестиугольник
                     базовых векторов окружности, по умолчанию 0.866 = sqrt(3)/2);
      - Scale      - настроечный коэффициент (1.0 = брать предел ШИМ как есть);
      - Kudc       - коэффициент снижения по напряжению ЗПТ (см. ниже), о.е.;
      - U_max_cap  - верхняя страховочная граница (по умолчанию 1.0 о.е.).

    Важно об учёте напряжения ЗПТ. Модуль ШИМ в этом проекте пересчитывает
    фактическое фазное напряжение как duty*Udc, поэтому максимально реализуемое
    напряжение в per-unit домене регуляторов уже равно pwm.U_lim и НЕ требует
    дополнительного домножения на Udc (иначе шина учитывается дважды). Поэтому
    по умолчанию Kudc = 1.0 и U_max = pwm.U_lim*Scale - это напрямую заменяет
    старые константы +/-1.0 динамической величиной, привязанной к ШИМ.

    Дополнительно доступно явное снижение предела при просадке напряжения ЗПТ
    (Kudc < 1): если UdcDerateEnable установлен, то при Udc <= UdcDerateZero
    предел снижается до нуля, при Udc >= UdcDerateFull - не снижается, а между
    ними - линейно интерполируется. Это полезно для защиты от насыщения и
    перерегулирования на сильно просевшей шине. По умолчанию эта функция
    выключена, чтобы не менять нормальное поведение привода.

    Полученное U_max назначается на пределы выхода обоих токовых регуляторов:
    pid_out_max = +U_max, pid_out_min = -U_max. Финальное ограничение модуля
    вектора напряжения по окружности по-прежнему выполняет модуль ШИМ
    (PWM_Module_NormInput).

    При Enable == 0 модуль не вмешивается в пределы регуляторов: они остаются
    равными значениям из словаря объектов CANopen / по умолчанию (старое
    поведение с константами +/-1.0), что позволяет делать прямое A/B-сравнение.

    Переключатели Enable, Scale, пороги снижения UdcDerate* - собственные поля
    модуля и НЕ входят в словарь объектов CANopen, поэтому они не перезаписываются
    при восстановлении параметров из EEPROM процедурами co1_Init/co2_Init: их
    скомпилированные значения по умолчанию держатся надёжно. Для оперативного
    A/B-сравнения их можно менять напрямую по JTAG. Расчитанное U_max каждый такт
    дублируется в свободную viewable-переменную словаря Debug1 для контроля.
*/

//! см. TCurLim
struct SCurLim {
	Uint16 Enable;            //!< 1 - назначать динамические пределы, 0 - не менять (legacy)
	long   Scale;             //!< Масштабирующий коэффициент, о.е. (IQ24), по умолчанию 1.0
	Uint16 UdcDerateEnable;   //!< 1 - включить снижение предела по просадке ЗПТ, 0 - выключено
	long   UdcDerateFull;     //!< Udc (о.е., IQ24), выше которого снижение не применяется
	long   UdcDerateZero;     //!< Udc (о.е., IQ24), при/ниже которого предел снижается до 0
	long   U_max_cap;         //!< Страховочный потолок U_max, о.е. (IQ24), по умолчанию 1.0
	TFilter fUdc;             //!< Инерционный фильтр измеренного напряжения ЗПТ
	long   Udc;               //!< Отфильтрованное напряжение ЗПТ, о.е. (IQ24) - для контроля
	long   U_max;             //!< Расчитанный предел выхода регуляторов тока, о.е. (IQ24) - для контроля
	void (*calc)(struct SCurLim*);   //!< Указатель на функцию расчёта
	void (*init)(struct SCurLim*);   //!< Указатель на функцию инициализации
};

typedef struct SCurLim TCurLim;


//! Инициализатор по умолчанию: динамический предел = pwm.U_lim, снижение по Udc выключено.
#define CUR_LIM_DEFAULTS { \
    .Enable = 1, \
    .Scale = _IQ(1.0), \
    .UdcDerateEnable = 0, \
    .UdcDerateFull = _IQ(0.8), \
    .UdcDerateZero = _IQ(0.3), \
    .U_max_cap = _IQ(1.0), \
    .fUdc = { .T = _IQ(0.02), .calc = TFilter_Calc }, \
    .calc = CurLim_Calc, \
    .init = CurLim_Init \
}


//! \memberof TCurLim
void CurLim_Init(TCurLim*);
//! \memberof TCurLim
void CurLim_Calc(TCurLim*);


#ifdef __cplusplus
}
#endif

#endif

/*@}*/
