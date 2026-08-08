/*!
 \file      v_data_log.c
 \brief     4-х канальный логгер длЯ осциллографированиЯ в реальном времени TDataLog (см. TDataLog)

 \author	Коллектив ООО НПФ Вектор
 \version   v 2.01 18/09/2012
 */

/** \addtogroup v_data_log */
/*@{*/

#include "DSP.h"
#include "co_ODvars.h"
#include "V_data_log.h"
#include "CANOpen_drv.h"

//! Фоновый обработчик.

/*! работает в фоне. Нужно длЯ установки адресов переменных через CAN.
 если нет CAN, можно записать нужные адреса вручную в TDataLog.dlog_iptr  */
//! \memberof TDataLog
void dlog_background_analizer(TDataLog *p) {
	//перевод длины предыстории из % в точки
	p->trig_shift_int = (p->trig_shift&0xFFFFFF)>>16;	//делаем количество точек
	if (p->trig_shift_int > 255)	//ограничим
		p->trig_shift_int = 255;

	unsigned char mode;

	//если не стоит хоть один из первых шести бит control,
	//то ничего анализировать и делать не надо
	if (!(p->control & 0x3F))
		return;
	//В любом случае перед всеми изменениЯми
	//тормозим даталоггер. Иначе он может начать записывать данные по прерыванию,
	//пока тут, в фоне, идет, скажем, запись адресов.
	//И тогда в лучшем случае будет каша из данных, в худшем - останов контроллера
	//при обращении по неправильному адресу.
	p->mode_reset = 0;
	p->valid_points_num = 0;	//т.к. даталоггер останавливался (mode = 0), то предыстория неактуальна, т.к.
								//1) за время стопа даталоггера между записанной предысторией и новыми данными будет разрыв по времени и
								//2) выбранные записываемые переменные вообще могли смениться и предыстория останется от старых, а новые данные - от новых
	//спрашиваем адреса у драйвера CANopen
#if DLOG_DATA_SIZE == 16
	if (p->control & 1)
		if (!(p->dlog_iptr1 = (type_data*)co_getAddr(p->pco_vars, p->ind_subind1))) //если адрес=0
			p->dlog_iptr1 = (type_data*)&p->ind_subind1; //пусть указывает на ind_subind
	if (p->control & 2)
		if (!(p->dlog_iptr2 = (type_data*)co_getAddr(p->pco_vars, p->ind_subind2)))
			p->dlog_iptr2 = (type_data*)&p->ind_subind2;
	if (p->control & 4)
		if (!(p->dlog_iptr3 = (type_data*)co_getAddr(p->pco_vars, p->ind_subind3)))
			p->dlog_iptr3 = (type_data*)&p->ind_subind3;
	if (p->control & 8)
		if (!(p->dlog_iptr4 = (type_data*)co_getAddr(p->pco_vars, p->ind_subind4)))
			p->dlog_iptr4 = (type_data*)&p->ind_subind4;
#endif
#if DLOG_DATA_SIZE == 32
	if (p->control & 1)
	{
		if (co_getObjectInfo(p->pco_vars, p->ind_subind1, (TObjectInfo*)&p->object1Info) == 0)
		{
			p->object1Info.varAddr = (void*)(&p->ind_subind1);
			p->object1Info.varSize = 32;
		}
	}
	if (p->control & 2)
	{
		if (co_getObjectInfo(p->pco_vars, p->ind_subind2, (TObjectInfo*)&p->object2Info) == 0)
		{
			p->object2Info.varAddr = (void*)(&p->ind_subind2);
			p->object2Info.varSize = 32;
		}
	}
	if (p->control & 4)
	{
		if (co_getObjectInfo(p->pco_vars, p->ind_subind3, (TObjectInfo*)&p->object3Info) == 0)
		{
			p->object3Info.varAddr = (void*)(&p->ind_subind3);
			p->object3Info.varSize = 32;
		}
	}
	if (p->control & 8)
	{
		if (co_getObjectInfo(p->pco_vars, p->ind_subind4, (TObjectInfo*)&p->object4Info) == 0)
		{
			p->object4Info.varAddr = (void*)(&p->ind_subind4);
			p->object4Info.varSize = 32;
		}
	}
	p->highPartOfValue = 0;
#endif
	p->WriteDelimiter = (p->control >> 16) & 0xFF; //прореживание. ОграничиваетсЯ 8ю битами
	mode = (p->control >> 4) & 3; //режим - 2 бита
	p->control = 0; //все обработали, обнулЯем, чтобы при след. вызове не начать обрабатывать снова
	dlog_set_mode(mode, p); //длЯ установки режима вызываетсЯ спец. функциЯ. НапрЯмую нельзЯ

}

