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

 \file      SMProtect.c
 \brief     Модуль защит.   (см. TSM_Protect)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \addtogroup SMProtect
 @{ */

#include "DSP.h"
#include "main.h"
#include "stdlib.h"

int16 WriteCounter = 0;

//!Инициализация

//!Присваивание всяких переменных
//! \memberof TSM_Protect
void SM_Protect_Init(TSM_Protect *p) {
	p->state_prev = 0xff;
	p->state = 0x00;

#if defined (HW_MOTORCONTROLBOARD)
/*
 * 		Инициализация кнопки СБРОС АВАРИЙ не требуется, так как мы используем для этого дискретный вход
 */
#endif

}

//! Быстрый расчет.

//!Обрабатывает все аварии и при их возникновении останавливает привод.
//! \memberof TSM_Protect
void SM_Protect_Fast_Calc(TSM_Protect *p) {

	//аппаратная авария от инвертора
	if (pwm.PDP_Fault) {
		if ((sm_ctrl.state != CTRL_STOP) && (sm_ctrl.state != CTRL_RUN))
			sm_prot.bit_fault1 |= F_PDPINT;
	}

	if (!DRV_FAULT) {
		//имеется какая-то аппаратная авария
		sm_prot.bit_fault1 |= F_PDPINT;
	}

#if defined (HW_MOTORCONTROLBOARD)
	if (!DRV_FAULT1) {
		//имеется какая-то аппаратная авария
		sm_prot.bit_fault2 |= F_PDP_SWITCH_A0;
	}

	if (!DRV_FAULT2) {
		//имеется какая-то аппаратная авария
		sm_prot.bit_fault2 |= F_PDP_SWITCH_A1;
	}

	if (!DRV_FAULT3) {
		//имеется какая-то аппаратная авария
		sm_prot.bit_fault2 |= F_PDP_SWITCH_A2;
	}

	if (!DRV_FAULT4) {
		//имеется какая-то аппаратная авария
		sm_prot.bit_fault2 |= F_PDP_SWITCH_LOWER;
	}
#endif

	/*Защита по максимальному току*/
	if (adc.Imeas_a > sm_prot.Imax_protect)
		sm_prot.bit_fault2 |= F_CTRL_MAX_I_PH_A;
	if (adc.Imeas_a < -sm_prot.Imax_protect)
		sm_prot.bit_fault2 |= F_CTRL_MAX_I_PH_A;
	if (adc.Imeas_b > sm_prot.Imax_protect)
		sm_prot.bit_fault2 |= F_CTRL_MAX_I_PH_B;
	if (adc.Imeas_b < -sm_prot.Imax_protect)
		sm_prot.bit_fault2 |= F_CTRL_MAX_I_PH_B;
	if (adc.Imeas_c > sm_prot.Imax_protect)
		sm_prot.bit_fault2 |= F_CTRL_MAX_I_PH_C;
	if (adc.Imeas_c < -sm_prot.Imax_protect)
		sm_prot.bit_fault2 |= F_CTRL_MAX_I_PH_C;

	//при превышении скорости выше аварийной нормы 
	if (labs(cur_par.speed) > sm_prot.speed_max) {
		sm_prot.bit_fault2 |= F_CTRL_SPEED_MAX; // максиммальное Ud
	}

	//при превышении напряжения выше аварийной нормы 
	if (adc.Udc_meas > sm_prot.Umax_protect) {
		sm_prot.bit_fault1 |= F_CTRL_HI_UDC; // максиммальное Ud
	}

	//снижение напряжения
	if (adc.Udc_meas < sm_prot.Umin_protect) { //если привод работал то ловим аварию
		if (sm_ctrl.state != CTRL_STOP) {
			sm_prot.bit_fault1 |= F_CTRL_LOW_UDC;
		}
	}

	//при превышении температуры выше аварийной нормы
	if (adc.T_meas > sm_prot.T_max) {
		sm_prot.bit_fault1 |= F_CTRL_HI_TEMP; // максимальная температура
	}

#if defined(HW_VECTORCARD_SIMULATOR) || defined(HW_NIIET_BOARD_SIMULATOR) || defined(HW_MOTORCONTROLBOARD_SIMULATOR)
	if(model.fault){
		sm_prot.bit_fault1 |= F_MODEL_FAULT;
	}
#endif


	DINT; //Запрещение прерываний
		  //Аварии, требующие полного останова
	p->masked_bit_fault1 = p->bit_fault1 & p->mask_fault1;//маскирование флагов аварий
	p->masked_bit_fault2 = p->bit_fault2 & p->mask_fault2;
	EINT;      //Разрешение прерываний


	//считаем дискретный автомат защит
	if (p->state_prev != p->state) //смена состояния?
		p->E = 1;//Возводим флаг первого вхождения "entry"
	else
		p->E = 0;//иначе сбрасываем
	p->state_prev = p->state;

	switch (p->state) {//в зависимости от текущего состояния
	case PROT_OFF: { //Защита выключена
		if (p->E == 1) { //Первое вхождение
		}

		//Обнуляем все аварии
		p->bit_fault1 = 0;
		p->bit_fault2 = 0;

		//Пропускаем некоторый таймаут после включения контроллера
		//чтобы не ловить ложные срабатывания аварий АЦП
		if (p->powered_okCounter++ > 5000) {
			p->state = PROT_ON_OK;
		}
		break;
	}

	case PROT_ON_OK: {  //Норма
		if (p->E == 1) {        //Первое вхождение
			drv_status.bit.fault = 0;
		}
		//есть аварии?
		if ((p->masked_bit_fault1 | p->masked_bit_fault2) != 0) {
			p->state = PROT_FAIL; //переходим в состояние аварии
		}
		break;
	}

	case PROT_FAIL: { //состояние аварии (сработала защита)
		if (p->E == 1) {
			cmd.all = 0;
		}
		drv_status.bit.fault = 1;
		//Выключение ШИМ. Пока там все остальные дискретные автоматы пробурлятся, чтобы их не ждать
		pwm.Off(&pwm);
		//логика сбороса аварии
		if ((cmd.bit.trip_reset == 1) || (RESET_FAULT_BUTTON == 1)) {//команда на сброс аварии
			p->state = PROT_ON_OK;//идем в "норма"
			p->clearDrvFault = 1;	//отправить драйверу ключей команду сброса
			//обнуляем все флаги аварий
			p->bit_fault1 = 0;
			p->bit_fault2 = 0;
			cmd.all = 0;//командное слово
			//в самом низу!!! пытаемся сбросить флаг pdp аппаратный
			//если авария все еще имеется, то сразу произойдет прерывание
			//и возведется флаг аварии
			if (NT_PWM0->TZFLG_bit.OST == 1) {  //имеется флаг аппаратной аварии
												//флаг можно сбрасывать
				NT_PWM0->TZCLR = 0x7;
				NT_PWM1->TZCLR = 0x7;
				NT_PWM2->TZCLR = 0x7;
			}

			#if defined(HW_VECTORCARD_SIMULATOR) || defined(HW_NIIET_BOARD_SIMULATOR) || defined(HW_MOTORCONTROLBOARD_SIMULATOR)
				model.fault=0;
			#endif

		}
		break;
	}
	}
}

//! \memberof TSM_Protect
void SM_Protect_ms_Calc(TSM_Protect *p) {
	if (WriteCounter <= 15)   //если первое слово
	{
		if (((sm_prot.masked_bit_fault1 >> WriteCounter) & 0x1) != 0)  //стоит ii-й флаг аварии
		{
			if (((sm_prot.bit_fault_written1 >> WriteCounter) & 0x1) == 0)  //и она не записана
			{
				FaultLog.write(&FaultLog, WriteCounter + 1);
				sm_prot.bit_fault_written1 |= (1 << WriteCounter);
			}
		}
		else
			sm_prot.bit_fault_written1 &= ~(1 << WriteCounter);
	}
	else if (WriteCounter <= 31) //второе слово
	{
		if (((sm_prot.masked_bit_fault2 >> (WriteCounter - 16)) & 0x1) != 0)  //стоит ii-й флаг аварии
		{
			if (((sm_prot.bit_fault_written2 >> (WriteCounter - 16)) & 0x1) == 0)  //и она только что поЯвилась
			{
				FaultLog.write(&FaultLog, WriteCounter + 1);
				sm_prot.bit_fault_written2 |= (1 << (WriteCounter - 16));
			}
		}
		else
			sm_prot.bit_fault_written2 &= ~(1 << (WriteCounter - 16));
	}
	WriteCounter++;
	if (WriteCounter >= (33 - 1))
		WriteCounter = 0;

	// Проверка восстановления параметров CANopen из EEPROM
	if( (co1_vars.ParamRestorationError) || (co2_vars.ParamRestorationError) )
		p->bit_fault1 |= F_CAN_RESTORATION_ERROR;
}

//! Медленный расчет.

//! \memberof TSM_Protect
void SM_Protect_Slow_Calc(TSM_Protect *p) {
	/* Проверка аппаратных защит драйвера. Он автоматически вырубает ключи при авариях,
	 * и опускает сигналы /FAULT и /OCTW, но чтобы понять, что именно случилось, надо по SPI прочитать статусы.
	 * Поэтому, видимо, можно в фоне.
	 */
	if (p->readDrvSts == 1) {
		p->readDrvSts = 0;
		drv8301.read(&drv8301);

	}

	/* Чтобы сбросить аварию, надо прописать ему битик GATE_RESET */
	if (p->clearDrvFault == 1) {
		p->clearDrvFault = 0;
		drv8301.CTRL1.bit.GATE_RESET = 1;
		drv8301.write(&drv8301, CNTRL_REG_1_ADDR);
		drv8301.write(&drv8301, CNTRL_REG_2_ADDR);
	}
}

/*@}*/
