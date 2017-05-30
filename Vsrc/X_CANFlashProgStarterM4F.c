/* ==================================================================================
//File name:       X_CANFlashProgStarter.c
//
//Originator:	Aliamkin Dmitriy I
//
//Description:   инициализирует процесс прожига по CANу
//ѕ–»ћ≈„јЌ»≈:    
//=====================================================================================
//-------------------------------------------------------------------------------------
//  01.11.2012	Release	Rev 1.4 »справлена ошибка в функции StartFlashProgrammer
	26.04.2011	Release	Rev 1.3
//----------------------------------------------------------------------------------*/
#include "DSP.h"
#include "main.h"
#include "X_CANFlashProgData.h"
#include "X_CANFlashProgStarter.h"
void GPIO_reset();
//в данной функции важно загружать код прошивальщика в ќ«” непосредственно перед переходом в прошивальщик, 
//иначе можно испортить данные кода прошивальщика, скопированные в ќ«” (так как там могут распологататься 
//переменные изменяемые в данной функции)
TFlashProgSettings FPsettings;
void StartFlashProgrammer() 							
{
	Uint16 i,j;
	volatile Uint8* sourcePtr;
	volatile Uint8* destPtr;
	volatile Uint16* sourcePtr16;
	volatile Uint16* destPtr16;
	void (*FlashProgProjectCall)(void);

	//запрещаем все прерывания
	DINT;
	//еще запрещаем все прерывани€ NVIC
	/*NVIC->ICER[0] = 0xFFFFFFFF;
	NVIC->ICER[1] = 0xFFFFFFFF;
	NVIC->ICER[2] = 0xFFFFFFFF;
	NVIC->ICER[3] = 0xFFFFFFFF;
	NVIC->ICER[4] = 0xFFFFFFFF;*/

	//переводим все GPIO в исходное состояние
	GPIO_reset();
	//инициализируем GPIO CANов тут, так как прошивальщик не знает через какие ножки вылазит CAN у пользовател€
	#ifdef FP_DEVICE_M4F
		#ifdef CAN_1_ENABLE
			co_CAN1GpioInit();
		#endif
		#ifdef CAN_2_ENABLE
			co_CAN2GpioInit();
		#endif
	#endif
	//передаем настройки прошивальщика
	FPsettings.magic_number = 0x1234;
	
	#if defined(CAN_1_ENABLE) && defined(CAN_2_ENABLE)
		//если разрешены оба CAN-а, CANx_enable	выставлены в call_back функциях
		FPsettings.CANa_bitRate = co1_vars.co_bitRate;
		FPsettings.CANa_nodeID = co1_vars.co_nodeID;
		FPsettings.CANb_bitRate = co2_vars.co_bitRate;
		FPsettings.CANb_nodeID = co2_vars.co_nodeID;
	#else
		#ifdef CAN_1_ENABLE
			FPsettings.CANa_enable = 1;
			FPsettings.CANb_enable = 0;
			FPsettings.CANa_bitRate = co1_vars.co_bitRate;
			FPsettings.CANa_nodeID = co1_vars.co_nodeID;
		#else
			FPsettings.CANa_enable = 0;
		#endif
		
		#ifdef CAN_2_ENABLE
			FPsettings.CANa_enable = 0;
			FPsettings.CANb_enable = 1;	
			FPsettings.CANb_bitRate = co2_vars.co_bitRate;
			FPsettings.CANb_nodeID = co2_vars.co_nodeID;
		#else
			FPsettings.CANb_enable = 0;
		#endif 
	#endif	
	
	//копируем настройки прошивальщика, по нужному адресу
	sourcePtr = (Uint8*)&FPsettings;
	destPtr = ((Uint8*)0x20000064);//настройки всегда хранятся на 2 слова ниже стартового адреса
	for(i=0; i<sizeof(TFlashProgSettings); i++)
	{
		*destPtr++ = *sourcePtr++;
	}
	
	//копируем все части прошивальщика туда куда нужно
	sourcePtr16 = (Uint16*)&FPData[0];
	for(i=0; i<FPNumOfAddrs; i++)
	{
		destPtr16 = (Uint16*)FPAddrs[i];
		for(j=0; j<(FPPartsLength[i]); j++)
		*destPtr16++ = *sourcePtr16++;
	}
 
//	NT_TIMER1->INT.INTCLEAR = 1;
	NT_TIMER1->INTSTATUS_INTCLEAR_bit.INT = 1; //TODO: ”бедитьс€ в правильности этого

	//идем в прошивальщик
	((void (*)(void))(0x20000001))();
}

void GPIO_reset()
{
	K1921VK01T_GPIO_SafeInit();
}

long swu_vars = 0;
