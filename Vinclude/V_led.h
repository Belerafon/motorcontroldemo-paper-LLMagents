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

 \file      Vled.h
 \brief     Управление светодиодами
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 */

#ifndef VINCLUDE_V_LED_H_
#define VINCLUDE_V_LED_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include "DSP.h"

#define LED1	0
#define LED2	1
#define LED3	2
#define LED4	3
#define LED5	4
#define LED6	5

#define LED_ALL	6

struct Sled;
typedef volatile struct Sled Tled;

struct Sled{
	Uint16 timer1, timer2, timer3, timer4, timer5, timer6, timer7, timer8;
	Uint16 mode, prevMode;
	Uint16 mode3counter;
	void (*init)(Tled*);
	void (*toggle)(Tled*, Uint16);
	void (*on)(Tled*, Uint16);
	void (*off)(Tled*, Uint16);
	void (*msCalc)(Tled*);
};



#define LED_DEFAULTS { \
    .init = LED_init, \
    .toggle = LED_toggle, \
    .on = LED_on, \
    .off = LED_off, \
    .msCalc = LED_msCalc \
}
void LED_init (Tled*);
void LED_toggle (Tled*, Uint16);
void LED_on (Tled*, Uint16);
void LED_off (Tled*, Uint16);
void LED_msCalc (Tled*);

#ifdef __cplusplus
}
#endif

#endif /* VINCLUDE_V_LED_H_ */
