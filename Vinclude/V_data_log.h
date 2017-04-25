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

 \file      v_data_log.h
 \brief     4-х канальный логгер для осциллографирования в реальном времени (см. TDataLog)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \defgroup  v_data_log Цифровой осциллограф (см. TDataLog)

 @{
*/


#ifndef V_DATA_LOG_H
#define V_DATA_LOG_H

#ifdef __cplusplus
extern "C"
{
#endif

//! change "int" to "long" or "float" according to variable type
  typedef int16 type_data;

//! Размер массивов для осциллографирования.

//!Если нужно менять, то это делать надо не только тут - слишком много завязано на это число
//!Переделать сложно, так как на это число завязка косвенная: например, для ускорения работы
//!применяются такие конструкции: p->Wcounter&=0xFF; Где FF - маска (символизирует 256 точек)
#define NUMBER_SAMPLES  256

  /*! \class TDataLog
    \brief 4-х канальный логгер для осциллографирования в реальном времени

    Класс \a TDataLog, основанный на структуре SDataLog, позволяет записывать выбранные 4 переменных в массивы
    из 255 точек с заданной дискретизацией по времени. Кроме того, имеет
    связь с драйвером CANOpen для автоматизации указания этих переменных: можно
    "зарядить" в даталоггер любой существующий элемент словаря.*/

//! см. TDataLog
  struct SDataLog
  {
	type_data buff[(NUMBER_SAMPLES*4)];	//4 массива для отснятых осциллограмм, слепленные в один большой
	type_data  *dlog_iptr1; 	//!< указатели на переменные для записи (например. ток фазы A, скорость и т.д.)
	type_data  *dlog_iptr2;		//!< Input: Second input pointer (Q15)
    type_data  *dlog_iptr3;
    type_data  *dlog_iptr4;
    int16  dlog_cntr_max;  		//!< Parameter: Maximum number of samples
    Uint16 Wcounter;            //!< Счетчик по массиву для записи
    Uint16 Rcounter;            //!< Счетчик по массиву для чтения
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
    1 - однократная запись\n
    2 - запись по кругу без остановки\n
    3 - однократная запись одной переменной во все 4 массива последовательно, для получения длинной осциллограммы.\n
    Устанавливать переменную надо только вызовом функции set_mode! */
    Uint16 mode_reset;             //!< задается режим работы
    Uint16 mode_reset_prev;        //!< предыдущий режим работы
	int E;							// флаг энтри

    /*! "control" имеет 32 разряда. Первые 8 бит первого слова отведем под флаги, второе слово под установку разреживания.\n
         7   6   5   4   3   2   1   0\n
       | x | x | x | x | x | x | x | x |\n
         |   |   |   |   первые 4 бита - какие из 4х переменых интересуют (какие сдреса обновлять, какие данные записывать)\n
         |   |   2 бита - желаемый режим даталоггера. Ноль, один, два.\n
         |   бит "данные готовы" - устанавливается контроллером\n
         резерв */
    long control;                  //!< управление даталоггером через внешние программы

    Uint16 WriteDelimiter;        //!<делитель данных при записи
    Uint16 WriteDelimiterCounter; //!<Счетчик для делителя
    long ind_subind1;              //!<записывается индекс и подындекс для словаря
    long ind_subind2;
    long ind_subind3;
    long ind_subind4;
    unsigned long next_value_var;       //!<через эту переменную выдвигаются записанные значения при считывании
    Uint16 StartBits;	//!< По каким событиям запускать осциллограф. Один выставленный бит - одно событие
    int16 buff_num;              //!<номер массива, из которого идет выдвижение значений в next_value_var
    Uint16 OneShotOperation;	//!<флаг синхронизации осциллографа только по первому событию (переход в 1 возможен только из 2)
    Uint16 BlockTransferSupportFlag;
    void (*update)(volatile struct SDataLog *);			//!<Pointer to update function
    void (*set_mode)(Uint16 mode_reset,volatile struct SDataLog *);	//!<Pointer to set_mode function
    void (*background_analizer)(volatile struct SDataLog *);	//!<Pointer to background_analizer function
  } ;

  typedef volatile struct SDataLog TDataLog;





//! Инициализатор по-умолчанию для данного класса
#define DATALOG_DEFAULTS {  {0},\
    (type_data *)0x00000300, \
    (type_data *)0x00000300, \
    (type_data *)0x00000300, \
    (type_data *)0x00000300, \
    NUMBER_SAMPLES, \
    0,0, \
    ((type_data *)&dlog.buff), \
    ((type_data *)&dlog.buff+NUMBER_SAMPLES), \
    ((type_data *)&dlog.buff+NUMBER_SAMPLES*2), \
    ((type_data *)&dlog.buff+NUMBER_SAMPLES*3), \
    {0}, ((type_data *)&dlog.upload_buff), 0, 0, 0, 0, 0,\
    2, 2, 0,\
    0,\
    0,0,\
    0,0,0,0,\
    0,0,\
    0,\
    0,\
	0,\
    data_log_update,\
    dlog_set_mode,\
    dlog_background_analizer }


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


