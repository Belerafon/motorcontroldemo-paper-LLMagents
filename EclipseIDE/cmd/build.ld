/******************************************************************************
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
 
 * @file		build.ld
 * @brief		Файл распределения памяти для выполнения из FLASH
 * @version		v1.0
 * @date		11 декабря 2015
 *
 * @author 		ООО "НПФ Вектор", http://motorcontrol.ru
 *
 ******************************************************************************/

/* Entry Point */
ENTRY(Reset_Handler)


/* Generate a link error if heap and stack don't fit into RAM */
_Stack_Size = 0x1000; /* required amount of stack */

/* Specify the memory areas */
/* Буквы в скобках определяют атрибуты: доступ на чтение, 
 * запись, исполнение, выделение памяти. */
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x00000000, LENGTH = 1024K
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 192K
}
/* Конец РАМы на НИИЭТЕ (с запасиком) - отсюда внутрь будет расти стек*/
_estack = ORIGIN(RAM) + LENGTH(RAM) - 4;  


/* Define output sections */
SECTIONS
{
  . = ORIGIN(RAM);
  /* таблица векторов прерываний, обязана быть выровнена по 0х80 */  
  .isr_vector ALIGN(0x80): 
  {
    __isr_vector_flash_start = LOADADDR (.isr_vector); /* Берем адрес, где на флеше лежит эта таблица */
    __isr_vector_ram_start = .;
    KEEP(*(.isr_vector)) /* Startup code - KEEP запрещает удалить секцию сборщику мусора */
    . = ALIGN(4);
    __isr_vector_ram_end = .; /* конец оперативки, куда будет скопирован код */
  } >RAM AT>FLASH

  /* Секция для кода, который исполняется в оперативке, а лежит на флеше.
   * В начале мейна он копируется из ROM в RAM.*/
  .fastcode ALIGN(4): {
	 __fastcode_flash_start = LOADADDR (.fastcode); /* Берем адрес, где на флеше лежит код. */
	 /* Точка - это курсор текущего размещения в памяти. Т.е. в  
	  * __fastcode_ram_start будет лежать адрес оперативки, где будет исполняться скопированный код.*/
	 __fastcode_ram_start = .;
	 *(.glue_7t) *(.glue_7)
	*(.fastcode)
	*(.fastcode*)
	. = ALIGN (4);
	__fastcode_ram_end = .; /* конец оперативки, куда будет скопирован код */
   } >RAM AT>FLASH
   
  /* The program code and other data goes into RAM */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
	*(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    
    /* define a global symbols at end of code - указатель на конец кода, 
  	  т.к. точка - это переменная с инкрементирующимся значением адреса (называют её курсор)*/
    _etext = .;  
  } >FLASH


    /* .preinit_array - секция для указателей на функции пре-инициализации,
     *                  вызываемых перед любыми другими функциями
     *                  инициализации (перед выполнением кода из секции
     *                  '.init' и вызовами функций из секции '.init_array').
     *
     * Специальная секция с именем '.preinit_array' и вышеназванным
     * назначением описана в System V gABI. Вызов функций, указатели на
     * которые содержатся в секции '.preinit_array', возлагается на библиотеку
     * времени выполнения (runtime library) -- например, на C runtime. Если в
     * библиотеке не предусмотрен такой вызов, то функции из секции
     * '.preinit_array' не будут вызваны.
     *
     * Секция '.preinit_array' предназначена для пре-инициализации
     * исполняемого файла (executable), выполняемой перед инициализацией
     * динамически скомпонованных с ним разделяемых объектов (shared objects).
     *
     * Символы __preinit_array_start и __preinit_array_end используются
     * библиотекой C времени выполнения (newlib, glibc).
     */
  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  
    /* .init_array - секции с указателями на функции инициализации,
     *               выполняющиеся перед вызовом входной точки программы, т.е.
     *               перед вызовом функции main.
     *
     * Специальная секция с именем '.init_array' и вышеназванным назначением
     * описана в System V gABI. Вызов функций, указатели на которые содержатся в
     * секции '.init_array', возлагается на библиотеку времени выполнения
     * (runtime library) -- например, на C runtime. Если в библиотеке не
     * предусмотрен такой вызов, то функции из секции '.init_array' не будут
     * вызваны.
     *
     * GCC использует секцию '.init_array' для обеспечения вызова статических
     * конструкторов: функций, объявленных с __attribute__((constructor)).
     *
     * Для статических конструкторов с объявленным приоритетом PRIORITY,
     * используются секции с именем '.init_array.PRIORITY'
     *
     * Также GCC использует секцию '.init_array' для вызова конструкторов
     * статических объектов C++.
     *
     * Символы __init_array_start и __init_array_end используются библиотекой
     * C времени выполнения (newlib, glibc).
     */
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  /* .fini_array - секции с указателями на функции терминации,
   *               выполняющиеся по завершению программы (после возврата из
   *               входной точки программы, т.е. из функции main).
   *
   * Специальная секция с именем '.fini_array' и вышеназванным назначением
   * описана в System V gABI. Вызов функций, указатели на которые содержатся в
   * секции '.fini_array', возлагается на библиотеку времени выполнения
   * (runtime library) -- например, на C runtime. Если в библиотеке не
   * предусмотрен такой вызов, то функции из секции '.fini_array' не будут
   * вызваны.
   *
   * GCC использует секцию '.fini_array' для обеспечения вызова статических
   * деструкторов: функций, объявленных с __attribute__((destructor)).
   *
   * Для статических деструкторов с объявленным приоритетом PRIORITY,
   * используются секции с именем '.fini_array.PRIORITY'
   *
   * Символы __fini_array_start и __fini_array_end используются библиотекой
   * C времени выполнения (newlib, glibc).
   */
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(.fini_array*))
    KEEP (*(SORT(.fini_array.*)))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = .;


  /* Выражение _sidata, которое следует за ключевым словом AT, 
   * определяет адрес загрузки секции. По умолчанию, если Вы 
   * не использовали ключевое слово AT, адрес загрузки равен адресу перемещения. 
   * Так как _sidata до этого было равно точке (указатель на текущий адрес),
   * то .data пойдет сразу дальше после неё. Как бы динамическое смещение такое. 
   * */
  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : AT ( _sidata )
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  
  /* Функци проверяет, что после всего кода осталось место под стек. 
   * Стек сделан снизу оперативки!
   * Не размещайте свои секции после этой функции, только перед!*/
  ._user_heap_stack :
  {
    . = ALIGN(4);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Stack_Size + 4;
    . = ALIGN(4);
  } >RAM
  
  
  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  /*  Тут какие-то атрибуты об особенностях компиляции, специфичная инфа производителя и хз еще что, читайте
   * 4.3.6 Build Attributes  http://infocenter.arm.com/help/topic/com.arm.doc.ihi0044e/IHI0044E_aaelf.pdf */
  .ARM.attributes 0 : { *(.ARM.attributes) }
}
