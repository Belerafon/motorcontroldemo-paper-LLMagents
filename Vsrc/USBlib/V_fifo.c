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

 \file      V_fifo.c
 \brief     Кольцевой масив типа FIFO. Позволяет записывать данные типа "Uint8"
 пакетами произвольной длины.

 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 08/08/2017

 \addtogroup V_fifo
@{
*/

#include "DSP.h"
#include "V_fifo.h"

Tfifo fifosha = FIFO_DEFAULTS;

//! Сколько свободного места осталось
//! Буфер пуст, если rp==wp и при этом wp не обернулся вокруг буфера (либо
//! обернулся, но rp тоже обернулся)
//!  \memberof Tfifo
//!
//!  \param *p указатель на модуль FIFO
//!  \return количетсво свободных (доступных для записи) элементов в буфере
int16 getFreeSpace (Tfifo *p){
    if ((p->rp == p->wp) && (p->wrapFlag == 0)) return p->size;
    else return ((p->size - p->wp + p->rp) & (p->size - 1));
}

//! Сколько данных есть в буфере
//! Полный - если rp==wp при полном обороте wp вокруг буфера
//!  \memberof Tfifo
//!
//! \param *p указатель на модуль FIFO
//! \return количетсво заполненых (доступных для чтения) элементов в буфере
int16 getFill (Tfifo *p){
    if ((p->rp == p->wp) && (p->wrapFlag == 1)) return p->size;
    else return ((p->wp - p->rp) & (p->size - 1));
}


//! Положить в буфер fifo len слов из буфера data
//!  \memberof Tfifo
//!
//! \param *p указатель на модуль FIFO
//! \param len количетсво данных для записи
//! \param *data указатель на массив данных, которые нужно записать
//! \return stat статус завершения операции (ОК/Буфер Занят/Нет места)
int16 Fifo_Put (Tfifo*p, Uint16 len, Uint8* data){
    int i;

    // Проверяем, что фифо не занят другими делами
    if (p->busy == 1) return FIFO_BUSY;
    else p->busy = 1;

    // Проверка, что достаточно места для len elementoв
    if (p->availSpace(p) >= len){
    	// Кладём в буфер элементы. Если вдруг перешли за границы буфера - ставим флаг wrapFlag
        for (i = 0; i < len; i++){
            p->buff[p->wp] = *(data + i);
            p->wp++;
            if (p->wp == p->size) {
                p->wp = 0;
                p->wrapFlag = 1;
            }
        }
        p->busy = 0;
        return FIFO_OK;
    } else {
    	p->busy = 0;
        return FIFO_NOT_ENOUGH_SPACE;
    }
}

//! Взять из буфера len слов
//!  \memberof Tfifo
//!
//! \param *p указатель на модуль FIFO
//! \param len количетсво данных для чтения
//! \param *data указатель на массив, в который нужно сложить данные
//! \return stat статус завершения операции (ОК/Буфер Занят/Нет данных)
int16 Fifo_Get (Tfifo*p, Uint16 len, Uint8* data){
    int i;

    // Проверяем, что фифо не занят другими делами
    if (p->busy == 1) return FIFO_BUSY;
    else p->busy = 1;

    if (p->filledSpace(p) >= len){
        for (i = 0; i < len; i++){
            *(data + i) = p->buff[p->rp];
            p->rp++;
            if (p->rp == p->size) {
                p->rp = 0;
                p->wrapFlag = 0;
            }
        }
        p->busy = 0;
        return FIFO_OK;
    } else {
    	p->busy = 0;
        return FIFO_NOT_ENOUGH_DATA;
    }
}

//! Сброс ФИФО. Фактически данные внутри FIFO не стираются, но указатели
//! чтения и записи сбрасываются
//!  \memberof Tfifo
//!
//! \param *p указатель на модуль FIFO
//! \return нет
void Fifo_Clear (Tfifo*p){
	p->wp = 0;
	p->rp = 0;
	p->wrapFlag = 0;
}
/*@}*/
