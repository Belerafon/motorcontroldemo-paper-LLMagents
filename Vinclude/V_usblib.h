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

 \file      V_usblib.c
 \brief     Набор функций для инициализации драйвера USB. Основан на примере
 			USB VCP" от НИИЭТ:
 			https://bitbucket.org/niietcm4/
 			---> / chip / K1921VK01T / GCC / USB / CDC_VCP /

 \author    ООО "НПФ Вектор". http://motorcontrol.ru, НИИЭТ http://niiet.ru
 \version   v 1.0 08/08/2017

*/

#ifndef VINCLUDE_V_USBLIB_H_
#define VINCLUDE_V_USBLIB_H_

int USB_init_hardware(void);
void USB_enable_peripheral (void);
void SystemInit (void);

extern Uint8 usbActiveFlag;
#endif /* VINCLUDE_V_USBLIB_H_ */
