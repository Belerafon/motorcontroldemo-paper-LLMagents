/******************************************************************************
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

 * @file		DSP.h
 * @brief		Файл подключения заголовочных файлов периферии и объявления системных констант
 * @author    	ООО "НПФ Вектор". http://motorcontrol.ru
 * @version		v1.0
 * @date		11 декабря 2015

 ******************************************************************************/

#ifndef VINCLUDE_DSP_H_
#define VINCLUDE_DSP_H_

// Выбор частоты кварца. Влияет на делители при настройке тактирования
//#define QUARTZ_10MHZ
#define QUARTZ_12MHZ

//частота тактирования проца
#define CORE_CLK 100000000


#define __CM4_REV               0x0001	//в K1921VK01T.h есть объявление __CM4F_REV, но в core_cm4.h проверяется именно __CM4_REV. Если не объявлено там объявляется нулем и больше нигде, вроде, не используется эта константа
#define __CHECK_DEVICE_DEFINES


#include <stdint.h>

#include "K1921BK01T.h"
#include "core_cm4.h"             /* Cortex-M4 processor and core peripherals */
#include "EPwm_defines.h"	//Файл объявления системных констант для работы с ШИМ модулями


typedef char int8;
typedef unsigned char Uint8;
typedef unsigned short int Uint16;
typedef unsigned int Uint32;
typedef short int int16;
typedef int int32;
typedef unsigned long long Uint64;
typedef long long int64;

//Указатели на секцию памяти fastcode, где лежат функции для копирования из флеша в оперативку.
//Определены в файле компоновки памяти.
extern int __fastcode_ram_start;
extern int __fastcode_ram_end;
extern int __fastcode_flash_start;

extern int __isr_vector_flash_start;
extern int __isr_vector_ram_start;
extern int __isr_vector_ram_end;

//! Запрет прерываний
#define DINT __disable_irq()
//! Разрешение прерываний
#define EINT __enable_irq()


//приоритеты прерываний
#define IRQ_PRIORITY_TZ 1
#define IRQ_PRIORITY_ADC 2
#define IRQ_PRIORITY_CAP 3
#define IRQ_PRIORITY_EQEP 3
#define IRQ_PRIORITY_10K 4
#define IRQ_PRIORITY_CAN 5
#define IRQ_PRIORITY_1K 6




#endif /* VINCLUDE_DSP_H_ */
