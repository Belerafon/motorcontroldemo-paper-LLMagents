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

 \file      V_ModBus.c
 \brief     Драйвер ModBus (см. TModBus)
 24-03-2017		Перенос драйвера на К1921ВК01Т НИИЭТ. Сильный рефакторинг.

 13-08-2012		Теперь поле type структуры MB_Record используется как признак записи/чтения
				для не битовых HR/IR-переменных (раньше там был только признак записи).

 24-01-2012     Теперь проще выбирать между SciaRegs, ScibRegs и ScicRegs. Это сделано
				дефайном. Переименована структура RS в MODBUS_RS


 26-02-2011     Исправлены какие-то баги, добавлена возможность отслеживать запись в объекты
				типа "регистр" за счет изменения номера бита в словаре, поддержка большего
				кол-ва процессоров.


  27-11-2009    Сделана доработка драйвера для поддержки broadcast (широковещательных) сообщений. Естесственно,
                что поддерживаются только команды записи (0x5 - write coils и 0x6 - write single register), т.к.
                broadcast сообщения являются неподтверждаемыми сообщениями. Иными словами читать бесполезно, т.к.
                ответа не придет... (Алямкин)

  21-10-2008    Исправлена ошибка переполнения входного регистра - при переполнении выставлялась frame error (из драйвера SYS_VID800)
  29-01-2008    Иправлена ошибка расчета таймаутов
  28-01-2008    Иправлена ошибка в функции пойска нужного элемента словаря

  29-11-2007    Драйвер переделан для работы с МК17.1
                                1.Исправлена ошибка при обнулении битовых регистров
                                2.Исправлен неправильный перебор элементов в словаре
                                3.Добавлена возможность автоматического перезапуска драйвера при большом количестве ошибок на линии

                                Замечание. Особенность реализации драйвера не предполагает наличия регистра № 0х0000 в словаре
                                Для устранения недостатка необходимо в инициализации переделать пойск последних элементов
                                массивов с поля номера регистра на поле адреса переменной регистра.
  16-11-2006    Release Rev 1.0

 \author        Alecksey Anuchin
 \version   v 1.2 24/03/2017

*/

/** \addtogroup V_ModBus */
/*@{*/
#include "V_ModBus.h"
#include "main.h"

//Макросы для переключения RS485 между приемом и передачей
#define MODBUS_RX_RS485 //NT_GPIOC->MASKHIGHBYTE_bit[4].MASKHB = 4
#define MODBUS_TX_RS485 //NT_GPIOC->MASKHIGHBYTE_bit[4].MASKHB = 0
//Макрос для осциллографирования ножек UART (удобно для отладки, когда вообще ничего не работает)
#define MODBUS_GPIO_VALUE  (NT_GPIOD->DATA>>1) & 3
//Частота тактирования блока UART (настраивается в инициализации микроконтроллера)
#define UART_CLOCK_FREQ 25000000


