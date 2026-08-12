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

 \file    SMCtrl.c
 \brief   Основной ДА переключения систем управления (см. TSM_Ctrl)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016
 
 */

/** \addtogroup SMCtrl */
/*@{*/

#include "DSP.h"
#include "V_IQmath.h"
#include "main.h"

static long CurrentRegulatorVoltageLimit;

//! Синхронизирует насыщение токовых ПИД с доступным напряжением инвертора.
static void SM_Ctrl_UpdateCurrentRegulatorLimits(void) {
	CurrentRegulatorVoltageLimit = PWM_Module_GetVoltageLimit(&pwm, adc.Udc_meas);

	pid_id.pid_out_max = CurrentRegulatorVoltageLimit;
	pid_id.pid_out_min = -CurrentRegulatorVoltageLimit;
	pid_iq.pid_out_max = CurrentRegulatorVoltageLimit;
	pid_iq.pid_out_min = -CurrentRegulatorVoltageLimit;
	pid_ia.pid_out_max = CurrentRegulatorVoltageLimit;
	pid_ia.pid_out_min = -CurrentRegulatorVoltageLimit;
}

//! Вписывает результирующий вектор d/q в доступную окружность напряжения.
//! Добавочная ошибка насыщения не даёт интеграторам накапливаться, когда
//! каждая компонента допустима отдельно, но их результирующий вектор уже нет.
static void SM_Ctrl_LimitCurrentRegulatorVector(void) {
	long voltageMagnitude;
	long scale;
	long limitedD;
	long limitedQ;

	voltageMagnitude = _IQmag(pid_id.pid_out_reg3, pid_iq.pid_out_reg3);
	if (voltageMagnitude > CurrentRegulatorVoltageLimit) {
		scale = _IQdiv(CurrentRegulatorVoltageLimit, voltageMagnitude);
		limitedD = _IQmpy(pid_id.pid_out_reg3, scale);
		limitedQ = _IQmpy(pid_iq.pid_out_reg3, scale);

		pid_id.saterr_reg3Add = limitedD - pid_id.pid_out_reg3;
		pid_iq.saterr_reg3Add = limitedQ - pid_iq.pid_out_reg3;
		pid_id.pid_out_reg3 = limitedD;
		pid_iq.pid_out_reg3 = limitedQ;
	} else {
		pid_id.saterr_reg3Add = 0;
		pid_iq.saterr_reg3Add = 0;
	}
}


//!Инициализация
//!Всякие обнуления
//! \memberof TSM_Ctrl
void SM_Ctrl_Init(TSM_Ctrl *p) {
	p->state_prev = 0xff;
	p->state = 0x00;
}

//!Фоновый расчет

//! \memberof TSM_Ctrl
void SM_Ctrl_Slow_Calc(TSM_Ctrl *p) {

}

//!Миллисекундный расчет

//! \memberof TSM_Ctrl
void SM_Ctrl_ms_Calc(TSM_Ctrl *p) {

}

//!Быстрый расчет

//!Реализует все структуры управления (режимы работы), такие как 
//!Векторное управление, скалярное, режим постоянного тока и т.п.
//!Здесь реализован основной ДА переключения систем управления.
//!Кроме того, происходит расчет некоторых вспомогательных модулей.
//!Можно сказать, здесь находится "ядро" всей системы управления,
//!и именно в этой функции происходит непосредственное управление двигателем с
//!использованием всех остальных модулей.

