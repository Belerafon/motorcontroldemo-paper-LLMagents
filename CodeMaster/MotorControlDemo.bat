REM   D:\Eclipse_GCC_ARM_wks\motorcontroldemo\CodeMaster\MotorControlDemo.ipp - command file
REM   Created: Jun 20, 2017  13:32:08

PushD
CD..
If Exist "D:\Eclipse_GCC_ARM_wks\motorcontroldemo\CodeMaster\Debug\Exe\MotorControlDemo.ELF" Del "D:\Eclipse_GCC_ARM_wks\motorcontroldemo\CodeMaster\Debug\Exe\MotorControlDemo.ELF"
If Exist "D:\Eclipse_GCC_ARM_wks\motorcontroldemo\CodeMaster\Debug\Exe\MotorControlDemo.MCE" Del "D:\Eclipse_GCC_ARM_wks\motorcontroldemo\CodeMaster\Debug\Exe\MotorControlDemo.MCE"
If Exist "D:\Eclipse_GCC_ARM_wks\motorcontroldemo\CodeMaster\Debug\Exe\MotorControlDemo.HEX" Del "D:\Eclipse_GCC_ARM_wks\motorcontroldemo\CodeMaster\Debug\Exe\MotorControlDemo.HEX"
"C:\Program Files (x86)\CodeMasterPlusPlus-ARM\1_04_00\CMC-ARM\Bin\MCAARM.EXE" @D:\Eclipse_GCC_ARM_wks\motorcontroldemo\CodeMaster\startup_K1921BK01T.{1}
"C:\Program Files (x86)\CodeMasterPlusPlus-ARM\1_04_00\CMC-ARM\Bin\MCCARM.EXE" @D:\Eclipse_GCC_ARM_wks\motorcontroldemo\CodeMaster\CANBlockTransferInterface.{2}
"C:\Program Files (x86)\CodeMasterPlusPlus-ARM\1_04_00\CMC-ARM\Bin\MCCARM.EXE" @D:\Eclipse_GCC_ARM_wks\motorcontroldemo\CodeMaster\K1921VK01T_init.{3}
"C:\Program Files (x86)\CodeMasterPlusPlus-ARM\1_04_00\CMC-ARM\Bin\MCCARM.EXE" @D:\Eclipse_GCC_ARM_wks\motorcontroldemo\CodeMaster\V_data_log.{4}
"C:\Program Files (x86)\CodeMasterPlusPlus-ARM\1_04_00\CMC-ARM\Bin\MCLINK.EXE" @D:\Eclipse_GCC_ARM_wks\motorcontroldemo\CodeMaster\MotorControlDemo.(1)
PopD
