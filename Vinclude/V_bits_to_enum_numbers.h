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
 
 \file      V_bits_to_enum_numbers.h
 \brief     Модуль "листания" битовых переменных (см. TBitsToEnumNums)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \defgroup  V_bits_to_enum_numbers Модуль "листания" битовых переменных (см. TBitsToEnumNums)
 @{
*/

#ifndef V_BITS_TO_ENUM_NUMS_H
#define V_BITS_TO_ENUM_NUMS_H

#ifdef __cplusplus
extern "C"
{
#endif

//!количество заразервированных мест для битовых переменных
#define NUM_OF_W_PTRS 2

  /*! \class TBitsToEnumNums
        \brief Модуль "листания" битовых переменных

        Класс \a TBitsToEnumNums, основанный на структуре SBitsToEnumNums, преобразует
        битовые переменные в числа (соответствующие номерам  битов входной переменой),
        меняющиеся с заданным интервалом. Значение в самом младшем бите соответствует 1.
        Выдает 0 если никакие биты в проверяемых словах не установлены. Предполагается для
        "пролистывания" битов слов аварий и предупреждений в целях вывода на ПУ. */

//! см. TBitsToEnumNums
  struct SBitsToEnumNums
  {
    Uint16 out_refresh_devisor;//!< делитель, задающий темп листания
    Uint16 counter;//!<счетчик таймаута листания
    Uint16 num_of_words;//!<кол-во битовых переменных
    Uint16 last_word;//!<текущее слово
    Uint16 last_bit;//!<текущий бит
    Uint16 *w_ptrs[NUM_OF_W_PTRS]; //!<указатель на битовые переменные (несколько)
    Uint16 output;//!<выход модуля (листающееся число)
    void (*calc)(volatile struct SBitsToEnumNums*);
  };

  typedef volatile struct SBitsToEnumNums TBitsToEnumNums;


//!инициализатор по умолчанию
#define BITS_TO_ENUM_NUMS_DEFAULTS {1000,0,4,0,0,{0},0,\
    BitsToEnumNums,\
  }
//! \memberof TBitsToEnumNums
  void BitsToEnumNums(TBitsToEnumNums*);


#ifdef __cplusplus
}
#endif

#endif


/*@}*/

