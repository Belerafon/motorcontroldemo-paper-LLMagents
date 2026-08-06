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

 \file      main.c
 \brief     Главный файл проекта. Содержит main(), а также обработчики прерываний.
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 */

/** \addtogroup MAIN */
/*@{*/
#include "main.h"
#include <string.h> //для memcpy

void InitCLK(void);

TClarke clarke = CLARKE_DEFAULTS; //!<Фазные преобразования
TPark park = PARK_DEFAULTS; //!<Координатные преобразования
TIPark ipark = IPARK_DEFAULTS; //!<Обратные координатные преобразования
TPidReg3 pid_id = PIDREG3_DEFAULTS; //!<Рег. тока по оси d
TPidReg3 pid_iq = PIDREG3_DEFAULTS; //!<Рег. тока по оси q
TPidReg3 pid_ia = PIDREG3_DEFAULTS; //!<Рег. тока якоря ДПТ
TPidReg3 pid_spd = PIDREG3_DEFAULTS; //!<Рег. скорости
TPidReg3_pos pid_pos = PIDREG3_DEFAULTS_POS; //!<Рег. положения
TRelayReg i_a_reg = RELAY_REG_DEFAULTS;	//!<регулятор тока фазы A (для модели SRM)
TRelayReg i_b_reg = RELAY_REG_DEFAULTS;	//!<регулятор тока фазы A (для модели SRM)
TRelayReg i_c_reg = RELAY_REG_DEFAULTS;	//!<регулятор тока фазы A (для модели SRM)
TVhzProf vhz = VHZPROF_DEFAULTS; 	//!< закон U/f=const
TSM_Sys sm_sys = SM_Sys_DEFAULTS; //!< Главная оболочка для вызова всех модулей
Uint16 disp_group_number = 0; 	//!< Необходимо для драйвера CANOpen и Unicon
TDataLog dlog = DATALOG_DEFAULTS; //!< Модуль осциллографирования переменных CANOpen
TSM_Protect sm_prot = SM_PROTECT_DEFAULTS; 	//!< Модуль защит
TBitsToEnumNums pult_faults_lister = BITS_TO_ENUM_NUMS_DEFAULTS; //!<Листалка аварий для Unicon
TSM_Ctrl sm_ctrl = SM_CTRL_DEFAULTS; //!< Главный дискретный автомат, реализует структуру управления
TSM_CmdLogic sm_cmd_logic = SM_CMD_LOGIC_DEFAULTS; //!< Обработка пользовательских команд управления
TSM_Net sm_net = SM_NET_DEFAULTS; 	//!< Оболочка для вызова сетевых драйверов
TRMPCtrl rmp = V_RMP_CTRL_DEFAULTS; 	//!< Задатчик интенсивности
TAdcDrv adc = ADC_DRV_DEFAULTS; 	//!< Модуль АЦП
TPWM_Module pwm = PWM_Module_DEFAULTS; 	//!< Модуль ШИМ
Texcitation pwm_ex = excitation_DEFAULTS;	//!< Модуль возбудителя СД
TDPReCAP DPReCAP = DPRECAP_DEFAULTS; //!< Модуль ДПР на элементах Холла
TposspeedEqep posspeedEqep = POSSPEED_DEFAULTS; //!< Модуль ДПР типа энкодер
TCurPar cur_par = TCUR_PAR_DEFAULTS; //!< Модуль расчета и хранения текущих показателей привода - мощность, скорость
TUserMemory UserMem = USERMEMORY_DEFAULTS; //!< Модуль работы с энергонезависимой памятью.
Tled leds = LED_DEFAULTS; //!< модуль для красивого мигания светодиодами
Tdrv8301interface drv8301 = DRV8301_INTERFACE_DEFAULTS; //!< Модуль для настройки драйвера ключей drv8301
TCanBTInterface Can1BTInterface = T_CANBT_INTERFACE_DEFAULTS;//!<Пакетная передача CANopen
TCanBTInterface Can2BTInterface = T_CANBT_INTERFACE_DEFAULTS;//!<Пакетная передача CANopen
TRTCClock RTCclock = RTC_CLOCK_DEFAULTS; //!< Модуль работы с часами реальноговремени. "spi" - по историческим причинам совместимости
TAutoOffset AutoOffset = AUTO_OFFSET_DEFAULTS;	//!<Автоматическая подстройка смещения токов АЦП
TSSI_Encoder SSI_Encoder = SSI_ENCODER_DEFAULTS; //!<Драйвер обработки датчика положения с SSI интерфейсом
TMotorModel model = MOTOR_MODEL_DEFAULTS;	//!< Модели электродвигателей для отладки "на столе" в режиме симулятора
TRotorObserver RotorObserver = ROTOR_OBSERVER_DEFAULTS; //!<Датчиковый наблюдатель потокосцепления ротора асинхронного двигателя
TCANtoRS CANtoRS = CAN_TO_RS_DEFAULTS; //!<Модуль для работы с драйвером CANopen через UART (RS). Посылки CAN запаковываются в UART
TModBus ModBus = MODBUS_DEFAULTS; //!<Драйвер для работы по протоколу MODBUS
TMBVarsConv MBVarsConv = MBVARSCONV_DEFAULTS;//!< Модуль преобразования данных из формата 16 разрядов Modbus в формат системы управления (IQ 24)
TDrvInterface drv_interface = DRV_INTERFACE_DEFAULTS;	//!<Интерфейс длЯ работы с банками аварий, событий и т.п.
TLogger FaultLog = LOGGER_DEFAULTS;			//!<Протоколирование аварий
TGlobalTime global_time = GLOBAL_TIME_DEFAULTS; //!<Работа с часами
TUdControl udControl = UD_CONTROL_DEFAULTS; //!<Плавная зарядка ЗПТ через термисторы
TFanControl FanControl = FAN_CONTROL_DEFAULTS; //!<Вентилятор