void  ModBus_RS_Init(TModBus *p){
	// Выбрать и разрешить периферийную функцию
	NT_COMMON_REG->GPIOPCTLD_bit.PIN1 = 1;	 //UART_TxD[3]
	NT_COMMON_REG->GPIOPCTLD_bit.PIN2 = 1;	//UART_RxD[3]
	NT_GPIOD->ALTFUNCSET = (1 << 1) + (1 << 2);

	// Изменить периферийную функцию F12 и F13, чтобы они не заблокировали UART3
	NT_COMMON_REG->GPIOPCTLF_bit.PIN12 = 1;
	NT_COMMON_REG->GPIOPCTLF_bit.PIN13 = 1;
	p->MBInternals.NT_UART=NT_UART3;//Используемый в драйвере номер UART (1,2,3)

	p->MBInternals.NT_UART->CR_bit.UARTEN = 1; 		// Разрешить работу UART

	//Текущая реализация драйвера не использует FIFO, обрабатывает данные по одному байту
	//и вызывается в прерывании 10кГц. Поэтому частоты обмена больше 57600 не поддерживаются (будет терять байты)
	if (p->BaudRate>57600)
		p->BaudRate=57600;

	// Настройка частоты в двух регистрах:
	// задание делителя частоты для получения BaudRate
	// Т.к. блок UART тактируется с частотой 25 МГц (так настроено в ините микроконтроллера),
	// то для получения частоты, например, 57600 бит/с необходим
	// делитель 25000000 / (16 * 57600) = 27,126736111111111111111111111111
	// Целая часть I = 27
	// Дробная часть F = (int)( 0.126736111*64 + 0.5 ) = 8
	// Формулы см. в документации
	float DivResFloat=(float)UART_CLOCK_FREQ / (16 * p->BaudRate);
	long DivResInt=(long)DivResFloat;
	long DivResFrac=(int)((DivResFloat-DivResInt)*64 + 0.5);

	p->MBInternals.NT_UART->IBRD_bit.BAUD_DIVINT= DivResInt;
	p->MBInternals.NT_UART->FBRD_bit.BAUD_DIVFRAC = DivResFrac;

	// 8 бит данных, 1 стоп бит, без контроля четности
	p->MBInternals.NT_UART->LCR_H_bit.SPS = 0;		// Нет проверки четности
	p->MBInternals.NT_UART->LCR_H_bit.WLEN = 3;		// Длина посылки 8 бит
	p->MBInternals.NT_UART->LCR_H_bit.FEN = 0;		// Не использовать FIFO
	p->MBInternals.NT_UART->CR_bit.TXE = 1;			// Разрешить приём
	p->MBInternals.NT_UART->CR_bit.RXE = 1;			// Разрешить передачу
	p->MBInternals.NT_UART->LCR_H_bit.STP2 = 0;		// 1 стоп-бит

}



void  ModBus_ResetError(TModBus *p)
{
	// Биты 0-3 - биты ошибок, сбрасываются записью в них "1"
	p->MBInternals.NT_UART->RSR_ECR |= 0x000F;
}

int16 ModBus_CheckDataReady(TModBus *p)
{
	// return 1 - если буфер приёмника не пуст
	return (!p->MBInternals.NT_UART->FR_bit.RXFE);
}

int16  ModBus_CheckTxEmpty(TModBus *p) //Если все отправлено (пуст и основной и теневой регистры)
{
	// return 1 - если буфер передатчика пуст
	return (p->MBInternals.NT_UART->FR_bit.TXFE);
}

int16  ModBus_CheckTxReady(TModBus *p) //В передатчик можно положить следующий символ (буфер свободен)
{
	// "1" - если буфер передатчика пуст
	return (p->MBInternals.NT_UART->FR_bit.TXFE);
}

void  ModBus_Tx(TModBus *p, int16 data)  // Передача данных
{
	Uint8 tmp_data = data;
	p->MBInternals.NT_UART->DR_bit.DATA = tmp_data;
}

//!Инициализация.
//!настройка таймаутов,
//!обработка словаря объектов для последующего быстрого поиска

