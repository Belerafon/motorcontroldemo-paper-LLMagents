/*!
 \file      v_data_log.h
 \brief     4-х канальный логгер длЯ осциллографированиЯ в реальном времени (см. TDataLog)
В версии 2.0 реализован 32-битный режим работы. Основные положения:
Тип данных даталоггера определяется символьной константой в заголовочном файле 'V_data_log.h'
(один из вариантов нужно раскомментировать):
//#define DLOG_DATA_SIZE 16
#define DLOG_DATA_SIZE 32
Для 16-битного режима даталоггер работает "классическим" методом. Старый Юникон работает как было.
Новый Юникон при этом не получает флаг 32-битного режима (см. ниже) и работает по "классическому" алгоритму
(правда, буфер принимаемых данных у него теперь 32-битный, но это тонкости). При буферной передаче каждый
канал передаётся, как и раньше, одним запросом.
32-битный режим:
В переменную 'dlog.control' добавлен флаг '32-битные данные', по которому новый Юникон узнаёт тип даталоггера.
При SDO-чтении осциллограмм данные, как и раньше, выдвигаются через 32-битную переменную 'dlog.next_value_var'
(там, кроме 16-битного значения, есть всякие поля). В неё добавлены два флага:
- флаг, что в поле "значение" - старшие 16 бит точки
- флаг, что переменная 16-разрядная
Юникон сначала запрашивает младшую часть значения, потом - старшую и т.д. по всем точкам.
Если переменная 16-разрядная, то старшая часть не запрашивается. При буферной передаче:
Т.к. один буферный запрос ограничен у нас 256-тью 16-битными словами, то Юникон сначала запрашивает
первые 256 слов (128 32-битных точек), затем - остальные 256 слов. Т.е. используется два буферных запроса.

 \author	Коллектив ООО НПФ Вектор
 \version   v 2.0 10/04/2017
 \defgroup  v_data_log Цифровой осциллограф (см. TDataLog)
 @{
*/


#ifndef V_DATA_LOG_H
#define V_DATA_LOG_H

#include "CANOpen_drv.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Размер значений, передаваемых через логгер (бит)
//#define DLOG_DATA_SIZE 16
#define DLOG_DATA_SIZE 32


#if DLOG_DATA_SIZE == 16
typedef int16 type_data;
#elif DLOG_DATA_SIZE == 32
typedef int32 type_data;
#endif


//! Размер массивов длЯ осциллографированиЯ.

//!Если нужно менЯть, то это делать надо не только тут - слишком много завЯзано на это число
//!Переделать сложно, так как на это число завЯзка косвеннаЯ: например, длЯ ускорениЯ работы
//!применЯютсЯ такие конструкции: p->Wcounter&=0xFF; Где FF - маска (символизирует 256 точек)
#define NUMBER_SAMPLES  256

  /*! \class TDataLog
    \brief 4-х канальный логгер длЯ осциллографированиЯ в реальном времени

    Класс \a TDataLog, основанный на структуре SDataLog, позволЯет записывать выбранные 4 переменных в массивы
    из 255 точек с заданной дискретизацией по времени. Кроме того, имеет
    свЯзь с драйвером CANOpen длЯ автоматизации указаниЯ этих переменных: можно
    "зарЯдить" в даталоггер любой существующий элемент словарЯ.*/

