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

 \file      Vled.c
 \brief     Управление светодиодами
 \author    ООО "НПФ Вектор". http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \addtogroup led
 */

#include "DSP.h"
#include "main.h"

//Для  VectorCARD
#if defined (HW_VECTORCARD_DRV8301_EVM) || defined (HW_VECTORCARD_DRV8312_EVM) || defined (HW_VECTORCARD_SIMULATOR)

#define VD1_ON NT_GPIOC->MASKLOWBYTE_bit[4].MASKLB = 0			//NT_GPIOC->DATA &= ~(1 << 2)
#define VD1_OFF NT_GPIOC->MASKLOWBYTE_bit[4].MASKLB = 4			//NT_GPIOC->DATA |= (1 << 2)

#define VD2_ON NT_GPIOC->MASKLOWBYTE_bit[8].MASKLB = 0			//NT_GPIOC->DATA &= ~(1 << 3)
#define VD2_OFF NT_GPIOC->MASKLOWBYTE_bit[8].MASKLB = 8			//NT_GPIOC->DATA |= (1 << 3)

#define VD3_ON NT_GPIOB->MASKLOWBYTE_bit[32].MASKLB = 0			//NT_GPIOB->DATA &= ~(1 << 5)
#define VD3_OFF NT_GPIOB->MASKLOWBYTE_bit[32].MASKLB = 32		//NT_GPIOB->DATA |= (1 << 5)

#define VD4_ON NT_GPIOB->MASKLOWBYTE_bit[64].MASKLB = 0			//NT_GPIOB->DATA &= ~(1 << 6)
#define VD4_OFF NT_GPIOB->MASKLOWBYTE_bit[64].MASKLB = 64		//NT_GPIOB->DATA |= (1 << 6)

#define VD5_ON NT_GPIOB->MASKLOWBYTE_bit[128].MASKLB = 0		//NT_GPIOB->DATA &= ~(1 << 7)
#define VD5_OFF NT_GPIOB->MASKLOWBYTE_bit[128].MASKLB = 128		//NT_GPIOB->DATA |= (1 << 7)

#define VD6_ON NT_GPIOB->MASKHIGHBYTE_bit[1].MASKHB = 0			//NT_GPIOB->DATA &= ~(1 << 8)
#define VD6_OFF NT_GPIOB->MASKHIGHBYTE_bit[1].MASKHB = 1		//NT_GPIOB->DATA |= (1 << 8)

//Диоды на плате инвертора TI (синей)
#define VDSTATUS_ON NT_GPIOG->MASKHIGHBYTE_bit[2].MASKHB = 2			//NT_GPIOG->DATA.bit.GPIO9 = 1
#define VDSTATUS_OFF NT_GPIOG->MASKHIGHBYTE_bit[2].MASKHB = 0			//NT_GPIOG->DATA.bit.GPIO9 = 0

#define VDLED10_ON NT_GPIOA->MASKHIGHBYTE_bit[2].MASKHB = 2				//NT_GPIOA->DATA.bit.GPIO9 = 1
#define VDLED10_OFF NT_GPIOA->MASKHIGHBYTE_bit[2].MASKHB = 0			//NT_GPIOA->DATA.bit.GPIO9 = 0

#define VDLED11_ON NT_GPIOE->MASKHIGHBYTE_bit[32].MASKHB = 32				//NT_GPIOE->DATA.bit.GPIO13 = 1
#define VDLED11_OFF NT_GPIOE->MASKHIGHBYTE_bit[32].MASKHB = 0				//NT_GPIOE->DATA.bit.GPIO13 = 0
#endif

#if defined (HW_VECTOR_MK_40_4)

#define VD1_ON NT_GPIOA->MASKHIGHBYTE_bit[8].MASKLB = 0				//NT_GPIOA->DATA &= ~(1 << 11)	Желтый светодиод
#define VD1_OFF NT_GPIOA->MASKHIGHBYTE_bit[8].MASKLB = 8			//NT_GPIOA->DATA |= (1 << 11)

