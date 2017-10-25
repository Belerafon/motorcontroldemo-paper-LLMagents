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

 \file      V_DIO.c
 \brief     ћодуль дискретных входов/выходов
 \author    ќќќ "Ќѕ‘ ¬ектор". http://motorcontrol.ru
 \version   v 1.0 25/08/2017

 \addtogroup
 @{*/



#include "main.h"
#include "filter.h"



// ¬ектор состояния дискретных выходов
volatile Uint16 output_vect = 0;
// ћаска инвертирования вектора состояния дискретных выходов
volatile Uint16 output_mask = 0;

// ¬ектор состояния дискретных входов
volatile Uint16 input_vect = 0;
// ћаска инвертирования вектора состояния дискретных входов
volatile Uint16 input_mask = 0;
//  оэффициент фильтрации дискретных входов Kf = 0.001/Tф
// (Tф - постоянная времени фильтра, сек)
volatile _iq inputKf = _IQ(0.03); //¬рем€ срабатывани€ составл€етс€ 60мс при inputKf=_IQ(0.03) и inputOn = _IQ(0.8) в 1к√ц прерывании;
// ѕорог перехода дискретного входа в состояние OFF (0)
volatile _iq inputOff = _IQ(0.3);
// ѕорог перехода дискретного входа в состояние ON (1)
volatile _iq inputOn = _IQ(0.8);
// ѕромежуточный вектор состояния дискретных входов (без учЄта маски инвертирования)
Uint16 input = 0;

// ‘ильтры дискретных входов
TFilter fIn1 = FILTER_DEFAULTS;
TFilter fIn2 = FILTER_DEFAULTS;
TFilter fIn3 = FILTER_DEFAULTS;


void DIO_Init()
{
#if defined (HW_MOTORCONTROLBOARD)
	//ѕроинитим дискретные входы
	NT_GPIOB->ALTFUNCCLR = (1 << 5);							//выключить альт. функцию
	NT_GPIOA->ALTFUNCCLR = (1 << 14) | (1 << 15);				//выключить альт. функцию
	NT_GPIOB->OUTENCLR = (1 << 5);								//на вход
	NT_GPIOA->OUTENCLR = (1 << 14) | (1 << 15);					//на вход

	//ѕроинитим дискретные выходы
	NT_GPIOB->ALTFUNCCLR = (1 << 6) | (1 << 15) | (1 << 4);		//выключить альт. функцию
	NT_GPIOB->OUTENSET = (1 << 6) | (1 << 15) | (1 << 4);		//на выход
#endif
}


void DIO_slow_calc()
{
	// ѕолучаем вектор состояния дискретных выходов с учЄтом маски инвертирования
	Uint16 output = output_vect ^ output_mask;
	// ¬ыводим управляющее воздействие по ¬ыходу 1
	if (BIT_IS_SET(output, 0))
		D_OUT1_OFF;
    else
    	D_OUT1_ON;
	// ¬ыводим управляющее воздействие по ¬ыходу 2
	if (BIT_IS_SET(output, 1))
		D_OUT2_OFF;
    else
    	D_OUT2_ON;
	// ¬ыводим управляющее воздействие по ¬ыходу 3
	if (BIT_IS_SET(output, 2))
		D_OUT3_OFF;
    else
    	D_OUT3_ON;


	// ќбновляем коэффициенты фильтрации дискретных входов
	fIn1.T = inputKf;
	fIn2.T = inputKf;
	fIn3.T = inputKf;
	// ѕолучаем состояние ¬хода 1 с учЄтом фильтрации
	if (fIn1.output > inputOn) SET_BIT(input, 0);
	if (fIn1.output < inputOff) CLEAR_BIT(input, 0);
	// ѕолучаем состояние ¬хода 2 с учЄтом фильтрации
	if (fIn2.output > inputOn) SET_BIT(input, 1);
	if (fIn2.output < inputOff) CLEAR_BIT(input, 1);
	// ѕолучаем состояние ¬хода 3 с учЄтом фильтрации
	if (fIn3.output > inputOn) SET_BIT(input, 2);
	if (fIn3.output < inputOff) CLEAR_BIT(input, 2);

	input_vect = input ^ input_mask;
}


void DIO_fast_calc()
{
	// »нерционный фильтр дискретного ¬хода 1
    fIn1.input = (D_IN1 == 1) ? _IQ(1.0) : 0;
    fIn1.calc(&fIn1);
	// »нерционный фильтр дискретного ¬хода 2
    fIn2.input = (D_IN2 == 1) ? _IQ(1.0) : 0;
    fIn2.calc(&fIn2);
	// »нерционный фильтр дискретного ¬хода 3
    fIn3.input = (D_IN3 == 1) ? _IQ(1.0) : 0;
    fIn3.calc(&fIn3);
}

/*@}*/

