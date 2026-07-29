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

 \file      V_DIO.h
 \brief     Модуль дискретных входов/выходов
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 25/08/2017

 \addtogroup
 @{*/


#ifndef V_DIO_H
#define V_DIO_H

#ifdef __cplusplus
extern "C" {
#endif


// Макрос установки в '1' заданного бита
// var - целеваЯ переменнаЯ
// index - индекс (смещение) устанавливаемого бита
#define SET_BIT(var, index) var |= (1 << index);

// Макрос сброса в '0' заданного бита
// var - целеваЯ переменнаЯ
// index - индекс (смещение) очищаемого бита
#define CLEAR_BIT(var, index) var &= (~(1 << index));

// Макрос проверки ненулевого состоЯния заданного бита
// var - целеваЯ переменнаЯ
// index - индекс (смещение) проверЯемого бита
#define BIT_IS_SET(var, index) ((var & (1 << index)) != 0)

// Макрос проверки нулевого состоЯния заданного бита
// var - целеваЯ переменнаЯ
// index - индекс (смещение) проверЯемого бита
#define BIT_IS_CLEAR(var, index) ((var & (1 << index)) == 0)


// Дефайним дискретные входы
#ifdef HW_MOTORCONTROLBOARD
#define D_IN1			((NT_GPIOB->DATA & (1 << 5)) == 0)			//B5	Старт
#define D_IN2			((NT_GPIOA->DATA & (1 << 14)) == 0)			//A14	Стоп
#define D_IN3			((NT_GPIOA->DATA & (1 << 15)) == 0)			//A15	Сброс аварий
#else
#define D_IN1	0
#define D_IN2	0
#define D_IN3	0
#endif

// Дефайним дискретные выходы
#ifdef HW_MOTORCONTROLBOARD
#define D_OUT1_ON		NT_GPIOB->MASKLOWBYTE_bit[64].MASKLB = 64			//NT_GPIOB->DATA |= (1 << 6)
#define D_OUT1_OFF		NT_GPIOB->MASKLOWBYTE_bit[64].MASKLB = 0			//NT_GPIOB->DATA &= ~(1 << 6)
#define D_OUT2_ON		NT_GPIOB->MASKHIGHBYTE_bit[128].MASKHB = 128		//NT_GPIOB->DATA |= (1 << 15)
#define D_OUT2_OFF		NT_GPIOB->MASKHIGHBYTE_bit[128].MASKHB = 0			//NT_GPIOB->DATA &= ~(1 << 15)
#define D_OUT3_ON		NT_GPIOB->MASKLOWBYTE_bit[16].MASKLB = 16			//NT_GPIOB->DATA |= (1 << 4)
#define D_OUT3_OFF		NT_GPIOB->MASKLOWBYTE_bit[16].MASKLB = 0			//NT_GPIOB->DATA &= ~(1 << 4)
#else
#define D_OUT1_ON
#define D_OUT1_OFF
#define D_OUT2_ON
#define D_OUT2_OFF
#define D_OUT3_ON
#define D_OUT3_OFF
#endif



// Вектор состоЯниЯ дискретных выходов
extern volatile Uint16 output_vect;
// Маска инвертированиЯ вектора состоЯниЯ дискретных выходов
extern volatile Uint16 output_mask;

// Вектор состоЯниЯ дискретных входов
extern volatile Uint16 input_vect;
// Маска инвертированиЯ вектора состоЯниЯ дискретных входов
extern volatile Uint16 input_mask;

// Коэффициент фильтрации дискретных входов Kf = 0.0001/Tф
// (Tф - постоЯннаЯ времени фильтра, сек)
extern volatile _iq inputKf;
#define in_Kf inputKf	// псевдоним длЯ уже имеющейся переменной в словаре CANopen

// Порог перехода дискретного входа в состоЯние OFF (0)
extern volatile _iq inputOff;
// Порог перехода дискретного входа в состоЯние ON (1)
extern volatile _iq inputOn;


void DIO_Init();

void DIO_slow_calc();	// вызывать в фоновой программе

void DIO_fast_calc();	// вызывать с частотой 1кГц или 10 кГц


#ifdef __cplusplus
}
#endif

#endif

/*@}*/