#define VD2_ON NT_GPIOA->MASKHIGHBYTE_bit[16].MASKLB = 0			//NT_GPIOA->DATA &= ~(1 << 12)	Красный светодиод
#define VD2_OFF NT_GPIOA->MASKHIGHBYTE_bit[16].MASKLB = 16			//NT_GPIOA->DATA |= (1 << 12)

#define VD3_ON
#define VD3_OFF

#define VD4_ON
#define VD4_OFF

#endif

#if defined (HW_MOTORCONTROLBOARD) || defined (HW_MOTORCONTROLBOARD_SIMULATOR)

#define VD1_ON NT_GPIOA->MASKLOWBYTE_bit[2].MASKLB = 0			//NT_GPIOA->DATA &= ~(1 << 1)	Красный светодиод	VD18
#define VD1_OFF NT_GPIOA->MASKLOWBYTE_bit[2].MASKLB = 2			//NT_GPIOA->DATA |= (1 << 1)

#define VD2_ON NT_GPIOA->MASKLOWBYTE_bit[1].MASKLB = 0			//NT_GPIOA->DATA &= ~(1 << 0)	Желтый светодиод	VD19
#define VD2_OFF NT_GPIOA->MASKLOWBYTE_bit[1].MASKLB = 1			//NT_GPIOA->DATA |= (1 << 0)

#define VD3_ON NT_GPIOH->MASKLOWBYTE_bit[128].MASKLB = 0	 	//NT_GPIOH->DATA &= ~(1 << 7)	Зеленый светодиод	VD1
#define VD3_OFF NT_GPIOH->MASKLOWBYTE_bit[128].MASKLB = 128		//NT_GPIOH->DATA |= (1 << 7)

#define VD4_ON NT_GPIOH->MASKLOWBYTE_bit[1].MASKLB = 0			//NT_GPIOH->DATA &= ~(1 << 0)	Зеленый светодиод	VD2
#define VD4_OFF NT_GPIOH->MASKLOWBYTE_bit[1].MASKLB = 1			//NT_GPIOH->DATA |= (1 << 0)



#endif

void LED_init(Tled *p) {
//Для  VectorCARD
#if defined (HW_VECTORCARD_DRV8301_EVM) || defined (HW_VECTORCARD_DRV8312_EVM) || defined (HW_VECTORCARD_SIMULATOR)
	/* Разрешить ногам работать на выход */
	NT_GPIOA->ALTFUNCCLR = (1 << 9);
	NT_GPIOB->ALTFUNCCLR = (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8);
	NT_GPIOC->ALTFUNCCLR = (1 << 2) | (1 << 3);
	NT_GPIOE->ALTFUNCCLR = (1 << 13);
	NT_GPIOG->ALTFUNCCLR = (1 << 9);

	NT_GPIOA->OUTENSET |= (1 << 9);
	NT_GPIOB->OUTENSET |= (1 << 5) | (1 << 6) | (1 << 7)
	| (1 << 8);
	NT_GPIOC->OUTENSET |= (1 << 2) | (1 << 3);
	NT_GPIOE->OUTENSET |= (1 << 13);
	NT_GPIOG->OUTENSET |= (1 << 9);

	NT_GPIOD->ALTFUNCCLR = (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);
	NT_GPIOD->OUTENCLR = (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6);

	leds.mode = 2; //наиболее красивый тип мигания
#endif

#if defined (HW_VECTOR_MK_40_4)
	/* Разрешить ногам работать на выход */
	NT_GPIOA->ALTFUNCCLR = (1 << 11) | (1 << 12);	//Желтый и красный

	NT_GPIOA->OUTENSET |= (1 << 11) | (1 << 12);
#endif

#if defined (HW_MOTORCONTROLBOARD) || defined (HW_MOTORCONTROLBOARD_SIMULATOR)
	/* Разрешить ногам работать на выход */
	NT_GPIOA->ALTFUNCCLR = (1 << 0) | (1 << 1);	//Желтый и красный
	NT_GPIOH->ALTFUNCCLR = (1 << 0) | (1 << 7);	//Зеленый и зеленый

	NT_GPIOA->OUTENSET |= (1 << 0) | (1 << 1);	//Желтый и красный
	NT_GPIOH->OUTENSET |= (1 << 0) | (1 << 7);	//Зеленый и зеленый

	NT_GPIOB->ALTFUNCCLR = (1 << 6) | (1 << 15);
	NT_GPIOB->OUTENSET |= (1 << 6) | (1 << 15);

#endif
}

