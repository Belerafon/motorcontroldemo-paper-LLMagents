/*    Copyright 2017 	АО "НИИЭТ" и ООО "НПФ ВЕКТОР"

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
// V_motorParams_SRM.h
//Файл с массивами параметров модели для различных вариантов ВИД

//структура с параметрами двигателя
struct Ssrm_param
{
	 float r_s; 		//!< сопротивление статора Swithed Reluctance Motor
	 float pp; 			//!< число пар полюсов (зубцов на роторе) Swithed Reluctance Motor
	 float j;			//!< момент инерции
	 float qep; 		//!< число меток энкодера
	 float sk;			//!< коэффициент датчика скорости Swithed Reluctance Motor
	 float RatedPower;	//!< номинальная мощность (справочная величина, не используется в расчетах)
	 float RatedSpeed;	//!< номинальная скорость (справочная величина, не используется в расчетах)
	 float RatedCurrent;//!< номинальный действующий ток
};
typedef volatile struct Ssrm_param Tsrm_param;

// !!! Внимание! Последний набор параметров в массиве ОБяЗАТЕЛЬНО должен быть нулевым!
Tsrm_param SRMDATA[] = {\
//	    |r_s	|pp		|j				|qep		|sk									|p_n	|n_n	|i_n
		{3,		4,		0.031,			4000,		14.323944878270580219199538703526,	8.5,	1500,	13},\
		{2,		4,		0.04227273,		4000,		4.7746482927568600730665129011754,	12,		1500,	18},\
		{1.5,	4,		0.062,			4000,		4.7746482927568600730665129011754,	15,		1500,	23},\
		{1,		4,		0.10427273,		4000,		14.323944878270580219199538703526,	18,		1500,	27.5},\
		{0.6,	4,		0.155,			4000,		4.7746482927568600730665129011754,	20,		1500,	30},\
		{0.3,	8,		0.42272727,		4000,		14.323944878270580219199538703526,	27,		1000,	20.5},\
		{0.1,	4,		1.104272727,	4000,		4.7746482927568600730665129011754,	57,		1500,	86.5},\
		{0.05,	4,		1.155,			4000,		4.7746482927568600730665129011754,	60,		1000,	91},\
		{0.03,	4,		2.211363636,	4000,		4.7746482927568600730665129011754,	120,	1500,	182},\
		{0.02,	8,		3.31,			4000,		14.323944878270580219199538703526,	90,		1000,	70},\
		{0,		0,		0,				0,			0,									0,		0,		0},\
};

//Структурка точки для кривой намагничивания (задается по точкам, с переменным шагом)
struct SFluxCurvePoint
{
	float i;	//ток
	float psi;	//потокосцепление
};
typedef volatile struct SFluxCurvePoint TFluxCurvePoint;


//Кривые намагничивания. Формат: {ток, потокосцепление}
//Согласованное положение
#define SRM_FLUX_CURVE_S	{{0, 0}, {1.125, 0.45}, {2.25, 0.9}, {3.375, 1.35}, {4.5, 1.58}, {6.0, 1.7}, {7.5, 1.8}, {9.0, 1.89}, {10.5, 1.98}, {12.0, 2.06}, {13.5, 2.13}, {-1, -1}}
//Расоогласованное положение
#define SRM_FLUX_CURVE_R	{{0, 0}, {1.35, 0.111}, {2.7, 0.221}, {4.05, 0.331}, {5.4, 0.44}, {6.75, 0.54}, {8.1, 0.648}, {9.45, 0.756}, {10.8, 0.863}, {12.15, 0.969}, {13.5, 1.075}, {-1, -1}}

#define SRD_SURFACE_RESOLUTION 100		//разрешение поверхностей момента от положения и потока M=f(theta,psi), потокосцепления от тока и положения psi=f(i,theta), тока от потокосцепления и положения i=f(psi,theta)
										//ПРИМЕЧАНИЕ: задается число отрезков, т.е. число точек будет на 1 больше (например, 11 точек и 10 отрезков между ними), аналогично заданы разрешения других кривых и поверхностей

#define SRD_DELTA_THETA 0.01			//маленькое приращение углу для расчета момента SRD через разность коэнергий для соседних с текущим положений ротора
#define SRD_DELTA_THETA_INV 100.0		// 1/SRD_DELTA_THETA


TFluxCurvePoint Psi_I_s[] = SRM_FLUX_CURVE_S;	//кривая намагничивания для согласованного положения (зуб-зуб, максимальная индуктивность)
TFluxCurvePoint Psi_I_r[] = SRM_FLUX_CURVE_R;	//кривая намагничивания для рассогласованного положения (зуб-паз, миниимальная индуктивность)
float Psi_I_s_acc[SRD_SURFACE_RESOLUTION+1];	//кривая намагничивания для согласованного положения - уточненная, с постоянным шагом по току
float Psi_I_r_acc[SRD_SURFACE_RESOLUTION+1];	//кривая намагничивания для рассогласованного положения - уточненная, с постоянным шагом по току

float Psi_ITheta[SRD_SURFACE_RESOLUTION+1][SRD_SURFACE_RESOLUTION+1];		//Поверхность зависимости потокосцепления от тока и положения psi=f(i,theta)
float I_PsiTheta[SRD_SURFACE_RESOLUTION+1][SRD_SURFACE_RESOLUTION+1];		//Поверхность зависимости тока от потокосцепления и положения i=f(psi,theta)

float M_ITheta[SRD_SURFACE_RESOLUTION+1][SRD_SURFACE_RESOLUTION+1];			//поверхность зависимости момента одной фазы SRD от тока и положения M=f(i,theta)


float MotorModel_FluxCubicInterpolation (volatile float Isource, volatile TFluxCurvePoint SourceArray[], volatile int SourceResolution);


