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
 \version   v 1.0

 \addtogroup V_FieldWeakening
 @{*/

#ifndef V_FIELD_WEAKENING_H
#define V_FIELD_WEAKENING_H

#include "build.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \class TFieldWeakening
  \brief Автоматическое ослабление поля векторной СУ

  Класс \a TFieldWeakening, основанный на структуре SFieldWeakening,
  поддерживает амплитуду вектора напряжения статора на уровне уставки
  U_ref за счёт автоматической выдачи отрицательного задания тока по
  оси d (оси возбуждения). Пока напряжения хватает (амплитуда меньше
  U_ref), выход Id_out удерживается равным нулю и контур работает как
  раньше. Как только амплитуда вектора напряжения превышает U_ref,
  интегратор регулятора уводит Id_out в отрицательную область, что
  компенсирует противо-ЭДС и позволяет разгонять двигатель выше
  базовой скорости.

  Модуль вызывается в быстром прерывании после расчёта ШИМ, когда
  сформирован pwm.U_mag. Выход Id_out подаётся как задание тока оси d
  (pid_id.pid_ref_reg3) на следующем такте.
*/

//! см. TFieldWeakening
struct SFieldWeakening {
	Uint16 Enabled;	//!< Разрешение ослабления поля (0 - выключено, совместимое поведение; 1 - включено)
	long U_ref;		//!< Уставка амплитуды напряжения, о.е. IQ24, ниже U_lim модуля ШИМ
	long Kp;		//!< Коэффициент пропорциональной части, о.е. IQ24
	long Ki;		//!< Коэффициент интегральной части, о.е. IQ24
	long Kc;		//!< Коэффициент антираскрутки интегратора, о.е. IQ24
	long Id_min;	//!< Минимально допустимое (самое отрицательное) задание Id, о.е. IQ24 (<= 0)
	long U_mag;		//!< Текущая амплитуда вектора напряжения, о.е. IQ24 (для чтения/диагностики)
	long err;		//!< Текущая ошибка U_ref - U_mag, о.е. IQ24 (для чтения/диагностики)
	long up;		//!< Пропорциональная часть, о.е. IQ24 (для чтения/диагностики)
	long ui;		//!< Интегральная часть, о.е. IQ24 (внутреннее состояние)
	long Id_out;	//!< Выход - задание тока оси d, о.е. IQ24 (<= 0). Подаётся в pid_id.pid_ref_reg3
	Uint16 active;	//!< Признак активного ослабления поля (Id_out < 0). Для чтения
	void (*init)(volatile struct SFieldWeakening*);
	void (*calc)(volatile struct SFieldWeakening*);
	void (*reset)(volatile struct SFieldWeakening*);
};

typedef volatile struct SFieldWeakening TFieldWeakening;

//! Инициализатор по умолчанию.
//! Enabled = 0: при включении питания поведение СУ не меняется.
//! U_ref = 0.8 о.е. чуть ниже U_lim модуля ШИМ (0.866 по умолчанию).
//! Ki/Kc задают устойчивый интегральный регулятор; Kp = 0 по умолчанию.
//! Id_min ограничивает глубину ослабления поля безопасным значением.
#define FIELD_WEAKENING_DEFAULTS { \
    .Enabled = 0, \
    .U_ref = _IQ(0.8), \
    .Kp = _IQ(0.0), \
    .Ki = _IQ(0.5), \
    .Kc = _IQ(0.5), \
    .Id_min = _IQ(-0.8), \
    .init = FieldWeakening_init, \
    .calc = FieldWeakening_calc, \
    .reset = FieldWeakening_reset \
}

//! \memberof TFieldWeakening
void FieldWeakening_init(volatile struct SFieldWeakening*);
//! \memberof TFieldWeakening
void FieldWeakening_calc(volatile struct SFieldWeakening*);
//! \memberof TFieldWeakening
void FieldWeakening_reset(volatile struct SFieldWeakening*);

#ifdef __cplusplus
}
#endif

#endif

/*@}*/