TRefs refs; //!< Структура с заданиями (токи, скорости)
TCmd cmd = { 0 }; //!< Структура с командами управления
TDrvStatus drv_status = { 0 }; //!< Текущий статус привода
TDrvParams drv_params; //!< Параметры двигателя
TSysSwitches sw; //!< Различные дискретные настройки системы управления

Uint32 VendorToken=0x11111111;//!< Уникальный ключ производителя, нужный для программы UniCON и COODEdit для различных наборов текстов разных произхводителей
int drv_status_code; //!<Статус системы управления в виде константы (ГОТОВ, РАБОТА и т.п.)

//Переменные для отладки - выведены в словарь CANOpen, 
//В них можно присваивать любую другую переменную и наблюдать её
//в UniCon, а также использовать их напрямую в ПО для отладки и менять на ходу.
volatile long Debug1 = 0;
volatile long Debug2 = 0;
volatile Uint16 Debug3 = 0;
volatile Uint16 Debug4 = 0;
volatile long DebugW1 = 1;	//декаплинг регуляторов тока: 0 = выкл, 1 = вкл
volatile long DebugW2 = 0;
volatile long DebugW3 = 0;
volatile long DebugW4 = 0;
volatile float DebugF1 = 0;
volatile float DebugF2 = 0;
volatile float DebugF3 = 0;
volatile float DebugF4 = 0;

//Счетчики прерываний модуля захвата
Uint16 CounterCAP_isr = 0;
Uint16 cap0_counter = 0;
Uint16 cap1_counter = 0;
Uint16 cap2_counter = 0;

Uint16 LoopCounter = 0; //!< Счетчик итераций фонового цикла

//!С этой функции начинается запуск программы
//! \memberof MAIN_C
int main(void) {

	/* Настройка тактирования, включение периферии */
	InitCLK();
	// Копирование некоторых функций и всех прерываний в RAM
#if defined (__GNUC__)
	memcpy(&__fastcode_ram_start, &__fastcode_flash_start,
			((Uint32) (&__fastcode_ram_end) - (Uint32) (&__fastcode_ram_start)));

	memcpy(&__isr_vector_ram_start, &__isr_vector_flash_start,
			((Uint32) (&__isr_vector_ram_end)
					- (Uint32) (&__isr_vector_ram_start)));
#elif defined (__CMCPPARM__)
	// Для CodeMaster непонятно, как разместить таблицу прерываний во флеше, а обращатся к ней в раме,
	// так что пока так.
#endif

	pwm.Off(&pwm); //выключить ШИМ (на всякий случай)

	//Инициализация, собственно, всего.
	sm_sys.init(&sm_sys);
	EINT;//разрешение прерываний

	while (1) { //Фоновый цикл
		LoopCounter++;
		sm_sys.slow_calc(&sm_sys); //Фоновый расчет
	}
}