void LED_toggle(Tled *p, Uint16 led) {
//Для  VectorCARD
#if defined (HW_VECTORCARD_DRV8301_EVM) || defined (HW_VECTORCARD_DRV8312_EVM) || defined (HW_VECTORCARD_SIMULATOR)

	switch (led) {
		case LED1:
		//NT_GPIOA->DATA.bit.GPIO11 = NT_GPIOA->DATA.bit.GPIO11 ^ 1;
		NT_GPIOA->MASKHIGHBYTE_bit[8].MASKHB ^= 8;
		break;

		case LED2:
		//NT_GPIOA->DATA.bit.GPIO12 = NT_GPIOA->DATA.bit.GPIO12 ^ 1;
		NT_GPIOA->MASKHIGHBYTE_bit[16].MASKHB ^= 16;
		break;

		case LED_ALL:
		//NT_GPIOA->DATA.bit.GPIO11 = NT_GPIOA->DATA.bit.GPIO11 ^ 1;
		//NT_GPIOA->DATA.bit.GPIO12 = NT_GPIOA->DATA.bit.GPIO12 ^ 1;
		NT_GPIOA->MASKHIGHBYTE_bit[24].MASKHB ^= 24;
		break;
	}
#endif
}

void LED_on(Tled *p, Uint16 led) {
//Для  VectorCARD
#if defined (HW_VECTORCARD_DRV8301_EVM) || defined (HW_VECTORCARD_DRV8312_EVM) || defined (HW_VECTORCARD_SIMULATOR)

	switch (led) {
		case LED1:
		VD1_ON;
		break;

		case LED2:
		VD2_ON;
		break;

		case LED3:
		VD3_ON;
		break;

		case LED4:
		VD4_ON;
		break;

		case LED5:
		VD5_ON;
		break;

		case LED6:
		VD6_ON;
		break;

		case LED_ALL:
		VD1_ON;
		VD2_ON;
		VD3_ON;
		VD4_ON;
		VD5_ON;
		VD6_ON;
		break;
	}
#endif
}

void LED_off(Tled *p, Uint16 led) {
//Для  VectorCARD
#if defined (HW_VECTORCARD_DRV8301_EVM) || defined (HW_VECTORCARD_DRV8312_EVM) || defined (HW_VECTORCARD_SIMULATOR)

	switch (led) {
		case LED1:
		VD1_OFF;
		break;

		case LED2:
		VD2_OFF;
		break;

		case LED3:
		VD3_OFF;
		break;

		case LED4:
		VD4_OFF;
		break;

		case LED5:
		VD5_OFF;
		break;

		case LED6:
		VD6_OFF;
		break;

		case LED_ALL:
		VD1_OFF;
		VD2_OFF;
		VD3_OFF;
		VD4_OFF;
		VD5_OFF;
		VD6_OFF;
		break;
	}
#endif
}

void LED_clearAll(Tled *p) {
//Для  VectorCARD
#if defined (HW_VECTORCARD_DRV8301_EVM) || defined (HW_VECTORCARD_DRV8312_EVM) || defined (HW_VECTORCARD_SIMULATOR)
	p->timer1 = p->timer2 = p->timer3 = p->timer4 = p->timer5 = p->timer6 = 0;
	p->mode3counter = 0;
	VD1_OFF;
	VD2_OFF;
	VD3_OFF;
	VD4_OFF;
	VD5_OFF;
	VD6_OFF;
#endif
}

