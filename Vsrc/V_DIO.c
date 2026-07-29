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

 \file      V_DIO.c
 \brief     Модуль дискретных входов/выходов
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 25/08/2017

 \addtogroup
 @{*/



#include "main.h"
#include "filter.h"



// Вектор состоЯниЯ дискретных выходов
volatile Uint16 output_vect = 0;
// Маска инвертированиЯ вектора состоЯниЯ дискретных выходов
volatile Uint16 output_mask = 0;

// Вектор состоЯниЯ дискретных входов
volatile Uint16 input_vect = 0;
// Маска инвертированиЯ вектора состоЯниЯ дискретных входов
volatile Uint16 input_mask = 0;
// Коэффициент фильтрации дискретных входов Kf = 0.001/Tф
// (Tф - постоЯннаЯ времени фильтра, сек)
volatile _iq inputKf = _IQ(0.03); //Время срабатывания составляется 60мс при inputKf=_IQ(0.03) и inputOn = _IQ(0.8) в 1кГц прерывании;
// Порог перехода дискретного входа в состоЯние OFF (0)
volatile _iq inputOff = _IQ(0.3);
// Порог перехода дискретного входа в состоЯние ON (1)
volatile _iq inputOn = _IQ(0.8);
// Промежуточный вектор состоЯниЯ дискретных входов (без учёта маски инвертированиЯ)
Uint16 input = 0;

// Фильтры дискретных входов
TFilter fIn1 = FILTER_DEFAULTS;
TFilter fIn2 = FILTER_DEFAULTS;
TFilter fIn3 = FILTER_DEFAULTS;


void DIO_Init()
{
#if defined (HW_MOTORCONTROLBOARD)
	//Проинитим дискретные входы
	NT_GPIOB->ALTFUNCCLR = (1 << 5);							//выключить альт. функцию
	NT_GPIOA->ALTFUNCCLR = (1 << 14) | (1 << 15);				//выключить альт. функцию
	NT_GPIOB->OUTENCLR = (1 << 5);								//на вход
	NT_GPIOA->OUTENCLR = (1 << 14) | (1 << 15);					//на вход

	//Проинитим дискретные выходы
	NT_GPIOB->ALTFUNCCLR = (1 << 6) | (1 << 15) | (1 << 4);		//выключить альт. функцию
	NT_GPIOB->OUTENSET = (1 << 6) | (1 << 15) | (1 << 4);		//на выход
#endif
}


void DIO_slow_calc()
{
	/*
	// Получаем вектор состоЯниЯ дискретных выходов с учётом маски инвертированиЯ
	Uint16 output = output_vect ^ output_mask;
	// Выводим управлЯющее воздействие по Выходу 1
	if (BIT_IS_SET(output, 0))
		D_OUT1_OFF;
    else
    	D_OUT1_ON;
	// Выводим управлЯющее воздействие по Выходу 2
	if (BIT_IS_SET(output, 1))
		D_OUT2_OFF;
    else
    	D_OUT2_ON;
	// Выводим управлЯющее воздействие по Выходу 3
	if (BIT_IS_SET(output, 2))
		D_OUT3_OFF;
    else
    	D_OUT3_ON;
*/

	// ОбновлЯем коэффициенты фильтрации дискретных входов
	fIn1.T = inputKf;
	fIn2.T = inputKf;
	fIn3.T = inputKf;
	// Получаем состоЯние Входа 1 с учётом фильтрации
	if (fIn1.output > inputOn) SET_BIT(input, 0);
	if (fIn1.output < inputOff) CLEAR_BIT(input, 0);
	// Получаем состоЯние Входа 2 с учётом фильтрации
	if (fIn2.output > inputOn) SET_BIT(input, 1);
	if (fIn2.output < inputOff) CLEAR_BIT(input, 1);
	// Получаем состоЯние Входа 3 с учётом фильтрации
	if (fIn3.output > inputOn) SET_BIT(input, 2);
	if (fIn3.output < inputOff) CLEAR_BIT(input, 2);

	input_vect = input ^ input_mask;
}


void DIO_fast_calc()
{
	// Инерционный фильтр дискретного Входа 1
    fIn1.input = (D_IN1 == 1) ? _IQ(1.0) : 0;
    fIn1.calc(&fIn1);
	// Инерционный фильтр дискретного Входа 2
    fIn2.input = (D_IN2 == 1) ? _IQ(1.0) : 0;
    fIn2.calc(&fIn2);
	// Инерционный фильтр дискретного Входа 3
    fIn3.input = (D_IN3 == 1) ? _IQ(1.0) : 0;
    fIn3.calc(&fIn3);
}

/*@}*/