//! см. TDataLog
  struct SDataLog
  {
    type_data buff[(NUMBER_SAMPLES*4)];	//4 массива для отснятых осциллограмм, слепленные в один большой
#if DLOG_DATA_SIZE == 16
    type_data  *dlog_iptr1; 	//!< указатели на переменные для записи (например. ток фазы A, скорость и т.д.)
    type_data  *dlog_iptr2;		//!< Input: Second input pointer (Q15)
    type_data  *dlog_iptr3;
    type_data  *dlog_iptr4;
#endif
    int  dlog_cntr_max;  		//!< Parameter: Maximum number of samples
    Uint16 Wcounter;            //!< Счетчик по массиву длЯ записи
    Uint16 Rcounter;            //!< Счетчик по массиву длЯ чтениЯ
    type_data*  dl_buffer1_adr;  	//!< Указатели на массивы, где лежат отснятые осциллограммы
    type_data*  dl_buffer2_adr;  	//!< Parameter: Buffer starting address 2
    type_data*  dl_buffer3_adr;
    type_data*  dl_buffer4_adr;

    type_data upload_buff[NUMBER_SAMPLES];	//массив для отправки осциллограммы в юникон. Сюда копируется осциллограмма из одного из четырех массивов buff со смещением, чтобы начало осциллограммы было в начале массива.
    type_data*  dl_upload_buffer_adr;		//указатель на массив для отправки.
    Uint32 trig_shift;						//сдвиг первой точки после срабатывания триггера. Используется, чтобы задать, сколько точек предыстории сохранять.
    Uint16 trig_shift_int;					//то же, переведенное в точки
    Uint16 first_point_written;				//первая точка, относящаяся к записываемой осциллограмме с учетом длины предыстории и точки срабатывания триггера
    Uint16 prehistory_length;				//длина записанной предыстории (если даталоггер переинициализировался кнопкой "обновить", она обнуляется)
    Uint16 valid_points_num;				//счетчик валидных точек предыстории, записанных после переинициализации кнопкой "обновить" в юниконе

    /*!  Режимов работы четыре:\n
    0 - стоп\n
    1 - однократнаЯ запись\n
    2 - запись по кругу без остановки\n
    3 - однократнаЯ запись одной переменной во все 4 массива последовательно, длЯ получениЯ длинной осциллограммы.\n
    Устанавливать переменную надо только вызовом функции set_mode! */
    Uint16 mode_reset;             //!< задаетсЯ режим работы
    Uint16 mode_reset_prev;        //!< предыдущий режим работы
    int E;							// флаг энтри

    /*! "control" имеет 32 разрЯда. Первые 16 бит - флаги, последние 16 бит - установка разреживания.\n
      9   8   7   6   5   4   3   2   1   0\n
    | x | x | x | x | x | x | x | x | x | x |\n
      |   |   |   |   |   |   первые 4 бита - какие из 4х переменых интересуют (какие адреса обновлЯть, какие данные записывать)\n
      |   |   |   |   2 бита - желаемый режим даталоггера: 0, 1, 2\n
      |   |   |   бит "данные готовы" - устанавливаетсЯ контроллером\n
      |   |   бит "поддерживается блочная передача"\n
      |   бит "32-битные данные"\n
      резерв */
    long control;                  //!< управление даталоггером через внешние программы

    unsigned int WriteDelimiter;        //!<делитель данных при записи
    unsigned int WriteDelimiterCounter; //!<Счетчик длЯ делителЯ
    long ind_subind1;              //!<записываетсЯ индекс и подындекс длЯ словарЯ
    long ind_subind2;
    long ind_subind3;
    long ind_subind4;
    unsigned long next_value_var;	//!<через эту переменную выдвигаютсЯ записанные значениЯ при считывании:
    								//!<биты 0-15: значение точки
    								//!<биты 16-23: Rcounter (отдаваемая точка)
    								//!<биты 24-25: buff_num
    								//!<бит 26: флаг, что в поле "значение" - старшие 16 бит точки (используется при DLOG_DATA_SIZE = 32)
    								//!<бит 27: флаг, что переменная 16-разрядная (используется при DLOG_DATA_SIZE = 32)
    Uint16 StartBits;
    int buff_num;              	//!<номер массива, из которого идет выдвижение значений в next_value_var
    int ResetAfterStart; 		//!<флаг обнулениЯ счетчика записи при запуске
    Uint16 OneShotOperation;	//!<флаг синхронизации осциллографа только по первому событию (переход в 1 возможен только из 2)
    TCo_OdVars *pco_vars;       //!<адрес структуры драйвера CANopen
    Uint32 sampling_rate;		// частота выборки данных (вызова функции 'update'), Гц
#if DLOG_DATA_SIZE == 32
    int highPartOfValue;		//!<флаг выдачи старшей части
    TObjectInfo object1Info;	//!<информация о записываемом объекте №1
    TObjectInfo object2Info;	//!<информация о записываемом объекте №2
    TObjectInfo object3Info;	//!<информация о записываемом объекте №3
    TObjectInfo object4Info;	//!<информация о записываемом объекте №4
#endif
    void (*update)(volatile struct SDataLog *);			//!<Pointer to update function
    void (*set_mode)(Uint16 mode_reset,volatile struct SDataLog *);	//!<Pointer to set_mode function
    void (*background_analizer)(volatile struct SDataLog *);	//!<Pointer to background_analizer function
  } ;

