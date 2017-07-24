/* ==================================================================================
//File name:       DRV_INTERFACE.h
//
//Originator:	Aliamkin Dmitriy I
//
//Description:
//ѕ–»ћ≈„јЌ»≈:
//=====================================================================================
//-------------------------------------------------------------------------------------
// 02.11.2005	Release	Rev 1.0
//----------------------------------------------------------------------------------*/
// пример использования в модуле main
#ifndef DRV_INTERFACE_H
#define DRV_INTERFACE_H

#ifdef __cplusplus
extern "C"
{
#endif

#define DRV_INTERFACE_RESET_TIME 10	//выдержка в сек перед обнулением параметра Ans_Data

//!запрос на чтение аварии
#define DATA_REQUEST_READ_FAULT 1
//!запрос на запись аварии
#define DATA_REQUEST_WRITE_FAULT 2
//!запрос на запись рабочего времени
#define DATA_REQUEST_WRITE_WORK_TIME 3
//!запрос на чтение рабочего времени
#define DATA_REQUEST_READ_WORK_TIME 4
#define DATA_REQUEST_WRITE_OPER_TIME 5
//!запрос на чтение времени во включенном состоянии
#define DATA_REQUEST_READ_OPER_TIME 6
//!очистка всего
#define DATA_REQUEST_CLEAR_ALL 10


  struct SDrvInterface
  {
    long req_data;
    long ans_data;
    long data_High;
    long data_Low;

    long INTERFACE_time_sec;//!<
    long INTERFACE_delta_time_sec;//!<
    void (*calc)(struct SDrvInterface*);
  };

  typedef struct SDrvInterface TDrvInterface;

#define DRV_INTERFACE_DEFAULTS {0,0,0,0,\
    0,0,\
    DRV_INTERFACE_Calc,\
  }

  void DRV_INTERFACE_Calc(TDrvInterface*);

#ifdef __cplusplus
}
#endif

#endif