void LED_mode0(Tled *p) {
//Для  VectorCARD
#if defined (HW_VECTORCARD_DRV8301_EVM) || defined (HW_VECTORCARD_DRV8312_EVM) || defined (HW_VECTORCARD_SIMULATOR)

	if (p->timer1 >= 300) {
		p->timer1 = p->timer2 = p->timer3 = p->timer4 = p->timer5 = p->timer6 =
		0;
	}

	if (p->timer1 >= 0 && p->timer1 < 50)
	VD1_ON;
	else
	VD1_OFF;

	if (p->timer2 >= 50 && p->timer2 < 100)
	VD2_ON;
	else
	VD2_OFF;

	if (p->timer3 >= 100 && p->timer3 < 150)
	VD3_ON;
	else
	VD3_OFF;

	if (p->timer4 >= 150 && p->timer4 < 200)
	VD4_ON;
	else
	VD4_OFF;

	if (p->timer5 >= 200 && p->timer5 < 250)
	VD5_ON;
	else
	VD5_OFF;

	if (p->timer6 >= 250 && p->timer6 < 300)
	VD6_ON;
	else
	VD6_OFF;
#endif

}

void LED_mode1(Tled *p) {
//Для  VectorCARD
#if defined (HW_VECTORCARD_DRV8301_EVM) || defined (HW_VECTORCARD_DRV8312_EVM) || defined (HW_VECTORCARD_SIMULATOR)

	if (p->timer1 >= 400) {
		p->timer1 = p->timer2 = p->timer3 = p->timer4 = p->timer5 = p->timer6 =
		0;
	}

	if (p->timer1 >= 0 && p->timer1 < 200) {
		VD1_ON;
		VD3_ON;
		VD5_ON;
	} else {
		VD1_OFF;
		VD3_OFF;
		VD5_OFF;
	}

	if (p->timer1 >= 200 && p->timer1 < 400) {
		VD2_ON;
		VD4_ON;
		VD6_ON;
	} else {
		VD2_OFF;
		VD4_OFF;
		VD6_OFF;
	}
#endif
}

void LED_mode2(Tled *p) {
//Для  VectorCARD
#if defined (HW_VECTORCARD_DRV8301_EVM) || defined (HW_VECTORCARD_DRV8312_EVM) || defined (HW_VECTORCARD_SIMULATOR)

	if (p->timer1 >= 720) {
		p->timer1 = p->timer2 = p->timer3 = p->timer4 = p->timer5 = p->timer6 =
		0;
	}

	if (p->timer1 >= 0 && p->timer1 < 360)
	VD1_ON;
	else
	VD1_OFF;

	if (p->timer2 >= 60 && p->timer2 < 420)
	VD2_ON;
	else
	VD2_OFF;

	if (p->timer3 >= 120 && p->timer3 < 480)
	VD3_ON;
	else
	VD3_OFF;

	if (p->timer4 >= 180 && p->timer4 < 540)
	VD4_ON;
	else
	VD4_OFF;

	if (p->timer5 >= 240 && p->timer5 < 600)
	VD5_ON;
	else
	VD5_OFF;

	if (p->timer6 >= 300 && p->timer6 < 660)
	VD6_ON;
	else
	VD6_OFF;
#endif

}