typedef volatile struct SDataLog TDataLog;



#if defined(CAN_1_ENABLE)
#define COX_VARS co1_vars
#elif defined(CAN_2_ENABLE)
#define COX_VARS co2_vars
#endif

//! Инициализатор по-умолчанию длЯ данного класса
#if DLOG_DATA_SIZE == 16
#define DATALOG_DEFAULTS { \
    .buff = {0}, \
    .dlog_iptr1 = (type_data *)0x00000300, \
    .dlog_iptr2 = (type_data *)0x00000300, \
    .dlog_iptr3 = (type_data *)0x00000300, \
    .dlog_iptr4 = (type_data *)0x00000300, \
    .dlog_cntr_max = NUMBER_SAMPLES, \
    .dl_buffer1_adr = ((type_data *)&dlog.buff), \
    .dl_buffer2_adr = ((type_data *)&dlog.buff+NUMBER_SAMPLES), \
    .dl_buffer3_adr = ((type_data *)&dlog.buff+NUMBER_SAMPLES*2), \
    .dl_buffer4_adr = ((type_data *)&dlog.buff+NUMBER_SAMPLES*3), \
    .upload_buff = {0}, \
    .dl_upload_buffer_adr = ((type_data *)&dlog.upload_buff), \
    .mode_reset = 2, \
    .mode_reset_prev = 2, \
    .OneShotOperation = 1, \
    .pco_vars = &COX_VARS, \
    .sampling_rate = FAST_CALC_FREQ, \
    .update = data_log_update, \
    .set_mode = dlog_set_mode, \
    .background_analizer = dlog_background_analizer \
}
#endif

#if DLOG_DATA_SIZE == 32
#define DATALOG_DEFAULTS { \
    .buff = {0}, \
    .dlog_cntr_max = NUMBER_SAMPLES, \
    .dl_buffer1_adr = ((type_data *)&dlog.buff), \
    .dl_buffer2_adr = ((type_data *)&dlog.buff+NUMBER_SAMPLES), \
    .dl_buffer3_adr = ((type_data *)&dlog.buff+NUMBER_SAMPLES*2), \
    .dl_buffer4_adr = ((type_data *)&dlog.buff+NUMBER_SAMPLES*3), \
    .upload_buff = {0}, \
    .dl_upload_buffer_adr = ((type_data *)&dlog.upload_buff), \
    .mode_reset = 2, \
    .mode_reset_prev = 2, \
    .OneShotOperation = 1, \
    .pco_vars = &COX_VARS, \
    .sampling_rate = FAST_CALC_FREQ, \
    .object1Info = {0}, \
    .object2Info = {0}, \
    .object3Info = {0}, \
    .object4Info = {0}, \
    .update = data_log_update, \
    .set_mode = dlog_set_mode, \
    .background_analizer = dlog_background_analizer \
}
#endif



//! \memberof TDataLog
  void data_log_update(TDataLog *);
//! \memberof TDataLog
  void dlog_set_mode(Uint16 mode_reset,TDataLog *);
//! \memberof TDataLog
  void dlog_background_analizer(TDataLog *);

#ifdef __cplusplus
}
#endif

#endif

/*@}*/


