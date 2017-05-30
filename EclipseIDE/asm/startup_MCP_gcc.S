/**
  ******************************************************************************
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

  * @file      startup_MCP.s
  * @author    NIIET
  * @version   V1.0.0
  * @date      28 - September - 2014
  * @brief     NIIET MC01 vector table for Sourcery Codebench.
  *            This module performs:
  *                - Set the initial SP
  *                - Set the initial PC == Reset_Handler,
  *                - Set the vector table entries with the exceptions ISR address
  *                - Branches to main in the C library (which eventually
  *                  calls main()).
  *				   - enables FPU (COMING SOON!)
  *            After Reset the Cortex-M4 processor is in Thread mode,
  *            priority is Privileged, and the Stack is set to Main.
  */
    
  .syntax unified
  .cpu cortex-m3
  .fpu vfpv4
  .thumb

.global  g_pfnVectors
.global  Default_Handler

/* start address for the initialization values of the .data section. 
defined in linker script */
.word  _sidata
/* start address for the .data section. defined in linker script */  
.word  _sdata
/* end address for the .data section. defined in linker script */
.word  _edata
/* start address for the .bss section. defined in linker script */
.word  _sbss
/* end address for the .bss section. defined in linker script */
.word  _ebss
/* stack used for SystemInit_ExtMemCtl; always internal RAM used */

/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called. 
 * @param  None
 * @retval : None
*/

    .section  .text.Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:  

/* Copy the data segment initializers from flash to SRAM */  
  movs  r1, #0
  b  LoopCopyDataInit

CopyDataInit:
  ldr  r3, =_sidata
  ldr  r3, [r3, r1]
  str  r3, [r0, r1]
  adds  r1, r1, #4

LoopCopyDataInit:
  ldr  r0, =_sdata
  ldr  r3, =_edata
  adds  r2, r0, r1
  cmp  r2, r3
  bcc  CopyDataInit
  ldr  r2, =_sbss
  b  LoopFillZerobss
/* Zero fill the bss segment. */  
FillZerobss:
  movs  r3, #0
  str  r3, [r2], #4
    
LoopFillZerobss:
  ldr  r3, = _ebss
  cmp  r2, r3
  bcc  FillZerobss


/* Включение плавоточки*/
  ldr.w R0, =0xE000ED88
  ldr R1, [R0]
  orr R1, R1, #(0xF << 20)
  str R1, [R0]
  dsb
  isb /*reset pipeline now the FPU is enabled*/

//  ldr r0, = _estack			/* Эти две строки - загрузка SP адресом 2000а000 в случае, если флешка потёрта. Если проект шьётся во флеш - удалить.*/
//  msr msp, r0					/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
  bl  main
  bx  lr    
.size  Reset_Handler, .-Reset_Handler

/**
 * @brief  This is the code that gets called when the processor receives an 
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 * @param  None     
 * @retval None       
*/
    .section  .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b  Infinite_Loop
  .size  Default_Handler, .-Default_Handler
/******************************************************************************
*
* The minimal vector table for a Cortex M3. Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
* 
*******************************************************************************/
   .section  .isr_vector,"a",%progbits
  .type  g_pfnVectors, %object
  .size  g_pfnVectors, .-g_pfnVectors
    
    
