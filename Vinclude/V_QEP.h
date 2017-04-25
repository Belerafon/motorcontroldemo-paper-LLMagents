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

 \file      V_QEP.h
 \brief     Модуль оценки скорости и положения при помощи eQEP (см. TposspeedEqep)
 \author    ООО "НПФ Вектор". Все права защищены. http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \defgroup  V_QEP Модуль оценки скорости и положения при помощи eQEP (см. TposspeedEqep)

 \addtogroup V_QEP
@{*/


#include "V_IQmath.h"
#include "filter.h"
#ifndef V_POSSPEED_EQEP_H
#define V_POSSPEED_EQEP_H

#ifdef __cplusplus
extern "C" {
    #endif


    struct SPOSSPEEDCTL_BITS {						// bits
Uint16 InitType:
        2;		// 0:1		 INPUT: режим инициализации положения при пуске:
        //                  0 - всё выставляется вручную
        //					1 - выставляем вектор тока, далее пуск в векторном режиме(исходное положение - 0 электрических градусов);
        //					2 - пуск в векторном режиме(исходное положение - последнее сохранённое)
Uint16 index_en:
        1;		// 2		 INPUT: наличие репера
        //					0 - репер отсутствует
        //					1 - репер подключён;
Uint16 dir:
        1;			// 3		 INPUT: направление движения(в случае неправильного направления);
        //			        0 - (по умолчанию)прямое включение;
        //			 		1 - инверсия;
Uint16 CmdInit:
        1;		// 4		 INPUT: для режима инициализации
Uint16 Min_clk:
        4;		// 5:8		 INPUT: минимальное число меток для захвата на низкой скорости - НЕ МЕНяТЬ!!!!!
        //					Min_clk=2 - 4 метки
        //							3 - 8 меток - по умолчанию
        // 							4 - 16 меток
        //							5 - 32 метки
        //							6 - 64 метки
        //							7 - 128 меток
        //							8 - 256 меток
        //							9 - 512 меток
        //							10 - 1024 метки
Uint16 rsvd:
        7;			// 9:15		 reserved;
    };

    union SPOSSPEED_CTL {
        Uint16              all;
        struct SPOSSPEEDCTL_BITS  bit;
    };

    struct SPOSSPEEDFLG1_BITS {						// bits
Uint16 UTO_tmp:
        1;		// 0		 TEMPORARY FLAG: флаг time-out;
Uint16 PCO_tmp:
        1;		// 1		 TEMPORARY FLAG: флаг положительного переполнения счётчика;
Uint16 PCU_tmp:
        1;		// 2		 TEMPORARY FLAG: флаг отрицательного переполнения счётчика;
Uint16 UPPS_tmp:
        4;		// 3:6       TEMPORARY: значение текущего кванта перемещения для захвата;
Uint16 CCPS_tmp:
        3;		// 7:9       TEMPORARY: значение текущего коэффициента деления;
Uint16 first_launch:
        2;	// 10:11	 FLAG: инициализация первого запуска;
Uint16 accuracy:
        2;		// 12:13	 FLAG: флаг переключения алгоритмов;
Uint16 first_theta:
        1;	// 14 		 FLAG: флаг первого расчёта углового преращения;
Uint16 transit:
        1;		// 15		 FLAG: флаг перехода на алгоритм высоких скоростей;

    };

    union SPOSSPEED_FLG1 {
        Uint16              all;
        struct SPOSSPEEDFLG1_BITS  bit;
    };

    struct SPOSSPEEDFLG2_BITS {						// bits
Uint16 pos_ident:
        1;		// 0		 FLAG: флаг точной идентификации абсолютного положения ротора;
Uint16 UTO_tmp2:
        1;		// 1		 TEMPORARY FLAG: флаг time-out;
Uint16 PCO_tmp2:
        1;		// 2		 TEMPORARY FLAG: флаг положительного переполнения счётчика;
Uint16 PCU_tmp2:
        1; 		// 3
Uint16 lost_UTO:
        1;		// 4
Uint16 Dir_prev:
        1;		// 5
Uint16 Cdef:
        1;			// 6
Uint16 Delay_flux:
        1;	// 7
Uint16 first_index:
        1;	// 8
Uint16 rsvd:
        8;			// 9:15		 reserved;
    };

    union SPOSSPEED_FLG2 {
        Uint16              all;
        struct SPOSSPEEDFLG2_BITS  bit;
    };

    
    
/*! \class TposspeedEqep
      \brief Модуль обработки квадратурного декодера (ДПР типа "энкодер")

      Класс \a TposspeedEqep, основанный на структуре SposspeedEqep, 
      является модулем для определения углового положения и скорости ротора 
      двигателя по данным от квадратурного энкодера. Для работы с датчиком
      используется периферийный модуль процессора EQEP. Положение вала theta_elec определяется
      по аппаратному счетчику QPOSCNT, который увеличивает и уменьшает
      свое значение автоматически при вращении вала. После включения процессора
      абсолютное угловое положение вала не может быть оперелено, пока не произойдет
      событие индексации (сработает реперная метка). Модуль возводит флаг Posspeed_FLG2.bit.pos_ident,
      как только это происходит. До этого момента значение углового положения не валидное и не должно использоваться
      системой управления. Для калибровки датчика положения (смещения нулевого положения)
      используется переменная AngleOffset.
      Для вычисления скорости speed_elec используется аппаратный счетчик QCPRD,
      который измеряет время между заданным количеством  меток датчика. Для обеспечения приемлемой
      точности на высоких и низких скоростях "заданное" количество меток переключается на ходу при помощи
      делителя UPPS. Так как скорость на малых скоростях вращения пульсирует,
      имеется инерционный фильтр скорости первого порядка, значение которого,
      speed.output, может быть выведено пользователю.
    */

//! см. TposspeedEqep
    struct SposspeedEqep {
        Uint32 					resol;		//!<INPUT: разрешение датчика в метках/оборот;
        float					resol_inv;   //!<обратная величина к разрешению датчика resol
        Uint16					pole_pairs;	//!<INPUT: число пар полюсов;
        Uint16					SpeedCalcType; //!< Тип расчета скорости вращения: 0 - аппаратно, 1 - программно по углу.
        float 					speed_nom;	//!<INPUT: номинальная скорость в об/мин;
        _iq 					speed_elec;	//!<OUTPUT: электрическая скорость в Q24(относительные единицы)
        _iq 					theta_elec;	//!<OUTPUT: электрический угол в Q24(относительные единицы)
        float 					k_low;		//!<PARAMETER;
        Uint32 					Poscnt_res;	//!<Механическ.угол в метках;
        Uint16 					Poscnt_res16;	//!<Механическ.угол в метках 16ти разрядный для осца;
        Uint32 					Poscnt_resPrev;	//!<Механическ.угол в метках предыдущее значение;
        int16					MoveK;		//!<TEMPORARY;
        _iq 					theta_mech;	//!<Механический угол;
        _iq						prevThetaMech; //!<Механический угол предыдущее значение;
        _iq 					theta_el_tmp;	//!<TEMPORARY: электрический угол;
        Uint16 					speed_calc_skip; //!<пропуск расчета скорости, флаг
        _iq 					speedK;		//!<TEMPORARY: делитель частоты, тактирующей таймер захвата;
        union {                                                               /*!< Status interrupt */
            Uint16 					Qcprdlat16_tmp;	//!<TEMPORARY: копия защёлки времени;
            Uint32 					Qcprdlat_tmp;	//!<TEMPORARY: копия защёлки времени;
        };  //!<Для наблюдения в юниконе
        union {                                                               /*!< Status interrupt */
           uint16_t QEPSTS;                                               /*!< QEPSTS    : type used for word access */
           _QEP_QEPSTS_bits QEPSTS_bit;                                   /*!< QEPSTS_bit: structure used for bit access */
        };  //!<Для наблюдения в юниконе
        Uint16 				DirPrev;
        _iq					speed_tmpIQ;	//!<TEMPORARY;
        TFilter					speed_filter;          //!<Фильтр скорости
        Uint16					skip_counter;//!< Счетчик пропуска расчета скорости после смены UPPS
        union SPOSSPEED_CTL		        Posspeed_CTL;
        union SPOSSPEED_FLG1	                Posspeed_FLG1;
        union SPOSSPEED_FLG2	                Posspeed_FLG2;
        Uint16 					GPIOsValue;//!< Текущее значение ножек GPIO модуля QEP
        Uint16   				Index_eventFlag;
        Uint16                  UPPS_forWatch;//!< Текущее значение UPPS
        _iq                     AngleOffset;//!< Смещение электрического угла в градусах
        int16					RevolutionCounter; // Количество механических оборотов;
        int16					Poscnt_resContinouosInt8;//!<Механическ.угол в метках c учетом кол-ва оборотов (переменная 8 разрядов отсекается)
        int16					Poscnt_resContinouosInt;//!<Механическ.угол в метках c учетом кол-ва оборотов (переменная 16 разрядов)
        long					Poscnt_resContinouosLong;//!<Механическ.угол в метках c учетом кол-ва оборотов (переменная 32 разрядов)
        _iq 					theta_mechContinouos;	//!<Механический угол в градусах абсолютный;
        _iq 					theta_elecContinouos;	//!<Электрический угол в градусах абсолютный;

    	long speed_elec_temp; //Скорость нефильтрованная
    	long theta_start; //внутренняя перременная: Начальный угол
    	long theta_finish;//внутренняя перременная: Конечный угол
    	long d_fi;//внутренняя перременная: разница в угле
    	long KThetaToSpeed;	//внутренняя перременная: коэф. пересчета произв. угла в скорость
        void (*init)(volatile struct SposspeedEqep*);
        void (*calc)(volatile struct SposspeedEqep*);
        void (*slow_calc)(volatile struct SposspeedEqep*);
        void (*index)(volatile struct SposspeedEqep*);
    };




    typedef  volatile struct SposspeedEqep TposspeedEqep;

    //! Инициализатор по-умолчанию.
    #define POSSPEED_DEFAULTS {0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0,0, 0,{0},{0},0,0,FILTER_DEFAULTS,0,\
                               0,0x0790,0,0,0,0,0,0,0,0,0,0,0,\
							   0,0,0,0,0,\
							   TposspeedEqep_init,\
							   TposspeedEqep_Calc,\
							   TposspeedEqep_SlowCalc,\
							   TposspeedEqep_IndexEvent,\
                              }

//! \memberof TposspeedEqep
    void TposspeedEqep_init(TposspeedEqep*);
//! \memberof TposspeedEqep
    void TposspeedEqep_Calc(TposspeedEqep*);
//! \memberof TposspeedEqep
    void TposspeedEqep_SlowCalc(TposspeedEqep*);
//! \memberof TposspeedEqep
    void TposspeedEqep_IndexEvent(TposspeedEqep*);
    #ifdef __cplusplus
}
#endif

#endif
/*@}*/















