/*!
     Copyright 2017 	јќ "Ќ»»Ё“" и ќќќ "Ќѕ‘ ¬≈ “ќ–"

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
 \brief     ћодуль дискретных входов/выходов
 \author    ќќќ "Ќѕ‘ ¬ектор". http://motorcontrol.ru
 \version   v 1.0 25/08/2017

 \addtogroup
 @{*/


#ifndef V_DIO_H
#define V_DIO_H

#ifdef __cplusplus
extern "C" {
#endif


// ћакрос установки в '1' заданного бита
// var - целевая переменная
// index - индекс (смещение) устанавливаемого бита
#define SET_BIT(var, index) var |= (1 << index);

// ћакрос сброса в '0' заданного бита
// var - целевая переменная
// index - индекс (смещение) очищаемого бита
#define CLEAR_BIT(var, index) var &= (~(1 << index));

// ћакрос проверки ненулевого состояни€ заданного бита
// var - целевая переменная
// index - индекс (смещение) проверяемого бита
#define BIT_IS_SET(var, index) ((var & (1 << index)) != 0)

// ћакрос проверки нулевого состояни€ заданного бита
// var - целевая переменная
// index - индекс (смещение) проверяемого бита
#define BIT_IS_CLEAR(var, index) ((var & (1 << index)) == 0)


// ƒефайним дискретные входы
#ifdef HW_MOTORCONTROLBOARD
#define D_IN1			((NT_GPIOB->DATA & (1 << 5)) == 0)			//B5	—тарт
#define D_IN2			((NT_GPIOA->DATA & (1 << 14)) == 0)			//A14	—топ
#define D_IN3			((NT_GPIOA->DATA & (1 << 15)) == 0)			//A15	—брос аварий
#else
#define D_IN1	0
#define D_IN2	0
#define D_IN3	0
#endif

// ƒефайним дискретные выходы
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



// ¬ектор состояния дискретных выходов
extern volatile Uint16 output_vect;
// ћаска инвертирования вектора состояния дискретных выходов
extern volatile Uint16 output_mask;

// ¬ектор состояния дискретных входов
extern volatile Uint16 input_vect;
// ћаска инвертирования вектора состояния дискретных входов
extern volatile Uint16 input_mask;

//  оэффициент фильтрации дискретных входов Kf = 0.0001/Tф
// (Tф - постоянная времени фильтра, сек)
extern volatile _iq inputKf;
#define in_Kf inputKf	// псевдоним для уже имеющейс€ переменной в словаре CANopen

// ѕорог перехода дискретного входа в состояние OFF (0)
extern volatile _iq inputOff;
// ѕорог перехода дискретного входа в состояние ON (1)
extern volatile _iq inputOn;


void DIO_Init();

void DIO_slow_calc();	// вызывать в фоновой программе

void DIO_fast_calc();	// вызывать с частотой 1к√ц или 10 к√ц


#ifdef __cplusplus
}
#endif

#endif

/*@}*/