g_pfnVectors:
  .word  _estack
  .word  Reset_Handler
  .word  NMI_Handler
  .word  HardFault_Handler
  .word  MemManage_Handler
  .word  BusFault_Handler
  .word  UsageFault_Handler
  .word  0
  .word  0
  .word  0
  .word  0
  .word  SVC_Handler
  .word  DebugMon_Handler
  .word  0
  .word  PendSV_Handler
  .word  SysTick_Handler


  /* External Interrupts */
 .word     WWDG_IRQHandler            /*INT_0 : Window WatchDog   */
 .word     I2C0_IRQHandler            /*INT_1 : I2C1              */
 .word     I2C1_IRQHandler            /*INT_2 : I2C2              */
 .word     TIM0_IRQHandler            /*INT_3 : TIM1              */
 .word     TIM1_IRQHandler            /*INT_4 : TIM2              */
 .word     TIM2_IRQHandler            /*INT_5 : TIM3              */
 .word     DMA1_Stream0_IRQHandler    /*INT_6 : Stream 0          */
 .word     DMA1_Stream1_IRQHandler    /*INT_7                     */
 .word     DMA1_Stream2_IRQHandler    /*INT_8                     */
 .word     DMA1_Stream3_IRQHandler    /*INT_9                     */
 .word     DMA1_Stream4_IRQHandler    /*INT_10                    */
 .word     DMA1_Stream5_IRQHandler    /*INT_11                    */
 .word     DMA1_Stream6_IRQHandler    /*INT_12                    */
 .word     DMA1_Stream7_IRQHandler    /*INT_13                    */
 .word     DMA1_Stream8_IRQHandler    /*INT_14                    */
 .word     DMA1_Stream9_IRQHandler    /*INT_15                    */
 .word     DMA1_Stream10_IRQHandler   /*INT_16                    */
 .word     DMA1_Stream11_IRQHandler   /*INT_17                    */
 .word     DMA1_Stream12_IRQHandler   /*INT_18                    */
 .word     DMA1_Stream13_IRQHandler   /*INT_19                    */
 .word     DMA1_Stream14_IRQHandler   /*INT_20                    */
 .word     DMA1_Stream15_IRQHandler   /*INT_21                    */
 .word     DMA1_Stream16_IRQHandler   /*INT_22                    */
 .word     DMA1_Stream17_IRQHandler   /*INT_23                    */
 .word     DMA1_Stream18_IRQHandler   /*INT_24                    */
 .word     DMA1_Stream19_IRQHandler   /*INT_25                    */
 .word     DMA1_Stream20_IRQHandler   /*INT_26                    */
 .word     DMA1_Stream21_IRQHandler   /*INT_27                    */
 .word     DMA1_Stream22_IRQHandler   /*INT_28                    */
 .word     DMA1_Stream23_IRQHandler   /*INT_29                    */
 .word     USART0_MX_IRQHandler       /*INT_30 :                  */
 .word     USART0_RX_IRQHandler       /*INT_31 :                  */
 .word     USART0_TX_IRQHandler       /*INT_32 :                  */
 .word     USART0_RT_IRQHandler       /*INT_33 :                  */
 .word     USART0_E_IRQHandler        /*INT_34 :                  */
 .word     USART0_IRQHandler          /*INT_35 :                  */
 .word     USART1_MX_IRQHandler       /*INT_36 :                  */
 .word     USART1_RX_IRQHandler       /*INT_37 :                  */
 .word     USART1_TX_IRQHandler       /*INT_38 :                  */
 .word     USART1_RT_IRQHandler       /*INT_39 :                  */
 .word     USART1_E_IRQHandler        /*INT_40 :                  */
 .word     USART1_IRQHandler          /*INT_41 :                  */
 .word     USART2_MX_IRQHandler       /*INT_42 :                  */
 .word     USART2_RX_IRQHandler       /*INT_43 :                  */
 .word     USART2_TX_IRQHandler       /*INT_44 :                  */
 .word     USART2_RT_IRQHandler       /*INT_45 :                  */
 .word     USART2_E_IRQHandler        /*INT_46 :                  */
 .word     USART2_IRQHandler          /*INT_47 :                  */
 .word     USART3_MX_IRQHandler       /*INT_48 :                  */
 .word     USART3_RX_IRQHandler       /*INT_49 :                  */
 .word     USART3_TX_IRQHandler       /*INT_50 :                  */
 .word     USART3_RT_IRQHandler       /*INT_51 :                  */
 .word     USART3_E_IRQHandler        /*INT_52 :                  */
 .word     USART3_IRQHandler          /*INT_53 :                  */
 .word     EPWM_0_IRQHandler          /*INT_54 :                  */
 .word     EPWM_HD_0_IRQHandler       /*INT_55 :                  */
 .word     EPWM_TZ_0_IRQHandler       /*INT_56 :                  */
 .word     EPWM_1_IRQHandler          /*INT_57 :                  */
 .word     EPWM_HD_1_IRQHandler       /*INT_58 :                  */
 .word     EPWM_TZ_1_IRQHandler       /*INT_59 :                  */
 .word     EPWM_2_IRQHandler          /*INT_60 :                  */
 .word     EPWM_HD_2_IRQHandler       /*INT_61 :                  */
 .word     EPWM_TZ_2_IRQHandler       /*INT_62 :                  */
 .word     EPWM_3_IRQHandler          /*INT_63 :                  */
 .word     EPWM_HD_3_IRQHandler       /*INT_64 :                  */
 .word     EPWM_TZ_3_IRQHandler       /*INT_65 :                  */
 .word     EPWM_4_IRQHandler          /*INT_66 :                  */
 .word     EPWM_HD_4_IRQHandler       /*INT_67 :                  */
 .word     EPWM_TZ_4_IRQHandler       /*INT_68 :                  */
 .word     EPWM_5_IRQHandler          /*INT_69 :                  */
 .word     EPWM_HD_5_IRQHandler       /*INT_70 :                  */
 .word     EPWM_TZ_5_IRQHandler       /*INT_71 :                  */
 .word     EPWM_6_IRQHandler          /*INT_72 :                  */
 .word     EPWM_HD_6_IRQHandler       /*INT_73 :                  */
 .word     EPWM_TZ_6_IRQHandler       /*INT_74 :                  */
 .word     EPWM_7_IRQHandler          /*INT_75 :                  */
 .word     EPWM_HD_7_IRQHandler       /*INT_76 :                  */
 .word     EPWM_TZ_7_IRQHandler       /*INT_77 :                  */
 .word     EPWM_8_IRQHandler          /*INT_78 :                  */
 .word     EPWM_HD_8_IRQHandler       /*INT_79 :                  */
 .word     EPWM_TZ_8_IRQHandler       /*INT_80 :                  */
 .word     ADC_SEQ0_IRQHandler        /*INT_81 :                  */
 .word     ADC_SEQ1_IRQHandler        /*INT_82 :                  */
 .word     ADC_SEQ2_IRQHandler        /*INT_83 :                  */
 .word     ADC_SEQ3_IRQHandler        /*INT_84 :                  */
 .word     ADC_SEQ4_IRQHandler        /*INT_85 :                  */
 .word     ADC_SEQ5_IRQHandler        /*INT_86 :                  */
 .word     ADC_SEQ6_IRQHandler        /*INT_87 :                  */
 .word     ADC_SEQ7_IRQHandler        /*INT_88 :                  */
 .word     ADC_CompInt_IRQHandler     /*INT_89 : ECAP_0           */
 .word     ECAP0_IRQHandler           /*INT_90 : ECAP_1           */
 .word     ECAP1_IRQHandler           /*INT_91 : ECAP_2           */
 .word     ECAP2_IRQHandler           /*INT_92 : ECAP_3           */
 .word     ECAP3_IRQHandler           /*INT_93 : ECAP_4           */
 .word     ECAP4_IRQHandler           /*INT_94 : ECAP_5           */
 .word     ECAP5_IRQHandler           /*INT_95 : ECAP_6           */
 .word     EQEP0_IRQHandler           /*INT_96 : EQEP             */
 .word     EQEP1_IRQHandler           /*INT_97 : EQEP             */
 .word     BootFlash_IRQ              /*INT_98 : BOOT_FLASH       */
 .word     CMP1_IRQHandler            /*INT_99 : CMP1             */
 .word     CMP2_IRQHandler            /*INT_100 : CMP2            */
 .word     CMP3_IRQHandler            /*INT_101 : CMP3            */
 .word     SPI0_IRQHandler            /*INT_102 : SPI0            */
 .word     SPI1_IRQHandler            /*INT_103 : SPI1            */
 .word     SPI2_IRQHandler            /*INT_104 : SPI2            */
 .word     SPI3_IRQHandler            /*INT_105 : SPI3            */
 .word     UserFlash_IRQ              /*INT_106 : USER_FLASH      */
 .word     GPIOA_IRQHandler           /*INT_107 : GPIO_INT0       */
 .word     GPIOB_IRQHandler           /*INT_108 : GPIO_INT1       */
 .word     GPIOC_IRQHandler           /*INT_109 : GPIO_INT2       */
 .word     GPIOD_IRQHandler           /*INT_110 : GPIO_INT3       */
 .word     GPIOE_IRQHandler           /*INT_111 : GPIO_INT4       */
 .word     GPIOF_IRQHandler           /*INT_112 : GPIO_INT5       */
 .word     GPIOG_IRQHandler           /*INT_113 : GPIO_INT6       */
 .word     GPIOH_IRQHandler           /*INT_114 : GPIO_INT7       */
 .word     Ethernet_IRQHandler        /*INT_115 : Ethernet        */
 .word     CAN0_IRQHandler            /*INT_116 : CAN0            */
 .word     CAN1_IRQHandler            /*INT_117 : CAN1            */
 .word     CAN2_IRQHandler            /*INT_118 : CAN2            */
 .word     CAN3_IRQHandler            /*INT_119 : CAN3            */
 .word     CAN4_IRQHandler            /*INT_120 : CAN4            */
 .word     CAN5_IRQHandler            /*INT_121 : CAN5            */
 .word     CAN6_IRQHandler            /*INT_122 : CAN6            */
 .word     CAN7_IRQHandler            /*INT_123 : CAN7            */
 .word     CAN8_IRQHandler            /*INT_124 : CAN8            */
 .word     CAN9_IRQHandler            /*INT_125 : CAN9            */
 .word     CAN10_IRQHandler           /* INT_126 : CAN10          */
 .word     CAN11_IRQHandler           /* INT_127 : CAN11          */
 .word     CAN12_IRQHandler           /* INT_128 : CAN12          */
 .word     CAN13_IRQHandler           /* INT_129 : CAN13          */
 .word     CAN14_IRQHandler           /* INT_130 : CAN14          */
 .word     CAN15_IRQHandler           /* INT_131 : CAN15          */