//! \memberof TModBus
void ModBus_Init(TModBus *p)
{
  Uint16 x;

  ModBus_RS_Init(p);

  p->MBInternals.TimeOut1_5=(int16)(((int32)p->ExecutionFreq*(int32)17)/p->BaudRate +2);       // 17 ~ 11*1.5
  p->MBInternals.TimeOut2_5=(int16)(((int32)p->ExecutionFreq*(int32)28)/p->BaudRate);          // 28 ~ 11*2.5

  p->MBInternals.RecievedCounter=-1;


////// HR
  // поиск размера массива HR
  x=0;
  while (mbodHR[x].index!=0)
    x++;
  p->MBInternals.NumOfHRs=x;

  // поиск среднего элемента массива HR
  p->MBInternals.MiddleOfHRs=x;
  x=0;
  while ((p->MBInternals.MiddleOfHRs>>1)!=0)
  {
    p->MBInternals.MiddleOfHRs>>=1;
    x=x+x+1;
  }
  p->MBInternals.MiddleOfHRs=x;

  // поиск начального инкремента для метода последовательного
  // приближения
  p->MBInternals.InitialStepOfHRs=1;
  while (x!=0)
  {
    x>>=1;
    p->MBInternals.InitialStepOfHRs<<=1;
  }
  p->MBInternals.InitialStepOfHRs>>=1;

////// Coils
  // поиск размера массива C
  x=0;
  while (mbodC[x].index!=0)
    x++;
  p->MBInternals.NumOfCs=x;

  // поиск среднего элемента массива C
  p->MBInternals.MiddleOfCs=x;
  x=0;
  while ((p->MBInternals.MiddleOfCs>>1)!=0)
  {
    p->MBInternals.MiddleOfCs>>=1;
    x=x+x+1;
  }
  p->MBInternals.MiddleOfCs=x;

  // поиск начального инкремента для метода последовательного
  // приближения
  p->MBInternals.InitialStepOfCs=1;
  while (x!=0)
  {
    x>>=1;
    p->MBInternals.InitialStepOfCs<<=1;
  }
  p->MBInternals.InitialStepOfCs>>=1;

// Discrete Inputs
  // поиск размера массива DI
  x=0;
  while (mbodDI[x].index!=0)
    x++;
  p->MBInternals.NumOfDIs=x;

  // поиск среднего элемента массива DI
  p->MBInternals.MiddleOfDIs=x;
  x=0;
  while ((p->MBInternals.MiddleOfDIs>>1)!=0)
  {
    p->MBInternals.MiddleOfDIs>>=1;
    x=x+x+1;
  }
  p->MBInternals.MiddleOfDIs=x;

  // поиск начального инкремента для метода последовательного
  // приближения
  p->MBInternals.InitialStepOfDIs=1;
  while (x!=0)
  {
    x>>=1;
    p->MBInternals.InitialStepOfDIs<<=1;
  }
  p->MBInternals.InitialStepOfDIs>>=1;

////// IR
  // поиск размера массива HR
  x=0;
  while (mbodIR[x].index!=0)
    x++;
  p->MBInternals.NumOfIRs=x;

  // поиск среднего элемента массива HR
  p->MBInternals.MiddleOfIRs=x;
  x=0;
  while ((p->MBInternals.MiddleOfIRs>>1)!=0)
  {
    p->MBInternals.MiddleOfIRs>>=1;
    x=x+x+1;
  }
  p->MBInternals.MiddleOfIRs=x;

  // поиск начального инкремента для метода последовательного
  // приближения
  p->MBInternals.InitialStepOfIRs=1;
  while (x!=0)
  {
    x>>=1;
    p->MBInternals.InitialStepOfIRs<<=1;
  }
  p->MBInternals.InitialStepOfIRs>>=1;


  for (x=0; x<1000; x++)
    ;
}



//!Расчет

//!Фунция обработки посылок ModBus

