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
 
 \file      V_SSI_Encoder.h
 \brief     Модуль оценки скорости и положения при помощи цифрового энкодера, работающего по интерфейсу SSI (см. TSSI_Encoder)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 1.0 25/04/2016

 \defgroup  V_SSI_Encoder Модуль оценки скорости и положения при помощи SSI Encoder (см. TSSI_Encoder)

 \addtogroup V_SSI_Encoder
@{*/


#include "V_IQmath.h"
#include "filter.h"
#ifndef V_SSI_ENCODER_H
#define V_SSI_ENCODER_H

#ifdef __cplusplus
extern "C" {
    #endif

/*! \class TSSI_Encoder
      \brief Модуль обработки энкодера, работающего по SSI (упрощенный SPI). А именно для датчика HENGSTLER absolute encoder AC 58.

      Класс \a TSSI_Encoder, основанный на структуре SSI_Encoder,
      является модулем для определения углового положения и скорости ротора 
      двигателя по данным от энкодера, работающего по SSI.
      Датчик предполагается подключать на разъем J8 платы из комплекта DRV8301-HC-EVM.
      Вот только микросхему гальванической развязки (U5) Texas туда поставили почему-то такой стороной, что микроконтролер
      работает как slave стройство. Чтобы сделать его мастером, нужно поставить другую аналогичную микросхему, или...
      перевернуть U5 вверх ногами, поставив маркировкой вниз, лапками к верху :)
    */

//! см. TSSI_Encoder
    struct S_SSI_Encoder {
        Uint32 					resol;		//!<INPUT: разрешение датчика в метках/оборот;
        float					resol_inv;   //!<обратная величина к разрешению датчика resol
        Uint16					pole_pairs;	//!<INPUT: число пар полюсов;
        Uint16					rotation_dir; //!<INPUT: направление вращения

        float 					speed_nom;	//!<INPUT: номинальная скорость в об/мин;
        _iq 					speed_elec;	//!<OUTPUT: электрическая скорость в Q24(относительные единицы)
        _iq 					theta_elec;	//!<OUTPUT: электрический угол в Q24(относительные единицы)
        Uint16 					Poscnt_res;	//!<Механическ.угол в метках;

        Uint16 					Poscnt_resPrev;	//!<Механическ.угол в метках предыдущее значение;
        _iq 					theta_mech;	//!<Механический угол;
        _iq						prevThetaMech; //!<Механический угол предыдущее значение;
        _iq 					theta_el_tmp;	//!<TEMPORARY: электрический угол;
        _iq						theta_mech_filtered; //!<Механический угол фильтрованный;
        _iq						theta_mech_filterK; //!<Коэффициент фильтра мех. угла;

        _iq                     AngleOffset;//!< Смещение электрического угла в градусах
        int16					RevolutionCounter; // Количество механических оборотов;

        int16					Poscnt_resContinouosInt8;//!<Механическ.угол в метках c учетом кол-ва оборотов (переменная 8 разрядов отсекается)
        int16					Poscnt_resContinouosInt;//!<Механическ.угол в метках c учетом кол-ва оборотов (переменная 16 разрядов)
        long					Poscnt_resContinouosLong;//!<Механическ.угол в метках c учетом кол-ва оборотов (переменная 32 разрядов)
        _iq 					theta_mechContinouos;	//!<Механический угол в градусах абсолютный;
        _iq 					theta_elecContinouos;	//!<Электрический угол в градусах абсолютный;

        void (*init)(volatile struct S_SSI_Encoder*);
        void (*calc)(volatile struct S_SSI_Encoder*);
        void (*read)(volatile struct S_SSI_Encoder*);
    };



    typedef  volatile struct S_SSI_Encoder TSSI_Encoder;

    //! Инициализатор по-умолчанию.
    #define SSI_ENCODER_DEFAULTS {0,0,0,0, 0,0,0,0, 0,0,0,0,0,0, 0,0, 0,0,0,0,0,\
                               SSI_Encoder_init,\
                               SSI_Encoder_Calc,\
							   SSI_Encoder_Read,\
                              }

//! \memberof TSSI_Encoder
    void SSI_Encoder_init(TSSI_Encoder*);
//! \memberof TSSI_Encoder
    void SSI_Encoder_Calc(TSSI_Encoder*);
//! \memberof TSSI_Encoder
    void SSI_Encoder_Read(TSSI_Encoder*);

    #ifdef __cplusplus
}
#endif

#endif
/*@}*/















