/* ==================================================================================
//File name:       X_CANFlashProgData.h
//
//Originator:	Aliamkin Dmitriy I
//
//Description:   инициализирует процесс прожига по CANу
//ПРИМЕЧАНИЕ:    
//=====================================================================================
//-------------------------------------------------------------------------------------
// 16.12.2011	Release	Rev 1.0
//----------------------------------------------------------------------------------*/
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
