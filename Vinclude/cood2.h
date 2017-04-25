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

 \file      cood2.h
 \brief     Объявляет некоторые служебные переменные, соторые должны быть определены в cood2.c
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 2017_01_25

 @{
*/

#include "DSP.h"

//********************************************************************************************************
//глобальные переменные 2-го CAN-а
extern  int16 const CO2_OD_TBL1[];
extern  Uint16 const CO2_OD_TBL2[];
extern  long const CO2_OD_TBL3[];
extern  Uint16 const CO2_TYPE_DEF_TABLE[];
extern  long const CO2_OD_CALLBACK_TBL[];

extern  Uint16 const co2_first1000;
extern  Uint16 const co2_first2000;
extern  Uint16 const co2_first3000;
extern  Uint16 const co2_first4000;
extern  Uint16 const co2_first5000;

extern  Uint16 const co2_SPIrange1rw;
extern  Uint16 const co2_SPIrange1rwCRC;
extern  Uint16 const co2_SPIrange1rwp;
extern  Uint16 const co2_SPIrange1rwpCRC;
extern  Uint16 const co2_SPIrange1rwps;
extern  Uint16 const co2_SPIrange1rwpsCRC;

extern Uint16 const co2_SPIrange2rw;
extern Uint16 const co2_SPIrange2rwCRC;
extern Uint16 const co2_SPIrange2rwp;
extern Uint16 const co2_SPIrange2rwpCRC;
extern Uint16 const co2_SPIrange2rwps;
extern Uint16 const co2_SPIrange2rwpsCRC;

extern Uint16 const co2_SPIrange3rw;
extern Uint16 const co2_SPIrange3rwCRC;
extern Uint16 const co2_SPIrange3rwp;
extern Uint16 const co2_SPIrange3rwpCRC;
extern Uint16 const co2_SPIrange3rwps;
extern Uint16 const co2_SPIrange3rwpsCRC;

extern Uint16 const co2_SPIrange4rw;
extern Uint16 const co2_SPIrange4rwCRC;
extern Uint16 const co2_SPIrange4rwp;
extern Uint16 const co2_SPIrange4rwpCRC;
extern Uint16 const co2_SPIrange4rwps;
extern Uint16 const co2_SPIrange4rwpsCRC;

extern Uint16 const co2_SPIrange5rw;
extern Uint16 const co2_SPIrange5rwCRC;
extern Uint16 const co2_SPIrange5rwp;
extern Uint16 const co2_SPIrange5rwpCRC;
extern Uint16 const co2_SPIrange5rwps;
extern Uint16 const co2_SPIrange5rwpsCRC;

extern  Uint16 const co2_numOfInd;

/*@}*/

