REM   D:\User\Aliamkin\Projects\VectorCard\VectorCARDServoInverter\MotorControlDemo\CodeMaster\MotorControlDemo.ipp - command file
REM   Created: Apr 26, 2017  15:12:56

PushD
CD..
If Exist "D:\User\Aliamkin\Projects\VectorCard\VectorCARDServoInverter\MotorControlDemo\CodeMaster\Debug\Exe\MotorControlDemo.ELF" Del "D:\User\Aliamkin\Projects\VectorCard\VectorCARDServoInverter\MotorControlDemo\CodeMaster\Debug\Exe\MotorControlDemo.ELF"
If Exist "D:\User\Aliamkin\Projects\VectorCard\VectorCARDServoInverter\MotorControlDemo\CodeMaster\Debug\Exe\MotorControlDemo.MCE" Del "D:\User\Aliamkin\Projects\VectorCard\VectorCARDServoInverter\MotorControlDemo\CodeMaster\Debug\Exe\MotorControlDemo.MCE"
If Exist "D:\User\Aliamkin\Projects\VectorCard\VectorCARDServoInverter\MotorControlDemo\CodeMaster\Debug\Exe\MotorControlDemo.HEX" Del "D:\User\Aliamkin\Projects\VectorCard\VectorCARDServoInverter\MotorControlDemo\CodeMaster\Debug\Exe\MotorControlDemo.HEX"
"C:\Program Files (x86)\CodeMasterPlusPlus-ARM\1_03_00\CMC-ARM\Bin\MCAARM.EXE" @D:\User\Aliamkin\Projects\VectorCard\VectorCARDServoInverter\MotorControlDemo\CodeMaster\startup_K1921BK01T.{1}
"C:\Program Files (x86)\CodeMasterPlusPlus-ARM\1_03_00\CMC-ARM\Bin\MCCARM.EXE" @D:\User\Aliamkin\Projects\VectorCard\VectorCARDServoInverter\MotorControlDemo\CodeMaster\CANBlockTransferInterface.{2}
"C:\Program Files (x86)\CodeMasterPlusPlus-ARM\1_03_00\CMC-ARM\Bin\MCCARM.EXE" @D:\User\Aliamkin\Projects\VectorCard\VectorCARDServoInverter\MotorControlDemo\CodeMaster\K1921VK01T_init.{3}
"C:\Program Files (x86)\CodeMasterPlusPlus-ARM\1_03_00\CMC-ARM\Bin\MCCARM.EXE" @D:\User\Aliamkin\Projects\VectorCard\VectorCARDServoInverter\MotorControlDemo\CodeMaster\V_CurPar.{4}
"C:\Program Files (x86)\CodeMasterPlusPlus-ARM\1_03_00\CMC-ARM\Bin\MCLINK.EXE" @D:\User\Aliamkin\Projects\VectorCard\VectorCARDServoInverter\MotorControlDemo\CodeMaster\MotorControlDemo.(1)
PopD