Uint32 CpuTimerIsr1 = 0;
Uint32 TIsr1 = 0;
Uint16 msCounter = 0;

//! Прервыние, вызываемое по таймеру с частотой 1кГц
//! \memberof MAIN_C
void TIM1_IRQHandler(void) {
	CpuTimerIsr1 = NT_TIMER2->VALUE; //Засекается время выполнения функции
	sm_sys.ms_calc(&sm_sys); //миллисекундный расчет всего

	msCounter++;

	TIsr1 = (CpuTimerIsr1 - NT_TIMER2->VALUE) & 0xFFFFFF; //время выполнения функции
	if (TIsr1 > 97000) {
		sm_prot.bit_fault1 |= F_PROGRAM_1K; //если расчет слишком долгий, ошибка
	}

	NT_TIMER1->INTSTATUS_INTCLEAR_bit.INT = 1;	//сброс прерывания
}

Uint16 FastCounter = 0;
Uint32 CpuTimerIsr10 = 0;
Uint32 TIsr10 = 0;

//! Прервыние, вызываемое по таймеру с частотой 10кГц
//! \memberof MAIN_C
void TIM0_IRQHandler(void) {
	CpuTimerIsr10 = NT_TIMER2->VALUE; //Засекается время выполнения функции
	sm_sys.fast_calc(&sm_sys); //расчет 10кГц всего

	FastCounter++;
	TIsr10 = (CpuTimerIsr10 - NT_TIMER2->VALUE) & 0xFFFFFF; //время выполнения функции
	if (TIsr10 > 9700) {
		sm_prot.bit_fault1 |= F_PROGRAM_10K; //если расчет слишком долгий, ошибка
	}
	NT_TIMER0->INTSTATUS_INTCLEAR_bit.INT = 1;	//сброс прерывания
}

Uint16 ePWM0_TZ_isr_counter = 0;
//!Прерывание, возникающее при аппаратной аварии
//! \memberof MAIN_C
#if defined (__GNUC__)
void EPWM_TZ_0_IRQHandler(void)
#elif defined (__CMCPPARM__)
void PWM0_TZ_IRQHandler(void)
#endif
{
	//Так как аппаратная авария возникает при включении ШИМ
	//и удерживается пока заряжаются будстрепные конденсаторы,
	//в прерывании на нее не реагируем
	//Хотя в "настоящем" инверторе на высокое напряжение это, конечно, надо делать
	//Здесь микросхема драйверов защитит всё сама, такого выхода у неё нет
	ePWM0_TZ_isr_counter++;
	/*
	 pwm12.Off(&pwm12);

	 if (sm_ctrl.state!=CTRL_STOP)
	 {
	 sm_prot.bit_fault1|= F_PDPINT;
	 }
	 sm_ctrl.state=CTRL_STOP;
	 //сбрасываем флаги прерываний по этой ножке
	 */
}

//!Прерывание, возникающее по событиям захвата модуля CAP0
//! \memberof MAIN_C
#if defined (__GNUC__)
void ECAP0_IRQHandler(void)
#elif defined (__CMCPPARM__)
void CAP0_IRQHandler(void)
#endif
{
	//Подтверждаем это прерывание для NVIC - иначе при выходе из функции оно возникнет опять
	NT_CAP0->PEINT = 1;

	if (DPReCAP.CAPCalcEna1 == 0) { //если функция вызвалась повторно
		DPReCAP.CAP_WrongEdgeCnt = (++DPReCAP.CAP_WrongEdgeCnt) & 0xFF;
		DPReCAP.CAP_WrongEdgeCnt1++;
		return;
	}
	DPReCAP.CAPCalcEna1 = 0;//расчет будет разрешен, когда тикнет прерывание 10кГц. Чаще считать нет смысла, это помехи

	CounterCAP_isr++;//общий счётчик всех прерываний модуля захвата
	CounterCAP_isr = CounterCAP_isr & 0xF;
	cap0_counter++;//счётчик прерываний именно этого канала
	//исходя из состояния ног трех датчиков Холла высчитывает текущий угол с точностью 60 градусов.
	//выходом функции является  DPReCAP.Angle6 - угол с точностью 60 градусов.
	DPReCAP.Angle6Calc(&DPReCAP);
	//обработчик модуля захвата канала1 (0, если считать с нуля, но не переименовывать же какждый раз функции, в зависимости от версии заголовочных файлов...).
	//Засекает время между этим импульсом и предыдущими для расчета интерполятора угла и частоты вращения (скорости)
	DPReCAP.CAP1Calc(&DPReCAP);

	//Подтверждение прерываний
	NT_CAP0->ECCLR_bit.CEVT0 = 1;
	NT_CAP0->ECCLR_bit.CEVT1 = 1;
	NT_CAP0->ECCLR_bit.INT = 1;

}

