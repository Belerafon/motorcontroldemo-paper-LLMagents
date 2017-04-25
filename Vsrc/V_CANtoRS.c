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

 \file      v_CANtoRS.c
 \brief     Преобразователь посылок CAN в RS и обратно. Работает 
совместно с драйвером CANOpen
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \addtogroup v_CANtoRS
@{
*/
#include "DSP.h"
#include "main.h"



//! \memberof TCANtoRS
void CANtoRS_init(TCANtoRS *p) {

	// Выбрать и разрешить периферийную функцию
	NT_COMMON_REG->GPIOPCTLD_bit.PIN1 = 1;	 //UART_TxD[3]
	NT_COMMON_REG->GPIOPCTLD_bit.PIN2 = 1;	//UART_RxD[3]
	NT_GPIOD->ALTFUNCSET = (1 << 1) + (1 << 2);

	// Изменить периферийную функцию F12 и F13, чтобы они не заблокировали UART3
	NT_COMMON_REG->GPIOPCTLF_bit.PIN12 = 1;
	NT_COMMON_REG->GPIOPCTLF_bit.PIN13 = 1;

	p->NT_UART=NT_UART3;//Используемый в драйвере номер UART (1,2,3)

	p->NT_UART->CR_bit.UARTEN = 1; // Разрешить работу UART

	// Настройка частоты в двух регистрах:
	// задание делителя частоты для получения BaudRate
	// Т.к. блок UART тактируется с частотой 25 МГц (так настроено в ините микроконтроллера),
	// то для получения частоты 57600 бит/с необходим
	// делитель 25000000 / (16 * 57600) = 27,126736111111111111111111111111
	// Целая часть I = 27
	// Дробная часть F = (int)( 0.126736111*64 + 0.5 ) = 8
	// Формулы см. в документации
	p->NT_UART->IBRD_bit.BAUD_DIVINT= 27;
	p->NT_UART->FBRD_bit.BAUD_DIVFRAC = 8;



	p->NT_UART->LCR_H_bit.SPS = 0;		// Нет проверки четности
	p->NT_UART->LCR_H_bit.WLEN = 3;		// Длина посылки 8 бит
	p->NT_UART->LCR_H_bit.FEN = 1;		// Использовать FIFO
	p->NT_UART->CR_bit.TXE = 1;			// Разрешить приём
	p->NT_UART->CR_bit.RXE = 1;			// Разрешить передачу
	p->NT_UART->LCR_H_bit.STP2 = 0;		// 1 стоп-бит

    p->CounterWrongCRC=0;
    p->CounterRes=0;
    p->CounterSended=0;
    p->PacketInWait=0;

}



//! Из заданного для передачи массива функция формирует
//!API пакет, а затем вызывает функцию
//!побайтовой отправки

 //! \memberof TCANtoRS
Uint16 CANtoRS_SendP(Uint16* Data, int16 len, TCANtoRS *p) {
    volatile int16 i;
    Uint16 MyCRC;
    p->buf_out[0]=0x7E;
    for (i=0;i<len;i++)
        p->buf_out[i+1]=Data[i];
    MyCRC=CANtoRS_C_CRC(p,Data,len);
    p->buf_out[len+1]=MyCRC & 0xFF;//первый байт
    p->buf_out[len+2]=(MyCRC>>8) & 0xFF;//второй байт
    p->all_len=1+len+2;//полная длина=сигнатура+данные + контрольная сумма

    i = 0;
    // Если буфер TX не заполнен, то отправляем все байты из буфера
    while ((p->NT_UART->FR_bit.TXFF != 1) && (i < p->all_len)){
    	p->NT_UART->DR = p->buf_out[i];
    	i++;
    }


    return 1;
}



#define CRC_MAGIC_NUMBER 0xA001
//! Функция считает контрольную сумму пакета 
//! и сравнивает с принятой. Если все верно, то возвращает 1
 //! \memberof TCANtoRS
#if defined (__GNUC__)
__attribute__((section(".fastcode")))
#endif
Uint16 CANtoRS_C_CRC(TCANtoRS *p,Uint16 *Data,Uint16 len) { //проверка контрольной суммы принятого API пакета
    int16 i,j;
    Uint16 MyCalcCRC=0xFFFF;

    for (j=0;j<len;j++) {
        MyCalcCRC=MyCalcCRC^(Data[j++]&0xFF);
        for (i=0; i<16; i++)
            if (MyCalcCRC&0x01) {
                MyCalcCRC=(MyCalcCRC>>1);
                MyCalcCRC=MyCalcCRC^CRC_MAGIC_NUMBER;
            } else
                MyCalcCRC=(MyCalcCRC>>1);
    }

    return MyCalcCRC;
}



/*! Функция вызывается, когда пришли какие-то данные,
и обрабатывает их как переданный CAN пакет. Выделяет
идентификатор, длину и прочее. Рез-т помещает в структуру
p->MSG */
 //! \memberof TCANtoRS
void CANtoRS_Analysis(TCANtoRS *p) {
    int16 i,j;
    //посылка данных начинается с 12го байта,
    //до этого служебная информация API пакета
    p->MSG.id=0;
    p->MSG.id=(p->ReadPackData[0] & 7)<<8; //нижние 3 бита сдвигаем наверх;
    p->MSG.id|=p->ReadPackData[1];
    p->MSG.dlc=(p->ReadPackData[0]>>4) & 0xF; //верхние 4 бита сдвигаем вниз; длина посылки
    for (i=2,j=0;i<CANTORS_READ_DATA_MAX_LEN;i++,j++)
        p->MSG.data[j]=p->ReadPackData[i];
    p->callback_go=1;
}





/*! Побайтовый обработчик приходящих по SCI данных.
Работает по принципу дискретного автомата и пытается
найти в посылке API пакет. Сначала ждет заданную сигнатуру,
потом длину и т.п. В конце проверяет контрольную сумму и
вызывает дальнейшие обработчики */
 //! \memberof TCANtoRS
#if defined (__GNUC__)
__attribute__((section(".fastcode")))
#endif
void CANtoRS_Receive(TCANtoRS *p) {
    Uint16 MyReadCRC_u16;
    Uint16 MyCalcCRC;
    Uint32 temp_byte;
    Uint16 repeat;
    for (repeat=0;repeat<7; repeat++) {//обработка не более n байт за один вход в функцию

        switch (p->APIpacketMode) {

        case 0: { //ожидание сигнатуры


            if (p->NT_UART->FR_bit.RXFE)
            	return;
            temp_byte = p->NT_UART->DR_bit.DATA;
            if (temp_byte!=0x7e) {
                p->MessDrop3++;
                return;
            }
            p->ReadPackDataCounter=0;//счетчик данных
            p->APIpacketMode=1; //сигрнатура найдена, режим приема данных
            break;
        };
        case 1: { //ожидание данных(полезная нагрузка) пакета
            //все последующие байты складываем в API буфер

            if (p->NT_UART->FR_bit.RXFE)
            	return;
            temp_byte = p->NT_UART->DR_bit.DATA;
            p->ReadPackData[p->ReadPackDataCounter++]=temp_byte;
            if (p->ReadPackDataCounter>=CANTORS_READ_DATA_MAX_LEN) {
                p->ReadPackDataCounter=0;//счетчик данных
                p->ReadCRCCounter=0;//счетчик контрольной суммы
                p->APIpacketMode=2;  //данные приняты, режим приема и проверси CRC
            }
            break;
        };

        case 2: { //ожидание  CRC
            //все последующие байты складываем в CRC буфер

            if (p->NT_UART->FR_bit.RXFE)
            	return;
            temp_byte = p->NT_UART->DR_bit.DATA;
            p->ReadCRC[p->ReadCRCCounter++]=temp_byte;
            if (p->ReadCRCCounter>=2) {
                p->ReadCRCCounter=0;//счетчик контрольной суммы
                MyReadCRC_u16=(p->ReadCRC[0]&0xFF)+((p->ReadCRC[1]<<8)&0xFF00);//запаковываем двухбайтовую CRC в одну переменную
                MyCalcCRC=CANtoRS_C_CRC(p,p->ReadPackData,CANTORS_READ_DATA_MAX_LEN);
                if (MyCalcCRC!=MyReadCRC_u16) {
                    //ошибка!
                    p->CounterWrongCRC++;
                    p->APIpacketMode=0;
                    break;
                }//контрольная сумма принята и верна

                CANtoRS_Analysis(p);
                if (p->callback_go) { //если выставлен флаг, вызов callback
                    p->callback_go=0;
                    p->APIpacketMode=3;  //обработка callback
                }
                else
                    p->APIpacketMode=0;
                return;
            }
            break;
        };
        case 3: { //обработка callback
            p->CounterRes++;
            p->callback(&co2_vars, &(p->MSG));
            p->APIpacketMode=0;
            break;
        };
        default:
            return;
        }
    }
    return;
}



/*! Должна вызываться каждую секунду и отсылает
CAN пакеты HeartBeat. Если отослать не выходит (передача занята),
то сокращает последующий таймаут и, немного обождав, пытается
отправить заново */
 //! \memberof TCANtoRS
void CANtoRS_HeartBeat(TCANtoRS *p) {
    TZCanMsg MSG;
    int16 i;
    MSG.id=0xE<<7;//heartbeat
    MSG.id|=*p->nodeID;//номер узла
    MSG.dlc=1;//длина
    for (i=0;i<8;MSG.data[i++]=0);//очистка
    MSG.data[0]=5;//данные heartbeat
    if (!CANtoRS_Write(&MSG,p)) { //если не получилось отправить
        p->HeartCounter=(CANTORS_HEART_COUNTER_MAX-3); //вскоре пробуем сделать это еще раз
        p->HeartBeatGo=1;//и если будет событие на отправку SDO, но будет знать, что надо пропустить
    } else {
        p->HeartBeatGo=0;
    }
}


 //! \memberof TCANtoRS
Uint16 CANtoRS_WriteHelper(TZCanMsg* MSG,TCANtoRS *p) {
    if (p->HeartBeatGo) { //хочет отправиться HeartBeat
        CANtoRS_HeartBeat(p);
        p->MessDrop1++;
        return 0;//сообщение теряем...
    }
    if (CANtoRS_Write(MSG,p))
        p->CounterSended++;
    else {
        p->MessDrop2++;

        return 0;
    }
    return 1;
}


/*! Из структуры CAN пакета формирует массив-посылку,
где вначале 4 бита - длина, потом 1 бит пропуск и 11 идентификатор.
Далее непосредственно данные. После конструирования посылка отправляется*/
 //! \memberof TCANtoRS
Uint16 CANtoRS_Write(TZCanMsg* MSG,TCANtoRS *p) {
    if (!p->PacketInWait) { //нет пакета в буфере
        //копируем во временный буфер
        p->bufMSG=*MSG;
        p->PacketInWait=1;//флаг, что в буфере что-то есть
        return 1;
        //считается, что буфера на 1 позицию должно хватать
    } else
        return 0;
}

 //! \memberof TCANtoRS
Uint16 CANtoRS_Write_Real(TZCanMsg* MSG,TCANtoRS *p) {
    int16 i;

    p->TempData[0]=(MSG->dlc & 0xF)<<4;
    p->TempData[0]|=(MSG->id >> 8) & 7;
    p->TempData[1]=MSG->id & 0xFF;
    for (i=0;i<8;i++) p->TempData[i+2]=MSG->data[i];
    CANtoRS_SendP(p->TempData,10,p);
    return 1;
}

/*! Вызывается из миллисекундного таймера, отсчитывает 1 секунду
и вызывает функцию отправки HeartBeat. Заодно проверяет,
если в SCI возникла какая-то ошибка и он остановился, перезапускает.
 */

 //! \memberof TCANtoRS
#if defined (__GNUC__)
__attribute__((section(".fastcode")))
#endif
void CANtoRS_calc(TCANtoRS *p) {

    //если в буфере есть пакет, ожидающий отправки и передатчик свободен и не ждет heartbeat
    if (p->PacketInWait) {
        if (!CANtoRS_Write_Real(&p->bufMSG,p))//отправить не удалось?
            p->MessDrop2++;
        p->PacketInWait=0;//очищаем буфер
    }
    else{//если пакета на отправк нет, обрабатывается всё остальное. Можно бы и обрабатывать сразу, но не хватает ресурсов
      CANtoRS_Receive(p);
      p->HeartCounter++; //счетчик для HeartBeat

      //не пора ли отправить HeartBeat?
      if (p->HeartCounter>=(CANTORS_HEART_COUNTER_MAX-1)) {
          p->HeartCounter=0;
          CANtoRS_HeartBeat(p);
      }

    }
}



/*@}*/