//! \memberof TModBus
void ModBus_Execute(TModBus *p)
{
  Uint16 x;
  volatile Uint16 RxReg;
  if (ModBus.Enabled == 1){ // Если включен MODBUS
	  p->OfflineCounter+=1;

	  //долго нет собщений по сети?
	  if (p->OfflineCounter>p->OfflineCounterMax)
	  {
		p->isOnline=0;//Мы не в сети
		p->OfflineCounter=p->OfflineCounterMax+1;//чтоб не убежал
	  }
	  else
		p->isOnline=1;//в сети

	  //Контроль ошибок
	  if (p->errorCode)
	  {//Если висит ошибка
		p->error += 1;
		if (p->AutoRestart == 1)//Если разрешен авто сброс ошибок
			ModBus_ResetError(p);
	  }
	  if (p->clear_error == 1)//Если выставлен флаг однократного ручного сброса ошибки
	  {
		ModBus_ResetError(p);
		p->clear_error = 0;
		p->error = 0;
	  }
	  //Текущее значение GPIO для отладки
	  p->GPIOsValue=MODBUS_GPIO_VALUE;


	  if (p->MBInternals.TxState<1)//TxState==0 - передача не идет
		if (ModBus_CheckDataReady(p))//Если есть готовые принятные данные (байт)
		{
		  p->MBInternals.TimeOut=p->MBInternals.TimeOut1_5;//Таймаут равный полтора символа
		  RxReg = p->MBInternals.NT_UART->DR;//Прием очередного символа
		  p->errorCode=(RxReg & 0x0F00)>>8;
		  RxReg = RxReg & 0xFF;
		  p->received_data = RxReg;//Перекладывание принятых данных для отладки
		  p->ReceiveByteCounter++;
		  if (p->MBInternals.InCount==MODBUS_MAX_RX_COUNT)//Принятно слишком много байт
			return;
		  p->MBInternals.In[p->MBInternals.InCount++]=RxReg;//Принятые данные в массив
		  p->MBInternals.RxCRC^=RxReg;//Сразу же расчет контрольной суммы
		  for (RxReg=0; RxReg<8; RxReg++)//Это тоже продолжение расчета контрольной суммы
			if (p->MBInternals.RxCRC&0x01)
			{
			  p->MBInternals.RxCRC>>=1;
			  p->MBInternals.RxCRC^=p->MBInternals.MagicNumber;
			}
			else
			  p->MBInternals.RxCRC>>=1;
		  //больше ничего, если есть принятые данные, не делается
		  return;
		}
	  if (p->MBInternals.TimeOut==0)//Когда таймаут вышел, его обрабатывает условие p->MBInternals.TimeOut==1, а равен нулю он становится после приема
	  {
		p->MBInternals.InCount=0;//Обнуление счетчика массива
		p->MBInternals.RxCRC=0xFFFF;//Инициализация контрольной суммы на прием
		if (ModBus_CheckTxEmpty(p))//Если буфер на отправку пуст
		{
		  MODBUS_RX_RS485;//Переключение RS485 на прием
		  p->MBInternals.TxState=0;//TxState==0 - передача не идет
		  p->MBInternals.TxCRC=0xFFFF;//Инициализация контрольной суммы на передачу
		}
		return;
	  }
	  if (p->MBInternals.TimeOut==1) //; Сюда попадаем, когда межу символами появляется пауза в 1,5 символа
		if (p->MBInternals.InCount!=0) //; Если количество входных данных не равно нулю, то рассматривем, что пришло
		{
		  //поддержка broadcast осуществляется тут (по другому я незнаю как еще  - Алямкин)
		  //далее переход в передачу - нам туда не нужно, потому обрабатываем здесь
		  //обрабатывать имеет смысл только команды на запись (0x5 - write coils и 0x6 - write single register).
		  if (p->MBInternals.In[0] == 0)//Входной массив, байт адреса равен 0 - широковещательный
		  {
			//0x5 - write coils
			if (p->MBInternals.In[1] == 5)
			{
			  x=(p->MBInternals.In[4]<<8)+p->MBInternals.In[5];//Данные
			  //0 - если нашла, 3 - если запрос некорректен,  2 - ошибка в поиске
			  p->MBInternals.temp=ModBus_FindVocAddr(p, mbodC,\
										   p->MBInternals.NumOfCs, 1,\
										   (p->MBInternals.In[2]<<8)+p->MBInternals.In[3],\
										   1,\
										   p->MBInternals.MiddleOfCs, p->MBInternals.InitialStepOfCs);
			  if ((x!=0xFF00)&&(x!=0x0000))//Данные не валидны, для битовой переменной должны быть FF00 или 0000
				p->MBInternals.temp=3;//делается как будто ModBus_FindVocAddr вернула некорректный запрос
			  if (p->MBInternals.temp==0)//если ModBus_FindVocAddr нашла адрес
			  {
				if (p->MBInternals.In[4]==0)   // reset bit
				  *(Uint16 *)(p->MBInternals.Addr->addr)&=(0xFFFF-(1<<p->MBInternals.Addr->type_or_acc));
				else                                                // set bit
				  *(Uint16 *)(p->MBInternals.Addr->addr)|=(1<<p->MBInternals.Addr->type_or_acc);
			  }
			}
			if (p->MBInternals.In[1] == 6)//0x6 - write HR
			{
			  p->MBInternals.temp=ModBus_FindVocAddr(p, mbodHR,\
										   p->MBInternals.NumOfHRs, 16,\
										   (p->MBInternals.In[2]<<8)+p->MBInternals.In[3],\
										   1,\
										   p->MBInternals.MiddleOfHRs, p->MBInternals.InitialStepOfHRs);
			  if (p->MBInternals.temp==0)//если ModBus_FindVocAddr нашла адрес
			  {
				*(Uint16 *)(p->MBInternals.Addr->addr)=(p->MBInternals.In[4]<<8)+p->MBInternals.In[5];
			  }
			}

			//делаем то же самое, что и ниже для правильной отбраковки сообщения и выходим
			p->MBInternals.InCount=0;//обнуление массива принятых байт
			p->MBInternals.TimeOut=0;//обнуление таймаута
			p->MBInternals.RxCRC=0xFFFF;//инициализация контрольной суммы
			return;
		  }

		  if (p->MBInternals.In[0]!=p->RSNodeID) //; Если номер узла не совпал с нашим, то выкидываем посылку.
		  {
			p->MBInternals.InCount=0;
			p->MBInternals.TimeOut=0;
			p->MBInternals.RxCRC=0xFFFF;
			return;
		  }
		  if (p->MBInternals.RxCRC!=0)    //; Если не сошлась контрольная сумма, то выкидываем посылку.
		  {
			p->MBInternals.InCount=0;
			p->MBInternals.TimeOut=0;
			p->MBInternals.RxCRC=0xFFFF;
			return;
		  }
		  if (p->MBInternals.RecievedCounter<0)//!!!                                                             //; Если еще не проинициализирован счетчик между фреймов (-1), то
		  {
			//; инициализируем счетчик и выходим.
			p->MBInternals.RecievedCounter=p->MBInternals.TimeOut2_5;
			return;
		  }
		  if (p->MBInternals.RecievedCounter>0)//!!!                                                                        //; Если счетчик паузы между фреймов еще положительный, то уменьшаем его
		  {
			//; и выходим
			p->MBInternals.RecievedCounter--;
			return;
		  }
		  //; Если счетчик RecievedCounter досчитал до нуля, то можно переключаться на передачу.
		  p->MBInternals.RecievedCounter=-1;
		  p->ReceivePacketCounter++;
		  MODBUS_TX_RS485;//RS485 на передачу
		  p->MBInternals.TxState=1;//Передача
		  p->MBInternals.RxCRC=0xFFFF;
		  p->MBInternals.TimeOut=p->MBInternals.TimeOut2_5;
		  ModBus_Tx(p,p->MBInternals.In[0]);//Передается первый байт - адрес устройства, почему-то взят из приема...
		  p->MBInternals.TxCRC^=p->MBInternals.In[0];//сразу расчет контрольноу суммы на передачу
		  //и это тоже расчет контрольной суммы
		  for (RxReg=0; RxReg<8; RxReg++)
			if (p->MBInternals.TxCRC&0x01)
			{
			  p->MBInternals.TxCRC>>=1;
			  p->MBInternals.TxCRC^=p->MBInternals.MagicNumber;
			}
			else
			  p->MBInternals.TxCRC>>=1;
		  return;
		}
		else//количество принятых данных равно нулю.. почему-то
		{
		  p->MBInternals.TimeOut=0;
		  return;
		}
	  p->MBInternals.TimeOut--;//отсчитывается таймаут
	  if (p->MBInternals.TxState<1)//если TxState==0 - нет передачи, т выход, т.к. ниже - передача.
		return;

	  if (ModBus_CheckTxReady(p)==0)//если передатчик занят
		return;

	  //Инициализация таймаута
	  p->MBInternals.TimeOut=p->MBInternals.TimeOut2_5;

	  // с этого момента включается дискретный автомат
	  // протокола
	  //p->TxState=0;

	  if (p->MBInternals.TxState==1)   //Состояние передачи 1
	  {
		p->OfflineCounter=0;//Счетчик для контроля пересылаемых пакетов ModBus
		p->MBInternals.temp=p->MBInternals.In[1];//Принятый массив, тут номер функции
		if (p->MBInternals.temp==1)    // Функция read Coils
		{
		  p->MBInternals.ToSend=p->MBInternals.temp;//для отправки - номер функции
		  p->MBInternals.temp=ModBus_FindVocAddr(p,mbodC,\
									   p->MBInternals.NumOfCs, 1,\
									   (p->MBInternals.In[2]<<8)+p->MBInternals.In[3],\
									   (p->MBInternals.In[4]<<8)+p->MBInternals.In[5],\
									   p->MBInternals.MiddleOfCs, p->MBInternals.InitialStepOfCs);
		  if (p->MBInternals.temp==0)//в словаре найден элемент
		  {
			p->MBInternals.TxState=10;
		  }
		  else//ошибка, в словаре элемент не найден
		  {
			p->MBInternals.ToSend|=0x80;//добавляется маска ошибки
			p->MBInternals.TxState=p->MBInternals.temp+3;
		  }
		}
		else if (p->MBInternals.temp==2)       // Код функции read DI
		{
		  p->MBInternals.ToSend=p->MBInternals.temp;//для отправки - номер функции
		  p->MBInternals.temp=ModBus_FindVocAddr(p,mbodDI,\
									   p->MBInternals.NumOfDIs, 1,\
									   (p->MBInternals.In[2]<<8)+p->MBInternals.In[3],\
									   (p->MBInternals.In[4]<<8)+p->MBInternals.In[5],\
									   p->MBInternals.MiddleOfDIs, p->MBInternals.InitialStepOfDIs);
		  if (p->MBInternals.temp==0)//в словаре найден элемент
		  {
			p->MBInternals.TxState=10;
		  }
		  else//ошибка, в словаре элемент не найден
		  {
			p->MBInternals.ToSend|=0x80;//добавляется маска ошибки
			p->MBInternals.TxState=p->MBInternals.temp+3;
		  }
		}
		else if (p->MBInternals.temp==3)       // Функция read HR
		{
		  p->MBInternals.ToSend=p->MBInternals.temp;
		  p->MBInternals.temp=ModBus_FindVocAddr(p,mbodHR,\
									   p->MBInternals.NumOfHRs, 16,\
									   (p->MBInternals.In[2]<<8)+p->MBInternals.In[3],\
									   (p->MBInternals.In[4]<<8)+p->MBInternals.In[5],\
									   p->MBInternals.MiddleOfHRs, p->MBInternals.InitialStepOfHRs);
		  if (p->MBInternals.temp==0)//в словаре найден элемент
		  {
			p->MBInternals.TxState=7;
		  }
		  else//ошибка, в словаре элемент не найден
		  {
			p->MBInternals.ToSend|=0x80;//добавляется маска ошибки
			p->MBInternals.TxState=p->MBInternals.temp+3;
		  }
		}
		else if (p->MBInternals.temp==4)       // Функция read IR
		{
		  p->MBInternals.ToSend=p->MBInternals.temp;
		  p->MBInternals.temp=ModBus_FindVocAddr(p,mbodIR,\
									   p->MBInternals.NumOfIRs, 16,\
									   (p->MBInternals.In[2]<<8)+p->MBInternals.In[3],\
									   (p->MBInternals.In[4]<<8)+p->MBInternals.In[5],\
									   p->MBInternals.MiddleOfIRs, p->MBInternals.InitialStepOfIRs);
		  if (p->MBInternals.temp==0)//в словаре найден элемент
		  {
			p->MBInternals.TxState=7;
		  }
		  else//ошибка, в словаре элемент не найден
		  {
			p->MBInternals.ToSend|=0x80;//добавляется маска ошибки
			p->MBInternals.TxState=p->MBInternals.temp+3;
		  }
		}
		else if (p->MBInternals.temp==5)       // Функция write Coil
		{
		  p->MBInternals.ToSend=p->MBInternals.temp;
		  x=(p->MBInternals.In[4]<<8)+p->MBInternals.In[5];//Данные
		  p->MBInternals.temp=ModBus_FindVocAddr(p,mbodC,\
									   p->MBInternals.NumOfCs, 1,\
									   (p->MBInternals.In[2]<<8)+p->MBInternals.In[3],\
									   1,\
									   p->MBInternals.MiddleOfCs, p->MBInternals.InitialStepOfCs);
		  if ((x!=0xFF00)&&(x!=0x0000))//Данные не валидны - можно только 0xFF00 и 0x0000
			p->MBInternals.temp=3;//Как будто бы ModBus_FindVocAddr вернула ошибку
		  if (p->MBInternals.temp==0)//в словаре найден элемент
		  {
			p->MBInternals.TxState=12;
			if (p->MBInternals.In[4]==0) // Операция над словарем - reset bit
			  *(Uint16 *)(p->MBInternals.Addr->addr)&=(0xFFFF-(1<<p->MBInternals.Addr->type_or_acc));
			else                                            // set bit
			  *(Uint16 *)(p->MBInternals.Addr->addr)|=(1<<p->MBInternals.Addr->type_or_acc);
			p->Refresh=1;
		  }
		  else//ошибка, в словаре элемент не найден
		  {
			p->MBInternals.ToSend|=0x80;//добавляется маска ошибки
			p->MBInternals.TxState=p->MBInternals.temp+3;
		  }
		}
		else if (p->MBInternals.temp==6)  // Функция write HR
		{
		  p->MBInternals.ToSend=p->MBInternals.temp;
		  p->MBInternals.temp=ModBus_FindVocAddr(p,mbodHR,\
									   p->MBInternals.NumOfHRs, 16,\
									   (p->MBInternals.In[2]<<8)+p->MBInternals.In[3],\
									   1,\
									   p->MBInternals.MiddleOfHRs, p->MBInternals.InitialStepOfHRs);
		  if (p->MBInternals.temp==0)//в словаре найден элемент
		  {
			p->MBInternals.TxState=12;
			//Операция над словарем - запись регистра
			*(Uint16 *)(p->MBInternals.Addr->addr)=(p->MBInternals.In[4]<<8)+p->MBInternals.In[5];
			p->MBInternals.Addr->type_or_acc = MB_VAR_WRITE;
		  }
		  else//ошибка, в словаре элемент не найден
		  {
			p->MBInternals.ToSend|=0x80;//добавляется маска ошибки
			p->MBInternals.TxState=p->MBInternals.temp+3;
		  }
		}
		else//никакая из поддерживаемых функций
		{
		  p->MBInternals.ToSend=p->MBInternals.temp|0x80;//добавляется маска ошибки
		  p->MBInternals.TxState=2;
		}
	  }
	  else if (p->MBInternals.TxState==12)     // send addr hi for write C
	  {
		p->MBInternals.TxState=13;
		p->MBInternals.ToSend=p->MBInternals.In[2];
	  }
	  else if (p->MBInternals.TxState==13)     // send addr low for write C
	  {
		p->MBInternals.TxState=14;
		p->MBInternals.ToSend=p->MBInternals.In[3];
	  }
	  else if (p->MBInternals.TxState==14)     // send data hi for write C
	  {
		p->MBInternals.TxState=15;
		p->MBInternals.ToSend=p->MBInternals.In[4];
	  }
	  else if (p->MBInternals.TxState==15)     // send data hi for write C
	  {
		p->MBInternals.TxState=3;
		p->MBInternals.ToSend=p->MBInternals.In[5];
	  }
	  else if (p->MBInternals.TxState==10)     // send length read Coils
	  {
		p->MBInternals.TxState=11;
		p->MBInternals.In[5]=p->MBInternals.In[5]+(p->MBInternals.In[4]<<8);
		p->MBInternals.ToSend=(p->MBInternals.In[5]+7)>>3;
	  }
	  else if (p->MBInternals.TxState==11)  // send data read Coils
	  {
		p->MBInternals.ToSend=0;
		for (x=0; x<8; x++)//Можно упаковать 8 битовых переменных за раз
		{
		  if (p->MBInternals.In[5]!=0)//количество запрашиваемых элементов словаря
		  {
			//Отправка элемента словаря
			p->MBInternals.ToSend|=(((*(Uint16 *)(p->MBInternals.Addr->addr))>>(p->MBInternals.Addr->type_or_acc))&0x01)<<x;
			p->MBInternals.Addr++;
			p->MBInternals.In[5]--;
		  }
		  if (p->MBInternals.In[5]==0)//Если все отправили
			p->MBInternals.TxState=3;
		}
	  }
	  else if (p->MBInternals.TxState==7) // send length read HR
	  {
		p->MBInternals.TxState=8;
		p->MBInternals.ToSend=p->MBInternals.In[5]<<1;
	  }
	  else if (p->MBInternals.TxState==8) // send HR high word
	  {
		p->MBInternals.TxState=9;
		//x= *p->MBInternals.Addr->addr;
		p->MBInternals.temp=*(Uint16 *)(p->MBInternals.Addr->addr);

		p->MBInternals.ToSend=p->MBInternals.temp>>8;
	  }
	  else if (p->MBInternals.TxState==9) // send HR low word
	  {
		p->MBInternals.ToSend=p->MBInternals.temp&0xFF;
		p->MBInternals.Addr->type_or_acc = MB_VAR_READ;
		p->MBInternals.Addr+=1;
		p->MBInternals.In[5]--;//уменьшение кол-ва запрошенных переменных во входном массиве
		if (p->MBInternals.In[5]==0)//Если это количество равно нулю (все переменные отправили)
		  p->MBInternals.TxState=3;
		else
		  p->MBInternals.TxState=8;
	  }
	  else if (p->MBInternals.TxState==2)      // no such function
	  {
		p->MBInternals.ToSend=1;
		p->MBInternals.TxState=3;
	  }
	  else if (p->MBInternals.TxState==5)      // send error 2
	  {
		p->MBInternals.ToSend=2;
		p->MBInternals.TxState=3;
	  }
	  else if (p->MBInternals.TxState==6)      // send error 3
	  {
		p->MBInternals.ToSend=3;
		p->MBInternals.TxState=3;
	  }
	  else if (p->MBInternals.TxState==3)      // tx crc low
	  {
		p->MBInternals.ToSend=p->MBInternals.TxCRC&0x00FF;
		p->MBInternals.TxState=4;
	  }
	  else if (p->MBInternals.TxState==4)      // tx crc high
	  {
		p->MBInternals.ToSend=p->MBInternals.TxCRC&0x00FF;
		p->MBInternals.TxState=0;
	  }
	  else
		p->MBInternals.TxState=0;

	  p->MBInternals.TimeOut=p->MBInternals.TimeOut2_5;

	  ModBus_Tx(p,p->MBInternals.ToSend);

	  p->MBInternals.TxCRC^=p->MBInternals.ToSend;
	  for (RxReg=0; RxReg<8; RxReg++)
		if (p->MBInternals.TxCRC&0x01)
		{
		  p->MBInternals.TxCRC>>=1;
		  p->MBInternals.TxCRC^=p->MBInternals.MagicNumber;
		}
		else
		  p->MBInternals.TxCRC>>=1;
	  return;

  }
}

