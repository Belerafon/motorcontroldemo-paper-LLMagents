/* ==================================================================================
//File name:       X_CANFlashProgStarter.h
//
//Originator:	Aliamkin Dmitriy I
//
//Description:   инициализирует процесс прожига по CANу
//ѕ–»ћ≈„јЌ»≈:    
//=====================================================================================
//-------------------------------------------------------------------------------------
// 16.12.2011	Release	Rev 1.0
//----------------------------------------------------------------------------------*/
#ifndef X_CANFLASHPROGSTARTER_H
#define X_CANFLASHPROGSTARTER_H

#ifdef __cplusplus
extern "C"
{
#endif

//#define FP_DEVICE_2810
//#define FP_DEVICE_28335
//#define FP_DEVICE_28035
//#define FP_DEVICE_28069
#define FP_DEVICE_M4F

//ѕереопределяемые пользователем макросы
#define FP_BLOCK_PWM 	pwm.Off(&pwm);	//выключение Ў»ћ
#define FP_CTRL_STOP 	sm_ctrl.state=CTRL_STOP
#define FP_FAULT_SET 	sm_prot.bit_fault2|= F_FLASH_PROGRAM


struct SFlashProgSettings{
	Uint16 magic_number;
	Uint16 CANa_nodeID;				//2000.0 номер данного узла
    Uint16 CANa_bitRate;			//2001.0
	Uint16 CANa_enable;
	Uint16 CANb_nodeID;				//2000.0 номер данного узла
    Uint16 CANb_bitRate;			//2001.0
	Uint16 CANb_enable;
};
typedef volatile struct SFlashProgSettings TFlashProgSettings;

extern TFlashProgSettings FPsettings;

/*
  struct SFlashProgStarter
  {
    Uint16 input;
    void(*start)(volatile struct SFlashProgStarter*);
  };

  typedef volatile struct SFlashProgStarter TFlashProgStarter;

#define TFLASHPROGSTARTER_DEFAULTS {0,\
								    StartFlashProgrammer,\
								   }
*/
void StartFlashProgrammer();
extern long swu_vars;


#ifdef __cplusplus
}
#endif

#endif