//!Прерывание, возникающее по событиям захвата модуля CAP1
//! \memberof MAIN_C
#if defined (__GNUC__)
void ECAP1_IRQHandler(void)
#elif defined (__CMCPPARM__)
void CAP1_IRQHandler(void)
#endif
{
	//Подтверждаем это прерывание для NVIC - иначе при выходе из функции оно возникнет опять
	NT_CAP1->PEINT = 1;

	if (DPReCAP.CAPCalcEna2 == 0) { //если функция вызвалась повторно
		DPReCAP.CAP_WrongEdgeCnt = (++DPReCAP.CAP_WrongEdgeCnt) & 0xFF;
		DPReCAP.CAP_WrongEdgeCnt2++;
		return;
	}
	DPReCAP.CAPCalcEna2 = 0;

	CounterCAP_isr++;
	CounterCAP_isr = CounterCAP_isr & 0xF;
	cap1_counter++;
	DPReCAP.Angle6Calc(&DPReCAP);
	DPReCAP.CAP2Calc(&DPReCAP);

	//Подтверждение прерываний
	NT_CAP1->ECCLR_bit.CEVT0 = 1;
	NT_CAP1->ECCLR_bit.CEVT1 = 1;
	NT_CAP1->ECCLR_bit.INT = 1;
}


//!Прерывание, возникающее по событиям захвата модуля CAP2
//! \memberof MAIN_C
#if defined (__GNUC__)
void ECAP2_IRQHandler(void)
#elif defined (__CMCPPARM__)
void CAP2_IRQHandler(void)
#endif
{
	//Подтверждаем это прерывание для NVIC - иначе при выходе из функции оно возникнет опять
	NT_CAP2->PEINT = 1;

	if (DPReCAP.CAPCalcEna3 == 0) { //если функция вызвалась повторно
		DPReCAP.CAP_WrongEdgeCnt = (++DPReCAP.CAP_WrongEdgeCnt) & 0xFF;
		DPReCAP.CAP_WrongEdgeCnt3++;
		return;
	}
	DPReCAP.CAPCalcEna3 = 0;

	CounterCAP_isr++;
	CounterCAP_isr = CounterCAP_isr & 0xF;
	cap2_counter++;
	DPReCAP.Angle6Calc(&DPReCAP); //если убрать, то в момент прихода метки на один период ШИМ косяк, так как прерывание посчиталось, а Angle6Calc нет
	DPReCAP.CAP3Calc(&DPReCAP);

	//Подтверждение прерываний
	NT_CAP2->ECCLR_bit.CEVT0 = 1;
	NT_CAP2->ECCLR_bit.CEVT1 = 1;
	NT_CAP2->ECCLR_bit.INT = 1;
}

//!Прерывание, возникающее по событию реперной метки(индекса) модуля QEP
//! \memberof MAIN_C
//! Но - на двигателе ACM601V36-1000 нет индексной метки.
//Так что этого прерывания c таким двигателем не будет (или будет от помех, как повезет)
#if defined (__GNUC__)
void EQEP1_IRQHandler(void)
#elif defined (__CMCPPARM__)
void QEP1_IRQHandler(void)
#endif
{
	//Обработка репера
	posspeedEqep.index(&posspeedEqep);
	//Подтверждаем это прерывание для NVIC
	NT_QEP1->INTCLR = 1;
	NT_QEP1->QCLR_bit.IEL = 1;
	NT_QEP1->QCLR_bit.INT = 1;
}

/*@}*/