//! \memberof TSM_Ctrl
void SM_Ctrl_Fast_Calc(TSM_Ctrl *p) {
	long Temp1;

//если система находится в перееинициализации,
//то пропускаем контур управления
	if (sm_sys.state == SYS_INIT) {
		return;
	}

	SM_Ctrl_UpdateCurrentRegulatorLimits();

//если команда остановить, то переходим
//в режим ОСТАНОВЛЕН
	if (cmd.bit.stop == 1)
		p->state = CTRL_STOP;
// если есть авария, то переходим в стоп
	if (sm_prot.state == PROT_FAIL)
		p->state = CTRL_STOP;

//считаем дискретный автомат
	if (p->state_prev != p->state)
		p->E = 1;
	else
		p->E = 0;
//нужно для дискретного автомата, определение
//первого вхождения
	p->state_prev = p->state;

	switch (p->state) {
	case CTRL_STOP: { //Состояние ОСТАНОВ
		if (p->E == 1) { //Если первое вхождение в состояние
			cmd.bit.start = 0;
			DPReCAP.speed = 0;
			DPReCAP.Ts = 0;
			DPReCAP.Tspeed = 0;
			global_time.WtiteSPI_flag = 1;    //записать времЯ работы
			//при переходе в стоп и работающем
			//осциллографе переводим его в режим
			//однократной записи, чтобы зафиксир. возможную аварию.
			if (dlog.StartBits & (1 << 8))    //Если пользователь задал этот бит
				dlog.set_mode(1, &dlog); //запускаем осциллограф по этому событию

			if (sm_prot.state==PROT_FAIL)//если мы перешли в стоп, а модуль защит в аварии
				if (dlog.mode_reset==2)//и даталоггер крутился
					dlog.mode_reset=1;//то остановим осц, пусть будет осца аварии
		}
		pwm.Off(&pwm);            //Выкл ШИМ
		//   refs.theta_elec=0;

		//Обнуление всего опасного
		pwm.UalphaRef = 0;
		pwm.UbetaRef = 0;
		pwm.update(&pwm);
		pid_id.reset(&pid_id);
		pid_iq.reset(&pid_iq);
		pid_ia.reset(&pid_ia);
		pid_id.saterr_reg3Add = 0;
		pid_iq.saterr_reg3Add = 0;
		pid_spd.reset(&pid_spd);
		pid_pos.reset(&pid_pos);
		cur_par.Is = 0;
		cur_par.power = 0;
		cur_par.speed = 0;
		//  cur_par.ThetaRefCurr=0;

		/* Конец */
		break;
	}

	case CTRL_RUN: {     //Состояние запуска
		if (p->E == 1) {
			/*ENTRY START*/
			drv_status.bit.running = 1;
			pwm.On(&pwm);//Включение модуля ШИМ
			p->ctrl_counter = 0;
			if (dlog.StartBits & (1 << 0))    //Если пользователь задал этот бит
				dlog.set_mode(1, &dlog); //запускаем осциллограф по этому событию
			/*ENTRY END*/
		}
		/*DO START*/

		p->ctrl_counter++;

		if (p->ctrl_counter < 100) { //Задержка 10 мс для заряда бутстрепной ёмкости
			pwm.ChargingMode = 1;//спец. режим заряда конденсаторов
			pwm.update(&pwm);
			break;
		}
		pwm.ChargingMode = 0;

		if (p->run_mode == CTRL_RUN_U2F) {//скалярный режим
			p->state = CTRL_RUN_U2F;
		}
		if (p->run_mode == CTRL_FLUXING) {//режим удержания (постоянный ток)
			p->state = CTRL_FLUXING;
		}
		if (p->run_mode == CTRL_RUN_I2F) {//частотно-токовый режим (вращение вектора тока)
			p->state = CTRL_RUN_I2F;
		}
		if (p->run_mode == CTRL_RUN_VECTOR_SM) {//векторная СУ синхронной машины с двумя контурами
			p->state = CTRL_RUN_VECTOR_SM;
		}
		if (p->run_mode == CTRL_RUN_VECTOR_SM_ENCODER_POS) {//векторная СУ синхронной машины  с обратной связью по энкодеру и контуром положения
			p->state = CTRL_RUN_VECTOR_SM_ENCODER_POS;
		}
		if (p->run_mode == CTRL_RUN_VECTOR_IM_ENCODER) {//векторная СУ синхронной машины  с обратной связью по энкодеру и контуром положения
			p->state = CTRL_RUN_VECTOR_IM_ENCODER;
		}
		if (p->run_mode == CTRL_RUN_VECTOR_IM_ENCODER_POS) {//векторная СУ синхронной машины  с обратной связью по энкодеру и контуром положения
			p->state = CTRL_RUN_VECTOR_IM_ENCODER_POS;
		}
		if (p->run_mode == CTRL_AUTO_SENSOR_TUNING) {//автоматическая настройка смещений в ДПР (частотно-токовая+доп. логика)
			p->state = CTRL_AUTO_SENSOR_TUNING;
		}
		if (p->run_mode == CTRL_DC_DRIVE) {// ДПТ
			p->state = CTRL_DC_DRIVE;
		}
		if (p->run_mode == CTRL_DC_DRIVE_FB) {// ДПТ замкн.
			p->state = CTRL_DC_DRIVE_FB;
		}
		if (p->run_mode == CTRL_RUN_SRM_AUTOCOMMUTATION) { //автокоммутация SRM
			p->state = CTRL_RUN_SRM_AUTOCOMMUTATION;
		}

		/*DO END*/
		break;
	}

	case CTRL_FLUXING: { //Режим постоянный ток
		if (p->E == 1) {
			/*ENTRY START*/
			pwm.On(&pwm);
			pid_id.ui_reg3 = 0;	//сбрасывем интегральные каналы
			pid_iq.ui_reg3 = 0;	//чтобы не накапливались
			pid_spd.ui_reg3 = 0;
			refs.theta_elec = 0;//заданный угол - 0. Но он есть в группе Задания, его можно менять на ходу!

			if (dlog.StartBits & (1 << 1))
				dlog.set_mode(1, &dlog);
			/*ENTRY END*/
		}
		/*DO START*/

		//два тока фаз из АЦП в модуль фазных преобразований
		clarke.as = adc.Imeas_a;
		clarke.bs = adc.Imeas_b;
		clarke.calc(&clarke);//расчет

		//из фазных преобразований в координатные преобразования.
		//Поворачиваем вектор на заданный угол refs.theta_elec
		park.ds = clarke.ds;
		park.qs = clarke.qs;
		park.ang = refs.theta_elec;
		park.calc(&park);

		//регулятор тока "как бы" оси d (на самом деле эта ось к двигателю к оси d не привязана, это свободная ось)
		pid_id.pid_ref_reg3 = refs.i_flux_ref; //задание на ток намагничивания
		pid_id.pid_fdb_reg3 = park.de;//обратная связь - то что пришло с фазных преобразований
		pid_id.calc(&pid_id);

		//регулятор тока "как бы" оси q (на самом деле эта ось к двигателю к оси q не привязана, это свободная ось)
		pid_iq.pid_ref_reg3 = 0;//задание - ноль
		pid_iq.pid_fdb_reg3 = park.qe;park.de;//обратная связь - то что пришло с фазных преобразований
		pid_iq.calc(&pid_iq);
		SM_Ctrl_LimitCurrentRegulatorVector();

		//обратные фазные преобразования.
		//Крутим вектор напряжения, который выдают регуляторы токов обратно на угол refs.theta_elec
		ipark.de = pid_id.pid_out_reg3;
		ipark.qe = pid_iq.pid_out_reg3;
		ipark.ang = refs.theta_elec;
		ipark.calc(&ipark);

		//получившиеся задания напряжения по осям альфа и бета отправляем на модуль ШИМ
		pwm.UalphaRef = ipark.ds;
		pwm.UbetaRef = ipark.qs;
		pwm.update(&pwm);

		//в наблюдаемые переменные - текущий амплитудный ток в статоре (обратная связь регулятора d)
		cur_par.Is = pid_id.pid_fdb_reg3;

		break;
	}

	case CTRL_RUN_U2F: { //Режим скалярное управление U(f)
		if (p->E == 1) {
			/*ENTRY START*/
			pwm.On(&pwm);
			rmp.output = 0;

			if (dlog.StartBits & (1 << 2)) //Если пользователь задал этот бит осциллографирования
				dlog.set_mode(1, &dlog); //запускаем осциллограф по этому событию

			/*ENTRY END*/
		}
		/*DO START*/
		//задание скорости на вход задатчика интенсивности
		rmp.input = refs.speed_ref;
		rmp.calc(&rmp);//расчет задатчика
		cur_par.speed = rmp.output;	//скорость (частоту вращения) показываем в налюдаемых параметрах, которая задана

		//угол refs.theta_elec - интеграл от скорости rmp.output.
		//Константа FAST_CALC_TS - период дискретизации
		//drv_params.freq_nom номинальная частота (базовый параметр частоты для относительных единиц измерения)
		refs.theta_elec += _IQmpy(_IQmpyI32(drv_params.freq_nom,FAST_CALC_TS),
				rmp.output);
		refs.theta_elec &= 0x00FFFFFF; //отсечение лишней верхней части, если угол больше единицы в IQ (360 градусов)
		//на вход кривой U/f подается текущая скорость (частота)
		vhz.freq = rmp.output;
		vhz.calc(&vhz);

		//в модуль координатных преоразований подаем заданную амплитуду напряжения vhz.vout с модуля U/f
		//Используем фазные преобразования как "крутилку" вектора напряжения
		ipark.de = vhz.vout;
		ipark.qe = 0;
		ipark.ang = refs.theta_elec;
		ipark.calc(&ipark);

		//получившиеся задания напряжения по осям альфа и бета отправляем на модуль ШИМ
		pwm.UalphaRef = ipark.ds;
		pwm.UbetaRef = ipark.qs;
		pwm.update(&pwm);

		/*DO END*/
		break;
	}

	case CTRL_RUN_I2F: { //Режим частотно-токового управления - вращающийся вектор тока.
		if (p->E == 1) {
			/*ENTRY START*/
			pid_id.ui_reg3 = 0;	//сбрасывем интегральные каналы
			pid_iq.ui_reg3 = 0;	//чтобы не накапливались
			pid_spd.ui_reg3 = 0;
			pid_id.pid_ref_reg3 = 0;
			pid_iq.pid_ref_reg3 = 0;

			pwm.On(&pwm);//влючение ШИМ
			rmp.output = 0;
			if (dlog.StartBits & (1 << 3))
				dlog.set_mode(1, &dlog);
			/*ENTRY END*/
		}
		/*DO START*/
		{

			//два тока фаз из АЦП в модуль фазных преобразований
			clarke.as = adc.Imeas_a;
			clarke.bs = adc.Imeas_b;
			clarke.calc(&clarke);

			//текущий вектор тока - в наблюдаемые параметры (чтобы на него смотреть)
			cur_par.Ialpha = clarke.ds;
			cur_par.Ibeta = clarke.qs;

			//задание скорости на вход задатчика интенсивности
			rmp.input = refs.speed_ref;
			rmp.calc(&rmp);//расчет задатчика

			//угол refs.theta_elec - интеграл от скорости rmp.output.
			//Константа FAST_CALC_TS - период дискретизации
			//drv_params.freq_nom номинальная частота (базовый параметр частоты для относительных единиц измерения)
			refs.theta_elec += _IQmpy(
					_IQmpyI32(drv_params.freq_nom,FAST_CALC_TS), rmp.output);
			cur_par.ThetaRefCurr = refs.theta_elec;
			refs.theta_elec &= 0x00FFFFFF;//отсечение лишней верхней части, если угол больше единицы в IQ (360 градусов)

			//текущий угол с датчика положения, переведенный в электрический из механического
			//cur_par.ThetaCurr = posspeedEqep.theta_elecContinouos;

			//Текущий угол - заданный угол
			cur_par.ThetaCurr = refs.theta_elec;

			//из фазных преобразований в координатные преобразования.
			//Поворачиваем вектор на заданный угол refs.theta_elec
			park.ds = clarke.ds;
			park.qs = clarke.qs;
			park.ang = refs.theta_elec;
			park.calc(&park);

			//регулятор тока "как бы" оси d (на самом деле эта ось к двигателю к оси d не привязана, это свободная ось)
			pid_id.pid_ref_reg3 = refs.i_flux_ref;
			pid_id.pid_fdb_reg3 = park.de;
			pid_id.calc(&pid_id);

			//регулятор тока "как бы" оси q (на самом деле эта ось к двигателю к оси q не привязана, это свободная ось)
			pid_iq.pid_ref_reg3 = 0;
			pid_iq.pid_fdb_reg3 = park.qe;
			pid_iq.calc(&pid_iq);
			SM_Ctrl_LimitCurrentRegulatorVector();

			//обратные фазные преобразования.
			//Крутим вектор напряжения, который выдают регуляторы токов обратно на угол refs.theta_elec
			ipark.de = pid_id.pid_out_reg3;
			ipark.qe = pid_iq.pid_out_reg3;
			ipark.ang = refs.theta_elec;
			ipark.calc(&ipark);

			//получившиеся задания напряжения по осям альфа и бета отправляем на модуль ШИМ
			pwm.UalphaRef = ipark.ds;
			pwm.UbetaRef = ipark.qs;
			pwm.update(&pwm);

			//в наблюдаемые переменные - текущий амплитудный ток в статоре (обратная связь регулятора d)
			cur_par.Is = pid_id.pid_fdb_reg3;
			//и текущую частоту (скорость)
			cur_par.speed = rmp.output;

			if (sw.excitation_ena)
			{
				pwm_ex.ref = refs.uf_ref;
				pwm_ex.calc(&pwm_ex);
			}

			/*DO END*/
		}
		break;
	}

	case CTRL_RUN_VECTOR_SM: {            //Датчиковая СУ для синхронной машины
		if (p->E == 1) {
			/*ENTRY START*/
			pwm.On(&pwm);            //Включение ШИМ
			rmp.output = 0;            //Обнуление задатчика интенсивности
			if (dlog.StartBits & (1 << 4)) //Если пользователь задал этот бит осциллографирования
				dlog.set_mode(1, &dlog);  //Запуск осциллографа по этому событию
			/*ENTRY END*/
		}
		/*DO START*/
		{
			if (drv_params.sens_type == POS_SENSOR_TYPE_HALL){  //В типе датчика выбран ДПР на элементах Холла
				cur_par.speed = DPReCAP.speed; //Текущая скорость - с ДПР
				refs.theta_elec = DPReCAP.Angle; //текущее угловое положение - с ДПР
			}
			if (drv_params.sens_type == POS_SENSOR_TYPE_ENCODER){  //В типе датчика выбран ДПР на элементах Холла
				cur_par.speed = posspeedEqep.speed_elec; //Текущая скорость - с ДПР
				refs.theta_elec = posspeedEqep.theta_elec; //текущее угловое положение - с ДПР
			}
			if (drv_params.sens_type == POS_SENSOR_TYPE_SSI){  //В типе датчика выбран ДПР цифровой с SSI интерфейсом
				cur_par.speed = SSI_Encoder.speed_elec; //Текущая скорость - с ДПР
				refs.theta_elec = SSI_Encoder.theta_elec; //текущее угловое положение - с ДПР
			}

			cur_par.Is = pid_iq.pid_fdb_reg3; //Текущий ток статора - ток по оси q

			//Преобразование токов из трехфазной системы координат в двухфазную
			clarke.as = adc.Imeas_a;
			clarke.bs = adc.Imeas_b;
			clarke.calc(&clarke);
			cur_par.Ialpha = clarke.ds;
			cur_par.Ibeta = clarke.qs;

			// задание скорости от задатчика интенсивности (ЗИ)
			rmp.input = refs.speed_ref;
			rmp.calc(&rmp);//Расчет ЗИ

			pid_spd.pid_ref_reg3 = rmp.output; //на воход регулятора скорости - выход ЗИ
			pid_spd.pid_fdb_reg3 = cur_par.speed; //обратная связь регулятора скорости - скорость ДПР


			pid_spd.pid_out_max = refs.Iq_ref; //Максимум выхода регулятора скорости - заданный ток

			if (sw.recuperation_ena & 1)            //если разрешена рекуперация
				pid_spd.pid_out_min = -refs.Iq_ref; //минимум рег. скор. - отрицательный ток
			else
				//иначе
				pid_spd.pid_out_min = 0; //нулевой ток не даст рекупераци при вращении в положительную сторону
			pid_spd.calc(&pid_spd); //расчет регулятора скорости


			cur_par.ThetaRefCurr = refs.theta_elec;
			refs.theta_elec &= 0x00FFFFFF; //Ограничение угла на 1.0 в формате 8.24

			//Поворот токов фаз на угол refs.theta_elec
			park.ds = clarke.ds;
			park.qs = clarke.qs;
			park.ang = refs.theta_elec;
			park.calc(&park); //расчет преобразования координат

			//задание рег. тока по оси q (оси момента) - выход регулятора скорости
			pid_iq.pid_ref_reg3 = pid_spd.pid_out_reg3;
			pid_iq.pid_fdb_reg3 = park.qe; //обратная связь рег. тока оси q - вычисленный ток по фазе q
			pid_iq.calc(&pid_iq); //расчет регулятора тока оси q

			pid_id.pid_ref_reg3 = 0; //задание тока по оси d - оси возбудения
			pid_id.pid_fdb_reg3 = park.de; //обратная связь рег. тока оси d - вычисленный ток по фазе d
			pid_id.calc(&pid_id); //расчет регулятора тока оси d
			SM_Ctrl_LimitCurrentRegulatorVector();

			//Обратный поворот на угол refs.theta_elec для 
			//выходов регуляторов тока (напряжений по осям d и q)
			ipark.de = pid_id.pid_out_reg3;
			ipark.qe = pid_iq.pid_out_reg3;
			ipark.ang = refs.theta_elec;
			ipark.calc(&ipark);            //расчет преобразования координат

			//Задание напряжение на модуль ШИМ после преобразования координат
			pwm.UalphaRef = ipark.ds;            //Ось альфа
			pwm.UbetaRef = ipark.qs;            //Ось бета
			pwm.update(&pwm);            //Расчет модуля ШИМ

			if (sw.excitation_ena)
			{
				pwm_ex.ref = refs.uf_ref;
				pwm_ex.calc(&pwm_ex);
			}

			/*DO END*/
		}
		break;
	}

	case CTRL_RUN_VECTOR_SM_ENCODER_POS: {            //Датчиковая СУ с энкодером и с контуром положения
		if (p->E == 1) {
			/*ENTRY START*/
			pwm.On(&pwm);            //Включение ШИМ
			rmp.output = 0;            //Обнуление задатчика интенсивности
			if (dlog.StartBits & (1 << 4)) //Если пользователь задал этот бит осциллографирования
				dlog.set_mode(1, &dlog);  //Запуск осциллографа по этому событию
			/*ENTRY END*/
		}
		/*DO START*/
		{
			cur_par.Is = pid_iq.pid_fdb_reg3; //Текущий ток статора - ток по оси q
			cur_par.speed = posspeedEqep.speed_elec; //Текущая скорость - с ДПР фильтрованная

			//Преобразование токов из трехфазной системы координат в двухфазную
			clarke.as = adc.Imeas_a;
			clarke.bs = adc.Imeas_b;
			clarke.calc(&clarke);
			cur_par.Ialpha = clarke.ds;
			cur_par.Ibeta = clarke.qs;


/*          //Для демо-режима контура положения сделаем задание положения, изменяющегося по синусу
			//Как быстро менять задание - пусть управляется из переменной для отладки DebugW1

			if (DebugW1!=0){//если какая-то скорость вращения задана
				p->AngleDemo+=(DebugW1>>10);//инкрементируем переменную
				if (p->AngleDemo>_IQ(2))//период равен 2, что в о.е. дает 360*2 градусов
					p->AngleDemo=0;

				//задание угла в контур положения
				//пусть будет синус от линейно увеличивающейся переменной, да еще и в 4 раза увеличим
				refs.theta_elec=_IQsinPU(p->AngleDemo)<<2;
			}
*/

			/*
//Другой демо-режим контура положения - задавать положение от двигателя датчика, абсолютного энкодера с интерфейсом SSI
//Для работы датчика его нужно, собственно, подключить (в комплект поставки не входит!),
//а также выбрать его в настройках

#define REVOLUTION_DROP_NUMBER 3
			//чтобы можно было вращать неограниченно в одну сторону и не переполнилась переменная по положению
			//но чтобы нельзя было накрутить 100500 оборотов вперед привода и он крутил бы дальше и дальше
			//ограничим максимальное расхождение в обототах числом REVOLUTION_DROP_NUMBER
			if (labs(SSI_Encoder.RevolutionCounter-posspeedEqep.RevolutionCounter)>REVOLUTION_DROP_NUMBER){
				if (posspeedEqep.RevolutionCounter>SSI_Encoder.RevolutionCounter){
					posspeedEqep.RevolutionCounter=SSI_Encoder.RevolutionCounter+REVOLUTION_DROP_NUMBER;
				}
				if (SSI_Encoder.RevolutionCounter>posspeedEqep.RevolutionCounter){
					SSI_Encoder.RevolutionCounter=posspeedEqep.RevolutionCounter+REVOLUTION_DROP_NUMBER;
				}
			}
			if ((posspeedEqep.RevolutionCounter>10) || (SSI_Encoder.RevolutionCounter>10)){
				posspeedEqep.RevolutionCounter-=10;
				SSI_Encoder.RevolutionCounter-=10;
			}
			if ((posspeedEqep.RevolutionCounter<-10) || (SSI_Encoder.RevolutionCounter<-10)){
				posspeedEqep.RevolutionCounter+=10;
				SSI_Encoder.RevolutionCounter+=10;
			}

			//задание угла - от SSI энкодера
			refs.theta_elec=SSI_Encoder.theta_elecContinouos;
*/


			//текущий угол с датчика положения, переведенный в электрический из механического
			cur_par.ThetaCurr = posspeedEqep.theta_elecContinouos;
			cur_par.ThetaRefCurr=refs.theta_elec;//заданный угол электрический (продублируем в эту переменную задание для удобства наблюдения)

			//ошибка - разница между заданием и текущим углом. Эта разница обычно считается внутри ПИД регулятора,
			//однако в данном случае нужно после расчета произвести ограничение, чтобы в регуляторе ничего не переполнилось
			long posRefError=refs.theta_elec - cur_par.ThetaCurr;


			//ограничим ошибку - чтобы при расчете регулятора внутри него не было переполнения.
			if (posRefError>_IQ(1))
				posRefError=_IQ(1);
			if (posRefError<-_IQ(1))
				posRefError=-_IQ(1);


			//обычно на ПИД регулятор подается задание и обратная связь, а внутри считается их разница.
			//но разницу мы посчитали выше (и ограничили). Потому пусть задание на регулятор будет всё время нулевым,
			//а в обратную связь подадим уже посчитанную и ограниченную ошибку (со знаком минус)
			pid_pos.pid_ref_reg3 = 0;//задавать задание на положение надо через refs.theta_elec (группа задания)
			pid_pos.pid_fdb_reg3 = -posRefError;//обратная связь
			pid_pos.pid_out_max=refs.speed_ref;//ограничение рег. положения - максимум задаваемой скорости
			pid_pos.pid_out_min=-refs.speed_ref;//и в минус
			pid_pos.saterr_reg3Add=pid_spd.saterr_reg3;//это улучшайзер. Учитывает насыщение рег. скорости и передает его в рег. положения.
			pid_pos.calc(&pid_pos); //расчет регулятора положения

			pid_spd.pid_ref_reg3 = pid_pos.pid_out_reg3; //на вход регулятора скорости - выход рег. положения
//			pid_spd.pid_fdb_reg3 = posspeedEqep.speed_filter.output; //обратная связь регулятора скорости - скорость ДПР
			pid_spd.pid_fdb_reg3 = cur_par.speed;

			pid_spd.pid_out_max = refs.Iq_ref; //Максимум выхода регулятора скорости - заданный ток
			pid_spd.pid_out_min = -refs.Iq_ref; //минимум рег. скор. - отрицательный ток
			pid_spd.calc(&pid_spd); //расчет регулятора скорости


			//Поворот токов фаз на угол refs.theta_elec
			park.ds = clarke.ds;
			park.qs = clarke.qs;
			park.ang = posspeedEqep.theta_elec;
			park.calc(&park); //расчет преобразования координат

			//задание рег. тока по оси q (оси момента) - выход регулятора скорости
			pid_iq.pid_ref_reg3 = pid_spd.pid_out_reg3;
			pid_iq.pid_fdb_reg3 = park.qe; //обратная связь рег. тока оси q - вычисленный ток по фазе q
			pid_iq.calc(&pid_iq); //расчет регулятора тока оси q

			pid_id.pid_ref_reg3 = 0; //задание тока по оси d - оси возбудения
			pid_id.pid_fdb_reg3 = park.de; //обратная связь рег. тока оси d - вычисленный ток по фазе d
			pid_id.calc(&pid_id); //расчет регулятора тока оси d
			SM_Ctrl_LimitCurrentRegulatorVector();

			//Обратный поворот на угол refs.theta_elec для
			//выходов регуляторов тока (напряжений по осям d и q)
			ipark.de = pid_id.pid_out_reg3;
			ipark.qe = pid_iq.pid_out_reg3;
			ipark.ang = posspeedEqep.theta_elec;
			ipark.calc(&ipark);            //расчет преобразования координат

			//Задание напряжение на модуль ШИМ после преобразования координат
			pwm.UalphaRef = ipark.ds;            //Ось альфа
			pwm.UbetaRef = ipark.qs;            //Ось бета
			pwm.update(&pwm);            //Расчет модуля ШИМ

			/*DO END*/
		}
		break;
	}


	case CTRL_RUN_VECTOR_IM_ENCODER: { //Датчиковая СУ для АД с энкодером и контуром скорости
		if (p->E == 1) {
			/*ENTRY START*/
			pwm.On(&pwm);            //Включение ШИМ
			RotorObserver.psi_d=0;
			RotorObserver.psi_q=0;
			rmp.output = 0;            //Обнуление задатчика интенсивности
			if (dlog.StartBits & (1 << 4)) //Если пользователь задал этот бит осциллографирования
				dlog.set_mode(1, &dlog);  //Запуск осциллографа по этому событию
			/*ENTRY END*/
		}
		/*DO START*/
		{
			cur_par.Is = _IQmag(pid_iq.pid_fdb_reg3,pid_id.pid_fdb_reg3) ; //Текущий ток статора - амплитуда из токов d и q
			cur_par.speed = posspeedEqep.speed_elec; //posspeedEqep.speed.output;//Текущая скорость - с ДПР фильтрованная

			//Преобразование токов из трехфазной системы координат в двухфазную
			clarke.as = adc.Imeas_a;
			clarke.bs = adc.Imeas_b;
			clarke.calc(&clarke);
			cur_par.Ialpha = clarke.ds;
			cur_par.Ibeta = clarke.qs;

			// задание скорости от задатчика интенсивности (ЗИ)
			rmp.input = refs.speed_ref;
			rmp.calc(&rmp);//Расчет ЗИ

			pid_spd.pid_ref_reg3 = rmp.output; //на воход регулятора скорости - выход ЗИ
			pid_spd.pid_fdb_reg3 = posspeedEqep.speed_elec; //обратная связь регулятора скорости - скорость ДПР


			pid_spd.pid_out_max = refs.Iq_ref; //Максимум выхода регулятора скорости - заданный ток

			if (sw.recuperation_ena & 1)            //если разрешена рекуперация
				pid_spd.pid_out_min = -refs.Iq_ref; //минимум рег. скор. - отрицательный ток
			else
				//иначе
				pid_spd.pid_out_min = 0; //нулевой ток не даст рекупераци при вращении в положительную сторону
			pid_spd.calc(&pid_spd); //расчет регулятора скорости


			//Поворот токов фаз на угол refs.theta_elec
			park.ds = clarke.ds;
			park.qs = clarke.qs;
			park.ang = posspeedEqep.theta_elec;
			park.calc(&park); //расчет преобразования координат

			RotorObserver.id=park.de;//на наблюдатель ротора АД - токи статора, повернутый на угол ДПРа
			RotorObserver.iq=park.qe;
			RotorObserver.calc(&RotorObserver);

			refs.theta_elec = RotorObserver.theta_psi_elec+posspeedEqep.theta_elec; //текущее угловое положение - с ДПР и наблюдателя ротора (оси x,y)
			refs.theta_elec &= 0x00FFFFFF; //Ограничение угла на 1.0 в формате 8.24
			cur_par.ThetaRefCurr = refs.theta_elec;


			//Теперь refs.theta_elec - угол потокосцепления ротора. Повторяем векторную систему от синхронной машины для этого угла
			park.ds = clarke.ds;
			park.qs = clarke.qs;
			//второе слагаемое - доворот для устойчивости регуляторов на высокой скорости. Дублироваться второй раз для ipark не должен!
			park.ang = refs.theta_elec;
			park.calc(&park);

			//регуляторы токов d, q используются для регулирования осей x, y (чтобы не делать разных названий)
			//задание рег. тока по оси y (оси момента) - выход регулятора скорости
			pid_iq.pid_ref_reg3 = pid_spd.pid_out_reg3;
			pid_iq.pid_fdb_reg3 = park.qe; //обратная связь рег. тока оси y - вычисленный ток по фазе y
			pid_iq.calc(&pid_iq); //расчет регулятора тока оси y

			pid_id.pid_ref_reg3 = _IQmpy(RotorObserver.FluxCurrentRatio,labs(pid_iq.pid_ref_reg3)); //задание тока по оси x - оси возбудения. Пропорционально амплитуде задания тока статора
			if (pid_id.pid_ref_reg3 < RotorObserver.FluxCurrentMin)
				pid_id.pid_ref_reg3=RotorObserver.FluxCurrentMin;
			pid_id.pid_fdb_reg3 = park.de; //обратная связь рег. тока оси x - вычисленный ток по фазе x
			pid_id.calc(&pid_id); //расчет регулятора тока оси x
			SM_Ctrl_LimitCurrentRegulatorVector();


			//Обратный поворот на угол refs.theta_elec для
			//выходов регуляторов тока (напряжений по осям x и y)
			ipark.de = pid_id.pid_out_reg3;
			ipark.qe = pid_iq.pid_out_reg3;
			ipark.ang = refs.theta_elec;
			ipark.calc(&ipark);            //расчет преобразования координат

			//Задание напряжение на модуль ШИМ после преобразования координат
			pwm.UalphaRef = ipark.ds;            //Ось альфа
			pwm.UbetaRef = ipark.qs;            //Ось бета
			pwm.update(&pwm);            //Расчет модуля ШИМ

			/*DO END*/
		}
		break;
	}

	case CTRL_RUN_VECTOR_IM_ENCODER_POS: { //Датчиковая СУ для АД с энкодером и контурами скорости и положения
		if (p->E == 1) {
			/*ENTRY START*/
			pwm.On(&pwm);            //Включение ШИМ
			RotorObserver.psi_d=0;
			RotorObserver.psi_q=0;
			rmp.output = 0;            //Обнуление задатчика интенсивности
			if (dlog.StartBits & (1 << 4)) //Если пользователь задал этот бит осциллографирования
				dlog.set_mode(1, &dlog);  //Запуск осциллографа по этому событию
			/*ENTRY END*/
		}
		/*DO START*/
		{
			cur_par.Is = _IQmag(pid_iq.pid_fdb_reg3,pid_id.pid_fdb_reg3) ; //Текущий ток статора - амплитуда из токов d и q
			cur_par.speed = posspeedEqep.speed_elec; //posspeedEqep.speed.output;//Текущая скорость - с ДПР фильтрованная

			//Преобразование токов из трехфазной системы координат в двухфазную
			clarke.as = adc.Imeas_a;
			clarke.bs = adc.Imeas_b;
			clarke.calc(&clarke);
			cur_par.Ialpha = clarke.ds;
			cur_par.Ibeta = clarke.qs;


/*          //Для демо-режима контура положения сделаем задание положения, изменяющегося по синусу
			//Как быстро менять задание - пусть управляется из переменной для отладки DebugW1

			if (DebugW1!=0){//если какая-то скорость вращения задана
				p->AngleDemo+=(DebugW1>>10);//инкрементируем переменную
				if (p->AngleDemo>_IQ(2))//период равен 2, что в о.е. дает 360*2 градусов
					p->AngleDemo=0;

				//задание угла в контур положения
				//пусть будет синус от линейно увеличивающейся переменной, да еще и в 4 раза увеличим
				refs.theta_elec=_IQsinPU(p->AngleDemo)<<2;
			}
*/

			/*
//Другой демо-режим контура положения - задавать положение от двигателя датчика, абсолютного энкодера с интерфейсом SSI
//Для работы датчика его нужно, собственно, подключить (в комплект поставки не входит!),
//а также выбрать его в настройках

#define REVOLUTION_DROP_NUMBER 3
			//чтобы можно было вращать неограниченно в одну сторону и не переполнилась переменная по положению
			//но чтобы нельзя было накрутить 100500 оборотов вперед привода и он крутил бы дальше и дальше
			//ограничим максимальное расхождение в обототах числом REVOLUTION_DROP_NUMBER
			if (labs(SSI_Encoder.RevolutionCounter-posspeedEqep.RevolutionCounter)>REVOLUTION_DROP_NUMBER){
				if (posspeedEqep.RevolutionCounter>SSI_Encoder.RevolutionCounter){
					posspeedEqep.RevolutionCounter=SSI_Encoder.RevolutionCounter+REVOLUTION_DROP_NUMBER;
				}
				if (SSI_Encoder.RevolutionCounter>posspeedEqep.RevolutionCounter){
					SSI_Encoder.RevolutionCounter=posspeedEqep.RevolutionCounter+REVOLUTION_DROP_NUMBER;
				}
			}
			if ((posspeedEqep.RevolutionCounter>10) || (SSI_Encoder.RevolutionCounter>10)){
				posspeedEqep.RevolutionCounter-=10;
				SSI_Encoder.RevolutionCounter-=10;
			}
			if ((posspeedEqep.RevolutionCounter<-10) || (SSI_Encoder.RevolutionCounter<-10)){
				posspeedEqep.RevolutionCounter+=10;
				SSI_Encoder.RevolutionCounter+=10;
			}

			//задание угла - от SSI энкодера
			refs.theta_elec=SSI_Encoder.theta_elecContinouos;

			refs.theta_elec=adc.ref_meas<<2; //задание положения от потенциометра
*/



			//текущий угол с датчика положения, переведенный в электрический из механического
			cur_par.ThetaCurr = posspeedEqep.theta_elecContinouos;
			cur_par.ThetaRefCurr=refs.theta_elec;//заданный угол электрический (продублируем в эту переменную задание для удобства наблюдения)

			//ошибка - разница между заданием и текущим углом. Эта разница обычно считается внутри ПИД регулятора,
			//однако в данном случае нужно после расчета произвести ограничение, чтобы в регуляторе ничего не переполнилось
			long posRefError=refs.theta_elec - cur_par.ThetaCurr;


			//ограничим ошибку - чтобы при расчете регулятора внутри него не было переполнения.
			if (posRefError>_IQ(1))
				posRefError=_IQ(1);
			if (posRefError<-_IQ(1))
				posRefError=-_IQ(1);


			//обычно на ПИД регулятор подается задание и обратная связь, а внутри считается их разница.
			//но разницу мы посчитали выше (и ограничили). Потому пусть задание на регулятор будет всё время нулевым,
			//а в обратную связь подадим уже посчитанную и ограниченную ошибку (со знаком минус)
			pid_pos.pid_ref_reg3 = 0;//задавать задание на положение надо через Temp1 (группа задания)
			pid_pos.pid_fdb_reg3 = -posRefError;//обратная связь
			pid_pos.pid_out_max=refs.speed_ref;//ограничение рег. положения - максимум задаваемой скорости
			pid_pos.pid_out_min=-refs.speed_ref;//и в минус
			pid_pos.saterr_reg3Add=pid_spd.saterr_reg3;//это улучшайзер. Учитывает насыщение рег. скорости и передает его в рег. положения.
			pid_pos.calc(&pid_pos); //расчет регулятора положения

			pid_spd.pid_ref_reg3 = pid_pos.pid_out_reg3; //на вход регулятора скорости - выход рег. положения
//			pid_spd.pid_fdb_reg3 = posspeedEqep.speed_filter.output; //обратная связь регулятора скорости - скорость ДПР
			pid_spd.pid_fdb_reg3 = cur_par.speed;

			pid_spd.pid_out_max = refs.Iq_ref; //Максимум выхода регулятора скорости - заданный ток
			pid_spd.pid_out_min = -refs.Iq_ref; //минимум рег. скор. - отрицательный ток
			pid_spd.calc(&pid_spd); //расчет регулятора скорости


			//Поворот токов фаз на угол Temp1
			park.ds = clarke.ds;
			park.qs = clarke.qs;
			park.ang = posspeedEqep.theta_elec;
			park.calc(&park); //расчет преобразования координат

			RotorObserver.id=park.de;//на наблюдатель ротора АД - токи статора, повернутый на угол ДПРа
			RotorObserver.iq=park.qe;
			RotorObserver.calc(&RotorObserver);

			Temp1 = RotorObserver.theta_psi_elec+posspeedEqep.theta_elec; //текущее угловое положение - с ДПР и наблюдателя ротора (оси x,y)
			Temp1 &= 0x00FFFFFF; //Ограничение угла на 1.0 в формате 8.24
			cur_par.ThetaRefCurr = Temp1;


			//Теперь Temp1 - угол потокосцепления ротора. Повторяем векторную систему от синхронной машины для этого угла
			park.ds = clarke.ds;
			park.qs = clarke.qs;
			//второе слагаемое - доворот для устойчивости регуляторов на высокой скорости. Дублироваться второй раз для ipark не должен!
			park.ang = Temp1;
			park.calc(&park);

			//регуляторы токов d, q используются для регулирования осей x, y (чтобы не делать разных названий)
			//задание рег. тока по оси y (оси момента) - выход регулятора скорости
			pid_iq.pid_ref_reg3 = pid_spd.pid_out_reg3;
			pid_iq.pid_fdb_reg3 = park.qe; //обратная связь рег. тока оси y - вычисленный ток по фазе y
			pid_iq.calc(&pid_iq); //расчет регулятора тока оси y

			pid_id.pid_ref_reg3 = _IQmpy(RotorObserver.FluxCurrentRatio,labs(pid_iq.pid_ref_reg3)); //задание тока по оси x - оси возбудения. Пропорционально амплитуде задания тока статора
			if (pid_id.pid_ref_reg3 < RotorObserver.FluxCurrentMin)
				pid_id.pid_ref_reg3=RotorObserver.FluxCurrentMin;
			pid_id.pid_fdb_reg3 = park.de; //обратная связь рег. тока оси x - вычисленный ток по фазе x
			pid_id.calc(&pid_id); //расчет регулятора тока оси x
			SM_Ctrl_LimitCurrentRegulatorVector();


			//Обратный поворот на угол Temp1 для
			//выходов регуляторов тока (напряжений по осям x и y)
			ipark.de = pid_id.pid_out_reg3;
			ipark.qe = pid_iq.pid_out_reg3;
			ipark.ang = Temp1;
			ipark.calc(&ipark);            //расчет преобразования координат

			//Задание напряжение на модуль ШИМ после преобразования координат
			pwm.UalphaRef = ipark.ds;            //Ось альфа
			pwm.UbetaRef = ipark.qs;            //Ось бета
			pwm.update(&pwm);            //Расчет модуля ШИМ

			/*DO END*/
		}
		break;
	}
	case CTRL_DC_DRIVE: {  // ДПТ разомкнутый
		if (p->E == 1) {
			/*ENTRY START*/
			pwm.On(&pwm);
			rmp.output = 0;
			if (dlog.StartBits & (1 << 3))
				dlog.set_mode(1, &dlog);
			/*ENTRY END*/
		}

		pwm.UalphaRef = refs.ua_ref;
		pwm.UbetaRef = refs.uf_ref;
		pwm.update(&pwm);

		break;
	}

	case CTRL_DC_DRIVE_FB: {  // ДПТ замкнутый
		if (p->E == 1) {
			/*ENTRY START*/
			pwm.On(&pwm);
			rmp.output = 0;
			if (dlog.StartBits & (1 << 3))
				dlog.set_mode(1, &dlog);
			/*ENTRY END*/
		}

		pid_spd.pid_ref_reg3 = refs.DCMspeed_ref; //задание
		pid_spd.pid_fdb_reg3 = posspeedEqep.speed_elec; //обратная связь регулятора скорости - скорость ДПР
/*
		pid_spd.pid_out_max = sm_prot.Imax_protect; //Максимум выхода регулятора скорости - заданный ток
		if (sw.recuperation_ena & 1)            //если разрешена рекуперация
			pid_spd.pid_out_min = -sm_prot.Imax_protect; //минимум рег. скор. - отрицательный ток
		else
			pid_spd.pid_out_min = 0; //нулевой ток не даст рекупераци при вращении в положительную сторону
*/
		pid_spd.calc(&pid_spd); //расчет регулятора скорости

		pid_ia.pid_ref_reg3 = pid_spd.pid_out_reg3;
		pid_ia.pid_fdb_reg3 = adc.Imeas_a; //обратная связь рег. тока оси y - вычисленный ток по фазе y
		pid_ia.calc(&pid_ia); //расчет регулятора тока оси y

		pwm.UalphaRef = pid_ia.pid_out_reg3;
		pwm.UbetaRef = refs.uf_ref;
		pwm.update(&pwm);

		break;
	}


	case CTRL_AUTO_SENSOR_TUNING: {  //Авто настройка ДПР
		if (p->E == 1) {
			/*ENTRY START*/
			pwm.On(&pwm);
			rmp.output = 0;
			DPReCAP.AngleOffset = 0;
			if (dlog.StartBits & (1 << 3))
				dlog.set_mode(1, &dlog);
			/*ENTRY END*/
		}
		/*DO START*/
		{
			//по структуре это тоже самое, что частотно-токовая система управления
			cur_par.Is = pid_id.pid_fdb_reg3;
			cur_par.speed = rmp.output;

			clarke.as = adc.Imeas_a;
			clarke.bs = adc.Imeas_b;
			clarke.calc(&clarke);
			cur_par.Ialpha = clarke.ds;
			cur_par.Ibeta = clarke.qs;

			rmp.input = refs.speed_ref;
			rmp.calc(&rmp);
			refs.theta_elec += _IQmpy(
					_IQmpyI32(drv_params.freq_nom,FAST_CALC_TS), rmp.output);
			cur_par.ThetaRefCurr = refs.theta_elec;
			refs.theta_elec &= 0x00FFFFFF;

			park.ds = clarke.ds;
			park.qs = clarke.qs;
			park.ang = refs.theta_elec;
			park.calc(&park);

			pid_iq.pid_ref_reg3 = 0;
			pid_iq.pid_fdb_reg3 = park.qe;
			pid_iq.calc(&pid_iq);

			pid_id.pid_ref_reg3 = refs.i_flux_ref;
			pid_id.pid_fdb_reg3 = park.de;
			pid_id.calc(&pid_id);
			SM_Ctrl_LimitCurrentRegulatorVector();

			ipark.de = pid_id.pid_out_reg3;
			ipark.qe = pid_iq.pid_out_reg3;
			ipark.ang = refs.theta_elec;
			ipark.calc(&ipark);

			pwm.UalphaRef = ipark.ds;
			pwm.UbetaRef = ipark.qs;

			pwm.update(&pwm);

			//автоматическая настрока смещения для датчика Холла
			if (p->AnglePrev != DPReCAP.Angle6) {
				if (((DPReCAP.Angle6 == _IQ(5.0 / 6.0)) && (p->AnglePrev == 0))
						|| ((p->AnglePrev == _IQ(5.0 / 6.0))
								&& (DPReCAP.Angle6 == 0))) {
					DPReCAP.AngleOffset =
							DPReCAP.AngleOffset
									+ _IQmpy(_IQ(0.5),
											((refs.theta_elec-DPReCAP.AngleOffset+_IQ(0.5))&0x00FFFFFF)-_IQ(0.5));
					DPReCAP.AngleOffset &= 0x00FFFFFF;
				}
				p->AnglePrev = DPReCAP.Angle6;
			}

			//автоматическая настрока смещения для энкодера
			posspeedEqep.AngleOffset =
					posspeedEqep.AngleOffset
							+ _IQmpy(_IQ(0.0001),
									((refs.theta_elec-posspeedEqep.theta_elec+_IQ(0.5))&0x00FFFFFF)-_IQ(0.5));
			posspeedEqep.AngleOffset &= 0x00FFFFFF;
			/*DO END*/
		}
		break;
	}

	case CTRL_RUN_SRM_AUTOCOMMUTATION: { //Режим автокоммутации SRM
		if (p->E == 1) {
			/*ENTRY START*/
			pwm.On(&pwm);	//влючение ШИМ
			rmp.output = 0;
			if (dlog.StartBits & (1 << 3))
				dlog.set_mode(1, &dlog);
			/*ENTRY END*/
		}
		/*DO START*/
		{
			cur_par.speed = DPReCAP.speed;	//Текущая скорость - с ДПР

			// задание скорости от задатчика интенсивности (ЗИ)
			rmp.input = refs.speed_ref;
			rmp.calc(&rmp);	//Расчет ЗИ

			pid_spd.pid_ref_reg3 = rmp.output;//на вход регулятора скорости - выход ЗИ
			pid_spd.pid_fdb_reg3 = DPReCAP.speed;//обратная связь регулятора скорости - скорость ДПР Холла

			pid_spd.pid_out_max = refs.Iq_ref;//Максимум выхода регулятора скорости - заданный ток
			pid_spd.pid_out_min = 0; //Минимум выхода регулятора скорости - 0
			pid_spd.calc(&pid_spd); //расчет регулятора скорости

			//выдача задания тока в фазы с положительным моментом, исходя из угла
			switch (DPReCAP.Angle6)
			{
				case _IQ(0.0 / 6.0):
				{
					i_a_reg.ref = pid_spd.pid_out_reg3;
					i_b_reg.ref = _IQ(-1);
					i_c_reg.ref = pid_spd.pid_out_reg3;
					break;
				}
				case _IQ(1.0 / 6.0):
				{
					i_a_reg.ref = pid_spd.pid_out_reg3;
					i_b_reg.ref = _IQ(-1);
					i_c_reg.ref = _IQ(-1);
					break;
				}
				case _IQ(2.0 / 6.0):
				{
					i_a_reg.ref = pid_spd.pid_out_reg3;
					i_b_reg.ref = pid_spd.pid_out_reg3;
					i_c_reg.ref = _IQ(-1);
					break;
				}
				case _IQ(3.0 / 6.0):
				{
					i_a_reg.ref = _IQ(-1);
					i_b_reg.ref = pid_spd.pid_out_reg3;
					i_c_reg.ref = _IQ(-1);
					break;
				}
				case _IQ(4.0 / 6.0):
				{
					i_a_reg.ref = _IQ(-1);
					i_b_reg.ref = pid_spd.pid_out_reg3;
					i_c_reg.ref = pid_spd.pid_out_reg3;
					break;
				}
				default:
				{
					i_a_reg.ref = _IQ(-1);
					i_b_reg.ref = _IQ(-1);
					i_c_reg.ref = pid_spd.pid_out_reg3;
					break;
				}
			}

			i_a_reg.fdb = adc.Imeas_a;
			i_b_reg.fdb = adc.Imeas_b;
			i_c_reg.fdb = adc.Imeas_c;

			i_a_reg.calc(&i_a_reg);
			i_b_reg.calc(&i_b_reg);
			i_c_reg.calc(&i_c_reg);

			pwm.UPhARef = i_a_reg.out;
			pwm.UPhBRef = i_b_reg.out;
			pwm.UPhCRef = i_c_reg.out;
			pwm.update(&pwm);

			/*DO END*/
		}
		break;
	}

	default: {
		break;
	}
	}

}

/*@}*/

