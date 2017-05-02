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

 \file      V_MotorModel.с
 \brief     Модуль моделей электродвигателей (см. TMotorModel)
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2017

 \addtogroup V_MotorModel
 @{ */


#include "V_MotorModel.h"
#include "math.h"
#include "V_MotorParams.h"
#include <V_IQmath.h>
#include "main.h"

#define ADC_MAX_VALUE 4095
#define ADC_HALF_VALUE 2047

//функция случайного "шума" для АЦП
int MotorModel_fastrand() {
	static unsigned long g_seed;
	g_seed = (214013 * g_seed + 2531011);
	return ((g_seed >> 16) & 0x3) -2;
}

//инициализация переменных модели
void MotorModel_Init(TMotorModel *p) {
	Uint16 MotorParametersValid=0;
	int i=0;

	p->hallSensor = 1;		//инициализация датчика Холла при нулевом угле
	p->motorInternals.dt = p->dt;			//длительность мертвого времени
	p->motorInternals.tpr = p->tpr;		//период ШИМ
	p->motorInternals.cmpr0 = p->motorInternals.tpr-p->cmpr0;	//уставки сравнения ШИМ. С инверсией, потому что так настроен модуль ШИМ
	p->motorInternals.cmpr1 = p->motorInternals.tpr-p->cmpr1;
	p->motorInternals.cmpr2 = p->motorInternals.tpr-p->cmpr2;
	p->motorInternals.cmpr3 = p->motorInternals.tpr-p->cmpr3;
	//шаг дискретизации модели (считается в прерывании 10кГц)
	p->motorInternals.t = 1.0 / 10000;

	p->motorInternals.cos30 = cos(30.0/360*MOTOR_MODEL_PI*2);
	p->fault = 0;				//код ошибки модели
	p->motorInternals.udSenseK = ADC_HALF_VALUE/540.0;
	p->motorInternals.type=p->MotorType&0x7;//передаем тип двигателя вниз в структуру параметров двигателя

	// Поиск макcимального номера набора параметров двигателя из массива
	// Нужно для защиты от введения номера несуществующего элемента массива
	// НАЧАЛО //
	if (p->motorInternals.type==MODEL_INDUCTION_MOTOR){
			// Если r_s, l_s, j одновременно равны нулю, значит наткнулись на нулевой набор параметров.
			// Значит предыдущий набор - последний рабочий в массиве, и номер больше, чем у него выбирать нельзя.
			while ((IMDATA[i].r_s != 0) && (IMDATA[i].l_s != 0) && (IMDATA[i].j != 0) && (i < 50)) i++;}
	else if (p->motorInternals.type==MODEL_SYNC_MOTOR){
			// Если l_sd, l_sq, j одновременно равны нулю, значит наткнулись на нулевой набор параметров.
			// Значит предыдущий набор - последний рабочий в массиве, и номер больше, чем у него выбирать нельзя.
			while ((SMDATA[i].l_sd != 0) && (SMDATA[i].l_sq != 0) && (SMDATA[i].j != 0) && (i < 50)) i++;}
	else if (p->motorInternals.type==MODEL_DC_MOTOR){
			// Если r_a, l_a, j одновременно равны нулю, значит наткнулись на нулевой набор параметров.
			// Значит предыдущий набор - последний рабочий в массиве, и номер больше, чем у него выбирать нельзя.
			while ((DCMDATA[i].r_a != 0) && (DCMDATA[i].l_a != 0) && (DCMDATA[i].j != 0) && (i < 50)) i++;}
	else if (p->motorInternals.type==MODEL_SRD_MOTOR){
			// Если r_s, pp, j одновременно равны нулю, значит наткнулись на нулевой набор параметров.
			// Значит предыдущий набор - последний рабочий в массиве, и номер больше, чем у него выбирать нельзя.
			while ((SRMDATA[i].r_s != 0) && (SRMDATA[i].pp != 0) && (SRMDATA[i].j != 0) && (i < 50)) i++;}
	p->MotorParametersNumMax = i;
	// КОНЕЦ //

	// проверка валидности введенного номера набора параметров
	// НАЧАЛО //
	if ((p->MotorParametersNum > 0) && (p->MotorParametersNum <= p->MotorParametersNumMax)) MotorParametersValid = 1;
	else
	{
		if (p->MotorParametersNum > p->MotorParametersNumMax){
			p->MotorParametersNum = p->MotorParametersNumMax;
			MotorParametersValid = 1;}
		else if (p->MotorParametersNum == 0) MotorParametersValid = 0;
	}
	// КОНЕЦ //

	// Инициализация параметров двигателей
	if (p->motorInternals.type==MODEL_INDUCTION_MOTOR){//тип двигателя - асинхронный
		if (MotorParametersValid){
			p->motorInternals.ls = IMDATA[p->MotorParametersNum-1].l_s;		//индуктивность статора
			p->motorInternals.lr = IMDATA[p->MotorParametersNum-1].l_r;		//индуктивность ротора
			p->motorInternals.lm = IMDATA[p->MotorParametersNum-1].l_m;		//взаимная индуктивность
			//Внимание: в параметрах ls и lr должна быть полная индуктивность статора и ротора. Иногда в параметрах двигателя приводятся индуктивности рассеяния
			//статора и ротора, которые меньше, чем l_m. Чтобы получить полную индуктивность статора или ротора, нужно прибавить l_m к l_s или l_r соответственно.
			//Проверим правильность ввода индуктивностей:
			if ((p->motorInternals.ls < p->motorInternals.lm) || (p->motorInternals.lr < p->motorInternals.lm))
				p->fault = MODEL_WRONG_PARAMETERS;
			p->motorInternals.rs = IMDATA[p->MotorParametersNum-1].r_s;				//сопротивление статора
			p->motorInternals.rr = IMDATA[p->MotorParametersNum-1].r_r;				//сопротивление ротора
			p->motorInternals.pp = IMDATA[p->MotorParametersNum-1].pp;				//число пар полюсов
			p->motorInternals.j = IMDATA[p->MotorParametersNum-1].j;				//момент инерции
			p->motorInternals.QEPResolution = IMDATA[p->MotorParametersNum-1].qep;	//разрешение энкодера
			p->motorInternals.RatedPower = IMDATA[p->MotorParametersNum-1].RatedPower; //номинальная мощность (справочная величина, не используется в расчетах)
			p->motorInternals.RatedSpeed = IMDATA[p->MotorParametersNum-1].RatedSpeed; //номинальная скорость (справочная величина, не используется в расчетах)
			p->motorInternals.RatedCurrent = IMDATA[p->MotorParametersNum-1].RatedCurrent; //номинальный действующий ток
		}

		// Инициализация коэффициентов и расчетных величин, завязанных на параметры двигателя
		p->motorInternals.MechLoss = p->motorInternals.j * 10; 		//механические потери
		p->motorInternals.speedK = 60/(4*MOTOR_MODEL_PI); //коэффициент датчика скорости

		p->motorInternals.iADCk = p->motorInternals.RatedCurrent*1.414*3; 			// Расчетный коэффициент АЦП равен троекратной амплитуде номинального тока
		p->motorInternals.iSenseK = (ADC_HALF_VALUE*1.0)/p->motorInternals.iADCk;	//коэффициент датчика тока для преобразования тока в амперах в код АЦП

		p->motorInternals.ks = -(p->motorInternals.lr/1000.0) / (p->motorInternals.lm/1000.0 * p->motorInternals.lm/1000.0 - p->motorInternals.ls/1000.0 * p->motorInternals.lr/1000.0);	//коэффициенты обратной матрицы индуктивностей
		p->motorInternals.kr = -(p->motorInternals.ls/1000.0) / (p->motorInternals.lm/1000.0 * p->motorInternals.lm/1000.0 - p->motorInternals.ls/1000.0 * p->motorInternals.lr/1000.0);
		p->motorInternals.km = (p->motorInternals.lm/1000.0) / (p->motorInternals.lm/1000.0 * p->motorInternals.lm/1000.0 - p->motorInternals.ls/1000.0 * p->motorInternals.lr/1000.0);

		p->motorInternals.psa = 0;				//потокосцепления статора в осях альфа,бета
		p->motorInternals.psb = 0;
	}

	if (p->motorInternals.type==MODEL_SYNC_MOTOR){//тип двигателя - синхронный
		if (MotorParametersValid){
			p->motorInternals.r_f = SMDATA[p->MotorParametersNum-1].r_f;			//сопротивление ОВ
			p->motorInternals.l_f = SMDATA[p->MotorParametersNum-1].l_f;			//индуктивность ОВ
			p->motorInternals.l_m = SMDATA[p->MotorParametersNum-1].l_m;			//коэффициент магнитного потока (взаимная индуктивность)
			p->motorInternals.rs = SMDATA[p->MotorParametersNum-1].r_s;				//сопротивление статора
			p->motorInternals.lsd = SMDATA[p->MotorParametersNum-1].l_sd;			//индуктивность статора
			p->motorInternals.lsq = SMDATA[p->MotorParametersNum-1].l_sq;			//индуктивность статора
			p->motorInternals.pp = SMDATA[p->MotorParametersNum-1].pp;				//число пар полюсов
			p->motorInternals.j = SMDATA[p->MotorParametersNum-1].j;				//момент инерции
			p->motorInternals.QEPResolution = SMDATA[p->MotorParametersNum-1].qep;	//разрешение энкодера
			p->motorInternals.m = SMDATA[p->MotorParametersNum-1].psi_pm;			//потокосцепление ротора равно потоку постоянных магнитов
			p->motorInternals.RatedPower = SMDATA[p->MotorParametersNum-1].RatedPower; //номинальная мощность (справочная величина, не используется в расчетах)
			p->motorInternals.RatedSpeed = SMDATA[p->MotorParametersNum-1].RatedSpeed; //номинальная скорость (справочная величина, не используется в расчетах)
			p->motorInternals.RatedCurrent = SMDATA[p->MotorParametersNum-1].RatedCurrent; //номинальный действующий ток
			p->motorInternals.RatedFluxCurrent = SMDATA[p->MotorParametersNum-1].RatedFluxCurrent; //номинальный ток ОВ

		}
		// Инициализация коэффициентов и расчетных величин, завязанных на параметры двигателя
		p->motorInternals.MechLoss = p->motorInternals.j * 10; 		//механические потери
		p->motorInternals.speedK = 60/(4*MOTOR_MODEL_PI); //коэффициент датчика скорости

		p->motorInternals.iADCk = p->motorInternals.RatedCurrent*1.414*3; 			// Расчетный коэффициент АЦП равен троекратной амплитуде номинального тока
		p->motorInternals.iSenseK = (ADC_HALF_VALUE*1.0)/p->motorInternals.iADCk;	//коэффициент датчика тока для преобразования тока в амперах в код АЦП

		p->motorInternals.ifADCk = p->motorInternals.RatedFluxCurrent*3; 			// Расчетный коэффициент АЦП равен троекратному номинальному току ОВ
		p->motorInternals.ifSenseK = (ADC_HALF_VALUE*1.0)/p->motorInternals.ifADCk;	//коэффициент датчика тока для преобразования тока в амперах в код АЦП

		p->motorInternals._1_lsd = 1.0/(p->motorInternals.lsd/1000);			//обратная величина
		p->motorInternals._1_lsq = 1.0/(p->motorInternals.lsq/1000);			//обратная величина

		if (p->motorInternals.l_f != 0)	//С обмоткой возбуждения
		{
			p->motorInternals._1_l_f = 1.0/(p->motorInternals.l_f/1000); // Обратная величина (переведенная из мГн в Гн)
			p->motorInternals.syncm_pm = 0;
			p->motorInternals.m = 0;			//потокосцепление ротора равно нулю
		}
		else							//С постоянными магнитами
		{
			p->motorInternals.syncm_pm = 1;
			p->motorInternals.ifADCk = 0; // Расчетный коэффициент АЦП тока ОВ
		}

		p->motorInternals.psd = p->motorInternals.m;//потокосцепления статора в осях d,q
		p->motorInternals.psq = 0;
		p->motorInternals.i_f = 0;
	}

	if (p->motorInternals.type==MODEL_DC_MOTOR){//тип двигателя - коллекторный двигатель постоянного тока
		if (MotorParametersValid){
			p->motorInternals.pp = 2;				//число пар полюсов (параметр не нужен, просто чтобы не мусор)
			p->motorInternals.r_f = DCMDATA[p->MotorParametersNum-1].r_f;			//сопротивление ОВ
			p->motorInternals.l_f = DCMDATA[p->MotorParametersNum-1].l_f;			//индуктивность ОВ
			p->motorInternals.l_m = DCMDATA[p->MotorParametersNum-1].l_m;			//коэффициент магнитного потока (взаимная индуктивность)
			p->motorInternals.r_a = DCMDATA[p->MotorParametersNum-1].r_a;			//сопротивление якоря
			p->motorInternals.l_a = DCMDATA[p->MotorParametersNum-1].l_a;			//индуктивность якоря
			p->motorInternals.RatedFluxCurrent = DCMDATA[p->MotorParametersNum-1].RatedFluxCurrent;			//Номинальный ток ОВ
			p->motorInternals.RatedCurrent = DCMDATA[p->MotorParametersNum-1].RatedCurrent;			//Номинальный ток якоря
			p->motorInternals.j = DCMDATA[p->MotorParametersNum-1].j;				//момент инерции
			p->motorInternals.QEPResolution = DCMDATA[p->MotorParametersNum-1].qep;	//разрешение энкодера
			p->motorInternals.RatedPower = DCMDATA[p->MotorParametersNum-1].RatedPower; //номинальная мощность (справочная величина, не используется в расчетах)
			p->motorInternals.RatedSpeed = DCMDATA[p->MotorParametersNum-1].RatedSpeed; //номинальная скорость (справочная величина, не используется в расчетах)

		}
		// Инициализация коэффициентов и расчетных величин, завязанных на параметры двигателя
		p->motorInternals.MechLoss = p->motorInternals.j * 10; 					//механические потери
		p->motorInternals.speedK = 60/(4*MOTOR_MODEL_PI); //коэффициент датчика скорости
		p->motorInternals.iADCk = (ADC_HALF_VALUE*1.0)/p->motorInternals.iSenseK; // Расчетный коэффициент АЦП тока Оя

		p->motorInternals.iADCk = p->motorInternals.RatedCurrent*1.414*3; 			// Расчетный коэффициент АЦП равен троекратной амплитуде номинального тока
		p->motorInternals.iSenseK = (ADC_HALF_VALUE*1.0)/p->motorInternals.iADCk;	//коэффициент датчика тока для преобразования тока в амперах в код АЦП

		p->motorInternals.ifADCk = p->motorInternals.RatedFluxCurrent*3; 			// Расчетный коэффициент АЦП равен троекратному номинальному току ОВ
		p->motorInternals.ifSenseK = (ADC_MAX_VALUE*1.0)/p->motorInternals.ifADCk;	//коэффициент датчика тока для преобразования тока в амперах в код АЦП

		p->motorInternals.r_ad = 0;

		p->motorInternals._1_l_a = 1.0/(p->motorInternals.l_a/1000); // Обратная величина (переведенная из мГн в Гн)
		if (p->motorInternals.l_f != 0)	// если ДПТ с обмоткой возбуждения
		{
			// Расчитываем необходимые коэффициенты для работы ОВ
			p->motorInternals._1_l_f = 1.0/(p->motorInternals.l_f/1000); // Обратная величина (переведенная из мГн в Гн)
			p->motorInternals.ifADCk = (ADC_HALF_VALUE*1.0)/p->motorInternals.ifSenseK; // Расчетный коэффициент АЦП тока ОВ
			p->motorInternals.dcm_pm = 0;	// Сбрасываем флаг возбуждения от ПМ
		}
		else	// ДПТ с постоянными магнитами
		{
			// В этом случае обмотку возбуждения не считаем
			p->motorInternals.dcm_pm = 1; // Устанавливаем флаг возбуждения от ПМ
			p->motorInternals.ifADCk = 0; // Расчетный коэффициент АЦП тока ОВ устанавливаем нулю
		}
		p->motorInternals.i_a = 0;					//ток ОВ
		p->motorInternals.i_f = 0;					//поток якоря
	}

	if (p->motorInternals.type==MODEL_SRD_MOTOR){//тип двигателя - ВИД (Switched Reluctance Drive, SRD)
		if (MotorParametersValid){
			p->motorInternals.rs = SRMDATA[p->MotorParametersNum-1].r_s;				//сопротивление фазной обмотки
			p->motorInternals.pp = SRMDATA[p->MotorParametersNum-1].pp;				//число пар полюсов
			p->motorInternals.j = SRMDATA[p->MotorParametersNum-1].j;					//момент инерции
			p->motorInternals.QEPResolution = SRMDATA[p->MotorParametersNum-1].qep;	//разрешение энкодера
			p->motorInternals.speedK = SRMDATA[p->MotorParametersNum-1].sk;			//коэффициент датчика скорости
			p->motorInternals.RatedPower = SRMDATA[p->MotorParametersNum-1].RatedPower; //номинальная мощность (справочная величина, не используется в расчетах)
			p->motorInternals.RatedSpeed = SRMDATA[p->MotorParametersNum-1].RatedSpeed; //номинальная скорость (справочная величина, не используется в расчетах)
			p->motorInternals.RatedCurrent = SRMDATA[p->MotorParametersNum-1].RatedCurrent; //номинальный действующий ток (справочная величина, не используется в расчетах)

		}
		// Инициализация коэффициентов и расчетных величин, завязанных на параметры двигателя
		p->motorInternals.MechLoss = p->motorInternals.j * 10; 				//механические потери
		p->motorInternals.inv_pp=1.0/p->motorInternals.pp;

		p->motorInternals.iADCk = p->motorInternals.RatedCurrent*1.414*3; 			// Расчетный коэффициент АЦП равен троекратной амплитуде номинального тока
		p->motorInternals.iSenseK = (ADC_MAX_VALUE*1.0)/p->motorInternals.iADCk;	//коэффициент датчика тока для преобразования тока в амперах в код АЦП

		p->motorInternals.ifADCk = p->motorInternals.RatedFluxCurrent*3; 			// Расчетный коэффициент АЦП равен троекратному номинальному току ОВ
		p->motorInternals.ifSenseK = (ADC_MAX_VALUE*1.0)/p->motorInternals.ifADCk;	//коэффициент датчика тока для преобразования тока в амперах в код АЦП


		p->motorInternals.isPhaseA=0;	//фазные токи
		p->motorInternals.isPhaseB=0;
		p->motorInternals.isPhaseC=0;
		p->motorInternals.psa=0;	//потокосцепления фаз
		p->motorInternals.psb=0;
		p->motorInternals.psc=0;

		//Определим количество отрезков (номер последней точки) кривых намагничивания
		volatile int Psi_I_s_resolution = 0;
		volatile int Psi_I_r_resolution = 0;
		//в последней точке должны быть отрицательные значения, как маркер окончания массива
		while ((Psi_I_s[Psi_I_s_resolution].i >= 0) && (Psi_I_s_resolution < 50)) Psi_I_s_resolution++;
		while ((Psi_I_r[Psi_I_r_resolution].i >= 0) && (Psi_I_r_resolution < 50)) Psi_I_r_resolution++;
		Psi_I_s_resolution--;	//уменьшаем на 1, чтобы получить количество отрезков
		Psi_I_r_resolution--;	//уменьшаем на 1, чтобы получить количество отрезков

		//расчет пределов изменения переменных (угол, ток, потокосцепление) и шага дискретизации по ним
		p->motorInternals.ThetaMax = 2*MOTOR_MODEL_PI;										//диапазон угла - 0..2Пи
		p->motorInternals.IMax = Psi_I_s[Psi_I_s_resolution].i;				//Считаем, что максимальный ток лежит в последней точке кривой для согласованного положения
		p->motorInternals.PsiMax = Psi_I_s[Psi_I_s_resolution].psi;			//Максимальный поток лежит в последней точке кривой для согласованного положения
		p->motorInternals.ThetaStep = p->motorInternals.ThetaMax/(SRD_SURFACE_RESOLUTION+1);	//приращение угла. Здесь, в отличие от тока и потока, делится на SRD_SURFACE_RESOLUTION+1,
																	//т.к. кривая зациклена, и не должно быть двух одинаковых соседних точек для углов 0 и 2пи. Т.е. точку 2пи в конце выкинули
		p->motorInternals.ThetaStep_inv = 1.0/p->motorInternals.ThetaStep;
		p->motorInternals.IStep = p->motorInternals.IMax/SRD_SURFACE_RESOLUTION;				//приращение тока
		p->motorInternals.IStep_inv = 1.0/p->motorInternals.IStep;
		p->motorInternals.PsiStep = p->motorInternals.PsiMax/SRD_SURFACE_RESOLUTION;			//приращение потока
		p->motorInternals.PsiStep_inv = 1.0/p->motorInternals.PsiStep;
//		p->motorInternals.j = srm_j[p->MotorParametersNum-1];		//уже присвоен выше (Савкин)
//		p->motorInternals.MechLoss = p->motorInternals.j * 10; 		//механические потери

		//Уточнение кривых намагничивания при помощи кубической интерполяции, инициализация поверхности psi=f(i,theta)
		for (int i=0; i<SRD_SURFACE_RESOLUTION+1; i++)
		{
			float TmpI = i*p->motorInternals.IStep;
			Psi_I_s_acc[i] = MotorModel_FluxCubicInterpolation (TmpI, Psi_I_s, Psi_I_s_resolution);	//в принципе, эти две уточненные кривые можно убрать, т.к. из них тут же генерится поверхность, а
			Psi_I_r_acc[i] = MotorModel_FluxCubicInterpolation (TmpI, Psi_I_r, Psi_I_r_resolution);	//сами кривые нигде не используются. Можно здесь просто оставить локальные переменные и сэкономить память
			for (int t=0; t<SRD_SURFACE_RESOLUTION+1; t++)
			{
				float TmpTheta = t*p->motorInternals.ThetaStep;
				Psi_ITheta[i][t] = (Psi_I_s_acc[i] + Psi_I_r_acc[i])*0.5 - (Psi_I_s_acc[i] - Psi_I_r_acc[i])*0.5*_IQtoF(_IQcos(_IQ(TmpTheta)));	//Эту поверхность тоже можно сделать локальной, она в расчете модели не участвует
			}
		}

		//Генерация поверхности i=f(psi,theta) из psi=f(i,theta)
		for (int t=0; t<SRD_SURFACE_RESOLUTION+1; t++)
		{
			//точки по углу будут те же, переворачивать поверхность будем для какждого дискретного угла psi=f(i) -> i=f(psi)
			for (int n=0; n<SRD_SURFACE_RESOLUTION+1; n++)
			{
				float TmpPsi = n*p->motorInternals.PsiStep;
				if (TmpPsi <= Psi_ITheta[0][t])	//ограничиваем минимум (на всякий случай)
				{
					I_PsiTheta[n][t] = 0;
				}
				else if (TmpPsi >= Psi_ITheta[SRD_SURFACE_RESOLUTION][t])	//ограничиваем максимум
				{
					I_PsiTheta[n][t] = p->motorInternals.IMax;	//на кривой намагничивания для данного угла может вообще не быть максимального потока (будет он только в согласованном положении). Поэтому, будет насыщение
				}
				else	//в ограничения не попали, интерполируем линейно.
				{
					for (int i = 1; i < SRD_SURFACE_RESOLUTION+1; i++)	//Ищем отрезок в исходной кривой psi=f(i), куда попал дискретный поток p
					{
						if (Psi_ITheta[i][t] >= TmpPsi)	//поток в следующей точке больше заданного, значит попали в нужный отрезок
						{
							I_PsiTheta[n][t] = p->motorInternals.IStep*i - (Psi_ITheta[i][t] - TmpPsi)*p->motorInternals.IStep/(Psi_ITheta[i][t] - Psi_ITheta[i-1][t]);
							break;
						}
					}
				}
			}
		}

		//Генерация поверхности момента M=f(i,theta)
		for (int t=0; t<SRD_SURFACE_RESOLUTION+1; t++)
		{
			float WcoPlus = 0;								//Коэнергия при угле Theta+
			float WcoMinus = 0;								//Коэнергия при угле Theta-
			M_ITheta[0][t] = 0;								//Момент при нулевом токе и любом угле - нулевой
			float PsiPlus_prev = 0;
			float PsiMinus_prev = 0;

			//Ищем соседние точки по положению, при этом зацикливаем для непрерывности линейной интерполяции по углу в районе theta=0 и theta=2Pi
			int tPlus = t+1;
			if (tPlus > SRD_SURFACE_RESOLUTION)
				tPlus -= (SRD_SURFACE_RESOLUTION+1);
			int tMinus = t-1;
			if (tMinus < 0)
				tMinus += (SRD_SURFACE_RESOLUTION+1);
			for (int i=1; i<SRD_SURFACE_RESOLUTION+1; i++)
			{
				//Ищем потокосцепление при заданном токе и угле чуть больше и чуть меньше (на SRD_DELTA_THETA) заданного при помощи линейной интерполяции
				float PsiPlus = Psi_ITheta[i][t] + (Psi_ITheta[i][tPlus] - Psi_ITheta[i][t])*SRD_DELTA_THETA*p->motorInternals.ThetaStep_inv;
				float PsiMinus = Psi_ITheta[i][t] - (Psi_ITheta[i][t] - Psi_ITheta[i][tMinus])*SRD_DELTA_THETA*p->motorInternals.ThetaStep_inv;
				//Считаем коэнергию - интеграл потокосцепления по току (методом трапеций)
				WcoPlus += (PsiPlus + PsiPlus_prev)*0.5*p->motorInternals.IStep;
				WcoMinus += (PsiMinus + PsiMinus_prev)*0.5*p->motorInternals.IStep;
				PsiPlus_prev = PsiPlus;
				PsiMinus_prev = PsiMinus;
				//PhaseTorque = p->motorInternals.pp*(p->motorInternals.wm-p->motorInternals.wp)*50;	// 50 = inv_0.02 dTeta				так было в исходном варианте
				M_ITheta[i][t] = p->motorInternals.pp*(WcoPlus - WcoMinus)*SRD_DELTA_THETA_INV*0.5;		//считаем момент, как производную коэнергии
			}
		}
	}

	// Расчет коэффициентов модели, неизменных во времени
	p->motorInternals.inv_tpr=1.0/p->motorInternals.tpr;
	p->motorInternals.inv_j=1.0/p->motorInternals.j;

	p->motorInternals.t2 = p->motorInternals.t / 2;				//половина шага дискретизации

	// Обнуление переменных состояния
	p->motorInternals.tetaR = 0;			//угол положения ротора электрический
	p->motorInternals.tetaRM = 0;				//угол положения ротора механический
	p->motorInternals.omega = 0;						//скорость, рад/с
	p->motorInternals.prd = 0;					//потокосцепления ротора в осях d,q
	p->motorInternals.prq = 0;
	p->motorInternals.isa = 0;						//токи статора в осях альфа,бета
	p->motorInternals.isb = 0;
	p->motorInternals.ira = 0;						//токи ротора в осях альфа,бета
	p->motorInternals.irb = 0;
	p->motorInternals.ird = 0;						//токи ротора в осях d,q
	p->motorInternals.irq = 0;


}

//расчет напряжений для двухфазного двигателя
void MotorModel_calcU42Phase(TMotorModel *p) {
	if (p->InvertorEna) { //инвертор включен
		// расчет напряжений питания
		// перерасчет влияния мертвого времени на скважность фазы в зависимости от знака тока
		if (p->motorInternals.i_a > 0) {
			if (p->motorInternals.cmpr0 > p->motorInternals.dt)
				p->motorInternals.dta = -p->motorInternals.dt;
			else
				p->motorInternals.dta = -p->motorInternals.cmpr0;
		} else {
			if ((p->motorInternals.cmpr0 + p->motorInternals.dt) < p->motorInternals.tpr)
				p->motorInternals.dta = +p->motorInternals.dt;
			else
				p->motorInternals.dta = p->motorInternals.tpr - p->motorInternals.cmpr0;
		}
		if (p->motorInternals.i_a < 0) {
			if (p->motorInternals.cmpr1 > p->motorInternals.dt)
				p->motorInternals.dtb = -p->motorInternals.dt;
			else
				p->motorInternals.dtb = -p->motorInternals.cmpr1;
		} else {
			if ((p->motorInternals.cmpr1 + p->motorInternals.dt) < p->motorInternals.tpr)
				p->motorInternals.dtb = +p->motorInternals.dt;
			else
				p->motorInternals.dtb = p->motorInternals.tpr - p->motorInternals.cmpr1;
		}
		//расчет потенциалов с учетом влияния мертвого времени
		p->motorInternals.fia = p->motorInternals.udc * (p->motorInternals.cmpr0 + (p->motorInternals.dta >> 1)) / p->motorInternals.tpr;
		p->motorInternals.fib = p->motorInternals.udc * (p->motorInternals.cmpr1 + (p->motorInternals.dtb >> 1)) / p->motorInternals.tpr;
		p->motorInternals.u_a = p->motorInternals.fia - p->motorInternals.fib;//Напряжение на якоре

		if (p->motorInternals.i_f > 0) {
			if (p->motorInternals.cmpr2 > p->motorInternals.dt)
				p->motorInternals.dtc = -p->motorInternals.dt;
			else
				p->motorInternals.dtc = -p->motorInternals.cmpr2;
		} else {
			if ((p->motorInternals.cmpr2 + p->motorInternals.dt) < p->motorInternals.tpr)
				p->motorInternals.dtc = +p->motorInternals.dt;
			else
				p->motorInternals.dtc = p->motorInternals.tpr - p->motorInternals.cmpr2;
		}
		//расчет потенциалов с учетом влияния мертвого времени
		p->motorInternals.fic = p->motorInternals.udc * (p->motorInternals.cmpr2 + (p->motorInternals.dtc >> 1)) / p->motorInternals.tpr;
		p->motorInternals.u_f = p->motorInternals.fic;//Напряжение на ОВ


		p->motorInternals.isPhaseA_prev = p->motorInternals.isPhaseA;
		p->motorInternals.CurrentInvertedFlag = 0;
	} else
	{//Инвертор выключен
/*
		if (p->motorInternals.isPhaseA > 0)
			p->motorInternals.u_a = -p->motorInternals.udc;
		else
			p->motorInternals.u_a = p->motorInternals.udc;
*/
		p->motorInternals.u_a = 0;

		p->motorInternals.fia = 0;
		p->motorInternals.fib = 0;
		p->motorInternals.fic = 0;

		if (p->motorInternals.isPhaseA * p->motorInternals.isPhaseA_prev < 0)		//ток сменил свой знак с момента выклчюения инвертора
			p->motorInternals.CurrentInvertedFlag = 7;//флаг, что все интересующие токи пересекли ноль

		if (p->motorInternals.CurrentInvertedFlag == 7) {//прикладываем нулевое напряжение, а в модели ДПТ обнулится формула тока
			p->motorInternals.u_a=0;
		}
		p->motorInternals.u_f = 0;//Напряжение ОВ равно нулю
	}


}

//расчет напряжений для трехфазного двигателя
void MotorModel_calcU43Phase(TMotorModel *p) {
	if (p->InvertorEna) { //инвертор включен
		// расчет напряжений питания
		// перерасчет влияния мертвого времени на скважность фазы в зависимости от знака тока
		if (p->motorInternals.isPhaseA > 0) {
			if (p->motorInternals.cmpr0 > p->motorInternals.dt)
				p->motorInternals.dta = -p->motorInternals.dt;
			else
				p->motorInternals.dta = -p->motorInternals.cmpr0;
		} else {
			if ((p->motorInternals.cmpr0 + p->motorInternals.dt) < p->motorInternals.tpr)
				p->motorInternals.dta = +p->motorInternals.dt;
			else
				p->motorInternals.dta = p->motorInternals.tpr - p->motorInternals.cmpr0;
		}
		if (p->motorInternals.isPhaseB > 0) {
			if (p->motorInternals.cmpr1 > p->motorInternals.dt)
				p->motorInternals.dtb = -p->motorInternals.dt;
			else
				p->motorInternals.dtb = -p->motorInternals.cmpr1;
		} else {
			if ((p->motorInternals.cmpr1 + p->motorInternals.dt) < p->motorInternals.tpr)
				p->motorInternals.dtb = +p->motorInternals.dt;
			else
				p->motorInternals.dtb = p->motorInternals.tpr - p->motorInternals.cmpr1;
		}
		if (p->motorInternals.isPhaseC > 0) {
			if (p->motorInternals.cmpr2 > p->motorInternals.dt)
				p->motorInternals.dtc = -p->motorInternals.dt;
			else
				p->motorInternals.dtc = -p->motorInternals.cmpr2;
		} else {
			if ((p->motorInternals.cmpr2 + p->motorInternals.dt) < p->motorInternals.tpr)
				p->motorInternals.dtc = +p->motorInternals.dt;
			else
				p->motorInternals.dtc = p->motorInternals.tpr - p->motorInternals.cmpr2;
		}

		//расчет потенциалов с учетом влияния мертвого времени
		p->motorInternals.fia = p->motorInternals.udc * (p->motorInternals.cmpr0 + (p->motorInternals.dta >> 1)) / p->motorInternals.tpr;
		p->motorInternals.fib = p->motorInternals.udc * (p->motorInternals.cmpr1 + (p->motorInternals.dtb >> 1)) / p->motorInternals.tpr;
		p->motorInternals.fic = p->motorInternals.udc * (p->motorInternals.cmpr2 + (p->motorInternals.dtc >> 1)) / p->motorInternals.tpr;
		p->motorInternals.fiav = (p->motorInternals.fia + p->motorInternals.fib + p->motorInternals.fic) / 3;		//потенциал срежней точки
		p->motorInternals.ua = p->motorInternals.fia - p->motorInternals.fiav;					//напряжения
		p->motorInternals.ub = p->motorInternals.fib - p->motorInternals.fiav;
		p->motorInternals.uc = p->motorInternals.fic - p->motorInternals.fiav;

		//напряжения в осях альфа,бета
		p->motorInternals.usa = p->motorInternals.ua;
		p->motorInternals.usb = 0.577350269 * p->motorInternals.ua + 1.154700538 * p->motorInternals.ub;

		p->motorInternals.isPhaseA_prev = p->motorInternals.isPhaseA;
		p->motorInternals.isPhaseB_prev = p->motorInternals.isPhaseB;
		p->motorInternals.isPhaseC_prev = p->motorInternals.isPhaseC;
		p->motorInternals.CurrentInvertedFlag = 0;

	} else { //инвертор выключен
		//сложно рассчитать напряжение, которое приложено к двигателю, когда инвертор выключен.
		//поэтому тут инвертор как бы в релейном режиме стремится поддерживать токи нулевыми, пока скорость не снизится до нуля,
		//а потом "замыкает" фазы двигателя. Это отличается от реальности, конечно, но лучше, чем не делать ничего.

		if (p->motorInternals.isPhaseA > 0)
			p->motorInternals.fia = 0;
		else
			p->motorInternals.fia = p->motorInternals.udc;

		if (p->motorInternals.isPhaseB > 0)
			p->motorInternals.fib = 0;
		else
			p->motorInternals.fib = p->motorInternals.udc;

		if (p->motorInternals.isPhaseC > 0)
			p->motorInternals.fic = 0;
		else
			p->motorInternals.fic = p->motorInternals.udc;

		if (p->motorInternals.isPhaseA * p->motorInternals.isPhaseA_prev < 0)		//ток сменил свой знак с момента выклчюения инвертора
			p->motorInternals.CurrentInvertedFlag |= 1;

		if (p->motorInternals.isPhaseB * p->motorInternals.isPhaseB_prev < 0)		//ток сменил свой знак с момента выклчюения инвертора
			p->motorInternals.CurrentInvertedFlag |= 2;

		if (p->motorInternals.isPhaseC * p->motorInternals.isPhaseC_prev < 0)		//ток сменил свой знак с момента выклчюения инвертора
			p->motorInternals.CurrentInvertedFlag |= 4;

		if (p->motorInternals.CurrentInvertedFlag == 7) {
			p->motorInternals.fia = 0;
			p->motorInternals.fib = 0;
			p->motorInternals.fic = 0;
		}

		p->motorInternals.fiav = (p->motorInternals.fia + p->motorInternals.fib + p->motorInternals.fic) * (1.0/3);		//потенциал срежней точки
		p->motorInternals.ua = p->motorInternals.fia - p->motorInternals.fiav;					//напряжения
		p->motorInternals.ub = p->motorInternals.fib - p->motorInternals.fiav;
		p->motorInternals.uc = p->motorInternals.fic - p->motorInternals.fiav;

		//напряжения в осях альфа,бета
		p->motorInternals.usa = p->motorInternals.ua;
		p->motorInternals.usb = 0.577350269 * p->motorInternals.ua + 1.154700538 * p->motorInternals.ub;
	}
}

//расчет напряжения для ОВ синхронного двигателя
void MotorModel_calcU4SyncMFieldWinding(TMotorModel *p) {
	if (p->InvertorEna) { //инвертор включен
		// расчет напряжений питания
		// перерасчет влияния мертвого времени на скважность фазы в зависимости от знака тока
		if (p->motorInternals.i_f > 0) {
			if (p->motorInternals.cmpr3 > p->motorInternals.dt)
				p->motorInternals.dtf = -p->motorInternals.dt;
			else
				p->motorInternals.dtf = -p->motorInternals.cmpr3;
		} else {
			if ((p->motorInternals.cmpr3 + p->motorInternals.dt) < p->motorInternals.tpr)
				p->motorInternals.dtf = +p->motorInternals.dt;
			else
				p->motorInternals.dtf = p->motorInternals.tpr - p->motorInternals.cmpr3;
		}
		//расчет потенциалов с учетом влияния мертвого времени
		p->motorInternals.u_f = p->motorInternals.udc * (p->motorInternals.cmpr3 + (p->motorInternals.dtf >> 1)) / p->motorInternals.tpr;//Напряжение на ОВ
	} else
	{//Инвертор выключен
		p->motorInternals.u_f = 0;//Напряжение ОВ равно нулю
	}


}

void MotorModel_MechModelExecute(TMotorModel *p) {
	float d_omega;
	p->loadTmp = 0;
	if (p->motorInternals.omega > 0)
		p->loadTmp = p->load + p->motorInternals.MechLoss;
	if (p->motorInternals.omega < 0)
		p->loadTmp = -p->load - p->motorInternals.MechLoss;

	d_omega = p->motorInternals.t / p->motorInternals.j * (p->motorInternals.torque - p->loadTmp);					//приращение скорости

	if ((fabs(d_omega) > fabs(p->motorInternals.omega)) && (fabs(p->motorInternals.torque) < p->load)) {
		p->motorInternals.omega = 0;
		d_omega = 0;
	}

	p->motorInternals.omega = p->motorInternals.omega + d_omega;	//скорость


}

//кубическая интерполяция для кривых намагничивания SRD
float MotorModel_FluxCubicInterpolation (volatile float Isource, volatile TFluxCurvePoint SourceArray[], volatile int SourceResolution)
{
	volatile float PsiResult=0;
	if (Isource <= SourceArray[0].i)	//ток меньше или равен минимуму
	{
		PsiResult = SourceArray[0].psi;
		return PsiResult;
	}
	if (Isource >= SourceArray[SourceResolution].i)	//ток больше или равен максимуму
	{
		PsiResult = SourceArray[SourceResolution].psi;
		return PsiResult;
	}
	for (int count = 1; count < SourceResolution+1; count++)	//попали внутрь границ, ищем нужный отрезок
	{
		if (SourceArray[count].i >= Isource)	//ток в следующей точке больше заданного, значит попали в нужный отрезок
		{
			volatile float a, b, c, d;	//коэффициенты кубического полинома
			volatile float f_d0, f_d1;	//производные в левой и правой точке отрезка
			volatile float x_m;			//масштабированный икс: такой, чтобы левая точка отрезка оказалась в нуле, а правая - в единице
			//Формула кубической интерполяции f(x)=a*x^3 + b*x^2 + c*x + d, производная: f(x)=3*a*x^2 + 2*b*x + c
			//Чтобы найти коэффициенты кубического полинома, нужно найти значения полинома на границах отрезка справа и слева,
			//а также производную полинома в этих точках. Для вычисления производных нужны значения исходной дискретной функции
			//в 2 соседних точках (левее и правее границ отрезка). Считаем ток=0 - левая граница отрезка, ток=1 - правая граница
			//a = 2*f(0) - 2*f(1) + f'(0) + f'(1)
			//b = -3*f(0) + 3*f(1) - 2*f'(0) - f'(1)
			//c = f'(0)
			//d = f(0)
			//Т.к. шаг по току неравномерный, а x[1]-x[0]!=1, производные ищются по формулам:
			//f'(0) = (f[1] - f[-1]) / {(x[1]-x[-1]) / (x[1]-x[0])}, т.е., исходная кривая растягивается по оси x, чтобы выполнялось x[1]-x[0]=1
			//f'(1) = (f[2] - f[0]) / {(x[2]-x[0]) / (x[1]-x[0])}
			//Упростим: f'(0) = (f[1] - f[-1]) *(x[1]-x[0]) / (x[1]-x[-1]), f'(1) = (f[2] - f[0]) *(x[1]-x[0]) / (x[2]-x[0])
			//Посчитав, коэффициенты, нужно выдать результат по формуле:
			//f(x)= a*xm^3 + b*xm^2 + c*xm + d,
			//где xm=(x-x[0])/(x[1]-x[0]), т.к. x[0] не равен 0, а x[1] не равен 1. Т.е., надо сдвинуть (вычитаем x[0]) и сжать обратно (делим на x[1]-x[0])
			//точка -1: count-2
			//точка 0: count-1
			//точка 1: count
			//точка 2: count+1

			//считаем производные в точках 0 и 1 (на краях отрезка)
			if (count == 1)	//крайний левый отрезок
			{
				//производную слева по-обычному посчитать нельзя, т.к. крайняя точка, посчитаем производную по самому отрезку
				f_d0 = SourceArray[count].psi - SourceArray[count-1].psi;	//на разность токов на краях отрезка не делим, т.к. приняли ее за единицу
			}
			else	//считаем левую производную как обычно
			{
				f_d0 = (SourceArray[count].psi - SourceArray[count-2].psi)*(SourceArray[count].i - SourceArray[count-1].i)/(SourceArray[count].i - SourceArray[count-2].i);
			}
			if (count == SourceResolution)	//крайний правый отрезок
			{
				//производную справа по-обычному посчитать нельзя, т.к. крайняя точка, посчитаем производную по самому отрезку
				f_d1 = SourceArray[count].psi - SourceArray[count-1].psi;	//на разность токов на краях отрезка не делим, т.к. приняли ее за единицу
			}
			else	//считаем правую производную как обычно
			{
				f_d1 = (SourceArray[count+1].psi - SourceArray[count-1].psi)*(SourceArray[count].i - SourceArray[count-1].i)/(SourceArray[count+1].i - SourceArray[count-1].i);
			}

			//считаем коэффициенты полинома
			a = 2*SourceArray[count-1].psi - 2*SourceArray[count].psi + f_d0 + f_d1;
			b = -3*SourceArray[count-1].psi + 3*SourceArray[count].psi - 2*f_d0 - f_d1;
			c = f_d0;
			d = SourceArray[count-1].psi;

			//считаем ток с учетом сдвинутости и растянутости кубической параболы относительно исходного графика по оси икс
			x_m = (Isource - SourceArray[count-1].i)/(SourceArray[count].i - SourceArray[count-1].i);

			PsiResult = a*x_m*x_m*x_m + b*x_m*x_m + c*x_m + d;	//считаем искомый поток
			return PsiResult;
		}
	}
	return PsiResult;//сюда не дойдем на самом деле, но чтобы компилятор не ругался
}

//двухмерная интерполяция по поверхности для поиска тока в модели SRD
float MotorModel_PhaseCurrentInterpolation (TMotorModel *p, volatile float Theta, volatile float PsiS)
{
	volatile float Current = 0;
	volatile float Current1, Current2;
	volatile int cntThetaMinus, cntThetaPlus, cntPsiMinus, cntPsiPlus;

	//ограничиваем входные переменные
	//по углу надо зациклить
	if (Theta < 0)
		Theta += p->motorInternals.ThetaMax;	//прибавляем 2пи
	if (Theta > p->motorInternals.ThetaMax)
		Theta -= p->motorInternals.ThetaMax;	//вычитаем 2пи
	cntThetaMinus = Theta*p->motorInternals.ThetaStep_inv;	//индекс массива по углу (меньший из двух)
	cntThetaPlus = cntThetaMinus + 1;			//больший из двух
	if (cntThetaPlus > SRD_SURFACE_RESOLUTION)		//если вышел за границу массива, зацикливаем
	{
		cntThetaPlus -= (SRD_SURFACE_RESOLUTION+1);
	}
	//по потоку надо просто ограничить сверху и снизу
	if (PsiS < 0)
		PsiS = 0;
	if (PsiS > p->motorInternals.PsiMax)
		PsiS = p->motorInternals.PsiMax;
	cntPsiMinus = PsiS*p->motorInternals.PsiStep_inv;		//индекс массива по потоку (меньший из двух)
	cntPsiPlus = cntPsiMinus + 1;				//больший из двух

	//первая интерполяция по углу
	Current1 = I_PsiTheta[cntPsiMinus][cntThetaMinus] + (I_PsiTheta[cntPsiMinus][cntThetaPlus] - I_PsiTheta[cntPsiMinus][cntThetaMinus])*(Theta - (float)cntThetaMinus*p->motorInternals.ThetaStep)*p->motorInternals.ThetaStep_inv;
	//если попали на верхнюю границу по потоку, надо интерполировать по сокращенным формулам, чтобы не выпасть за границы массива
	if (cntPsiMinus >= SRD_SURFACE_RESOLUTION)	//по потоку оказались на верхней границе
	{
		//интерполяция по потоку
		Current = Current1;	//интерполировать на границе по потоку не надо, момент будет равен моменту от первой интерполяции по углу
		return Current;
	}
	//вторая интерполяция по углу
	Current2 = I_PsiTheta[cntPsiPlus][cntThetaMinus] + (I_PsiTheta[cntPsiPlus][cntThetaPlus] - I_PsiTheta[cntPsiPlus][cntThetaMinus])*(Theta - (float)cntThetaMinus*p->motorInternals.ThetaStep)*p->motorInternals.ThetaStep_inv;
	//интерполяция по потоку
	Current = Current1 + (Current2 - Current1)*(PsiS - (float)cntPsiMinus*p->motorInternals.PsiStep)*p->motorInternals.PsiStep_inv;
	return Current;
}

//двухмерная интерполяция по поверхности для поиска момента в модели SRD
float MotorModel_PhaseTorqueInterpolation (TMotorModel *p, volatile float Theta, volatile float IPh)
{

	volatile float Torque = 0;
	volatile float Torque1, Torque2;
	volatile int cntThetaMinus, cntThetaPlus, cntIMinus, cntIPlus;

	//ограничиваем входные переменные
	//по углу надо зациклить
	if (Theta < 0)
		Theta += p->motorInternals.ThetaMax;	//прибавляем 2пи
	if (Theta > p->motorInternals.ThetaMax)
		Theta -= p->motorInternals.ThetaMax;	//вычитаем 2пи
	cntThetaMinus = Theta*p->motorInternals.ThetaStep_inv;	//индекс массива по углу (меньший из двух)
	cntThetaPlus = cntThetaMinus + 1;			//больший из двух
	if (cntThetaPlus > SRD_SURFACE_RESOLUTION)		//если вышел за границу массива, зацикливаем
	{
		cntThetaPlus -= (SRD_SURFACE_RESOLUTION+1);
	}
	//по току надо просто ограничить сверху и снизу
	if (IPh < 0)
		IPh = 0;
	if (IPh > p->motorInternals.IMax)
		IPh = p->motorInternals.IMax;
	cntIMinus = IPh*p->motorInternals.IStep_inv;		//индекс массива по току (меньший из двух)
	cntIPlus = cntIMinus + 1;				//больший из двух

	//первая интерполяция по углу
	Torque1 = M_ITheta[cntIMinus][cntThetaMinus] + (M_ITheta[cntIMinus][cntThetaPlus] - M_ITheta[cntIMinus][cntThetaMinus])*(Theta - (float)cntThetaMinus*p->motorInternals.ThetaStep)*p->motorInternals.ThetaStep_inv;
	//если попали на верхнюю границу по току, надо интерполировать по сокращенным формулам, чтобы не выпасть за границы массива
	if (cntIMinus >= SRD_SURFACE_RESOLUTION)	//по току оказались на верхней границе
	{
		//интерполяция по току
		Torque = Torque1;	//интерполировать на границе по току не надо, момент будет равен моменту от первой интерполяции по углу
		return Torque;
	}
	//вторая интерполяция по углу
	Torque2 = M_ITheta[cntIPlus][cntThetaMinus] + (M_ITheta[cntIPlus][cntThetaPlus] - M_ITheta[cntIPlus][cntThetaMinus])*(Theta - (float)cntThetaMinus*p->motorInternals.ThetaStep)*p->motorInternals.ThetaStep_inv;
	//интерполяция по току
	Torque = Torque1 + (Torque2 - Torque1)*(IPh - (float)cntIMinus*p->motorInternals.IStep)*p->motorInternals.IStep_inv;
	return Torque;
}

void Model_Execute_InductionMotor(TMotorModel *p) {
	MotorModel_calcU43Phase(p);	//расчет напряжений

	// Расчет синуса и косинуса угла ротора
	p->motorInternals.cosTetaR = _IQtoF(_IQcos(_IQ(p->motorInternals.tetaR)));//синус и косинус считаются с фиксированной точкой, т.к. флоатовские занимают тысячи тактов (видимо, какая-то медленная реализация, не использующая аппаратную поддержку).
	p->motorInternals.sinTetaR = _IQtoF(_IQsin(_IQ(p->motorInternals.tetaR)));//если удастся найти быстрый плавающий синус и косинус, то их можно считать во флоате

	// Расчет изменений потокосцеплений (предикторы)
	p->motorInternals.dpsa = (p->motorInternals.usa - p->motorInternals.isa * p->motorInternals.rs);
	p->motorInternals.dpsb = (p->motorInternals.usb - p->motorInternals.isb * p->motorInternals.rs);
	p->motorInternals.dprd = -(p->motorInternals.ird * p->motorInternals.rr);
	p->motorInternals.dprq = -(p->motorInternals.irq * p->motorInternals.rr);

	// Расчет предикторов потокосцеплений
	p->motorInternals.ppsa = p->motorInternals.psa + p->motorInternals.dpsa * p->motorInternals.t;
	p->motorInternals.ppsb = p->motorInternals.psb + p->motorInternals.dpsb * p->motorInternals.t;
	p->motorInternals.pprd = p->motorInternals.prd + p->motorInternals.dprd * p->motorInternals.t;
	p->motorInternals.pprq = p->motorInternals.prq + p->motorInternals.dprq * p->motorInternals.t;

	// Поворот потокосцеплений ротора в координаты alpha,beta
	p->motorInternals.ppra = p->motorInternals.pprd * p->motorInternals.cosTetaR - p->motorInternals.pprq * p->motorInternals.sinTetaR;
	p->motorInternals.pprb = +p->motorInternals.pprd * p->motorInternals.sinTetaR + p->motorInternals.pprq * p->motorInternals.cosTetaR;

	// Расчет токов для предикторного уравнения
	p->motorInternals.isa = p->motorInternals.ks * p->motorInternals.ppsa + p->motorInternals.km * p->motorInternals.ppra;
	p->motorInternals.isb = p->motorInternals.ks * p->motorInternals.ppsb + p->motorInternals.km * p->motorInternals.pprb;
	p->motorInternals.ira = p->motorInternals.km * p->motorInternals.ppsa + p->motorInternals.kr * p->motorInternals.ppra;
	p->motorInternals.irb = p->motorInternals.km * p->motorInternals.ppsb + p->motorInternals.kr * p->motorInternals.pprb;

	// Поворот роторных токов в d,q координаты
	p->motorInternals.ird = p->motorInternals.ira * p->motorInternals.cosTetaR + p->motorInternals.irb * p->motorInternals.sinTetaR;
	p->motorInternals.irq = -p->motorInternals.ira * p->motorInternals.sinTetaR + p->motorInternals.irb * p->motorInternals.cosTetaR;

	// Расчет изменений потокосцеплений по Рунге-Кутта второго порядка
	p->motorInternals.psa = p->motorInternals.psa + p->motorInternals.t2 * (p->motorInternals.dpsa + (p->motorInternals.usa - p->motorInternals.isa * p->motorInternals.rs));
	p->motorInternals.psb = p->motorInternals.psb + p->motorInternals.t2 * (p->motorInternals.dpsb + (p->motorInternals.usb - p->motorInternals.isb * p->motorInternals.rs));
	p->motorInternals.prd = p->motorInternals.prd + p->motorInternals.t2 * (p->motorInternals.dprd + (-p->motorInternals.ird * p->motorInternals.rr));
	p->motorInternals.prq = p->motorInternals.prq + p->motorInternals.t2 * (p->motorInternals.dprq + (-p->motorInternals.irq * p->motorInternals.rr));

	// Поворот потокосцеплений ротора в координаты alpha,beta
	p->motorInternals.pra = p->motorInternals.prd * p->motorInternals.cosTetaR - p->motorInternals.prq * p->motorInternals.sinTetaR;
	p->motorInternals.prb = p->motorInternals.prd * p->motorInternals.sinTetaR + p->motorInternals.prq * p->motorInternals.cosTetaR;

	//если инвертор выключен и все токи пересекли ноль (колеблются около нуля, расфорсировка прошла)
	//то обнуляем все потоки в двигателе (читерство, иначе сложный расчет будет)
	if ((p->InvertorEna == 0) && (p->motorInternals.CurrentInvertedFlag == 7)) {
		p->motorInternals.psa = 0;
		p->motorInternals.psb = 0;
		p->motorInternals.prd = 0;
		p->motorInternals.prq = 0;
		p->motorInternals.pra = 0;
		p->motorInternals.prb = 0;
	}

	// Расчет токов после уточнения изменения потокосцеплений
	p->motorInternals.isa = p->motorInternals.ks * p->motorInternals.psa + p->motorInternals.km * p->motorInternals.pra;
	p->motorInternals.isb = p->motorInternals.ks * p->motorInternals.psb + p->motorInternals.km * p->motorInternals.prb;
	p->motorInternals.ira = p->motorInternals.km * p->motorInternals.psa + p->motorInternals.kr * p->motorInternals.pra;
	p->motorInternals.irb = p->motorInternals.km * p->motorInternals.psb + p->motorInternals.kr * p->motorInternals.prb;

	// Поворот роторных токов в d,q координаты
	p->motorInternals.ird = p->motorInternals.ira * p->motorInternals.cosTetaR + p->motorInternals.irb * p->motorInternals.sinTetaR;
	p->motorInternals.irq = -p->motorInternals.ira * p->motorInternals.sinTetaR + p->motorInternals.irb * p->motorInternals.cosTetaR;

	// Расчет момента
	p->motorInternals.torque = 3.0 / 2.0 * p->motorInternals.pp * (p->motorInternals.psa * p->motorInternals.isb - p->motorInternals.psb * p->motorInternals.isa);

	// Расчет механики
	MotorModel_MechModelExecute(p);

	//токи из двухфазной системы в трехфазную
	p->motorInternals.isPhaseA = p->motorInternals.isa;
	p->motorInternals.isPhaseB = -0.5 * p->motorInternals.isa + p->motorInternals.cos30 * p->motorInternals.isb;
	p->motorInternals.isPhaseC = -p->motorInternals.isa - p->motorInternals.isPhaseB;

	//расчет измеряемых величин
	p->motorInternals.adcSpeedtemp = p->motorInternals.omega * p->motorInternals.speedK + ADC_HALF_VALUE;						//скорость (тахогенератор)
	p->motorInternals.iAtemp = p->motorInternals.isa * p->motorInternals.iSenseK + ADC_HALF_VALUE;				//ток фазы A
	p->motorInternals.iBtemp = p->motorInternals.isPhaseB * p->motorInternals.iSenseK + ADC_HALF_VALUE;			//ток фазы B
	p->motorInternals.tetaRM = p->motorInternals.tetaRM + p->motorInternals.t * p->motorInternals.omega;	//механическое положение ротора
	if (p->motorInternals.tetaRM > 2 * MOTOR_MODEL_PI)					//ограничиваем 0..2Пи
		p->motorInternals.tetaRM -= 2 * MOTOR_MODEL_PI;
	if (p->motorInternals.tetaRM < 0)
		p->motorInternals.tetaRM += 2 * MOTOR_MODEL_PI;
	p->motorInternals.tetaR=p->motorInternals.tetaRM*p->motorInternals.pp;
}

void Model_Execute_SyncMotor(TMotorModel *p) {
	MotorModel_calcU43Phase(p);	//расчет напряжений

	// Расчет потокосцепления ротора
	if (p->motorInternals.syncm_pm == 0) // Если с независимой ОВ
	{
		MotorModel_calcU4SyncMFieldWinding(p);	//расчет напряжения ОВ
		p->motorInternals.dif = (p->motorInternals.u_f - p->motorInternals.i_f * p->motorInternals.r_f)*p->motorInternals._1_l_f;
		p->motorInternals.i_f = p->motorInternals.i_f + p->motorInternals.dif * p->motorInternals.t;
		if (p->motorInternals.i_f <= p->motorInternals.RatedFluxCurrent)		// Учет насыщения
			p->motorInternals.m = p->motorInternals.i_f * p->motorInternals.l_m;
		else
			p->motorInternals.m = p->motorInternals.RatedFluxCurrent * p->motorInternals.l_m; // тут можно чуть изменить, введя небольшую надбавку.
	}	// Если с постоянными магнитами, то поток ротора присвоился еще в ините и не меняется

	// Расчет синуса и косинуса угла ротора
	p->motorInternals.cosTetaR = _IQtoF(_IQcos(_IQ(p->motorInternals.tetaR)));//синус и косинус считаются с фиксированной точкой, т.к. флоатовские занимают тысячи тактов (видимо, какая-то медленная реализация, не использующая аппаратную поддержку).
	p->motorInternals.sinTetaR = _IQtoF(_IQsin(_IQ(p->motorInternals.tetaR)));//если удастся найти быстрый плавающий синус и косинус, то их можно считать во флоате

	// Поворот напряжений из осей альфа.бета в оси d,q
	p->motorInternals.usd = p->motorInternals.usa * p->motorInternals.cosTetaR + p->motorInternals.usb * p->motorInternals.sinTetaR;
	p->motorInternals.usq = -p->motorInternals.usa * p->motorInternals.sinTetaR + p->motorInternals.usb * p->motorInternals.cosTetaR;

	// Расчет изменений потокосцеплений (предикторы)
	p->motorInternals.dpsd = (p->motorInternals.usd - p->motorInternals.isd * p->motorInternals.rs + p->motorInternals.psq * p->motorInternals.omega);
	p->motorInternals.dpsq = (p->motorInternals.usq - p->motorInternals.isq * p->motorInternals.rs - p->motorInternals.psd * p->motorInternals.omega);

	// Расчет предикторов потокосцеплений
	p->motorInternals.ppsd = p->motorInternals.psd + p->motorInternals.dpsd * p->motorInternals.t;
	p->motorInternals.ppsq = p->motorInternals.psq + p->motorInternals.dpsq * p->motorInternals.t;

	// Расчет токов для предикторного уравнения
	p->motorInternals.isd = (p->motorInternals.ppsd - p->motorInternals.m) * p->motorInternals._1_lsd;
	p->motorInternals.isq = p->motorInternals.ppsq * p->motorInternals._1_lsq;

	// Расчет изменений потокосцеплений по Рунге-Кутта второго порядка
	p->motorInternals.psd = p->motorInternals.psd + p->motorInternals.t2 * (p->motorInternals.dpsd + (p->motorInternals.usd - p->motorInternals.isd * p->motorInternals.rs + p->motorInternals.psq * p->motorInternals.omega));
	p->motorInternals.psq = p->motorInternals.psq + p->motorInternals.t2 * (p->motorInternals.dpsq + (p->motorInternals.usq - p->motorInternals.isq * p->motorInternals.rs - p->motorInternals.psd * p->motorInternals.omega));

	//если инвертор выключен и все токи пересекли ноль (колеблются около нуля, расфорсировка прошла)
	//то выключаем расчет токов статора, чтобы не дергались (читерство, иначе сложный расчет будет)
	if ((p->InvertorEna == 0) && (p->motorInternals.CurrentInvertedFlag == 7)) {
		p->motorInternals.psd = p->motorInternals.m;
		p->motorInternals.psq = 0;
	}

	// Расчет токов после уточнения изменения потокосцеплений
	p->motorInternals.isd = (p->motorInternals.psd - p->motorInternals.m) * p->motorInternals._1_lsd;
	p->motorInternals.isq = p->motorInternals.psq * p->motorInternals._1_lsq;

	//поворот токов в оси альфа,бета (для вывода на АЦП)
	p->motorInternals.isa = p->motorInternals.isd * p->motorInternals.cosTetaR - p->motorInternals.isq * p->motorInternals.sinTetaR;
	p->motorInternals.isb = +p->motorInternals.isd * p->motorInternals.sinTetaR + p->motorInternals.isq * p->motorInternals.cosTetaR;

	// Расчет момента
	p->motorInternals.torque = 3.0 / 2.0 * p->motorInternals.pp * (p->motorInternals.psd * p->motorInternals.isq - p->motorInternals.psq * p->motorInternals.isd);

	// Расчет механики
	MotorModel_MechModelExecute(p);

	//токи из двухфазной системы в трехфазную
	p->motorInternals.isPhaseA = p->motorInternals.isa;
	p->motorInternals.isPhaseB = -0.5 * p->motorInternals.isa + p->motorInternals.cos30 * p->motorInternals.isb;
	p->motorInternals.isPhaseC = -p->motorInternals.isa - p->motorInternals.isPhaseB;

	p->motorInternals.isPhaseD = p->motorInternals.i_f;

	//расчет измеряемых величин
	p->adc_code_Speed = p->motorInternals.omega * p->motorInternals.speedK + ADC_HALF_VALUE;						//скорость (тахогенератор)
	p->motorInternals.iAtemp = p->motorInternals.isa * p->motorInternals.iSenseK + ADC_HALF_VALUE;				//ток фазы A
	p->motorInternals.iBtemp = p->motorInternals.isPhaseB * p->motorInternals.iSenseK + ADC_HALF_VALUE;			//ток фазы B
	p->motorInternals.iDtemp = p->motorInternals.i_f * p->motorInternals.ifSenseK + ADC_HALF_VALUE;					//ток ОВ
	p->motorInternals.tetaRM = p->motorInternals.tetaRM + p->motorInternals.t * p->motorInternals.omega;	//механическое положение ротора
	if (p->motorInternals.tetaRM > 2 * MOTOR_MODEL_PI)					//ограничиваем 0..2Пи
		p->motorInternals.tetaRM -= 2 * MOTOR_MODEL_PI;
	if (p->motorInternals.tetaRM < 0)
		p->motorInternals.tetaRM += 2 * MOTOR_MODEL_PI;

	p->motorInternals.tetaR=p->motorInternals.tetaRM*p->motorInternals.pp;
}

void Model_Execute_DCMotor(TMotorModel *p) {
	// Преобразование напряжений
	MotorModel_calcU42Phase(p);

	// !!! Внимание! При соотношении параметров ОВ с малой индуктивность и большим сопротивлением расчет ОВ может выполняться некорректно
	// !!! Критическое соотношение параметров Lf=6.3 мГн и Rf=125 Ом

	// предварительный расчет приращений по току
	if (p->motorInternals.dcm_pm == 0) // Если с независимой ОВ
	{
		p->motorInternals.dif = (p->motorInternals.u_f - p->motorInternals.i_f * p->motorInternals.r_f)*p->motorInternals._1_l_f;
		p->motorInternals.i_f = p->motorInternals.i_f + p->motorInternals.dif * p->motorInternals.t;
		if (p->motorInternals.i_f <= p->motorInternals.RatedFluxCurrent)		// Учет насыщения
			p->motorInternals.kf = p->motorInternals.i_f * p->motorInternals.l_m;
		else
			p->motorInternals.kf = p->motorInternals.RatedFluxCurrent * p->motorInternals.l_m; // тут можно чуть изменить, введя небольшую надбавку.
	}
	else // Если с постоянными магнитами
		p->motorInternals.kf = p->motorInternals.l_m * 1.0;

	p->motorInternals.dia = (p->motorInternals.u_a - p->motorInternals.kf*p->motorInternals.omega - p->motorInternals.i_a*(p->motorInternals.r_a + p->motorInternals.r_ad))*p->motorInternals._1_l_a;
	p->motorInternals.i_a = p->motorInternals.i_a + p->motorInternals.dia * p->motorInternals.t;

	//если инвертор выключен и все токи пересекли ноль (колеблются около нуля, расфорсировка прошла)
	//то выключаем расчет токов статора, чтобы не дергались (читерство, иначе сложный расчет будет)
	if ((p->InvertorEna == 0) && (p->motorInternals.CurrentInvertedFlag == 7)) {
		p->motorInternals.i_a=0;
	}

	// Момент
	p->motorInternals.torque = p->motorInternals.kf * p->motorInternals.i_a;

	// вычисление механических величин
	MotorModel_MechModelExecute(p);

	//omega+=0.5;
	p->motorInternals.tetaRM = p->motorInternals.tetaRM + p->motorInternals.t * p->motorInternals.omega;		//положение
	if (p->motorInternals.tetaRM > 2 * MOTOR_MODEL_PI)						//ограничение 0..2Пи
		p->motorInternals.tetaRM -= 2 * MOTOR_MODEL_PI;
	if (p->motorInternals.tetaRM < 0)
		p->motorInternals.tetaRM += 2 * MOTOR_MODEL_PI;

	p->motorInternals.isPhaseA = p->motorInternals.i_a;
	p->motorInternals.isPhaseB = p->motorInternals.isPhaseA;
	p->motorInternals.isPhaseC = p->motorInternals.i_f;

	//расчет измеряемых величин
	p->motorInternals.adcSpeedtemp = p->motorInternals.omega * p->motorInternals.speedK + ADC_HALF_VALUE;			//скорость (тахогенератор)
	p->motorInternals.iAtemp = p->motorInternals.i_a * p->motorInternals.iSenseK + ADC_HALF_VALUE;					//ток якоря
	p->motorInternals.iCtemp = p->motorInternals.i_f * p->motorInternals.ifSenseK + ADC_HALF_VALUE;					//ток ОВ
}


void Model_Execute_SRDMotor(TMotorModel *p) {
	p->motorInternals.ua=p->motorInternals.udc*(p->motorInternals.cmpr0*p->motorInternals.inv_tpr*2-1);	//фазное напряжение
	p->motorInternals.psa+=(p->motorInternals.ua - p->motorInternals.isPhaseA*p->motorInternals.rs)*p->motorInternals.t;					//интегрируем потокосцепление
	if (p->motorInternals.psa<0)
		p->motorInternals.psa=0;
	p->motorInternals.isPhaseA=MotorModel_PhaseCurrentInterpolation(p,p->motorInternals.tetaR, p->motorInternals.psa);			//ищем ток фазы
	p->motorInternals.torque_a = MotorModel_PhaseTorqueInterpolation(p,p->motorInternals.tetaR, p->motorInternals.isPhaseA);		//ищем момент фазы
	//DebugW2 = _IQ(p->motorInternals.torque_a);

	p->motorInternals.ub=p->motorInternals.udc*(p->motorInternals.cmpr1*p->motorInternals.inv_tpr*2-1);	//фазное напряжение
	p->motorInternals.psb+=(p->motorInternals.ub - p->motorInternals.isPhaseB*p->motorInternals.rs)*p->motorInternals.t;					//интегрируем потокосцепление
	if (p->motorInternals.psb<0)
		p->motorInternals.psb=0;
	float TetaB = p->motorInternals.tetaR+MOTOR_MODEL_PI*1.33333;				//фазовый сдвиг
	if (TetaB > 2*MOTOR_MODEL_PI)
		TetaB -= 2*MOTOR_MODEL_PI;
	p->motorInternals.isPhaseB=MotorModel_PhaseCurrentInterpolation(p,TetaB, p->motorInternals.psb);					//ищем ток фазы
	p->motorInternals.torque_b = MotorModel_PhaseTorqueInterpolation(p,TetaB, p->motorInternals.isPhaseB);			//ищем момент фазы
	//DebugW3 = _IQ(p->motorInternals.torque_b);

	p->motorInternals.uc=p->motorInternals.udc*(p->motorInternals.cmpr2*p->motorInternals.inv_tpr*2-1);	//фазное напряжение
	p->motorInternals.psc+=(p->motorInternals.uc - p->motorInternals.isPhaseC*p->motorInternals.rs)*p->motorInternals.t;					//интегрируем потокосцепление
	if (p->motorInternals.psc<0)
		p->motorInternals.psc=0;
	float TetaC = p->motorInternals.tetaR+MOTOR_MODEL_PI*0.66667;				//фазовый сдвиг
	if (TetaC > 2*MOTOR_MODEL_PI)
		TetaC -= 2*MOTOR_MODEL_PI;
	p->motorInternals.isPhaseC=MotorModel_PhaseCurrentInterpolation(p,TetaC, p->motorInternals.psc);					//ищем ток фазы
	p->motorInternals.torque_c = MotorModel_PhaseTorqueInterpolation(p,TetaC, p->motorInternals.isPhaseC);			//ищем момент фазы
	//DebugW4 = _IQ(p->motorInternals.torque_c);

	//Суммарный момент всех фаз (фазы - независимые)
	p->motorInternals.torque = p->motorInternals.torque_a + p->motorInternals.torque_b + p->motorInternals.torque_c;

	// Расчет механики
	MotorModel_MechModelExecute(p);

	p->motorInternals.tetaRM+=p->motorInternals.t*p->motorInternals.omega;
	//p->motorInternals.tetaRM = 0.393;	//угол с максимальным моментом для фазы A, для теста
	if (p->motorInternals.tetaRM>2*MOTOR_MODEL_PI)
		p->motorInternals.tetaRM-=2*MOTOR_MODEL_PI;
	if (p->motorInternals.tetaRM<0)
		p->motorInternals.tetaRM+=2*MOTOR_MODEL_PI;

	p->motorInternals.tetaR=p->motorInternals.tetaRM*p->motorInternals.pp;
	while (p->motorInternals.tetaR>2*MOTOR_MODEL_PI)
		p->motorInternals.tetaR-=2*MOTOR_MODEL_PI;

	p->adc_code_Speed=p->motorInternals.omega*p->motorInternals.speedK+ADC_HALF_VALUE;
	p->motorInternals.iAtemp=p->motorInternals.isPhaseA*p->motorInternals.iSenseK;
	p->motorInternals.iBtemp=p->motorInternals.isPhaseB*p->motorInternals.iSenseK;
	p->motorInternals.iCtemp=p->motorInternals.isPhaseC*p->motorInternals.iSenseK;


}

//основная функция расчета модели, вызывается в рперывании 10 кГц
void MotorModel_Execute(TMotorModel *p) {

	p->motorInternals.cmpr0 = p->motorInternals.tpr-p->cmpr0;	//уставки сравнения ШИМ. С инверсией, потому что так настроен модуль ШИМ
	p->motorInternals.cmpr1 = p->motorInternals.tpr-p->cmpr1;
	p->motorInternals.cmpr2 = p->motorInternals.tpr-p->cmpr2;
	p->motorInternals.cmpr3 = p->motorInternals.tpr-p->cmpr3;

	if (p->fault == 0)		//если нет ошибки модели, считаем
	{
		if (p->motorInternals.type == MODEL_INDUCTION_MOTOR) {
			Model_Execute_InductionMotor(p);

		} else if (p->motorInternals.type == MODEL_SYNC_MOTOR) {
			Model_Execute_SyncMotor(p);

		} else if (p->motorInternals.type == MODEL_DC_MOTOR) {
			Model_Execute_DCMotor(p);

		}
		else if (p->motorInternals.type==MODEL_SRD_MOTOR){
			Model_Execute_SRDMotor(p);
		}
	} else {//модель в аварии
		p->motorInternals.iAtemp=ADC_HALF_VALUE;
		p->motorInternals.iBtemp=ADC_HALF_VALUE;
		p->motorInternals.iCtemp=ADC_HALF_VALUE;
		p->motorInternals.iDtemp=ADC_HALF_VALUE;

	}

	//Имитация сигнала с датчика Холла (3 элемента) в зависимости от текущего электрического положения
	//рассчитываем номер электрического сектора, где сейчас ротор из электрического угла
	Uint16 TempSector=p->motorInternals.tetaR * (1/(2 * MOTOR_MODEL_PI)*6);
	TempSector=TempSector%6;//угол не ограничен, поэтому используем остаток от делеления на кол-во секторов

	switch (TempSector){//в зависимости от номера сектора формируем побитовый код на элементах Холла
		case 0:{
			p->motorInternals.hallSensor=5;//101
			break;
		}
		case 1:{
			p->motorInternals.hallSensor=4;//100
			break;
		}
		case 2:{
			p->motorInternals.hallSensor=6;//110
			break;
		}
		case 3:{
			p->motorInternals.hallSensor=2;//010
			break;
		}
		case 4:{
			p->motorInternals.hallSensor=3;//011
			break;
		}
		case 5:{
			p->motorInternals.hallSensor=1;//001
			break;
		}

	}

	if (p->motorInternals.hallSensor!=p->motorInternals.hallSensorPrev){//Пришла новая метка ДПР Холла
		p->hallSensorOmega=p->motorInternals.hallSensorOmegaPrev;//На выход модели - скорость, которая была между предыдущими метками ДПР Холла
		p->motorInternals.hallSensorOmegaPrev=p->motorInternals.omega;//Запоминаем текущую скорость, чтобы отдать её потом
		p->hallSensorInterpAdd=0;
		p->motorInternals.hallSensorPrev=p->motorInternals.hallSensor;
		p->hallSensor=p->motorInternals.hallSensor;
	}

	if (fabs(p->motorInternals.omega)<1){//если двигатель почти стоит
		p->hallSensorOmega=0;
		p->motorInternals.hallSensorOmegaPrev=0;
		p->hallSensorInterpAdd=0;
	}
	if (p->hallSensorInterpAdd < (MOTOR_MODEL_PI/3))	//ограничиваем шестой частью угла
	p->hallSensorInterpAdd = p->hallSensorInterpAdd + p->motorInternals.t * p->hallSensorOmega *p->motorInternals.pp;	//механическое положение ротора

	p->motorInternals.power=p->motorInternals.omega*p->motorInternals.torque*0.001;//Мощность на валу в кВт
	p->qepCounter=p->motorInternals.tetaRM*MODEL_INV_2PI*p->motorInternals.QEPResolution*4;//Имитация счетчика инкрементального энкодера из текущего угла двигателя
	p->motorInternals.omega_rpm = p->motorInternals.omega*MODEL_INV_2PI*60;//Частота вращения вала из рад/с в об/мин
	p->motorInternals.Udc_temp=p->motorInternals.udc*p->motorInternals.udSenseK;//Имитация измерения напряжения ЗПТ


	//наложение шума АЦП
	p->motorInternals.iAtemp += MotorModel_fastrand();
	p->motorInternals.iBtemp += MotorModel_fastrand();
	p->motorInternals.iCtemp += MotorModel_fastrand();
	p->motorInternals.iDtemp += MotorModel_fastrand();
	p->motorInternals.adcSpeedtemp += MotorModel_fastrand();
	p->motorInternals.Udc_temp += MotorModel_fastrand();

	//насыщение каналов АЦП
	if (p->motorInternals.iAtemp > ADC_MAX_VALUE)
		p->motorInternals.iAtemp = ADC_MAX_VALUE;
	if (p->motorInternals.iAtemp < 0)
		p->motorInternals.iAtemp = 0;
	if (p->motorInternals.iBtemp > ADC_MAX_VALUE)
		p->motorInternals.iBtemp = ADC_MAX_VALUE;
	if (p->motorInternals.iBtemp < 0)
		p->motorInternals.iBtemp = 0;
	if (p->motorInternals.iCtemp > ADC_MAX_VALUE)
		p->motorInternals.iCtemp = ADC_MAX_VALUE;
	if (p->motorInternals.iCtemp < 0)
		p->motorInternals.iCtemp = 0;
	if (p->motorInternals.iDtemp > ADC_MAX_VALUE)
		p->motorInternals.iDtemp = ADC_MAX_VALUE;
	if (p->motorInternals.iDtemp < 0)
		p->motorInternals.iDtemp = 0;
	if (p->motorInternals.adcSpeedtemp > ADC_MAX_VALUE)
		p->motorInternals.adcSpeedtemp = ADC_MAX_VALUE;
	if (p->motorInternals.adcSpeedtemp < 0)
		p->motorInternals.adcSpeedtemp = 0;
	if (p->motorInternals.Udc_temp > ADC_MAX_VALUE)
		p->motorInternals.Udc_temp = ADC_MAX_VALUE;
	if (p->motorInternals.Udc_temp < 0)
		p->motorInternals.Udc_temp = 0;

	p->adc_code_iA = p->motorInternals.iAtemp;
	p->adc_code_iB = p->motorInternals.iBtemp;
	p->adc_code_iC = p->motorInternals.iCtemp;
	p->adc_code_iD = p->motorInternals.iDtemp;
	p->adc_code_Speed = p->motorInternals.adcSpeedtemp;
	p->adc_code_Udc =  p->motorInternals.Udc_temp;

}


/*@}*/