/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/
    .weak      NMI_Handler
    .thumb_set NMI_Handler					,Default_Handler
    .weak      HardFault_Handler
    .thumb_set HardFault_Handler			,Default_Handler
    .weak      MemManage_Handler
    .thumb_set MemManage_Handler			,Default_Handler
    .weak      BusFault_Handler
    .thumb_set BusFault_Handler				,Default_Handler
    .weak      UsageFault_Handler
    .thumb_set UsageFault_Handler			,Default_Handler
    .weak      SVC_Handler
    .thumb_set SVC_Handler					,Default_Handler
    .weak      DebugMon_Handler
    .thumb_set DebugMon_Handler				,Default_Handler
    .weak      PendSV_Handler
    .thumb_set PendSV_Handler				,Default_Handler
    .weak      SysTick_Handler
    .thumb_set SysTick_Handler				,Default_Handler
	.weak WWDG_IRQHandler
	.thumb_set WWDG_IRQHandler              ,Default_Handler
	.weak I2C0_IRQHandler
	.thumb_set I2C0_IRQHandler              ,Default_Handler
	.weak I2C1_IRQHandler
	.thumb_set I2C1_IRQHandler              ,Default_Handler
	.weak TIM0_IRQHandler
	.thumb_set TIM0_IRQHandler              ,Default_Handler
	.weak TIM1_IRQHandler
	.thumb_set TIM1_IRQHandler              ,Default_Handler
	.weak TIM2_IRQHandler
	.thumb_set TIM2_IRQHandler              ,Default_Handler
	.weak DMA1_Stream0_IRQHandler
	.thumb_set DMA1_Stream0_IRQHandler      ,Default_Handler
	.weak DMA1_Stream1_IRQHandler
	.thumb_set DMA1_Stream1_IRQHandler      ,Default_Handler
	.weak DMA1_Stream2_IRQHandler
	.thumb_set DMA1_Stream2_IRQHandler      ,Default_Handler
	.weak DMA1_Stream3_IRQHandler
	.thumb_set DMA1_Stream3_IRQHandler      ,Default_Handler
	.weak DMA1_Stream4_IRQHandler
	.thumb_set DMA1_Stream4_IRQHandler      ,Default_Handler
	.weak DMA1_Stream5_IRQHandler
	.thumb_set DMA1_Stream5_IRQHandler      ,Default_Handler
	.weak DMA1_Stream6_IRQHandler
	.thumb_set DMA1_Stream6_IRQHandler      ,Default_Handler
	.weak DMA1_Stream7_IRQHandler
	.thumb_set DMA1_Stream7_IRQHandler      ,Default_Handler
	.weak DMA1_Stream8_IRQHandler
	.thumb_set DMA1_Stream8_IRQHandler      ,Default_Handler
	.weak DMA1_Stream9_IRQHandler
	.thumb_set DMA1_Stream9_IRQHandler      ,Default_Handler
	.weak DMA1_Stream10_IRQHandler
	.thumb_set DMA1_Stream10_IRQHandler     ,Default_Handler
	.weak DMA1_Stream11_IRQHandler
	.thumb_set DMA1_Stream11_IRQHandler     ,Default_Handler
	.weak DMA1_Stream12_IRQHandler
	.thumb_set DMA1_Stream12_IRQHandler     ,Default_Handler
	.weak DMA1_Stream13_IRQHandler
	.thumb_set DMA1_Stream13_IRQHandler     ,Default_Handler
	.weak DMA1_Stream14_IRQHandler
	.thumb_set DMA1_Stream14_IRQHandler     ,Default_Handler
	.weak DMA1_Stream15_IRQHandler
	.thumb_set DMA1_Stream15_IRQHandler     ,Default_Handler
	.weak DMA1_Stream16_IRQHandler
	.thumb_set DMA1_Stream16_IRQHandler     ,Default_Handler
	.weak DMA1_Stream17_IRQHandler
	.thumb_set DMA1_Stream17_IRQHandler     ,Default_Handler
	.weak DMA1_Stream18_IRQHandler
	.thumb_set DMA1_Stream18_IRQHandler     ,Default_Handler
	.weak DMA1_Stream19_IRQHandler
	.thumb_set DMA1_Stream19_IRQHandler     ,Default_Handler
	.weak DMA1_Stream20_IRQHandler
	.thumb_set DMA1_Stream20_IRQHandler     ,Default_Handler
	.weak DMA1_Stream21_IRQHandler
	.thumb_set DMA1_Stream21_IRQHandler     ,Default_Handler
	.weak DMA1_Stream22_IRQHandler
	.thumb_set DMA1_Stream22_IRQHandler     ,Default_Handler
	.weak DMA1_Stream23_IRQHandler
	.thumb_set DMA1_Stream23_IRQHandler     ,Default_Handler
	.weak USART0_MX_IRQHandler
	.thumb_set USART0_MX_IRQHandler         ,Default_Handler
	.weak USART0_RX_IRQHandler
	.thumb_set USART0_RX_IRQHandler         ,Default_Handler
	.weak USART0_TX_IRQHandler
	.thumb_set USART0_TX_IRQHandler         ,Default_Handler
	.weak USART0_RT_IRQHandler
	.thumb_set USART0_RT_IRQHandler         ,Default_Handler
	.weak USART0_E_IRQHandler
	.thumb_set USART0_E_IRQHandler          ,Default_Handler
	.weak USART0_IRQHandler
	.thumb_set USART0_IRQHandler            ,Default_Handler
	.weak USART1_MX_IRQHandler
	.thumb_set USART1_MX_IRQHandler         ,Default_Handler
	.weak USART1_RX_IRQHandler
	.thumb_set USART1_RX_IRQHandler         ,Default_Handler
	.weak USART1_TX_IRQHandler
	.thumb_set USART1_TX_IRQHandler         ,Default_Handler
	.weak USART1_RT_IRQHandler
	.thumb_set USART1_RT_IRQHandler         ,Default_Handler
	.weak USART1_E_IRQHandler
	.thumb_set USART1_E_IRQHandler          ,Default_Handler
	.weak USART1_IRQHandler
	.thumb_set USART1_IRQHandler            ,Default_Handler
	.weak USART2_MX_IRQHandler
	.thumb_set USART2_MX_IRQHandler         ,Default_Handler
	.weak USART2_RX_IRQHandler
	.thumb_set USART2_RX_IRQHandler         ,Default_Handler
	.weak USART2_TX_IRQHandler
	.thumb_set USART2_TX_IRQHandler         ,Default_Handler
	.weak USART2_RT_IRQHandler
	.thumb_set USART2_RT_IRQHandler         ,Default_Handler
	.weak USART2_E_IRQHandler
	.thumb_set USART2_E_IRQHandler          ,Default_Handler
	.weak USART2_IRQHandler
	.thumb_set USART2_IRQHandler            ,Default_Handler
	.weak USART3_MX_IRQHandler
	.thumb_set USART3_MX_IRQHandler         ,Default_Handler
	.weak USART3_RX_IRQHandler
	.thumb_set USART3_RX_IRQHandler         ,Default_Handler
	.weak USART3_TX_IRQHandler
	.thumb_set USART3_TX_IRQHandler         ,Default_Handler
	.weak USART3_RT_IRQHandler
	.thumb_set USART3_RT_IRQHandler         ,Default_Handler
	.weak USART3_E_IRQHandler
	.thumb_set USART3_E_IRQHandler          ,Default_Handler
	.weak USART3_IRQHandler
	.thumb_set USART3_IRQHandler            ,Default_Handler
	.weak EPWM_0_IRQHandler
	.thumb_set EPWM_0_IRQHandler            ,Default_Handler
	.weak EPWM_HD_0_IRQHandler
	.thumb_set EPWM_HD_0_IRQHandler         ,Default_Handler
	.weak EPWM_TZ_0_IRQHandler
	.thumb_set EPWM_TZ_0_IRQHandler         ,Default_Handler
	.weak EPWM_1_IRQHandler
	.thumb_set EPWM_1_IRQHandler            ,Default_Handler
	.weak EPWM_HD_1_IRQHandler
	.thumb_set EPWM_HD_1_IRQHandler         ,Default_Handler
	.weak EPWM_TZ_1_IRQHandler
	.thumb_set EPWM_TZ_1_IRQHandler         ,Default_Handler
	.weak EPWM_2_IRQHandler
	.thumb_set EPWM_2_IRQHandler            ,Default_Handler
	.weak EPWM_HD_2_IRQHandler
	.thumb_set EPWM_HD_2_IRQHandler         ,Default_Handler
	.weak EPWM_TZ_2_IRQHandler
	.thumb_set EPWM_TZ_2_IRQHandler         ,Default_Handler
	.weak EPWM_3_IRQHandler
	.thumb_set EPWM_3_IRQHandler            ,Default_Handler
	.weak EPWM_HD_3_IRQHandler
	.thumb_set EPWM_HD_3_IRQHandler         ,Default_Handler
	.weak EPWM_TZ_3_IRQHandler
	.thumb_set EPWM_TZ_3_IRQHandler         ,Default_Handler
	.weak EPWM_4_IRQHandler
	.thumb_set EPWM_4_IRQHandler            ,Default_Handler
	.weak EPWM_HD_4_IRQHandler
	.thumb_set EPWM_HD_4_IRQHandler         ,Default_Handler
	.weak EPWM_TZ_4_IRQHandler
	.thumb_set EPWM_TZ_4_IRQHandler         ,Default_Handler
	.weak EPWM_5_IRQHandler
	.thumb_set EPWM_5_IRQHandler            ,Default_Handler
	.weak EPWM_HD_5_IRQHandler
	.thumb_set EPWM_HD_5_IRQHandler         ,Default_Handler
	.weak EPWM_TZ_5_IRQHandler
	.thumb_set EPWM_TZ_5_IRQHandler         ,Default_Handler
	.weak EPWM_6_IRQHandler
	.thumb_set EPWM_6_IRQHandler            ,Default_Handler
	.weak EPWM_HD_6_IRQHandler
	.thumb_set EPWM_HD_6_IRQHandler         ,Default_Handler
	.weak EPWM_TZ_6_IRQHandler
	.thumb_set EPWM_TZ_6_IRQHandler         ,Default_Handler
	.weak EPWM_7_IRQHandler
	.thumb_set EPWM_7_IRQHandler            ,Default_Handler
	.weak EPWM_HD_7_IRQHandler
	.thumb_set EPWM_HD_7_IRQHandler         ,Default_Handler
	.weak EPWM_TZ_7_IRQHandler
	.thumb_set EPWM_TZ_7_IRQHandler         ,Default_Handler
	.weak EPWM_8_IRQHandler
	.thumb_set EPWM_8_IRQHandler            ,Default_Handler
	.weak EPWM_HD_8_IRQHandler
	.thumb_set EPWM_HD_8_IRQHandler         ,Default_Handler
	.weak EPWM_TZ_8_IRQHandler
	.thumb_set EPWM_TZ_8_IRQHandler         ,Default_Handler
	.weak ADC_SEQ0_IRQHandler
	.thumb_set ADC_SEQ0_IRQHandler          ,Default_Handler
	.weak ADC_SEQ1_IRQHandler
	.thumb_set ADC_SEQ1_IRQHandler          ,Default_Handler
	.weak ADC_SEQ2_IRQHandler
	.thumb_set ADC_SEQ2_IRQHandler          ,Default_Handler
	.weak ADC_SEQ3_IRQHandler
	.thumb_set ADC_SEQ3_IRQHandler          ,Default_Handler
	.weak ADC_SEQ4_IRQHandler
	.thumb_set ADC_SEQ4_IRQHandler          ,Default_Handler
	.weak ADC_SEQ5_IRQHandler
	.thumb_set ADC_SEQ5_IRQHandler          ,Default_Handler
	.weak ADC_SEQ6_IRQHandler
	.thumb_set ADC_SEQ6_IRQHandler          ,Default_Handler
	.weak ADC_SEQ7_IRQHandler
	.thumb_set ADC_SEQ7_IRQHandler          ,Default_Handler
	.weak ADC_CompInt_IRQHandler
	.thumb_set ADC_CompInt_IRQHandler       ,Default_Handler
	.weak ECAP0_IRQHandler
	.thumb_set ECAP0_IRQHandler             ,Default_Handler
	.weak ECAP1_IRQHandler
	.thumb_set ECAP1_IRQHandler             ,Default_Handler
	.weak ECAP2_IRQHandler
	.thumb_set ECAP2_IRQHandler             ,Default_Handler
	.weak ECAP3_IRQHandler
	.thumb_set ECAP3_IRQHandler             ,Default_Handler
	.weak ECAP4_IRQHandler
	.thumb_set ECAP4_IRQHandler             ,Default_Handler
	.weak ECAP5_IRQHandler
	.thumb_set ECAP5_IRQHandler             ,Default_Handler
	.weak EQEP0_IRQHandler
	.thumb_set EQEP0_IRQHandler             ,Default_Handler
	.weak EQEP1_IRQHandler
	.thumb_set EQEP1_IRQHandler             ,Default_Handler
	.weak BootFlash_IRQ
	.thumb_set BootFlash_IRQ                ,Default_Handler
	.weak CMP1_IRQHandler
	.thumb_set CMP1_IRQHandler              ,Default_Handler
	.weak CMP2_IRQHandler
	.thumb_set CMP2_IRQHandler              ,Default_Handler
	.weak CMP3_IRQHandler
	.thumb_set CMP3_IRQHandler              ,Default_Handler
	.weak SPI0_IRQHandler
	.thumb_set SPI0_IRQHandler              ,Default_Handler
	.weak SPI1_IRQHandler
	.thumb_set SPI1_IRQHandler              ,Default_Handler
	.weak SPI2_IRQHandler
	.thumb_set SPI2_IRQHandler              ,Default_Handler
	.weak SPI3_IRQHandler
	.thumb_set SPI3_IRQHandler              ,Default_Handler
	.weak UserFlash_IRQ
	.thumb_set UserFlash_IRQ                ,Default_Handler
	.weak GPIOA_IRQHandler
	.thumb_set GPIOA_IRQHandler             ,Default_Handler
	.weak GPIOB_IRQHandler
	.thumb_set GPIOB_IRQHandler             ,Default_Handler
	.weak GPIOC_IRQHandler
	.thumb_set GPIOC_IRQHandler             ,Default_Handler
	.weak GPIOD_IRQHandler
	.thumb_set GPIOD_IRQHandler             ,Default_Handler
	.weak GPIOE_IRQHandler
	.thumb_set GPIOE_IRQHandler             ,Default_Handler
	.weak GPIOF_IRQHandler
	.thumb_set GPIOF_IRQHandler             ,Default_Handler
	.weak GPIOG_IRQHandler
	.thumb_set GPIOG_IRQHandler             ,Default_Handler
	.weak GPIOH_IRQHandler
	.thumb_set GPIOH_IRQHandler             ,Default_Handler
	.weak Ethernet_IRQHandler
	.thumb_set Ethernet_IRQHandler          ,Default_Handler
	.weak CAN0_IRQHandler
	.thumb_set CAN0_IRQHandler              ,Default_Handler
	.weak CAN1_IRQHandler
	.thumb_set CAN1_IRQHandler              ,Default_Handler
	.weak CAN2_IRQHandler
	.thumb_set CAN2_IRQHandler              ,Default_Handler
	.weak CAN3_IRQHandler
	.thumb_set CAN3_IRQHandler              ,Default_Handler
	.weak CAN4_IRQHandler
	.thumb_set CAN4_IRQHandler              ,Default_Handler
	.weak CAN5_IRQHandler
	.thumb_set CAN5_IRQHandler              ,Default_Handler
	.weak CAN6_IRQHandler
	.thumb_set CAN6_IRQHandler              ,Default_Handler
	.weak CAN7_IRQHandler
	.thumb_set CAN7_IRQHandler              ,Default_Handler
	.weak CAN8_IRQHandler
	.thumb_set CAN8_IRQHandler              ,Default_Handler
	.weak CAN9_IRQHandler
	.thumb_set CAN9_IRQHandler              ,Default_Handler
	.weak CAN10_IRQHandler
	.thumb_set CAN10_IRQHandler             ,Default_Handler
	.weak CAN11_IRQHandler
	.thumb_set CAN11_IRQHandler             ,Default_Handler
	.weak CAN12_IRQHandler
	.thumb_set CAN12_IRQHandler             ,Default_Handler
	.weak CAN13_IRQHandler
	.thumb_set CAN13_IRQHandler             ,Default_Handler
	.weak CAN14_IRQHandler
	.thumb_set CAN14_IRQHandler             ,Default_Handler
	.weak CAN15_IRQHandler
	.thumb_set CAN15_IRQHandler             ,Default_Handler


