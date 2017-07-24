/* ==================================================================================
//File name:       W_event_log.h
//
//Originator:	Aliamkin Dmitriy I
//
//Description:   —охранение событий и времени возникновения в Ёнќ«”
//ѕ–»ћ≈„јЌ»≈:    »спользуется драйвер работы с энергонезависимой памятью по I2C
//				 ѕоэтому вызов init(); должен быть после инициализации I2C
//=====================================================================================
//-------------------------------------------------------------------------------------
// 20.01.2009	Release	Rev 1.0
//----------------------------------------------------------------------------------*/
#ifndef W_EVENT_LOG_H
#define W_EVENT_LOG_H

#ifdef __cplusplus
extern "C"
{
#endif

//#define LOG_BUF_SIZE 50  	//сохраняем последние 50 событий
#define LOG_FIFO_SIZE 10	//размер фифо для буферирования сообщений прежде чем они будут записаны в SPI
//#define LOG_SPI_START_ADDR 3000						//начальный адрес в SPI

#define LOG_NUM_LENGTH 2		//длина номера события (16бит) - 2байта
#define LOG_TIME_LENGTH 4		//длина времени события (32бит) - 4байта
#define LOG_CODE_LENGTH 2		//длина кода события (16бит) - 2байта
#define LOG_LENGTH LOG_NUM_LENGTH + LOG_TIME_LENGTH + LOG_CODE_LENGTH	//общая длина записи ошибки

#define EVENT_FIFO_SUCCESSFUL 0
#define EVENT_FIFO_EMPTY 1
#define EVENT_FIFO_FULL 2
#define EVENT_FIFO_BUSY 3

typedef struct
{
    Uint16 ev_num;
    Uint32 ev_time;
    Uint16 ev_code;

}TEvent;

struct SEventFIFO
{
    Uint16 busy_flag;
    Uint16 size;
    Uint16 number_of_msgs;
    Uint16 read_ptr;
    Uint16 write_ptr;
    TEvent msg_array[LOG_FIFO_SIZE];
    Uint16(*read)(struct SEventFIFO*,TEvent*);
    Uint16(*write)(struct SEventFIFO*,TEvent*);
};

typedef struct SEventFIFO TEventFIFO;

#define EVENT_FIFO_DEFAULTS {0,\
    LOG_FIFO_SIZE,\
    0,0,0,\
    {0},\
    Event_FIFO_Read,\
    Event_FIFO_Write,\
}

Uint16 Event_FIFO_Read(TEventFIFO*,TEvent*);
Uint16 Event_FIFO_Write(TEventFIFO*,TEvent*);

struct SLogger
{
    Uint16 INTERNAL_last_log_index;
    Uint16 INTERNAL_last_log_num;
    Uint16 INTERNAL_last_log_code;
    Uint16 SPI_buf_size;
    Uint16 SPI_start_addr;

    Uint32* time_ptr;

    Uint16 bit_fault_written[4];	//флаги записанных аварий
    Uint16 WriteCounter;		//счетчик битов дл€ записи аварий в банк
    Uint16 mainDCDCwritten;
    Uint16 DCDC3phwritten;
    Uint16 mcwritten;
    Uint16 chargerwritten;

    TEventFIFO FIFO;
    void (*init)(volatile struct SLogger*,Uint16 BS,Uint16 SA,Uint32* tptr);
    void (*background_calc)(volatile struct SLogger*);
    void (*ms_calc)(volatile struct SLogger*);
    void (*clear)(volatile struct SLogger*);
    Uint16(*write)(volatile struct SLogger*,Uint16);
    void (*read)(volatile struct SLogger*,Uint16,TEvent*);
};

typedef volatile struct SLogger TLogger;

//т.к. часов реального времени нет (ћ  30.1), в качестве временнќго маркера аварий используетс€ врем€ включенного состо€ни€
#define LOGGER_DEFAULTS {0,0,0,1,0,\
	(Uint32*)&RTCclock.packed_time,\
	{0,0,0,0},0,0,0,0,0,\
    EVENT_FIFO_DEFAULTS,\
    Logger_Init,\
    Logger_Background_Calc,\
    Logger_ms_Calc,\
    Logger_Clear,\
    Logger_Event_Write,\
    Logger_Event_Read,\
}

void Logger_Init(TLogger*,Uint16 BS,Uint16 SA,Uint32* tptr);
void Logger_Background_Calc(TLogger*);
void Logger_ms_Calc(TLogger*);
void Logger_Clear(TLogger*);
Uint16 Logger_Event_Write(TLogger*,Uint16);
void Logger_Event_Read(TLogger* p,Uint16 shift_index,TEvent* event);



#ifdef __cplusplus
}
#endif

#endif
