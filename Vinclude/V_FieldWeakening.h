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

 \file      V_FieldWeakening.h
 \brief     Модуль ослабления поля (см. TFieldWeakening)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru

 \defgroup  V_FieldWeakening Ослабление поля (Field Weakening)
 @{
 */

#ifndef V_FIELD_WEAKENING_H
#define V_FIELD_WEAKENING_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "V_pid_reg3.h"

/*! \class TFieldWeakening
    \brief Автоматическое ослабление поля для расширения диапазона скорости

    Класс \a TFieldWeakening, основанный на структуре SFieldWeakening,
    реализует классический алгоритм ослабления поля (Field Weakening) для
    векторных систем управления синхронными машинами с постоянными магнитами
    (СДПМ/PMSM) и другими машинами, у которых регулятор тока по оси d в
    номинальном режиме держит нулевое задание.

    Принцип работы. По мере роста скорости двигателя растёт противо-ЭДС и,
    следовательно, амплитуда вектора напряжения, требуемого от инвертора,
    \argf  U_mag = |U_alpha| + |U_beta|
    Когда этот вектор приближается к предельно реализуемому напряжению
    U_lim (задаётся модулем ШИМ), инвертор выходит на насыщение и двигатель
    больше не может ускоряться. Чтобы обойти это ограничение, на ось d
    подаётся отрицательный ток - он создаёт встречный поток, который
    компенсирует поток постоянных магнитов и снижает суммарное потокосцепление,
    а с ним и противо-ЭДС. В результате при той же скорости требуется меньшее
    напряжение, и двигатель может вращаться быстрее при том же звене
    постоянного тока (разумеется, ценой снижения момента на валу).

    Регулятор представляет собой обычный ПИ-контур, удерживающий индекс
    модуляции
    \argf  mod_index = U_mag / U_lim
    около заданного значения U_setpoint (типично 0.8..0.9). Выход регулятора
    ограничен диапазоном [-Id_mag_max; 0] - это гарантирует, что при
    недогрузке по напряжению (на малой скорости) алгоритм выдаёт нулевую
    добавку Id и не мешает нормальному режиму, а вмешивается только тогда,
    когда напряжения начинает не хватать.

    Все коэффициенты ПИ-регулятора доступны через словарь объектов CANopen
    как поля структуры fw (pid.Kp_reg3, pid.Ki_reg3, ...). Включение и
    ограничение на время отладки выведены через переменные DebugW3 и DebugW4.

    Переключатель и ручки:
    - \ref DebugW3 - включение алгоритма: >0 (типично 1.0 в о.е.) - включён,
      0 - выключен, Id_ref = 0;
    - \ref DebugW4 - максимальный отрицательный ток оси d в о.е.
      (положительное число - модуль тока, типично 0.5);
    - fw.U_setpoint - уставка по индексу модуляции (по умолчанию _IQ(0.85),
      задаётся при компиляции);
    - fw.pid.Kp_reg3, fw.pid.Ki_reg3 - коэффициенты ПИ-регулятора
      (по умолчанию _IQ(0.05), _IQ(0.005)), доступны через словарь объектов
      как поля pid внутри fw.

    Диагностические переменные:
    - \ref Debug1 - текущий индекс модуляции mod_index (формат IQ24);
    - \ref Debug2 - выход регулятора ослабления Id_ref (формат IQ24).
      0 - алгоритм не активен, отрицательное число - величина компенсации.
*/

//! см. TFieldWeakening
struct SFieldWeakening {
	long enable;        //!< Включение алгоритма: 1 - активен, 0 - выключен
	long U_setpoint;    //!< Уставка индекса модуляции (типично _IQ(0.85))
	long Id_mag_max;    //!< Максимум отрицательного тока по оси d (положительное число, о.е.)
	long mod_index;     //!< Выход: текущий индекс модуляции U_mag/U_lim (формат IQ24, только чтение)
	long Id_ref;        //!< Выход: добавка к Id (0 или отрицательное число, формат IQ24)
	TPidReg3 pid;       //!< Внутренний ПИ-регулятор ослабления поля
	void (*init)(struct SFieldWeakening*); //!< Указатель на функцию инициализации
	void (*calc)(struct SFieldWeakening*); //!< Указатель на функцию быстрого расчета
};

typedef struct SFieldWeakening TFieldWeakening;

//! Инициализатор по-умолчанию.
//! U_setpoint=0.85 о.е., Id_mag_max=0.5 о.е. (=2.5 А при I_nom=5 А).
//! Kp=0.05, Ki=0.005 - медленный мягкий контур. Алгоритм по умолчанию
//! выключен - для включения пользователь должен задать DebugW3!=0.
#define FIELD_WEAKENING_DEFAULTS { \
    .U_setpoint = _IQ(0.85), \
    .Id_mag_max = _IQ(0.5), \
    .pid = { \
        .Kp_reg3 = _IQ(0.05), \
        .pid_out_max = _IQ(0), \
        .pid_out_min = _IQ(-0.5), \
        .Ki_reg3 = _IQ(0.005), \
        .Kc_reg3 = _IQ(0.5), \
        .Kd_reg3 = _IQ(0), \
        .calc = pid_reg3_calc, \
        .reset = pid_reg3_reset, \
    }, \
    .init = FieldWeakening_Init, \
    .calc = FieldWeakening_Calc, \
}

//! \memberof TFieldWeakening
void FieldWeakening_Init(TFieldWeakening*);
//! \memberof TFieldWeakening
void FieldWeakening_Calc(TFieldWeakening*);

#ifdef __cplusplus
}
#endif

#endif
/*@}*/
