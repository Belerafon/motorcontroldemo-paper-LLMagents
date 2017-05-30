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

 \file      V_adc.h
 \brief     Модуль обработки АЦП (см. TAdcDrv)
 \author    ООО "НПФ Вектор".  http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \defgroup  V_adc Модуль обработки АЦП (см. TAdcDrv)

 @{
*/


#ifndef V_ADC_H
#define V_ADC_H

#ifdef __cplusplus
extern "C"
{
#endif

  /*! \class TAdcDrv
      \brief Модуль обработки АЦП

      Класс \a TAdcDrv, основанный на структуре SAdcDrv, содержит в себе
      методы для обработки результатов АЦП. А именно: считывание, нормировку и
      приведение к относительным единицам, фильтрацию. У каждого измеряемого сигнала
      есть как минимум два параметра для настройки: коэффициент усиления и смещение.
      Настройку датчиков желательно производить так: подать нулевой сигнал и настроить
      смещение, чтобы измеряемое значение показывало ноль. Подать уровень сигнала, близкий
      к номинальному и настроить коэффициент усиления, чтобы измеренное значение соответствовало
      действительности.*/

//! см. TAdcDrv
  struct SAdcDrv
  {
    int16 Imeas_a_gain;    //!< Коэффициент усиления.
    int16 Imeas_a_offset;  //!< Смещение нуля.
    int32 Imeas_a;        //!< Измеренное значение усредненное.
    int32 IaGainNom;      //!< Вспомогательный коэфф. усиления.
    int32 IA_temp;        //!<Временная переменная

    int16 IASampleLength;			//длина выборки (кол-во точек на периоде ШИМ)
	int16 IAPointer;				//номер последней выборки
	int16 IABuf[4];					//массив точек для усреднения


    int16 Imeas_b_gain; 
    int16 Imeas_b_offset;  
    int32 Imeas_b;       
    int32 IbGainNom;
    int32 IB_temp;

    int16 IBSampleLength;			//длина выборки (кол-во точек на периоде ШИМ)
	int16 IBPointer;				//номер последней выборки
	int16 IBBuf[4];					//массив точек для усреднения


    int16 Imeas_c_gain;    
    int16 Imeas_c_offset; 
    int32 Imeas_c;        
    int32 IcGainNom;
    int16 IC_temp;

    int16 ICSampleLength;			//длина выборки (кол-во точек на периоде ШИМ)
	int16 ICPointer;				//номер последней выборки
	int16 ICBuf[4];					//массив точек для усреднения

    int16 Imeas_d_gain;
    int16 Imeas_d_offset;
    int32 Imeas_d;
    int32 IdGainNom;
    int16 ID_temp;

    int16 IDSampleLength;			//длина выборки (кол-во точек на периоде ШИМ)
	int16 IDPointer;				//номер последней выборки
	int16 IDBuf[4];					//массив точек для усреднения


    int16 Udc_meas_gain;
    int16 Udc_meas_offset;
    int32 Udc_meas;
    int32 UdcGainNom;
    int16 Udc_temp;

    int16 UdcSampleLength;			//длина выборки (кол-во точек на периоде ШИМ)
	int16 UdcPointer;				//номер последней выборки
	int16 UdcBuf[4];				//массив точек для усреднения


    void (*init)(struct SAdcDrv *);
    void (*fast_calc)(struct SAdcDrv*);
    void (*ms_calc)(struct SAdcDrv*);   
    void (*slow_calc)(struct SAdcDrv*);
  };


  typedef struct SAdcDrv TAdcDrv;

//!Инициализатор по умолчанию.
#define ADC_DRV_DEFAULTS {\
    54,0,0,0,0, \
	4,0,{0}, \
    54,0,0,0,0, \
	4,0,{0}, \
    54,0,0,0,0, \
	4,0,{0}, \
    54,0,0,0,0, \
	4,0,{0}, \
    800,0,0,0,0, \
	4,0,{0}, \
    AdcDrv_init,\
    AdcDrv_fast_calc,\
    AdcDrv_ms_calc,\
    AdcDrv_slow_calc,\
  }


//! \memberof TAdcDrv
  void AdcDrv_init(TAdcDrv*);
//! \memberof TAdcDrv
  void AdcDrv_slow_calc(TAdcDrv*);
//! \memberof TAdcDrv
  void AdcDrv_fast_calc(TAdcDrv*);
//! \memberof TAdcDrv
  void AdcDrv_ms_calc(TAdcDrv*);
  
#ifdef __cplusplus
}
#endif

#endif

/*@}*/