//! функциЯ длЯ установки mode_reset.

//! предполагаетсЯ длЯ управлениЯ логгером из других модулей
//! проверЯет mode_reset на валидность и обнулЯет счетчики
//! \memberof TDataLog

void dlog_set_mode(Uint16 mode_reset, TDataLog *p) {
	if (mode_reset > 3)
		return;
	//в 1 переводим только если текущий режим 2, чтоб синхронизироватьсЯ
	//по первому событию (очень полезно когда событий много)
	if (p->OneShotOperation == 1) {
		if ((mode_reset == 1)) {
			if (p->mode_reset == 2)
				p->mode_reset = mode_reset;
			else
				return;
		}
	} else
		p->mode_reset = mode_reset;

	if (mode_reset == 3)	//Хотим перейти в режим однократной записи 1024 точек.
		p->Wcounter = 0;	//в этом режиме нужно писать с первой точки первого массива. Сбрасываем счетчик.


	//при смене режима в любом случае сбрасываем флаг "данные готовы"
	p->control &= ~(1 << 6);
	p->WriteDelimiterCounter = 0;
#if DLOG_DATA_SIZE == 32
	p->highPartOfValue = 0;
#endif
}

//! Записывает очередные точки в массивы осциллограммы.

//!Должна вызыватьсЯ с заданной дискретизацией и занимаетсЯ записью массивов,
//! т.е. непосредственно осциллографированием. Умеет записывать данные
//! в массивы по-разному в зависимости от выбранного режима работы осциллографа
//! \memberof TDataLog

