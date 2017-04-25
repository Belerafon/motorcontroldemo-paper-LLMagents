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

 \file      V_AutoOffset.h  (IQ version)
 \brief     Автоматический расчет смещения для АЦП
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016


 

 Данное программное обеспечение распространяется "как есть", и Вы,
 его пользователь, принимаете на себя все риски, связанные с его использованием.
 ООО "НПФ Вектор" не несет никакой ответственности за возможные убытки,
 связанные с его использованием.

*/


#ifndef V_AUTO_OFFSET_H
#define V_AUTO_OFFSET_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "DSP.h"

  struct SAutoOffset
  {
    _iq FilterK;//!< Коэффициент фильтра (темп изменения смещения)
    int32 IA_int; //!< Интегратор для тока фазы A
    int32 IB_int; //!< Интегратор для тока фазы B
    int32 IC_int; //!< Интегратор для тока фазы C
    int Enabled; //!< Включено/выключено
    void (*init)(volatile struct SAutoOffset *);
    void (*ms_calc)(volatile struct SAutoOffset *);
    void (*slow_calc)(volatile struct SAutoOffset *);
  };

  typedef volatile struct SAutoOffset TAutoOffset;


#define AUTO_OFFSET_DEFAULTS {\
    _IQ(0.0001),\
    0,0,0,\
	0,\
    AutoOffset_init,\
	AutoOffset_calc,\
    AutoOffset_slow_calc}

  void AutoOffset_init(TAutoOffset *);
  void AutoOffset_calc(TAutoOffset *);
  void AutoOffset_slow_calc(TAutoOffset *);

#ifdef __cplusplus
}
#endif

#endif
