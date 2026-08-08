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

 \file      X_CANFlashProgStarter.h
 \brief     Инициализирует процесс прожига по CANу
 \author	Алямкин Д.
 \version   v 1.0 30/05/2017

 \defgroup TCANFlashProgStarter */
/*@{*/



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

//ПереопределЯемые пользователем макросы
#define FP_BLOCK_PWM 	pwm.Off(&pwm);	//выключение ШИМ
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

/*@}*/