//!Вспомогательная функция поиска


//!функция ищет элемент в таблице и возвращает ссылку на него в p->MBInternals.Addr
//!возвращает:
//!       0 - если нашла
//!       3 - если запрос некорректен
//!       2 - ошибка в пойске

//! \memberof TModBus
int16 ModBus_FindVocAddr(TModBus *p,volatile MB_Record /*const*/ Table[], Uint16 TableSize, int16 Type, Uint16 Index, int16 NumOfIndexs, int16 Mid, int16 Step)
{
  int16 x=0;
  if (Type==16)
  {
    if (NumOfIndexs==0)
      return(3);
    if (NumOfIndexs>0x7D)
      return(3);
  }
  else
  {
    if (NumOfIndexs==0)
      return(3);
    if (NumOfIndexs>0x7D0)
      return(3);
  }
  x+=Mid;
  Step<<=1;
  do
  {
    Step>>=1;
    if (Index==Table[x].index)
    {
      if ((x+NumOfIndexs)>TableSize)
        return(2);
      p->MBInternals.Addr=&Table[x];
      return(0);
    }
    else if (Index<Table[x].index)
      x-=Step;
    else
    {
      x+=Step;
      if (x>=TableSize)
        x=(TableSize-1);
    }
  }
  while (Step!=0);
  return (2);
  ;
}

/*@}*/