void data_log_update(TDataLog *p) {
//везде испоьзуетсЯ p-> , а не p* - так быстрее

//прореживание данных. Если не достигли уставки WriteDelimiter, выходим
//WriteDelimiter=1 - не делим
//WriteDelimiter=2 - берем каждую вторую
	if ((p->WriteDelimiterCounter++ + 1) < p->WriteDelimiter)
		return;
	else
		p->WriteDelimiterCounter = 0;

	if (p->mode_reset != p->mode_reset_prev) p->E=1;
    else p->E=0;
	p->mode_reset_prev = p->mode_reset;

	switch (p->mode_reset) {
	case 0: {
		p->Wcounter = 0;
		return;
	}

	case 1:  //однократнаЯ запись - дошли до конца и стоп (режим 0)
	{
		//При входе в режим получаем точку срабатывания триггера (это текущий Wcounter, с которым мы сюда зашли) и рассчитываем первую точку, относящуюся к данной осце (исходя из текущей точки и заданной длины предыстории)
        if (p->E==1)
        {
        	p->prehistory_length = p->valid_points_num;		//длина предыстории равна количеству валидных точек
        	if (p->prehistory_length > p->trig_shift_int)	//если предыстория длиннее заданного, ограничим
        		p->prehistory_length = p->trig_shift_int;
#if DLOG_DATA_SIZE == 16
        	p->Wcounter &= 0xFE;
        	p->prehistory_length &= 0xFE;
        	p->first_point_written = (p->Wcounter - p->prehistory_length) & 0xFE;
#endif
#if DLOG_DATA_SIZE == 32
        	p->first_point_written = (p->Wcounter - p->prehistory_length) & 0xFF;
        	p->highPartOfValue = 0;
#endif
        }
		p->Wcounter &= 0xFF; //защита - если вдруг в counter не валидное значение
#if DLOG_DATA_SIZE == 16
		p->dl_buffer1_adr[p->Wcounter] = *p->dlog_iptr1;
		p->dl_buffer2_adr[p->Wcounter] = *p->dlog_iptr2;
		p->dl_buffer3_adr[p->Wcounter] = *p->dlog_iptr3;
		p->dl_buffer4_adr[p->Wcounter] = *p->dlog_iptr4;
#endif
#if DLOG_DATA_SIZE == 32
		p->dl_buffer1_adr[p->Wcounter] = p->object1Info.varSize == 16 ?
			*((int16*)p->object1Info.varAddr) : *((int32*)p->object1Info.varAddr);
		p->dl_buffer2_adr[p->Wcounter] = p->object2Info.varSize == 16 ?
			*((int16*)p->object2Info.varAddr) : *((int32*)p->object2Info.varAddr);
		p->dl_buffer3_adr[p->Wcounter] = p->object3Info.varSize == 16 ?
			*((int16*)p->object3Info.varAddr) : *((int32*)p->object3Info.varAddr);
		p->dl_buffer4_adr[p->Wcounter] = p->object4Info.varSize == 16 ?
			*((int16*)p->object4Info.varAddr) : *((int32*)p->object4Info.varAddr);
#endif
		p->Wcounter++;
		p->Wcounter &= 0xFF;  //если прошли 256 точек, обнулитсЯ
		if (p->Wcounter == p->first_point_written)  //если дошли до последней записываемой точки
		{
			p->mode_reset = 0;                 //режим СТОП

	#if DLOG_DATA_SIZE == 16
			p->control |= 192;				// "данные готовы" + "поддерживается блочная передача"
	#else
			p->control |= 192 | (1 << 8);	// "данные готовы" + "поддерживается блочная передача" + "32-битные данные"
	#endif

		}
		return;
	}

	case 2: //записываем лог по кругу
	{
		if (p->E == 1)
		{
#if DLOG_DATA_SIZE == 32
        	p->highPartOfValue = 0;
#endif
		}
		p->valid_points_num++;	//считаем количество записанных валидных точек предыстории
		if (p->valid_points_num > 256)	//ограничиваем
			p->valid_points_num = 256;
		p->Wcounter &= 0xFF; //защита - если вдруг в counter не валидное значение
#if DLOG_DATA_SIZE == 16
		p->dl_buffer1_adr[p->Wcounter] = *p->dlog_iptr1;
		p->dl_buffer2_adr[p->Wcounter] = *p->dlog_iptr2;
		p->dl_buffer3_adr[p->Wcounter] = *p->dlog_iptr3;
		p->dl_buffer4_adr[p->Wcounter] = *p->dlog_iptr4;
#endif
#if DLOG_DATA_SIZE == 32
		p->dl_buffer1_adr[p->Wcounter] = p->object1Info.varSize == 16 ?
			*((int16*)p->object1Info.varAddr) : *((int32*)p->object1Info.varAddr);
		p->dl_buffer2_adr[p->Wcounter] = p->object2Info.varSize == 16 ?
			*((int16*)p->object2Info.varAddr) : *((int32*)p->object2Info.varAddr);
		p->dl_buffer3_adr[p->Wcounter] = p->object3Info.varSize == 16 ?
			*((int16*)p->object3Info.varAddr) : *((int32*)p->object3Info.varAddr);
		p->dl_buffer4_adr[p->Wcounter] = p->object4Info.varSize == 16 ?
			*((int16*)p->object4Info.varAddr) : *((int32*)p->object4Info.varAddr);
#endif
		p->Wcounter++;
		p->Wcounter &= 0xFF;  //если прошли 256 точек, обнулитсЯ
		return;
	}

	case 3: //режим однократной записи 1024 точек
	{
        if (p->E==1)
        {
        	p->first_point_written = 0;		//пишем с первой точки, предыстория не нужна.
#if DLOG_DATA_SIZE == 32
        	p->highPartOfValue = 0;
#endif
        }
		//с учетом того, что буферы в памЯти расположены последовательно,
		//записываем в первый, "заезжаЯ" на остальные три
#if DLOG_DATA_SIZE == 16
		p->dl_buffer1_adr[p->Wcounter] = *p->dlog_iptr1;
#endif
#if DLOG_DATA_SIZE == 32
		p->dl_buffer1_adr[p->Wcounter] = p->object1Info.varSize == 16 ?
			*((int16*)p->object1Info.varAddr) : *((int32*)p->object1Info.varAddr);
#endif
		p->Wcounter++;
		if (p->Wcounter >= 1024)  //если дошли до конца
		{
			p->mode_reset = 0;                 //режим СТОП

	#if DLOG_DATA_SIZE == 16
			p->control |= 192;				// "данные готовы" + "поддерживается блочная передача"
	#else
			p->control |= 192 | (1 << 8);	// "данные готовы" + "поддерживается блочная передача" + "32-битные данные"
	#endif

		}
		return;
	}
	}
}

/*@}*/

