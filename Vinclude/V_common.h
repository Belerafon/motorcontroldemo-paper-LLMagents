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

 \file      v_common.h
 \brief     Объявление структур данных различного назначения, которые еще слишком малы для того, чтобы
  организовывать их в отдельный модуль
 \author    ООО "НПФ Вектор". Все права защищены. http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \defgroup  v_common Объявление структур данных различного назначения

@{*/

#ifndef V_COMMON_H
#define V_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif


//!Битовое поле для битового статуса системы управления
  typedef struct
  {
	  unsigned int ready:
    1;
    unsigned int running:
    1;
    unsigned int fault:
    1;
    unsigned int alarm:
    1;
    unsigned int testing:
    1;
    unsigned int overheat:
    1;
  } DRV_STATUS_BIT;

//!Структура для статуса системы управления (содержит DRV_STATUS_BIT)
  typedef union
  {
    int all;
    DRV_STATUS_BIT bit;
  }TDrvStatus;


//!Битовое поле слова управления
  typedef struct
  {
unsigned int start:
    1;
unsigned int stop:
    1;
unsigned int trip_reset:
    1;
unsigned int reserved:
    5;
  } CMD_BIT;

//!Структура слова управления (содержит DRV_STATUS_BIT)
  typedef union
  {
    unsigned int all;
    CMD_BIT bit;
  }TCmd;

  
//! Структура, содержащия основные задания для привода
  typedef volatile struct
  {
    long speed_ref;//!< Задание скорости
    long Iq_ref;//!< Задание токоограничения для векторной системы управления 
    long i_flux_ref;//!< Задание тока для замкнутой по току системы управления (не векторной)
    long theta_elec;//!< Задание углового положения вектора тока/напряжения
	long ua_ref, uf_ref; // Задания тока якоря и тока ОВ для ДПТ
	long DCMspeed_ref;//!< Задание скорости
  } TRefs;

//! Структура для хранения параметров привода, двигателя, базовых величин системы управления
  typedef struct
  {
    long Rs;//!< Сопротивление фазы статора
    long Ls;//!< Индуктивность фазы статора
    int p;//!< Количество пар полюсов
    int I_nom;//!< Номинальный (базовый) фазный ток в формате 16.0
    long _1_I_nom;//!< 1/I_nom
    int U_nom;//!< Номинальное (базовое) фазное напряжение в формате 16.0
    long _1_U_nom;//!< 1/U_nom 
    int Udc_nom;//!< Номинальное (базовое) напряжение ЗПТ в формате 16.0
    long _1_Udc_nom;//!< 1/Udc_nom
    int speed_nom;//!< Номинальная (базовая) скорость в формате 16.0
    int freq_nom;//!< Номинальная (базовая) частота в формате 16.0
    int power;//!< Номинальная (базовая) мощность в формате 16.0
    int sens_type;//!< Тип датчика положения
  } TDrvParams;

  
//! Структура для битовых "переключателей", таких как направление вращения, разрешение рекуперации и т.п.
typedef struct {
		Uint16  recuperation_ena;//!< Разрешение рекуперации энергии при торможении
		Uint16  rotation_dir;//!< Направление вращения
		Uint16  Reboot;//!< Программная перезагрузка
		Uint16 	AutoOffset;//!< автосмещение АЦП
		Uint16 	HardwareType;//!< Тип железа, под который собран проект
		Uint16 	excitation_ena;//!< Разрешение возбудителя
		} TSysSwitches;

#ifdef __cplusplus
}
#endif

#endif

/*@}*/

