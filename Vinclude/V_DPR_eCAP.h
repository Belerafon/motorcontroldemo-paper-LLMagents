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
 
 \file      V_DPR_eCAP.h
 \brief     Модуль расчёта скорости и положения по трём датчикам на элементах Холла.
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

*/

#ifndef V_DPR_ECAP_H
#define V_DPR_ECAP_H

#ifdef __cplusplus
extern "C"
{
#endif

struct SDPReCAPFLG1_BITS  							// bits
  {
Uint16 AngleMode:
    1;			// 0		Режим расчета угла: 0 - 6 секторов, 1 - интерполяция
Uint16 SpeedMode:
    1;			// 1		Режим расчета скорости: 0 - по каждому датчику, 1 - по каждой метке
Uint16 Dir:
    1;				// 2       	Направление движения: 0 - по часовой, 1 -  против.
Uint16 PrevDir:
    1;			// 3       	Направление движения: 0 - по часовой, 1 -  против.
Uint16 CAPnumber:
    2;			// 4-5 		Номер eCAP, по которому было предыдущее прерывание.
Uint16 ZeroFLG:
    1;			// 6		Флаг обнуления скорости
Uint16 SpeedMinFLG:
    1;		// 7

Uint16 firstlaunch1:1;  //!< 1		FLAG: первое попадание в прерывание eCAP1.
Uint16 firstlaunch2:1;	//!< 2		FLAG: первое попадание в прерывание eCAP2.
Uint16 firstlaunch3:1;	//!< 3       	FLAG: первое попадание в прерывание eCAP3.
Uint16 rsvd:
	5;				// 8-15		reserved;
  };

  union SDPReCAP_FLG1
  {
    Uint16              all;
    struct SDPReCAPFLG1_BITS  bit;
  };

  struct SDPReCAP
  {
	int16 HallCode;			//Текущий код положения по датчикам Холла
    Uint32 Ts;				// Период для расчёта интерполированного угла
    Uint32 Tspeed;			// Период для расчёт скорости
    Uint32 TsNom;			// Период, соответсвующий номинальной скорости
    Uint32 TsNomMilsec;		// Период, соответсвующий номинальной скорости при тактировании 1 мс
    Uint32 PrevTs;			// Предыдущая величина периода для расчёта угла.
    _iq speed;				// Расчитанная скорость

    _iq speedMIN;			// Уставка минимальной скорости
    _iq speedMinREF;		// Скорость, соответствующая уставке для обнуления скорости
    _iq Angle;				// Расчитанный интерполированный угол
    _iq Angle6;				// Расчитанный декодированный угол
    _iq AnglePrev;			// Расчитанный декодированный угол предыдущий

    Uint32 cnt;				// Счётчик числа прерываний с момента пуска (дальше 2 не считает).
    Uint32 cnt2;			// Счётчик прерываний смомента реверса.
    Uint32 milsec;			// Время в мс с момента прихода прошлой метки
    Uint32 milsecFIX;		// Переститанная уставка для обнуления скорости
    Uint32 milsecREF;		// Уставка для обнуления скорости
    Uint32 milsecPrevREF;	// Предыдущее значение уставки для обнуления скорости

    Uint32 PrevTspeed;		// Предыдущая величина периода для расчёта скорости
    Uint32 PrevTspeed1;		// Предыдущая величина периода для расчёта скорости по датчику А.
    Uint32 PrevTspeed11;
    Uint32 PrevTspeed2;		// Предыдущая величина периода для расчёта скорости по датчику B.
    Uint32 PrevTspeed22;
    Uint32 PrevTspeed3;		// Предыдущая величина периода для расчёта скорости по датчику C.
    Uint32 PrevTspeed33;

	int16 CAPCalcEna1;
	int16 CAPCalcEna2;
	int16 CAPCalcEna3;

	int16 CAP_WrongEdgeCnt;
	int16 CAP_WrongEdgeCnt1;
	int16 CAP_WrongEdgeCnt2;
	int16 CAP_WrongEdgeCnt3;
	int16 CAP_WrongEdgeCntPrev;

	Uint16 WrongCodeCounter;
	Uint16 WrongCodeCounterPerSec;
	Uint16 WrongCodeCounterLimitPerSec;
	int32 AngleOffset;


	int16 ErrorLevel;
	int16 ErrorLevelCounter;
	int16 ErrorLevelTimeCounterBig;
	int16 ErrorLevelTimeCounter;
	int16 SensorFault;

	float SimulatorOmega2IQ_factor;

	union SDPReCAP_FLG1	DPReCAP_FLG1;


    void (*Init)(volatile struct SDPReCAP*);
    void (*AngleCalc)(volatile struct SDPReCAP*);
    void (*Angle6Calc)(volatile struct SDPReCAP*);
    void (*AngleErrorCalc)(volatile struct SDPReCAP*);
    void (*SpeedCalc)(volatile struct SDPReCAP*);
    void (*CAP1Calc)(volatile struct SDPReCAP*);
    void (*CAP2Calc)(volatile struct SDPReCAP*);
    void (*CAP3Calc)(volatile struct SDPReCAP*);
    void (*HelpCalc)(volatile struct SDPReCAP*);
    void (*slow_calc)(volatile struct SDPReCAP*);
    void (*ms_calc)(volatile struct SDPReCAP*);
    void (*calc_10k)(volatile struct SDPReCAP*);
  };
  typedef volatile struct SDPReCAP TDPReCAP;

  /******************************************************************************
   Инициализация структуры  по умолчанию
  ******************************************************************************/
#define DPRECAP_DEFAULTS  \
  {\
	0,0,0,0,0,0,0, \
    0,0,0,0,0, \
    0,0,0,0,0,0, \
    0,0,0,0,0,0,0,\
	0,0,0,\
	0,0,0,0,0,\
    0,0,0,0,\
	0,0,0,0,0,\
	0,0,\
    &DPReCAP_Init, \
    &DPReCAP_AngleCalc, \
    &DPReCAP_Angle6Calc, \
    &DPReCAP_AngleErrorCalc, \
    &DPReCAP_SpeedCalc, \
    &DPReCAP_CAP1Calc, \
    &DPReCAP_CAP2Calc, \
    &DPReCAP_CAP3Calc, \
    &DPReCAP_HelpCalc, \
    &DPReCAP_SlowCalc, \
    &DPReCAP_msCalc, \
	&DPReCAP_calc_10k\
  }

  /******************************************************************************
  Процедура инициализации.
  ******************************************************************************/
  void DPReCAP_Init(TDPReCAP*);

  /******************************************************************************
   Процедура расчёта угла.
  ******************************************************************************/
  void DPReCAP_Angle6Calc(TDPReCAP*);

  /******************************************************************************
   Процедура вычисления факта ошибочного изменения угла.
  ******************************************************************************/
  void DPReCAP_AngleErrorCalc(TDPReCAP*);

  /******************************************************************************
   Процедура расчёта интерполированного угла.
  ******************************************************************************/
  void DPReCAP_AngleCalc(TDPReCAP*);

  /******************************************************************************
   Процедура расчёта скорости.
  ******************************************************************************/
  void DPReCAP_SpeedCalc(TDPReCAP*);

  /******************************************************************************
  Расчёт прерывания 1
  ******************************************************************************/
  void DPReCAP_CAP1Calc(TDPReCAP*);

  /******************************************************************************
  Расчёт прерывания 2
  ******************************************************************************/
  void DPReCAP_CAP2Calc(TDPReCAP*);

  /******************************************************************************
  Расчёт прерывания 3
  ******************************************************************************/
  void DPReCAP_CAP3Calc(TDPReCAP*);

  /******************************************************************************
  Вспомогательные расчёт, общий для всех прерываний
  ******************************************************************************/
  void DPReCAP_HelpCalc(TDPReCAP*);


  /******************************************************************************
  Вспомогательные расчёт в фоне
  ******************************************************************************/
  void DPReCAP_SlowCalc(TDPReCAP*);

  /******************************************************************************
  Миллисекундный расчет
  ******************************************************************************/
  void DPReCAP_msCalc(TDPReCAP*);


  void DPReCAP_calc_10k(TDPReCAP* p);
#ifdef __cplusplus
}
#endif	// extern "C"

#endif	// V_DPR_ECAP_H
