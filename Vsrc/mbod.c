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

 \file      mbod.c
 \brief     Словарь объектов MODBUS
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2017

*/

/** \addtogroup MODBUS */
/*@{*/

#include "mbod.h"
#include "main.h"


/* Тестовые посылки
 * Формат байт в посылке:
 * Номер узла 1 байт, функция 1 байт, адрес 2 байта, данные/кол-во переменных 2 байта, контрольная сумма CRC16(LH)
 * Удобно тестировать, например, программой COMpump
 *
01 06 07 D0 00 01 48 87  - запустить привод (запись в 2000 значения 1)
01 06 07 D1 00 64 D9 6C - задать частоту вращения (2001) равной 100 об/мин
01 06 A8 6C 00 64 68 5C - задать защиту по напряжению Udc равной 100В

01 03 9C 42 00 01 0A 4E - запрос текущей частоты вращения
01 03 9C 76 00 01 4B 80	- запрос кодов аварий


В массивах:
mbodHR - переменные Holding Registers (на чтение и запись). Код функции	0x03(на чтение) 0x06(на запись)
mbodIR - переменные Input Registers (на чтение). Код функции	0x04
mbodC - битовые переменные Coil (для записи). Код функции 0x05(на запись)
mbodDI - битовые переменные Discrete Inputs (для чтения). Код функции 0x02(на чтение)

Первая цифра массива - адрес переменной в словаре  ModBus.
Вторая - номер бита для битовых переменных в переменной контроллера.
Третья - адрес переменной.


*/

volatile MB_Record mbodHR[] = {\
		2000,  0,(long)(&MBVarsConv.Vars.Command),/*Слово управления. 0й бит - пуск, 1й бит - стоп, 2й бит - сброс аварии */\
		2001,  0,(long)(&MBVarsConv.Vars.speed_ref), /*Задание частоты вращения в об/мин*/\
		40002, 0,(long)(&MBVarsConv.Vars.CurSpeed),/* Текущая частота вращения (об/мин) */\
		40007, 0,(long)(&MBVarsConv.Vars.CurIs),/*Ток статора амплитудный (А)*/\
		40054, 0,(long)(&MBVarsConv.Vars.Main_ErrorCode),/*Код аварии (сменяется, если аварий несколько)*/\
		43116,  0,(long)(&MBVarsConv.Vars.Umax_protect),/*Уставка макс. напряж ЗПТ*/\
        0,0,0	// конец массива, не удалять - по нулям определятеся размер словаря.
};

volatile MB_Record mbodIR[] = {\
                           	0,0,0	// конец массива, не удалять - по нулям определятеся размер словаря.
                           	};

volatile MB_Record mbodC[] =  {\
                           	0,0,0	// конец массива, не удалять - по нулям определятеся размер словаря.
                            };



volatile MB_Record mbodDI[] =  {0,0,0	// конец массива, не удалять - по нулям определятеся размер словаря.
                          	};

/*@}*/
