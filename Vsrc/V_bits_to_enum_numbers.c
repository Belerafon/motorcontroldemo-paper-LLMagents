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
 
 \file      V_bits_to_enum_numbers.c
 \brief     Модуль "листания" битовых переменных (см. TBitsToEnumNums)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 */

/** \addtogroup V_bits_to_enum_numbers */
/*@{*/

#include "DSP.h"
#include "V_bits_to_enum_numbers.h"

//! Преобразует битовые переменные в числа

//!Требуется в основном для модуля защит или предупреждений,
//!для преобразования выставленных бит аварии в словах аварии в
//!порядковый номер этих бит и "листания" этих номеров с определенной
//!частотой, если выставлено несколько бит. Такое преобразование нужно для 
//!удобства наблюдения аварий/предупреждений пользователем.
//!Вызывать необходимо с определенной дискретизацией (например, 10кГц).
//! \memberof TBitsToEnumNums
void BitsToEnumNums(TBitsToEnumNums* p) {
	Uint16 i;

	p->counter++;
	if (p->counter < p->out_refresh_devisor)
		return;
	p->counter = 0;

	//проверка правильности вызова
	if ((p->num_of_words > NUM_OF_W_PTRS) || (p->num_of_words == 0))
		return;

	//перебираем биты в порядке возрастания
	for (i = 0; i < (16 * p->num_of_words); i++) {
		p->last_bit++;
		if (p->last_bit > 15) {
			p->last_bit = 0;
			p->last_word++;
			if (p->last_word > (p->num_of_words - 1))
				p->last_word = 0;
		}
		//делаем проверку бита
		if ((*(p->w_ptrs[p->last_word])) & (0x1 << p->last_bit)) {
			//битик имеется выводим результат
			p->output = p->last_word * 16 + p->last_bit + 1;
			return;
		}
	}

	//если дошли до сюда значит нет битиков, потому выдаем 0
	p->output = 0;
}

/*@}*/