void LED_mode3(Tled *p) {
//Для  VectorCARD
#if defined (HW_VECTORCARD_DRV8301_EVM) || defined (HW_VECTORCARD_DRV8312_EVM) || defined (HW_VECTORCARD_SIMULATOR)

	if (p->timer1 >= (300 - p->mode3counter * 50)) {
		p->timer1 = p->timer2 = p->timer3 = p->timer4 = p->timer5 = p->timer6 =
		0;
		p->mode3counter++;
		if (p->mode3counter == 6)
		p->mode3counter = 0;
	}

	switch (p->mode3counter) {
		case 0:
		if (p->timer1 >= 0 && p->timer1 < 50)
		VD1_ON;
		else
		VD1_OFF;

		if (p->timer2 >= 50 && p->timer2 < 100)
		VD2_ON;
		else
		VD2_OFF;

		if (p->timer3 >= 100 && p->timer3 < 150)
		VD3_ON;
		else
		VD3_OFF;

		if (p->timer4 >= 150 && p->timer4 < 200)
		VD4_ON;
		else
		VD4_OFF;

		if (p->timer5 >= 200 && p->timer5 < 250)
		VD5_ON;
		else
		VD5_OFF;

		if (p->timer6 >= 250 && p->timer6 < 300)
		VD6_ON;
		else
		VD6_OFF;
		break;

		case 1:
		if (p->timer1 >= 0 && p->timer1 < 50)
		VD1_ON;
		else
		VD1_OFF;

		if (p->timer2 >= 50 && p->timer2 < 100)
		VD2_ON;
		else
		VD2_OFF;

		if (p->timer3 >= 100 && p->timer3 < 150)
		VD3_ON;
		else
		VD3_OFF;

		if (p->timer4 >= 150 && p->timer4 < 200)
		VD4_ON;
		else
		VD4_OFF;

		if (p->timer5 >= 200 && p->timer5 < 250)
		VD5_ON;
		else
		VD5_OFF;

		VD6_ON;
		break;

		case 2:
		if (p->timer1 >= 0 && p->timer1 < 50)
		VD1_ON;
		else
		VD1_OFF;

		if (p->timer2 >= 50 && p->timer2 < 100)
		VD2_ON;
		else
		VD2_OFF;

		if (p->timer3 >= 100 && p->timer3 < 150)
		VD3_ON;
		else
		VD3_OFF;

		if (p->timer4 >= 150 && p->timer4 < 200)
		VD4_ON;
		else
		VD4_OFF;

		VD5_ON;
		VD6_ON;
		break;

		case 3:
		if (p->timer1 >= 0 && p->timer1 < 50)
		VD1_ON;
		else
		VD1_OFF;

		if (p->timer2 >= 50 && p->timer2 < 100)
		VD2_ON;
		else
		VD2_OFF;

		if (p->timer3 >= 100 && p->timer3 < 150)
		VD3_ON;
		else
		VD3_OFF;

		VD4_ON;
		VD5_ON;
		VD6_ON;
		break;

		case 4:
		if (p->timer1 >= 0 && p->timer1 < 50)
		VD1_ON;
		else
		VD1_OFF;

		if (p->timer2 >= 50 && p->timer2 < 100)
		VD2_ON;
		else
		VD2_OFF;

		VD3_ON;
		VD4_ON;
		VD5_ON;
		VD6_ON;
		break;

		case 5:
		if (p->timer1 >= 0 && p->timer1 < 50)
		VD1_ON;
		else
		VD1_OFF;

		VD2_ON;
		VD3_ON;
		VD4_ON;
		VD5_ON;
		VD6_ON;
		break;
	}
#endif
}

