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
 
 \file      V_PWM_Module.c
 \brief     Модуль реализации векторной ШИМ (см. TPWM_Module)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \addtogroup V_PWM_Module 
 @{*/


#include "V_IQmath.h"
#include "V_PWM_Module.h"
#include "main.h"


#ifdef HW_VECTORCARD_DRV8301_EVM
#define GPIO_INV_ON  NT_GPIOG->MASKHIGHBYTE_bit[(1<<(8-8))].MASKHB = (1<<(8-8))
//Для данной конфигурации выключение инвертора аппаратное не используется, выключение ключей осуществляется выводами упралвения ключами
#define GPIO_INV_OFF NT_GPIOG->MASKHIGHBYTE_bit[(1<<(8-8))].MASKHB = (1<<(8-8))
#endif

#ifdef HW_VECTORCARD_DRV8312_EVM
//A[10], F[2], F[4]
#define GPIO_INV_ON  NT_GPIOA->MASKHIGHBYTE_bit[(1<<(10-8))].MASKHB = (1<<(10-8)); NT_GPIOF->MASKLOWBYTE_bit[(1<<2)].MASKLB = (1<<2);NT_GPIOF->MASKLOWBYTE_bit[(1<<4)].MASKLB = (1<<4)
#define GPIO_INV_OFF NT_GPIOA->MASKHIGHBYTE_bit[(1<<(10-8))].MASKHB = 0;NT_GPIOF->MASKLOWBYTE_bit[(1<<2)].MASKLB = 0;NT_GPIOF->MASKLOWBYTE_bit[(1<<4)].MASKLB = 0
#endif

#if defined(HW_VECTORCARD_SIMULATOR) || defined(HW_NIIET_BOARD_SIMULATOR) || defined (HW_MOTORCONTROLBOARD_SIMULATOR)
#define GPIO_INV_ON
#define GPIO_INV_OFF
#endif

#if defined(HW_VECTOR_MK_40_4)
#define GPIO_INV_ON
#define GPIO_INV_OFF
#endif

#if defined(HW_MOTORCONTROLBOARD)
#define GPIO_INV_ON
#define GPIO_INV_OFF
#endif

//! Ограничения величины "мёртвого времени"
#if defined(HW_MOTORCONTROLBOARD)
//! для одноплатного инвертора нельзя меньше 3мкс, чтобы не сжечь
#define DEAD_BAND_MIN	_IQ(3)
#define DEAD_BAND_MAX	_IQ(10)
#else
#define DEAD_BAND_MIN	_IQ(0)
#define DEAD_BAND_MAX	_IQ(10)
#endif

//! Инициализация

//! \memberof TPWM_Module
void PWM_Module_Init(TPWM_Module *p) {
	NT_COMMON_REG->PWM_SYNC_bit.TBCLKSYNC = 0; //синхронизация таймеров - сброс внутренних счетчиков
	/* Синхронизировать ШИМы */
	NT_COMMON_REG->PWM_CTRL = 0x8;

// ------------------------------------------------------------------------
	// Настраиваем модуль ePWM1
	// ------------------------------------------------------------------------
	if (p->Frequency < PWM_FREQ_MIN)
		p->Frequency = PWM_FREQ_MIN;
	if (p->Frequency > PWM_FREQ_MAX)
		p->Frequency = PWM_FREQ_MAX;
	NT_PWM0->TBPRD = _IQ10div(_IQ10(CORE_CLK/1000.0), p->Frequency << 1) >> 10; //период

	p->k_pwm = NT_PWM0->TBPRD;
	p->FreqPrev = p->Frequency;                 //предыдущая частота
	NT_PWM0->TBPHS_bit.TBPHS = 0x0000;            // Phase is 0
	NT_PWM0->TBCTR = 0x0000;                       // Clear counter

	// Setup counter mode
	NT_PWM0->TBCTL_bit.PRDLD = TB_SHADOW;        // загрузка TBPRD при TBCTR = 0
	NT_PWM0->TBCTL_bit.CTRMODE = TB_COUNT_UPDOWN;  // Count up-down
	NT_PWM0->TBCTL_bit.PHSEN = TB_DISABLE;         // Disable phase loading
	NT_PWM0->TBCTL_bit.PHSDIR = TB_UP;      // Считать вверх после загрузки фазы
	NT_PWM0->TBCTL_bit.HSPCLKDIV = 0;     // High Speed Time-base Clock Prescale
	NT_PWM0->TBCTL_bit.CLKDIV = 0;                 // Time-base Clock Prescale
	NT_PWM0->TBCTL_bit.SYNCOSEL = TB_CTR_ZERO; // выдаём синхро-сигнал при TBCTR = 0

	// Setup shadowing
	NT_PWM0->CMPCTL_bit.SHDWAMODE = CC_SHADOW;   //включить SHADOW для сравнения
	NT_PWM0->CMPCTL_bit.LOADAMODE = CC_CTR_ZERO;   // Load on zero

	NT_PWM0->CMPCTL_bit.SHDWBMODE = CC_SHADOW;   //включить SHADOW для сравнения
	NT_PWM0->CMPCTL_bit.LOADBMODE = CC_CTR_ZERO;   // Load on zero

	// Set Compare values
	NT_PWM0->CMPA_bit.CMPA = 0;                   // Set compare A value

	// Set actions
	NT_PWM0->AQCTLA = AQ_EPWM_DISABLE; // для начала события для PWM1A запрещены
	NT_PWM0->AQCTLA_bit.ZRO = 1; //обнуляем при нуле счетчика
	NT_PWM0->AQCTLA_bit.CAU = 2; //включаем при сравнении и инкрементиовании
	NT_PWM0->AQCTLA_bit.CAD = 1; //обнуляем при сравнении и декрементровании

	//Для PWMB тоже самое, что для PWMА. Без инверсии. Инверсия далее в модуле МВ
	NT_PWM0->AQCTLB = NT_PWM0->AQCTLA; // для начала события для PWM1B запрещены
	NT_PWM0->AQCTLB_bit.ZRO = NT_PWM0->AQCTLA_bit.ZRO; //обнуляем при нуле счетчика
	NT_PWM0->AQCTLB_bit.CAU = NT_PWM0->AQCTLA_bit.CAU; //включаем при сравнении и инкрементиовании
	NT_PWM0->AQCTLB_bit.CAD = NT_PWM0->AQCTLA_bit.CAD; //обнуляем при сравнении и декрементровании

	NT_PWM0->AQSFRC_bit.RLDCSF = 0; //по ТО надо писать 0

	// Setup Deadband
	// DBRED = DBFED = 150 * Tм_мкс / 4
	NT_PWM0->DBRED = _IQ4mpy(_IQ4(150 / 4), p->DeadBand >> 20) >> 4;
	NT_PWM0->DBFED = NT_PWM0->DBRED;
	NT_PWM0->DBCTL_bit.IN_MODE = DBA_RED_DBB_FED;    //s4=0, s5=1 на картинке DT
	NT_PWM0->DBCTL_bit.OUT_MODE = DB_FULL_ENABLE;    //S1=1, S2=2 на картинке DT
	NT_PWM0->DBCTL_bit.POLSEL = DB_ACTV_HIC;       // PWM1B = !PWM1A; S2=0, S3=1

	NT_PWM0->ETSEL_bit.INTSEL = ET_DISABLE;        // Disable INT (шимовское)
	NT_PWM0->ETSEL_bit.INTEN = 0;                  // Disable INT


	//разрешаем TZ быть источником аппаратной аварии (one-shot)
	//   NT_PWM0->TZSEL_bit.OSHT1 = TZ_ENABLE;
//    NT_PWM0->TZSEL_bit.OSHT2 = TZ_ENABLE;
//    NT_PWM0->TZSEL_bit.OSHT3 = TZ_ENABLE;

	// Trip-Zone
	NT_PWM0->TZCTL_bit.TZA = TZ_STATE;   // по событию "One-Shot Trip" переводим
	NT_PWM0->TZCTL_bit.TZB = TZ_STATE;          // ШИМ выходы в нужное состояние

	//Для VectorCARD от ШИМа запускается ADC
	//NT_PWM0->ETSEL_bit.SOCAEN = 1;		// Разрешить запуск ацп
	//NT_PWM0->ETSEL_bit.SOCASEL = 1;		// Запускать при CTR == 0 (Underflow)

	// ------------------------------------------------------------------------
	// Настраиваем модуль ePWM2
	// ------------------------------------------------------------------------
	// Setup TBCLK
	NT_PWM1->TBPRD = NT_PWM0->TBPRD;              //период такой же
	NT_PWM1->TBPHS_bit.TBPHS = 0x0001;            // Фаза равна 1 из-за задержки в один такт на синхронизацию
	NT_PWM1->TBCTR = 0x0000;                       // Clear counter

	// Setup counter mode
	NT_PWM1->TBCTL_bit.PRDLD = TB_SHADOW;        // загрузка TBPRD при TBCTR = 0
	NT_PWM1->TBCTL_bit.CTRMODE = TB_COUNT_UPDOWN;  // Count up-down
	NT_PWM1->TBCTL_bit.PHSEN = TB_ENABLE;         // Enable phase loading
	NT_PWM1->TBCTL_bit.PHSDIR = TB_UP;      // Считать вверх после загрузки фазы
	NT_PWM1->TBCTL_bit.HSPCLKDIV = 0;     // High Speed Time-base Clock Prescale
	NT_PWM1->TBCTL_bit.CLKDIV = 0;           // Time-base Clock Prescale
	NT_PWM1->TBCTL_bit.SYNCOSEL = TB_SYNC_IN; // пропускаем синхро-сигнал "насквозь"

	// Setup shadowing
	NT_PWM1->CMPCTL_bit.SHDWAMODE = CC_SHADOW;   //включить SHADOW для сравнения
	NT_PWM1->CMPCTL_bit.LOADAMODE = CC_CTR_ZERO;   // Load on zero

	NT_PWM1->CMPCTL_bit.SHDWBMODE = CC_SHADOW;   //включить SHADOW для сравнения
	NT_PWM1->CMPCTL_bit.LOADBMODE = CC_CTR_ZERO;   // Load on zero

	// Set Compare values
	NT_PWM1->CMPA_bit.CMPA = 0;                   // Set compare A value

	// Set actions
	NT_PWM1->AQCTLA = AQ_EPWM_DISABLE; // для начала события запрещены
	NT_PWM1->AQCTLA_bit.ZRO = 1; //обнуляем при нуле счетчика
	NT_PWM1->AQCTLA_bit.CAU = 2; //включаем при сравнении и инкрементиовании
	NT_PWM1->AQCTLA_bit.CAD = 1; //обнуляем при сравнении и декрементровании

	//Для PWMB тоже самое, что для PWMА. Без инверсии. Инверсия далее в модуле МВ
	NT_PWM1->AQCTLB = NT_PWM0->AQCTLA; // для начала события для PWM1B запрещены
	NT_PWM1->AQCTLB_bit.ZRO = NT_PWM0->AQCTLA_bit.ZRO; //обнуляем при нуле счетчика
	NT_PWM1->AQCTLB_bit.CAU = NT_PWM0->AQCTLA_bit.CAU; //включаем при сравнении и инкрементиовании
	NT_PWM1->AQCTLB_bit.CAD = NT_PWM0->AQCTLA_bit.CAD; //обнуляем при сравнении и декрементровании

	NT_PWM1->AQSFRC_bit.RLDCSF = 0; //по ТО надо писать 0

	// Active high complementary PWMs - Setup Deadband
	NT_PWM1->DBRED = NT_PWM0->DBRED;
	NT_PWM1->DBFED = NT_PWM1->DBRED;
	NT_PWM1->DBCTL_bit.IN_MODE = NT_PWM0->DBCTL_bit.IN_MODE;
	NT_PWM1->DBCTL_bit.OUT_MODE = NT_PWM0->DBCTL_bit.OUT_MODE;
	NT_PWM1->DBCTL_bit.POLSEL = NT_PWM0->DBCTL_bit.POLSEL;

	// Interrupt where we will change the Compare Values
	NT_PWM1->ETSEL_bit.INTSEL = ET_DISABLE;        // Disable INT
	NT_PWM1->ETSEL_bit.INTEN = 0;                  // Disable INT

	//разрешаем TZ быть источником аппаратной аварии (one-shot)
	//   NT_PWM1->TZSEL_bit.OSHT1 = TZ_ENABLE;
	//   NT_PWM1->TZSEL_bit.OSHT2 = TZ_ENABLE;
	//   NT_PWM1->TZSEL_bit.OSHT3 = TZ_ENABLE;

	// Trip-Zone
	NT_PWM1->TZCTL_bit.TZA = TZ_STATE;   // по событию "One-Shot Trip" переводим
	NT_PWM1->TZCTL_bit.TZB = TZ_STATE;          // ШИМ выходы в нужное состояние

	// ------------------------------------------------------------------------
	// Настраиваем модуль ePWM3
	// ------------------------------------------------------------------------
	// Setup TBCLK
	NT_PWM2->TBPRD = NT_PWM0->TBPRD;
	NT_PWM2->TBPHS_bit.TBPHS = 0x0001;            // Фаза равна 1 из-за задержки в один такт на синхронизацию
	NT_PWM2->TBCTR = 0x0000;                       // Clear counter

	// Setup counter mode
	NT_PWM2->TBCTL_bit.PRDLD = TB_SHADOW;        // загрузка TBPRD при TBCTR = 0
	NT_PWM2->TBCTL_bit.CTRMODE = TB_COUNT_UPDOWN;  // Count up-down
	NT_PWM2->TBCTL_bit.PHSEN = TB_ENABLE;         // Enable phase loading
	NT_PWM2->TBCTL_bit.PHSDIR = TB_UP;      // Считать вверх после загрузки фазы
	NT_PWM2->TBCTL_bit.HSPCLKDIV = 0;     // High Speed Time-base Clock Prescale
	NT_PWM2->TBCTL_bit.CLKDIV = 0;           // Time-base Clock Prescale
	NT_PWM2->TBCTL_bit.SYNCOSEL = TB_SYNC_IN; // разрешаем выдачу синхро-сигнала

	// Setup shadowing
	NT_PWM2->CMPCTL_bit.SHDWAMODE = CC_SHADOW;   //включить SHADOW для сравнения
	NT_PWM2->CMPCTL_bit.LOADAMODE = CC_CTR_ZERO;   // Load on zero

	NT_PWM2->CMPCTL_bit.SHDWBMODE = CC_SHADOW;   //включить SHADOW для сравнения
	NT_PWM2->CMPCTL_bit.LOADBMODE = CC_CTR_ZERO;   // Load on zero

	// Set Compare values
	NT_PWM2->CMPA_bit.CMPA = 0;                   // Set compare A value

	// Set actions
	NT_PWM2->AQCTLA = AQ_EPWM_DISABLE; // для начала события запрещены
	NT_PWM2->AQCTLA_bit.ZRO = 1; //обнуляем при нуле счетчика
	NT_PWM2->AQCTLA_bit.CAU = 2; //включаем при сравнении и инкрементиовании
	NT_PWM2->AQCTLA_bit.CAD = 1; //обнуляем при сравнении и декрементровании

	//Для PWMB тоже самое, что для PWMА. Без инверсии. Инверсия далее в модуле МВ
	NT_PWM2->AQCTLB = NT_PWM0->AQCTLA; // для начала события для PWM1B запрещены
	NT_PWM2->AQCTLB_bit.ZRO = NT_PWM0->AQCTLA_bit.ZRO; //обнуляем при нуле счетчика
	NT_PWM2->AQCTLB_bit.CAU = NT_PWM0->AQCTLA_bit.CAU; //включаем при сравнении и инкрементиовании
	NT_PWM2->AQCTLB_bit.CAD = NT_PWM0->AQCTLA_bit.CAD; //обнуляем при сравнении и декрементровании

	NT_PWM2->AQSFRC_bit.RLDCSF = 0; //по ТО надо писать 0

	// Active high complementary PWMs - Setup Deadband
	NT_PWM2->DBRED = NT_PWM0->DBRED;
	NT_PWM2->DBFED = NT_PWM2->DBRED;
	NT_PWM2->DBCTL_bit.IN_MODE = NT_PWM0->DBCTL_bit.IN_MODE;
	NT_PWM2->DBCTL_bit.OUT_MODE = NT_PWM0->DBCTL_bit.OUT_MODE;
	NT_PWM2->DBCTL_bit.POLSEL = NT_PWM0->DBCTL_bit.POLSEL;

	// Interrupt where we will change the Compare Values
	NT_PWM2->ETSEL_bit.INTSEL = ET_DISABLE;        // Disable INT
	NT_PWM2->ETSEL_bit.INTEN = 0;                  // Disable INT

	//разрешаем TZ быть источником аппаратной аварии (one-shot)
	//  NT_PWM2->TZSEL_bit.OSHT1 = TZ_ENABLE;
	//  NT_PWM2->TZSEL_bit.OSHT2 = TZ_ENABLE;
	//  NT_PWM2->TZSEL_bit.OSHT3 = TZ_ENABLE;


	// Trip-Zone
	NT_PWM2->TZCTL_bit.TZA = TZ_STATE;   // по событию "One-Shot Trip" переводим
	NT_PWM2->TZCTL_bit.TZB = TZ_STATE;          // ШИМ выходы в нужное состояние

	// ------------------------------------------------------------------------
	// Настраиваем модуль ePWM3 под АЦП
	// ------------------------------------------------------------------------
	// Setup TBCLK
	NT_PWM3->TBPRD = NT_PWM0->TBPRD;
	NT_PWM3->TBPHS_bit.TBPHS = 0x0001;      // Фаза равна 1 из-за задержки в один такт на синхронизацию
	NT_PWM3->TBCTR = 0x0000;                       // Clear counter

	// Setup counter mode
	NT_PWM3->TBCTL_bit.PRDLD = TB_SHADOW;        // загрузка TBPRD при TBCTR = 0
	NT_PWM3->TBCTL_bit.CTRMODE = TB_COUNT_UPDOWN;  // Count up-down
	NT_PWM3->TBCTL_bit.PHSEN = TB_ENABLE;         // Enable phase loading
	NT_PWM3->TBCTL_bit.PHSDIR = TB_UP;      // Считать вверх после загрузки фазы
	NT_PWM3->TBCTL_bit.HSPCLKDIV = 0;     // High Speed Time-base Clock Prescale
	NT_PWM3->TBCTL_bit.CLKDIV = 0;           // Time-base Clock Prescale
	NT_PWM3->TBCTL_bit.SYNCOSEL = TB_SYNC_IN; // разрешаем выдачу синхро-сигнала

	// Interrupt where we will change the Compare Values
	NT_PWM3->ETSEL_bit.INTSEL = ET_DISABLE;        // Disable INT
	NT_PWM3->ETSEL_bit.INTEN = 0;                  // Disable INT


	NT_PWM3->ETSEL_bit.SOCAEN = 1;		// Разрешить запуск ацп
	NT_PWM3->ETSEL_bit.SOCASEL = 1;		// Запускать при CTR == 0 (Underflow)

	//сбрасываем все флаги аварий
	NT_PWM0->TZCLR = 0x7;
	NT_PWM1->TZCLR = 0x7;
	NT_PWM2->TZCLR = 0x7;

	//настройка ножек
#ifdef HW_VECTORCARD_DRV8301_EVM
	//ШИМ 6 выводов
	NT_GPIOG->ALTFUNCSET = (1 << 2) + (1 << 3) + (1 << 4);		//ePWMxA(0), (1), (2)
	NT_GPIOA->ALTFUNCSET = (1 << 10);		//ePWMxB(0)
	NT_GPIOF->ALTFUNCSET = (1 << 2) + (1 << 4);		//ePWMxB(1), (2)

	// Периферийная функция - PWM
	NT_COMMON_REG->GPIOPCTLA_bit.PIN10 = 2;//ePWMxB(0)
	NT_COMMON_REG->GPIOPCTLF_bit.PIN2 = 0;//ePWMxB(1)
	NT_COMMON_REG->GPIOPCTLF_bit.PIN4 = 0;//ePWMxB(2)

	NT_COMMON_REG->GPIOPCTLG_bit.PIN2 = 0;//ePWMxA(0)
	NT_COMMON_REG->GPIOPCTLG_bit.PIN3 = 0;//ePWMxA(1)
	NT_COMMON_REG->GPIOPCTLG_bit.PIN4 = 0;//ePWMxA(2)

	//NT_GPIOG->DATAOUT |= (1 << 6);
	NT_GPIOG->OUTENSET = (1 << 6); //InvEnable
#endif

#if defined(HW_VECTORCARD_DRV8312_EVM)
	//ШИМ 3 вывода управления верхними ключами (нижние ключи ШИМят сами драйвером)
	NT_GPIOG->ALTFUNCSET = (1 << 2) + (1 << 3) + (1 << 4);	 //ePWMxA(0), (1), (2)

	// Периферийная функция - PWM
	NT_COMMON_REG->GPIOPCTLG_bit.PIN2 = 0;//ePWMxA(0)
	NT_COMMON_REG->GPIOPCTLG_bit.PIN3 = 0;//ePWMxA(1)
	NT_COMMON_REG->GPIOPCTLG_bit.PIN4 = 0;//ePWMxA(2)

	//выключаем альтернативные функции (на всякий)
	NT_GPIOA->ALTFUNCCLR = (1 << 10);		//A[10]
	NT_GPIOF->ALTFUNCCLR = (1 << 2) + (1 << 4);		//F[2], F[4]

	//A[10], F[2], F[4]  ножки резета
	NT_GPIOA->OUTENSET = (1 << 10);
	NT_GPIOF->OUTENSET = (1 << 2);
	NT_GPIOF->OUTENSET = (1 << 4);
#endif


#ifdef HW_VECTOR_MK_40_4
	NT_GPIOG->ALTFUNCSET_bit.ALTFUNCSET = (1 << 2) + (1 << 3) + (1 << 4); //ePWMxA(0), (1), (2)
	NT_GPIOF->ALTFUNCSET_bit.ALTFUNCSET = (1 << 0) + (1 << 2) + (1 << 4); //ePWMxB(0), (1), (2)

	// Периферийная функция - PWM
	NT_COMMON_REG->GPIOPCTLF_bit.PIN0 = 0;//ePWMxB(0)
	NT_COMMON_REG->GPIOPCTLF_bit.PIN2 = 0;//ePWMxB(1)
	NT_COMMON_REG->GPIOPCTLF_bit.PIN4 = 0;//ePWMxB(2)

	NT_COMMON_REG->GPIOPCTLG_bit.PIN2 = 0;//ePWMxA(0)
	NT_COMMON_REG->GPIOPCTLG_bit.PIN3 = 0;//ePWMxA(1)
	NT_COMMON_REG->GPIOPCTLG_bit.PIN4 = 0;//ePWMxA(2)

	//Вход аппаратной аварии
	NT_GPIOA->ALTFUNCCLR = (1<<8); //A[8]

#endif

#ifdef HW_MOTORCONTROLBOARD
	//ШИМ 6 выводов
	NT_GPIOG->ALTFUNCSET = (1 << 2) + (1 << 3) + (1 << 4);		//ePWMxA(0), (1), (2)
	NT_GPIOF->ALTFUNCSET = (1 << 0);							//ePWMxB(0)
	NT_GPIOA->ALTFUNCSET = (1 << 11) + (1 << 12);				//ePWMxB(1), (2)

	// Периферийная функция - PWM
	NT_COMMON_REG->GPIOPCTLF_bit.PIN0 = 0;		//ePWMxB(0)
	NT_COMMON_REG->GPIOPCTLA_bit.PIN11 = 2;		//ePWMxB(1)
	NT_COMMON_REG->GPIOPCTLA_bit.PIN12 = 2;		//ePWMxB(2)


	NT_COMMON_REG->GPIOPCTLG_bit.PIN2 = 0;//ePWMxA(0)
	NT_COMMON_REG->GPIOPCTLG_bit.PIN3 = 0;//ePWMxA(1)
	NT_COMMON_REG->GPIOPCTLG_bit.PIN4 = 0;//ePWMxA(2)
#endif

	//Синхронный запуск ШИМ
	NT_COMMON_REG->PWM_SYNC_bit.TBCLKSYNC = 0x1FF;

}



//! Нормирование входных величин.

//!Учитывает компенсацию напряжения при изменении Ud,
//!выполняет вписывание вектора в окружность, если требуется, и
//!выполняет смену базиса, относительно которого идет нормировка.
//! \memberof TPWM_Module
void PWM_Module_NormInput(TPWM_Module* p) {
    _iq knorm;

    p->UalphaNorm=p->UalphaRef;
    p->UbetaNorm=p->UbetaRef;

    //находим амплитуду (без учета ограничения)
    p->U_mag=_IQmag(p->UalphaNorm,p->UbetaNorm);


    p->UdCorTmp=_IQdiv(_IQ(1.0),(_IQ(1.0)+_IQmpy((adc.Udc_meas-_IQ(1.0)),p->UdCompK)));
    //учет пульсаций напряжения на Ud
    if (p->UdCompEnable&1) { //он включен?
        p->UalphaNorm=_IQmpy(p->UalphaNorm, p->UdCorTmp);//изменим пропорционально коэфф-ту коррекции
        p->UbetaNorm=_IQmpy(p->UbetaNorm, p->UdCorTmp);//и это тоже
    }

    //вписывание заданной амплитуды напряжения в окружность, вписываемую в шестиугольник базывых векторов
    //когда U_lim=1.0, это и есть такая окружность. Бывает, что мы хотим вписывать в шестиугольник. Тогда просто задираем вверх огрианичение U_lim
    if (p->U_lim>_IQ(1.0/0.866)) //но нет смысла задирать выше максимально-реализ. напряжения (больше базового вектора)
        p->U_lim=_IQ(1.0/0.866);
    /* рассчитываем амплитуду вектора, который хотим отработать*/
    knorm=_IQmag(p->UalphaNorm,p->UbetaNorm);//плюс нормируем от 540 к 311
    if (knorm>=p->U_lim) { //он больше, чем наше ограничение?
        knorm=_IQdiv(p->U_lim,knorm);//в эту же переменную, для экономии, рассчитываем нормировку
        p->UalphaNorm=_IQmpy(knorm,p->UalphaNorm);//уменьшаем пропорционально нормировке
        p->UbetaNorm=_IQmpy(knorm,p->UbetaNorm);//и это
        p->ULimitation=1;//флаг о том, что идет ограничение напряжения
    } else
        p->ULimitation=0;

    /* рассчитываем приведенные вектора*/
    /* До этого момента напряжение нормировалось относительно
    базиса фазного амплитудного значения, например, 220*sqrt(2)
    Теперь производится переход, где за единицу принимается максимально
    реализуемое напряжение (длина базового вектора)
    */
    p->UalphaNorm=_IQmpy(p->UalphaNorm,_IQ(0.866));
    p->UbetaNorm=_IQmpy(p->UbetaNorm,_IQ(0.866));
}



//! Функция 6ти секторной векторной ШИМ

//! \memberof TPWM_Module
void PWM_Module_No_SV_Update(TPWM_Module *p) {
    _iq lambda1;
    _iq lambda2;
    _iq lambda3;

    int16 gamma;
    int16 gamma1;
    long tmp_pwm;

    //Присвоение трех уставок сравнения только если счетчик таймера далек от нуля, чтобы
    //предотвратить частичное применение скважностей, что в векторной ШИМ может быть критично и испортить период ШИМ.
    DINT;
    if (NT_PWM0->TBCTR>30) {
        NT_PWM0->CMPA_bit.CMPA=(Uint16)p->GammaA;
        NT_PWM1->CMPA_bit.CMPA=(Uint16)p->GammaB;
        NT_PWM2->CMPA_bit.CMPA=(Uint16)p->GammaC;
    }
    EINT;

    PWM_Module_NormInput(p);

    //Расчет скважностей по методу Изосимова.
    //Подробнее см. диссертацию Чуева П.В. http://motorcontrol.ru/wp-content/uploads/2015/11/Chuev_vector_control.pdf
    tmp_pwm = _IQmpy(_1_SQRT3,p->UbetaNorm);  /*делим на корень из 3*/
    lambda1 = _IQmpy(p->k_pwm,(p->UalphaNorm - tmp_pwm));
    lambda2 = _IQmpy(p->k_pwm,2*tmp_pwm);
    lambda3 = _IQmpy(p->k_pwm,p->UalphaNorm + tmp_pwm);


    if (lambda1<=0)
        if (lambda3>0) {
            gamma=lambda3;
            gamma1=-lambda1;
            p->sector=1; /*110 начало */
            p->GammaA=p->k_pwm-gamma;
            p->GammaB=p->k_pwm-(gamma+gamma1+1);
            p->GammaC=p->k_pwm-0;

        } else if (lambda2>0) {
            gamma=lambda2;
            gamma1=-lambda3;
            p->sector=2; /*011 начало */
            p->GammaA=p->k_pwm-0;
            p->GammaB=p->k_pwm-(gamma+gamma1+1);
            p->GammaC=p->k_pwm-gamma1;
            /*011 начало */
        } else if (lambda1!=0) {
            gamma=-lambda1;
            gamma1=-lambda2;
            p->sector=3; /*011 начало */
            p->GammaA=p->k_pwm-0;
            p->GammaB=p->k_pwm-gamma;
            p->GammaC=p->k_pwm-(gamma+gamma1+1);
            /*011 начало */
        } else {
            gamma=-lambda3;
            gamma1 = lambda1;
            p->sector= 4; /*101 начало */
            p->GammaA=p->k_pwm-gamma1;
            p->GammaB=p->k_pwm-0;
            p->GammaC=p->k_pwm-(gamma+gamma1+1);
            /*101 начало */

        }
    else if (lambda2>0) {
        gamma=lambda1;
        gamma1=lambda2;
        p->sector= 0; /*110 начало */
        p->GammaA=p->k_pwm-(gamma+gamma1+1);
        p->GammaB=p->k_pwm-gamma1;
        p->GammaC=p->k_pwm-0;
        /*110 начало */
    } else if (lambda3<0) {
        gamma=-lambda3;
        gamma1 = lambda1;
        p->sector= 4; /*101 начало */
        p->GammaA=p->k_pwm-gamma1;
        p->GammaB=p->k_pwm-0;
        p->GammaC=p->k_pwm-(gamma+gamma1+1);
        /*101 начало */
    } else {
        gamma=-lambda2;
        gamma1=lambda3;
        p->sector=5; /*101 начало */
        p->GammaA=p->k_pwm-(gamma+gamma1+1);
        p->GammaB=p->k_pwm-0;
        p->GammaC=p->k_pwm-gamma;
        /*101 начало */
    }


    /*насыщения */

    if (p->GammaA<0) p->GammaA=0;
    if (p->GammaB<0) p->GammaB=0;
    if (p->GammaC<0) p->GammaC=0;

    if (p->GammaA>p->k_pwm) p->GammaA=p->k_pwm+1;
    if (p->GammaB>p->k_pwm) p->GammaB=p->k_pwm+1;
    if (p->GammaC>p->k_pwm) p->GammaC=p->k_pwm+1;

    //Присвоение трех уставок сравнения только если счетчик таймера далек от нуля, чтобы
    //предотвратить частичное применение скважностей, что в векторной ШИМ может быть критично и испортить период ШИМ.
    //Второй раз - чтобы предотвратить ситуацию с постоянной "незагрузкой" при равенстве частот расчета и ШИМ
    DINT;
    if (NT_PWM0->TBCTR>30) {
        NT_PWM0->CMPA_bit.CMPA=(Uint16)p->GammaA;
        NT_PWM1->CMPA_bit.CMPA=(Uint16)p->GammaB;
        NT_PWM2->CMPA_bit.CMPA=(Uint16)p->GammaC;
    }
    EINT;
}

//! Функция синусоидальной ШИМ

//! \memberof TPWM_Module
void PWM_Module_Sin_Update(TPWM_Module *p) {
	_iq PhasePtsA;
	_iq PhasePtsB;
	_iq PhasePtsC;
	_iq knorm;


	p->UalphaNorm = p->UalphaRef;
	p->UbetaNorm = p->UbetaRef;

	//находим амплитуду (без учета ограничения)
	p->U_mag = _IQmag(p->UalphaNorm, p->UbetaNorm);

	/* рассчитываем амплитуду вектора, который хотим отработать*/
	knorm = _IQmag(p->UalphaNorm, p->UbetaNorm);
	if (knorm >= p->U_lim) { //он больше, чем наше ограничение?
		knorm = _IQdiv(p->U_lim, knorm); //в эту же переменную, для экономии, рассчитываем нормировку
		p->UalphaNorm = _IQmpy(knorm, p->UalphaNorm); //уменьшаем пропорционально нормировке
		p->UbetaNorm = _IQmpy(knorm, p->UbetaNorm); //и это
		p->ULimitation = 1; //флаг о том, что идет ограничение напряжения
	} else
		p->ULimitation = 0;

	//Нормирование входного напряжения. Так как синусоидальная ШИМ формирует на 0.866 меньшее напряжение,
	//чем векторная ШИМ (270В амплитудного фазного напряжения вместо 311В), то для того, чтобы при том же задании в переменных
	//p->UalphaNorm, p->UbetaNorm получилось то же самое напряжение на выходе, что и в векторной ШИМ, нужно задания увеличить в 1/0.866 раз.
	//Деление на два нужно для последующих формул, чтобы переменые менялись в половинном от максимального диапазоне.
	p->UalphaNorm = _IQmpy(p->UalphaNorm, _IQ(1/0.866/2));
	p->UbetaNorm = _IQmpy(p->UbetaNorm, _IQ(1/0.866/2));

	/*фазное преобразование из системы альфа, бетта в a,b,c */

	PhasePtsA = _IQ(0.5) - (p->UalphaNorm);
	PhasePtsB = _IQ(0.5) - (_IQmpy(p->UbetaNorm,_IQ(0.8660254)) - (p->UalphaNorm >> 1));
	PhasePtsC = _IQ(0.5) - (-_IQmpy(p->UbetaNorm, _IQ(0.8660254)) - (p->UalphaNorm >> 1));

	p->GammaA = _IQmpy(p->k_pwm, PhasePtsA);
	p->GammaB = _IQmpy(p->k_pwm, PhasePtsB);
	p->GammaC = _IQmpy(p->k_pwm, PhasePtsC);

	/*насыщения */

	if (p->GammaA < 0)
		p->GammaA = 0;
	if (p->GammaB < 0)
		p->GammaB = 0;
	if (p->GammaC < 0)
		p->GammaC = 0;

	if (p->GammaA > p->k_pwm)
		p->GammaA = p->k_pwm + 1;
	if (p->GammaB > p->k_pwm)
		p->GammaB = p->k_pwm + 1;
	if (p->GammaC > p->k_pwm)
		p->GammaC = p->k_pwm + 1;


	NT_PWM0->CMPA_bit.CMPA = (Uint16) p->GammaA;
	NT_PWM1->CMPA_bit.CMPA = (Uint16) p->GammaB;
	NT_PWM2->CMPA_bit.CMPA = (Uint16) p->GammaC;

}

//! Функция непрерывной векторной ШИМ без привязки к шинам

//!В отличие от шестисекторной векторной ШИМ PWM_Module_No_SV_Update здесь
//!ни одна фаза не прижимается к верхней или нижней шине. Вместо этого во все
//!три фазы добавляется нулевая последовательность, равная половине суммы
//!максимального и минимального фазного потенциала (метод min/max, инжекция
//!нулевой последовательности). Это даёт такой же линейный диапазон, как у
//!векторной ШИМ (амплитуда до 1.0 в базисе фазного амплитудного значения),
//!т.е. на 15.5% больше, чем у синусоидальной ШИМ, но без пауз коммутации,
//!характерных для привязки к шине.
//! \memberof TPWM_Module
void PWM_Module_SVPWM_Update(TPWM_Module *p) {
	_iq knorm;
	_iq Va, Vb, Vc;
	_iq Vmax, Vmin, Voffset;

	p->UalphaNorm = p->UalphaRef;
	p->UbetaNorm = p->UbetaRef;

	//находим амплитуду (без учета ограничения)
	p->U_mag = _IQmag(p->UalphaNorm, p->UbetaNorm);

	/* рассчитываем амплитуду вектора, который хотим отработать*/
	knorm = _IQmag(p->UalphaNorm, p->UbetaNorm);
	if (knorm >= p->U_lim) { //он больше, чем наше ограничение?
		knorm = _IQdiv(p->U_lim, knorm); //в эту же переменную, для экономии, рассчитываем нормировку
		p->UalphaNorm = _IQmpy(knorm, p->UalphaNorm); //уменьшаем пропорционально нормировке
		p->UbetaNorm = _IQmpy(knorm, p->UbetaNorm); //и это
		p->ULimitation = 1; //флаг о том, что идет ограничение напряжения
	} else
		p->ULimitation = 0;

	//переход в переменные скважности. Как и в синусоидальной ШИМ, масштаб
	//1/sqrt(3), чтобы переменные менялись в диапазоне от -1/sqrt(3) до 1/sqrt(3).
	//Синусоидальная ШИМ из-за отсутствия нулевой последовательности насыщается
	//уже при амплитуде 0.866, непрерывная векторная - только при 1.0.
	p->UalphaNorm = _IQmpy(p->UalphaNorm, _IQ(1/0.866/2));
	p->UbetaNorm = _IQmpy(p->UbetaNorm, _IQ(1/0.866/2));

	/*фазное преобразование из системы альфа, бетта в a,b,c (обратное Кларка)*/
	Va = p->UalphaNorm;
	Vb = _IQmpy(p->UbetaNorm, _IQ(0.8660254)) - (p->UalphaNorm >> 1);
	Vc = -_IQmpy(p->UbetaNorm, _IQ(0.8660254)) - (p->UalphaNorm >> 1);

	/*инжекция нулевой последовательности: из каждого фазного потенциала
	  вычитаем половину суммы максимума и минимума. За счет этого общая точка
	  трех потенциалов центрируется относительно середины шины, а ни один из
	  потенциалов не прижимается к шине. SVPWM_offsetK (0..1) ослабляет
	  инжекцию для плавного A/B-сравнения с синусоидальной ШИМ.*/
	Vmax = Va;
	Vmin = Va;
	if (Vb > Vmax)
		Vmax = Vb;
	else if (Vb < Vmin)
		Vmin = Vb;
	if (Vc > Vmax)
		Vmax = Vc;
	else if (Vc < Vmin)
		Vmin = Vc;
	Voffset = _IQmpy((Vmax + Vmin), _IQ(0.5));
	Voffset = _IQmpy(Voffset, p->SVPWM_offsetK);
	p->SVPWM_offset = Voffset;
	Va -= Voffset;
	Vb -= Voffset;
	Vc -= Voffset;

	p->GammaA = _IQmpy(p->k_pwm, _IQ(0.5) - Va);
	p->GammaB = _IQmpy(p->k_pwm, _IQ(0.5) - Vb);
	p->GammaC = _IQmpy(p->k_pwm, _IQ(0.5) - Vc);

	/*насыщения (страховка; в линейном диапазоне скважности уже в пределах)*/

	if (p->GammaA < 0)
		p->GammaA = 0;
	if (p->GammaB < 0)
		p->GammaB = 0;
	if (p->GammaC < 0)
		p->GammaC = 0;

	if (p->GammaA > p->k_pwm)
		p->GammaA = p->k_pwm + 1;
	if (p->GammaB > p->k_pwm)
		p->GammaB = p->k_pwm + 1;
	if (p->GammaC > p->k_pwm)
		p->GammaC = p->k_pwm + 1;


	NT_PWM0->CMPA_bit.CMPA = (Uint16) p->GammaA;
	NT_PWM1->CMPA_bit.CMPA = (Uint16) p->GammaB;
	NT_PWM2->CMPA_bit.CMPA = (Uint16) p->GammaC;

}

//! Функция ШИМ с раздельными фазами (для модели SRM)

//! \memberof TPWM_Module
void PWM_Module_Separate_Update(TPWM_Module *p) {
	_iq PhasePtsA;
	_iq PhasePtsB;
	_iq PhasePtsC;
	_iq knorm;

	p->UPhARef = _IQmpy(p->UPhARef, _IQ(0.5));
	p->UPhBRef = _IQmpy(p->UPhBRef, _IQ(0.5));
	p->UPhCRef = _IQmpy(p->UPhCRef, _IQ(0.5));

	PhasePtsA = _IQ(0.5) - p->UPhARef;
	PhasePtsB = _IQ(0.5) - p->UPhBRef;
	PhasePtsC = _IQ(0.5) - p->UPhCRef;

	p->GammaA = _IQmpy(p->k_pwm, PhasePtsA);
	p->GammaB = _IQmpy(p->k_pwm, PhasePtsB);
	p->GammaC = _IQmpy(p->k_pwm, PhasePtsC);

	/*насыщения */

	if (p->GammaA < 0)
		p->GammaA = 0;
	if (p->GammaB < 0)
		p->GammaB = 0;
	if (p->GammaC < 0)
		p->GammaC = 0;

	if (p->GammaA > p->k_pwm)
		p->GammaA = p->k_pwm + 1;
	if (p->GammaB > p->k_pwm)
		p->GammaB = p->k_pwm + 1;
	if (p->GammaC > p->k_pwm)
		p->GammaC = p->k_pwm + 1;

	NT_PWM0->CMPA_bit.CMPA = (Uint16) p->GammaA;
	NT_PWM1->CMPA_bit.CMPA = (Uint16) p->GammaB;
	NT_PWM2->CMPA_bit.CMPA = (Uint16) p->GammaC;

}

void PWM_Module_DC_Update(TPWM_Module *p)
{
	int32 UalphaRef = 0, UbetaRef = 0;
	int32 gamma1_DC = 0, gamma2_DC = 0;
	int32 temp32;

	UalphaRef = p->UalphaRef;
	UbetaRef = p->UbetaRef;

	if (UalphaRef > _IQ(1))
		UalphaRef = _IQ(1);
	else if (UalphaRef < _IQ(-1))
		UalphaRef = _IQ(-1);

	if (UbetaRef > _IQ(1))
		UbetaRef = _IQ(1);
	else if (UbetaRef < _IQ(0))
		UbetaRef = _IQ(0);

	//данное преобразование даёт возможность отрабатывать напряжения в диапазоне -1 - 0 - +1 (в относительных единицах)
	//Напряжение 1 - скважность (1/2 + 0.5) = 1, Напряжение 0 - скважность 0,5 и т.д.
	gamma1_DC = -(UalphaRef >> 1) + _IQ(0.5); // (IQ), Приводим заданное напряжение к соответствующей скважности.

	//На низких частотах величина p->k_pwm большая, значит большой может быть и temp32.
	//по-этому для расчётов целесообразно использовать формат iq16.
	gamma1_DC = gamma1_DC >> 8; // (IQ16) переводим то, что выше, в iq16.
	temp32 = _IQ16mpy(gamma1_DC, _IQ16(p->k_pwm)); // (IQ16) переводим скважность в величину для загрузки в регистр сравнения.
	p->GammaA = temp32 >> 16; // (IQ0) Загружем целую часть в регистр сравнения.
	p->GammaB = p->k_pwm - p->GammaA; // в регистре сравнения EPwm2 то же, что и в EPwm1.

	//На низких частотах величина p->k_pwm большая, значит большой может быть и temp32.
	//по-этому для расчётов целесообразно использовать формат iq16.
	gamma2_DC = UbetaRef >> 8; // (IQ16) переводим то, что выше, в iq16.
	temp32 = _IQ16mpy(gamma2_DC, _IQ16(p->k_pwm)); // (IQ16) переводим скважность в величину для загрузки в регистр сравнения.
	p->GammaC = p->k_pwm - (temp32 >> 16); // (IQ0) Загружем целую часть в регистр сравнения.

	NT_PWM0->CMPA_bit.CMPA = (Uint16) p->GammaA;
	NT_PWM1->CMPA_bit.CMPA = (Uint16) p->GammaB;
	NT_PWM2->CMPA_bit.CMPA = (Uint16) p->GammaC;
}

//! Функция ШИМ для начальной зарядки будстрепных конденсаторов инвертора

//! \memberof TPWM_Module
void PWM_Module_ChargingMode(TPWM_Module *p) {
	p->GammaA = p->k_pwm;
	p->GammaB = p->k_pwm;
	p->GammaC = p->k_pwm;

	DINT;
	if (NT_PWM0->TBCTR > 30) {
		NT_PWM0->CMPA_bit.CMPA = (Uint16) p->GammaA;
		NT_PWM1->CMPA_bit.CMPA = (Uint16) p->GammaB;
		NT_PWM2->CMPA_bit.CMPA = (Uint16) p->GammaC;
	}
	EINT;
}

//! Общая функция-обертка для расчета ШИМ

//Вызывающает ту версию ШИМ (ту функцию), что выбрана в настройках.

//! \memberof TPWM_Module
void PWM_Module_Update(TPWM_Module *p) {

	if (p->ChargingMode)
		PWM_Module_ChargingMode(p);
	else {
		switch (p->PWM_type) {
		case PWM_TYPE_6SECT_NO_SV: {
			PWM_Module_No_SV_Update(p);
			break;
		}
		case PWM_TYPE_SIN_PWM: {
			PWM_Module_Sin_Update(p);
			break;
		}
		case PWM_TYPE_DC_DRIVE: {
			PWM_Module_DC_Update(p);
			break;
		}
		case PWM_TYPE_SRD: {
			PWM_Module_Separate_Update(p);
			break;
		}
		case PWM_TYPE_SVPWM: {
			PWM_Module_SVPWM_Update(p);
			break;
		}
		}
	}

}

//! Медленный расчет

//! Пересчитываются введенные пользователем величины
//! Величина мертвого времени, частота ШИМ и т.п.

//! \memberof TPWM_Module
void PWM_Module_SlowCalc(TPWM_Module *p) {
	Uint16 tmp;
	//проверка ограничений величины мертвого времени
	if (p->DeadBand < DEAD_BAND_MIN)
		p->DeadBand = DEAD_BAND_MIN;
	if (p->DeadBand > DEAD_BAND_MAX)
		p->DeadBand = DEAD_BAND_MAX;
	//пересчет МВ из формата IQ в мкс в такты таймера на 100мГц
	NT_PWM0->DBRED = _IQ4mpy(_IQ4(100), p->DeadBand >> 20) >> 4;
	//во все ключи то же самое
	NT_PWM0->DBFED = NT_PWM0->DBRED;

	NT_PWM1->DBFED = NT_PWM0->DBRED;
	NT_PWM1->DBRED = NT_PWM0->DBRED;
	NT_PWM2->DBFED = NT_PWM0->DBRED;
	NT_PWM2->DBRED = NT_PWM0->DBRED;

	if (p->MinGammaLimit < DEAD_BAND_MIN)
		p->MinGammaLimit = GAMMA_LIMIT_MIN;
	if (p->MinGammaLimit > DEAD_BAND_MAX)
		p->MinGammaLimit = GAMMA_LIMIT_MAX;
	NT_PWM0->FWDTH = _IQ4mpy(_IQ4(100), p->MinGammaLimit >> 20) >> 4;
	NT_PWM1->FWDTH = NT_PWM0->FWDTH;
	NT_PWM2->FWDTH = NT_PWM0->FWDTH;
	//частота ШИМ. Смена "на лету".
	if (p->FreqPrev != p->Frequency) { //сменили частоту
		DINT; //запрещение прерываний
		//проверка максимума/минимума
		if (p->Frequency < PWM_FREQ_MIN)
			p->Frequency = PWM_FREQ_MIN;
		if (p->Frequency > PWM_FREQ_MAX)
			p->Frequency = PWM_FREQ_MAX;
		//изменяем период
		p->k_pwm = _IQ10div(_IQ10(CORE_CLK/1000.0), p->Frequency << 1) >> 10; //период
		NT_PWM0->TBPRD = p->k_pwm;
		//для всех стоек то же самое
		NT_PWM1->TBPRD = p->k_pwm;
		NT_PWM2->TBPRD = p->k_pwm;
		NT_PWM3->TBPRD = p->k_pwm;        //таймер АЦП

		//посчитаем длину выборки для усреднения токов, исходя из частоты ШИМ
		adc.IASampleLength = (p->Frequency >> 10) / 10;	//Частота ШИМ в формате 22.10, приводим к инту и делим на 10 - частоту расчета системы управления
		if (adc.IASampleLength > 4)	//не более 4 точек
			adc.IASampleLength = 4;
		if (adc.IASampleLength < 1)	//не менее 1 точки
			adc.IASampleLength = 1;
		adc.IBSampleLength = adc.IASampleLength;
		adc.ICSampleLength = adc.IASampleLength;
		adc.UdcSampleLength = adc.IASampleLength;

		p->FreqPrev = p->Frequency; //предыдущая частота
		EINT;
	}

	NT_PWM3->TBPHS_bit.TBPHS = 1;//подбор сдвига для запуска АЦП: 1 - ровно на нуле таймера

}

//! Функция включения ШИМ (включение инвертора)

//! \memberof TPWM_Module
void PWM_Module_On(TPWM_Module *p) {
	p->Enabled = 1;	//флаг "включено"

	// Снимаем принудительную установку выходов
    NT_PWM0->AQCSFRC = 0;
    NT_PWM1->AQCSFRC = 0;
    NT_PWM2->AQCSFRC = 0;
	GPIO_INV_ON;

}

//! Функция выключения ШИМ (выключение инвертора)

//! \memberof TPWM_Module
void PWM_Module_Off(TPWM_Module *p) {

	p->Enabled = 0;	//флаг "выключено"
	// Принудительно обнулим все ножки

	NT_PWM0->AQCSFRC = 9;  // 0x1 - 0 на выходе PWMxA, 0x8 - 1 на выходе PWMxB
    NT_PWM1->AQCSFRC = 9;  // 0x1 - 0 на выходе PWMxA, 0x8 - 1 на выходе PWMxB
    NT_PWM2->AQCSFRC = 9;  // 0x1 - 0 на выходе PWMxA, 0x8 - 1 на выходе PWMxB
	GPIO_INV_OFF;

}

/*@}*/
