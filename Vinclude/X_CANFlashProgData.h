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

 \file      X_CANFlashProgData.h
 \brief     Бинарный файл прошивальщика - грузится в ОЗУ и ему передается управление, далее по CANopen можно обновить программу по flash.
 \author	Алямкин Д.
 \version   v 1.0 30/05/2017

 \defgroup X_CANFlashProgData */
/*@{*/



#ifndef X_CANFLASHPROGDATA_H
#define X_CANFLASHPROGDATA_H

#ifdef __cplusplus
extern "C"
{
#endif
//адрес структуры настроек
extern Uint32 const FPSettingsAddr;
//адрес начала программы прошивальщика
extern Uint32 const FPStartAddr;
//количество адресов в массиве FPAddrs
extern Uint16 const FPNumOfAddrs;
//адреса начала частей прошивальщика
extern Uint32 const FPAddrs[];
//длина частей прошивальщика
extern Uint16 const FPPartsLength[];
//данные прошивальщика
extern Uint16 const FPData[];
#ifdef __cplusplus
}
#endif

#endif

/*@}*/