void LED_msCalc(Tled *p) {
//Для  VectorCARD
#if defined (HW_VECTORCARD_DRV8301_EVM) || defined (HW_VECTORCARD_DRV8312_EVM) || defined (HW_VECTORCARD_SIMULATOR)

	// В зависимости от нажатой кнопки переключаем способ светодиоденья
	if ((NT_GPIOD->DATA_bit.DATA & (1 << 3)) == 0)
	p->mode = 0;
	if ((NT_GPIOD->DATA_bit.DATA & (1 << 4)) == 0)
	leds.mode = 1;
	if ((NT_GPIOD->DATA_bit.DATA & (1 << 5)) == 0)
	leds.mode = 2;
	if ((NT_GPIOD->DATA_bit.DATA & (1 << 6)) == 0)
	leds.mode = 3;

	if (p->mode != p->prevMode) {
		LED_clearAll(p);
		p->prevMode = p->mode;
	}
	p->timer1++;
	p->timer2++;
	p->timer3++;
	p->timer4++;
	p->timer5++;
	p->timer6++;

	switch (p->mode) {
		case 0:
		LED_mode0(p);
		break;
		case 1:
		LED_mode1(p);
		break;
		case 2:
		LED_mode2(p);
		break;
		case 3:
		LED_mode3(p);
		break;
	}

	p->timer1++;
	p->timer2++;
	if (p->timer1 >= 1000)
	p->timer1 = 0;
	if (p->timer2 >= 400)
	p->timer2 = 0;

	if (p->timer1 >= 900)
	VDSTATUS_ON;
	else
	VDSTATUS_OFF;

	if (p->timer2 >= 300 && sm_ctrl.state != CTRL_STOP)
	VDLED10_ON;
	else
	VDLED10_OFF;

	if (NT_GPIOG->DATA_bit.DATA & (1 << 8))
	VDLED11_ON;
	else
	VDLED11_OFF;
#endif

#if defined (HW_VECTOR_MK_40_4)
	//светодиоднаЯ интикациЯ на плате контроллера
	if (sm_prot.state == PROT_ON_OK)//если нет аварии
	{
		p->timer1++;
		if (p->timer1 > 500) {
			NT_GPIOA->MASKHIGHBYTE_bit[1 << (11 - 8)].MASKHB ^= 0xFF;
			p->timer1 = 0;
		}
		if (sm_ctrl.state == CTRL_STOP)
		NT_GPIOA->MASKHIGHBYTE_bit[1 << (12 - 8)].MASKHB = 0xFF;
		else
		NT_GPIOA->MASKHIGHBYTE_bit[1 << (12 - 8)].MASKHB = 0;
	} else  	//если есть авариЯ
	{
		p->timer1++;
		if (p->timer1 > 200) {
			NT_GPIOA->MASKHIGHBYTE_bit[1 << (11 - 8)].MASKHB ^= 0xFF;
			p->timer1 = 0;
		}

		if (sm_ctrl.state == CTRL_STOP)
		NT_GPIOA->MASKHIGHBYTE_bit[1 << (12 - 8)].MASKHB = 0xFF;
		else
		NT_GPIOA->MASKHIGHBYTE_bit[1 << (12 - 8)].MASKHB = 0;
	}
#endif

#if defined (HW_MOTORCONTROLBOARD) || defined (HW_MOTORCONTROLBOARD_SIMULATOR)
	//светодиоднаЯ интикациЯ на плате контроллера
	if (sm_prot.state == PROT_ON_OK)   //если нет аварии
	{
		p->timer1++;
		p->timer2++;
		if (p->timer1 >= 500) {
			p->timer1 = p->timer2 = 0;
		}

		VD1_OFF;
		VD2_ON;

		if (p->timer1 >= 0 && p->timer1 < 250)
			VD3_ON;
		else
			VD3_OFF;

		if (p->timer2 >= 250 && p->timer2 < 500)
			VD4_ON;
		else
			VD4_OFF;
	} else  	//если есть авария
	{
		p->timer1++;
		p->timer2++;
		if (p->timer1 >= 1000) {
			p->timer1 = p->timer2 = 0;
		}

		VD3_OFF;
		VD4_OFF;

		if (p->timer1 >= 0 && p->timer1 < 500)
			VD1_ON;
		else
			VD1_OFF;

		if (p->timer2 >= 500 && p->timer2 < 1000)
			VD2_ON;
		else
			VD2_OFF;
	}



	if (sm_ctrl.state == CMD_LOGIC_TURNED_OFF) //выключен
	{
		D_OUT2_ON;
		D_OUT1_OFF;
	}
	else{
		D_OUT2_OFF;
		D_OUT1_ON;
	}

#endif

}

