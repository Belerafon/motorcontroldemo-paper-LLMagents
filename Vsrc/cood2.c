//Файл: cood.c
//Профиль: D:\Eclipse_GCC_ARM_wks\VectorCARDServoInverter\MotorControlDemo\cood.xml
//Дата созданиЯ: 13.04.2017 15:36:17
//Пользователь: default
#include "DSP.h"
#include "main.h"
#include "cood2.h"
#include "co_ODvars.h"

// всего 125 индексов в словаре
// всего 854 элементов в словаре
Uint16 const co2_numOfInd = 125;
//Номер перечеслениЯ длЯ групп
Uint16 const co2_groupEnum = 15;
//Хэш-сумма:
Uint32 const co2_odHash[4] = {0x760211E8, 0xDBC1D28B, 0xDFE03DFD, 0x842D8E52};
//Таблица с адресами переменных 

long const CO2_OD_TBL3[] = { (long)(&co2_vars.co_deviceType), //[1000h.00h], Название=Тип устройства, ТекстПУ=Device Type, EngText=Device Type, группа=CAN
(long)(&co2_vars.co_errorRegister), //[1001h.00h], Название=Регистр ошибок, ТекстПУ=ErrReg, EngText=Error Register, группа=CAN
(long)(&co2_vars.co_deviceState), //[1002h.00h], Название=Manufacturer Status Register, ТекстПУ=DevSta, EngText=Manufacturer Status Register, группа=CAN
(long)(&co2_vars.co_emcyCOBID), //[1014h.00h], Название=COB-ID Emergency Object, ТекстПУ=eCOBID, EngText=COB-ID Emergency Object, группа=CAN
(long)(&co2_vars.co_CHBT), //[1016h.01h], Название=Время ожидания сообщения Heartbeat, ТекстПУ=CHBT, EngText=Consumer Heartbeat Time, группа=CAN
(long)(&co2_vars.co_PHBT), //[1017h.00h], Название=Период генерации сообщения Heartbeat, ТекстПУ=PHBT, EngText=Producer Heartbeat Time, группа=CAN
(long)(&co2_vars.co_vendorID), //[1018h.01h], Название=Код производителя, ТекстПУ=VendID, EngText=Vendor ID, группа=CAN
(long)(&co2_vars.co_productCode), //[1018h.02h], Название=Код продукта, ТекстПУ=PrCode, EngText=Product Code, группа=CAN
(long)(&co2_vars.co_revisionNumber), //[1018h.03h], Название=Номер ревизии, ТекстПУ=RevNum, EngText=Revision Number, группа=CAN
(long)(&co2_vars.co_serialNumber), //[1018h.04h], Название=Серийный номер, ТекстПУ=SerNum, EngText=Serial Number, группа=CAN
(long)(&co2_vars.co_csrxCOBID), //[1200h.01h], Название=COB-ID Client->Server(rx), ТекстПУ=CSRXID, EngText=COB-ID Client->Server(rx), группа=CAN
(long)(&co2_vars.co_sctxCOBID), //[1200h.02h], Название=COB-ID Server->Client(tx), ТекстПУ=SCTXID, EngText=COB-ID Server->Client(tx), группа=CAN
(long)(&co2_vars.co_cstxCOBID), //[1280h.01h], Название=COB-ID Client->Server(tx), ТекстПУ=CSTXID, EngText=COB-ID Client->Server(tx), группа=CAN
(long)(&co2_vars.co_scrxCOBID), //[1280h.02h], Название=COB-ID Server->Client(rx), ТекстПУ=SCRXID, EngText=COB-ID Server->Client(rx), группа=CAN
(long)(&co2_vars.co_ssNODEID), //[1280h.03h], Название=NODE-ID on SDO server, ТекстПУ=SDOSID, EngText=NODE-ID on SDO server, группа=CAN
(long)(&co2_vars.co_RPDO1COBID), //[1400h.01h], Название=COB-ID for RPDO1, ТекстПУ=IRPDO1, EngText=COB-ID for RPDO1, группа=CAN
(long)(&co2_vars.co_transType), //[1400h.02h], Название=Тип передачи, ТекстПУ=TransT, EngText=Transmission Type, группа=CAN
(long)(&co2_vars.co_RPDO2COBID), //[1401h.01h], Название=COB-ID for RPDO2, ТекстПУ=IRPDO2, EngText=COB-ID for RPDO2, группа=CAN
(long)(&co2_vars.co_transType), //[1401h.02h], Название=Тип передачи, ТекстПУ=TransT, EngText=Transmission Type, группа=CAN
(long)(&co2_vars.co_RPDO3COBID), //[1402h.01h], Название=COB-ID for RPDO3, ТекстПУ=IRPDO3, EngText=COB-ID for RPDO3, группа=CAN
(long)(&co2_vars.co_transType), //[1402h.02h], Название=Тип передачи, ТекстПУ=TransT, EngText=Transmission Type, группа=CAN
(long)(&co2_vars.co_RPDO4COBID), //[1403h.01h], Название=COB-ID for RPDO4, ТекстПУ=IRPDO4, EngText=COB-ID for RPDO4, группа=CAN
(long)(&co2_vars.co_transType), //[1403h.02h], Название=Тип передачи, ТекстПУ=TransT, EngText=Transmission Type, группа=CAN
(long)(&co2_vars.co_RPDO5COBID), //[1404h.01h], Название=COB-ID for RPDO5, ТекстПУ=IRPDO5, EngText=COB-ID for RPDO5, группа=CAN
(long)(&co2_vars.co_transType), //[1404h.02h], Название=Тип передачи, ТекстПУ=TransT, EngText=Transmission Type, группа=CAN
(long)(&co2_vars.co_RPDO6COBID), //[1405h.01h], Название=COB-ID for RPDO6, ТекстПУ=IRPDO6, EngText=COB-ID for RPDO6, группа=CAN
(long)(&co2_vars.co_transType), //[1405h.02h], Название=Тип передачи, ТекстПУ=TransT, EngText=Transmission Type, группа=CAN
(long)(&co2_vars.co_RPDO7COBID), //[1406h.01h], Название=COB-ID for RPDO7, ТекстПУ=IRPDO7, EngText=COB-ID for RPDO7, группа=CAN
(long)(&co2_vars.co_transType), //[1406h.02h], Название=Тип передачи, ТекстПУ=TransT, EngText=Transmission Type, группа=CAN
(long)(&co2_vars.co_RPDO8COBID), //[1407h.01h], Название=COB-ID for RPDO8, ТекстПУ=IRPDO8, EngText=COB-ID for RPDO8, группа=CAN
(long)(&co2_vars.co_transType), //[1407h.02h], Название=Тип передачи, ТекстПУ=TransT, EngText=Transmission Type, группа=CAN
(long)(&co2_vars.co_RPDO1_1Mapping), //[1600h.01h], Название=RPDO1-1 Mapping, ТекстПУ=RPM11, EngText=RPDO1-1 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO1_2Mapping), //[1600h.02h], Название=RPDO1-2 Mapping, ТекстПУ=RPM12, EngText=RPDO1-2 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO1_3Mapping), //[1600h.03h], Название=RPDO1-3 Mapping, ТекстПУ=RPM13, EngText=RPDO1-3 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO1_4Mapping), //[1600h.04h], Название=RPDO1-4 Mapping, ТекстПУ=RPM14, EngText=RPDO1-4 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO2_1Mapping), //[1601h.01h], Название=RPDO2-1 Mapping, ТекстПУ=RPM21, EngText=RPDO2-1 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO2_2Mapping), //[1601h.02h], Название=RPDO2-2 Mapping, ТекстПУ=RPM22, EngText=RPDO2-2 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO2_3Mapping), //[1601h.03h], Название=RPDO2-3 Mapping, ТекстПУ=RPM23, EngText=RPDO2-3 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO2_4Mapping), //[1601h.04h], Название=RPDO2-4 Mapping, ТекстПУ=RPM24, EngText=RPDO2-4 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO3_1Mapping), //[1602h.01h], Название=RPDO3-1 Mapping, ТекстПУ=RPM31, EngText=RPDO3-1 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO3_2Mapping), //[1602h.02h], Название=RPDO3-2 Mapping, ТекстПУ=RPM32, EngText=RPDO3-2 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO3_3Mapping), //[1602h.03h], Название=RPDO3-3 Mapping, ТекстПУ=RPM33, EngText=RPDO3-3 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO3_4Mapping), //[1602h.04h], Название=RPDO3-4 Mapping, ТекстПУ=RPM34, EngText=RPDO3-4 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO4_1Mapping), //[1603h.01h], Название=RPDO4-1 Mapping, ТекстПУ=RPM41, EngText=RPDO4-1 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO4_2Mapping), //[1603h.02h], Название=RPDO4-2 Mapping, ТекстПУ=RPM42, EngText=RPDO4-2 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO4_3Mapping), //[1603h.03h], Название=RPDO4-3 Mapping, ТекстПУ=RPM43, EngText=RPDO4-3 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO4_4Mapping), //[1603h.04h], Название=RPDO4-4 Mapping, ТекстПУ=RPM44, EngText=RPDO4-4 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO5_1Mapping), //[1604h.01h], Название=RPDO5-1 Mapping, ТекстПУ=RPM51, EngText=RPDO5-1 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO5_2Mapping), //[1604h.02h], Название=RPDO5-2 Mapping, ТекстПУ=RPM52, EngText=RPDO5-2 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO5_3Mapping), //[1604h.03h], Название=RPDO5-3 Mapping, ТекстПУ=RPM53, EngText=RPDO5-3 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO5_4Mapping), //[1604h.04h], Название=RPDO5-4 Mapping, ТекстПУ=RPM54, EngText=RPDO5-4 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO6_1Mapping), //[1605h.01h], Название=RPDO6-1 Mapping, ТекстПУ=RPM61, EngText=RPDO6-1 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO6_2Mapping), //[1605h.02h], Название=RPDO6-2 Mapping, ТекстПУ=RPM62, EngText=RPDO6-2 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO6_3Mapping), //[1605h.03h], Название=RPDO6-3 Mapping, ТекстПУ=RPM63, EngText=RPDO6-3 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO6_4Mapping), //[1605h.04h], Название=RPDO6-4 Mapping, ТекстПУ=RPM64, EngText=RPDO6-4 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO7_1Mapping), //[1606h.01h], Название=RPDO7-1 Mapping, ТекстПУ=RPM71, EngText=RPDO7-1 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO7_2Mapping), //[1606h.02h], Название=RPDO7-2 Mapping, ТекстПУ=RPM72, EngText=RPDO7-2 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO7_3Mapping), //[1606h.03h], Название=RPDO7-3 Mapping, ТекстПУ=RPM73, EngText=RPDO7-3 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO7_4Mapping), //[1606h.04h], Название=RPDO7-4 Mapping, ТекстПУ=RPM74, EngText=RPDO7-4 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO8_1Mapping), //[1607h.01h], Название=RPDO8-1 Mapping, ТекстПУ=RPM81, EngText=RPDO8-1 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO8_2Mapping), //[1607h.02h], Название=RPDO8-2 Mapping, ТекстПУ=RPM82, EngText=RPDO8-2 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO8_3Mapping), //[1607h.03h], Название=RPDO8-3 Mapping, ТекстПУ=RPM83, EngText=RPDO8-3 Mapping, группа=CAN
(long)(&co2_vars.co_RPDO8_4Mapping), //[1607h.04h], Название=RPDO8-4 Mapping, ТекстПУ=RPM84, EngText=RPDO8-4 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO1COBID), //[1800h.01h], Название=COB-ID for TPDO1, ТекстПУ=ITPDO1, EngText=COB-ID for TPDO1, группа=CAN
(long)(&co2_vars.co_transType), //[1800h.02h], Название=Тип передачи, ТекстПУ=TransT, EngText=Transmission Type, группа=CAN
(long)(&co2_vars.co_TPDO1ITime), //[1800h.03h], Название=Inhibit Time 1, ТекстПУ=ITime1, EngText=Inhibit Time 1, группа=CAN
(long)(&co2_vars.co_compatEntry), //[1800h.04h], Название=Compatibility Entry, ТекстПУ=CompEn, EngText=Compatibility Entry, группа=CAN
(long)(&co2_vars.co_TPDO1EventTimer), //[1800h.05h], Название=Event Timer 1, ТекстПУ=EvTmr1, EngText=Event Timer 1, группа=CAN
(long)(&co2_vars.co_TPDO2COBID), //[1801h.01h], Название=COB-ID for TPDO2, ТекстПУ=ITPDO2, EngText=COB-ID for TPDO2, группа=CAN
(long)(&co2_vars.co_transType), //[1801h.02h], Название=Тип передачи, ТекстПУ=TransT, EngText=Transmission Type, группа=CAN
(long)(&co2_vars.co_TPDO2ITime), //[1801h.03h], Название=Inhibit Time 2, ТекстПУ=ITime2, EngText=Inhibit Time 2, группа=CAN
(long)(&co2_vars.co_compatEntry), //[1801h.04h], Название=Compatibility Entry, ТекстПУ=CompEn, EngText=Compatibility Entry, группа=CAN
(long)(&co2_vars.co_TPDO2EventTimer), //[1801h.05h], Название=Event Timer 2, ТекстПУ=EvTmr2, EngText=Event Timer 2, группа=CAN
(long)(&co2_vars.co_TPDO3COBID), //[1802h.01h], Название=COB-ID for TPDO3, ТекстПУ=ITPDO3, EngText=COB-ID for TPDO3, группа=CAN
(long)(&co2_vars.co_transType), //[1802h.02h], Название=Тип передачи, ТекстПУ=TransT, EngText=Transmission Type, группа=CAN
(long)(&co2_vars.co_TPDO3ITime), //[1802h.03h], Название=Inhibit Time 3, ТекстПУ=ITime3, EngText=Inhibit Time 3, группа=CAN
(long)(&co2_vars.co_compatEntry), //[1802h.04h], Название=Compatibility Entry, ТекстПУ=CompEn, EngText=Compatibility Entry, группа=CAN
(long)(&co2_vars.co_TPDO3EventTimer), //[1802h.05h], Название=Event Timer 3, ТекстПУ=EvTmr3, EngText=Event Timer 3, группа=CAN
(long)(&co2_vars.co_TPDO4COBID), //[1803h.01h], Название=COB-ID for TPDO4, ТекстПУ=ITPDO4, EngText=COB-ID for TPDO4, группа=CAN
(long)(&co2_vars.co_transType), //[1803h.02h], Название=Тип передачи, ТекстПУ=TransT, EngText=Transmission Type, группа=CAN
(long)(&co2_vars.co_TPDO4ITime), //[1803h.03h], Название=Inhibit Time 4, ТекстПУ=ITime4, EngText=Inhibit Time 4, группа=CAN
(long)(&co2_vars.co_compatEntry), //[1803h.04h], Название=Compatibility Entry, ТекстПУ=CompEn, EngText=Compatibility Entry, группа=CAN
(long)(&co2_vars.co_TPDO4EventTimer), //[1803h.05h], Название=Event Timer 4, ТекстПУ=EvTmr4, EngText=Event Timer 4, группа=CAN
(long)(&co2_vars.co_TPDO5COBID), //[1804h.01h], Название=COB-ID for TPDO5, ТекстПУ=ITPDO5, EngText=COB-ID for TPDO5, группа=CAN
(long)(&co2_vars.co_transType), //[1804h.02h], Название=Тип передачи, ТекстПУ=TransT, EngText=Transmission Type, группа=CAN
(long)(&co2_vars.co_TPDO5ITime), //[1804h.03h], Название=Inhibit Time 5, ТекстПУ=ITime5, EngText=Inhibit Time 5, группа=CAN
(long)(&co2_vars.co_compatEntry), //[1804h.04h], Название=Compatibility Entry, ТекстПУ=CompEn, EngText=Compatibility Entry, группа=CAN
(long)(&co2_vars.co_TPDO5EventTimer), //[1804h.05h], Название=Event Timer 5, ТекстПУ=EvTmr5, EngText=Event Timer 5, группа=CAN
(long)(&co2_vars.co_TPDO6COBID), //[1805h.01h], Название=COB-ID for TPDO6, ТекстПУ=ITPDO6, EngText=COB-ID for TPDO6, группа=CAN
(long)(&co2_vars.co_transType), //[1805h.02h], Название=Тип передачи, ТекстПУ=TransT, EngText=Transmission Type, группа=CAN
(long)(&co2_vars.co_TPDO6ITime), //[1805h.03h], Название=Inhibit Time 6, ТекстПУ=ITime6, EngText=Inhibit Time 6, группа=CAN
(long)(&co2_vars.co_compatEntry), //[1805h.04h], Название=Compatibility Entry, ТекстПУ=CompEn, EngText=Compatibility Entry, группа=CAN
(long)(&co2_vars.co_TPDO6EventTimer), //[1805h.05h], Название=Event Timer 6, ТекстПУ=EvTmr6, EngText=Event Timer 6, группа=CAN
(long)(&co2_vars.co_TPDO7COBID), //[1806h.01h], Название=COB-ID for TPDO7, ТекстПУ=ITPDO7, EngText=COB-ID for TPDO7, группа=CAN
(long)(&co2_vars.co_transType), //[1806h.02h], Название=Тип передачи, ТекстПУ=TransT, EngText=Transmission Type, группа=CAN
(long)(&co2_vars.co_TPDO7ITime), //[1806h.03h], Название=Inhibit Time 7, ТекстПУ=ITime7, EngText=Inhibit Time 7, группа=CAN
(long)(&co2_vars.co_compatEntry), //[1806h.04h], Название=Compatibility Entry, ТекстПУ=CompEn, EngText=Compatibility Entry, группа=CAN
(long)(&co2_vars.co_TPDO7EventTimer), //[1806h.05h], Название=Event Timer 7, ТекстПУ=EvTmr7, EngText=Event Timer 7, группа=CAN
(long)(&co2_vars.co_TPDO8COBID), //[1807h.01h], Название=COB-ID for TPDO8, ТекстПУ=ITPDO8, EngText=COB-ID for TPDO8, группа=CAN
(long)(&co2_vars.co_transType), //[1807h.02h], Название=Тип передачи, ТекстПУ=TransT, EngText=Transmission Type, группа=CAN
(long)(&co2_vars.co_TPDO8ITime), //[1807h.03h], Название=Inhibit Time 8, ТекстПУ=ITime8, EngText=Inhibit Time 8, группа=CAN
(long)(&co2_vars.co_compatEntry), //[1807h.04h], Название=Compatibility Entry, ТекстПУ=CompEn, EngText=Compatibility Entry, группа=CAN
(long)(&co2_vars.co_TPDO8EventTimer), //[1807h.05h], Название=Event Timer 8, ТекстПУ=EvTmr8, EngText=Event Timer 8, группа=CAN
(long)(&co2_vars.co_TPDO1_1Mapping), //[1A00h.01h], Название=TPDO1-1 Mapping, ТекстПУ=TPM11, EngText=TPDO1-1 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO1_2Mapping), //[1A00h.02h], Название=TPDO1-2 Mapping, ТекстПУ=TPM12, EngText=TPDO1-2 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO1_3Mapping), //[1A00h.03h], Название=TPDO1-3 Mapping, ТекстПУ=TPM13, EngText=TPDO1-3 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO1_4Mapping), //[1A00h.04h], Название=TPDO1-4 Mapping, ТекстПУ=TPM14, EngText=TPDO1-4 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO2_1Mapping), //[1A01h.01h], Название=TPDO2-1 Mapping, ТекстПУ=TPM21, EngText=TPDO2-1 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO2_2Mapping), //[1A01h.02h], Название=TPDO2-2 Mapping, ТекстПУ=TPM22, EngText=TPDO2-2 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO2_3Mapping), //[1A01h.03h], Название=TPDO2-3 Mapping, ТекстПУ=TPM23, EngText=TPDO2-3 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO2_4Mapping), //[1A01h.04h], Название=TPDO2-4 Mapping, ТекстПУ=TPM24, EngText=TPDO2-4 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO3_1Mapping), //[1A02h.01h], Название=TPDO3-1 Mapping, ТекстПУ=TPM31, EngText=TPDO3-1 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO3_2Mapping), //[1A02h.02h], Название=TPDO3-2 Mapping, ТекстПУ=TPM32, EngText=TPDO3-2 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO3_3Mapping), //[1A02h.03h], Название=TPDO3-3 Mapping, ТекстПУ=TPM33, EngText=TPDO3-3 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO3_4Mapping), //[1A02h.04h], Название=TPDO3-4 Mapping, ТекстПУ=TPM34, EngText=TPDO3-4 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO4_1Mapping), //[1A03h.01h], Название=TPDO4-1 Mapping, ТекстПУ=TPM41, EngText=TPDO4-1 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO4_2Mapping), //[1A03h.02h], Название=TPDO4-2 Mapping, ТекстПУ=TPM42, EngText=TPDO4-2 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO4_3Mapping), //[1A03h.03h], Название=TPDO4-3 Mapping, ТекстПУ=TPM43, EngText=TPDO4-3 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO4_4Mapping), //[1A03h.04h], Название=TPDO4-4 Mapping, ТекстПУ=TPM44, EngText=TPDO4-4 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO5_1Mapping), //[1A04h.01h], Название=TPDO5-1 Mapping, ТекстПУ=TPM51, EngText=TPDO5-1 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO5_2Mapping), //[1A04h.02h], Название=TPDO5-2 Mapping, ТекстПУ=TPM52, EngText=TPDO5-2 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO5_3Mapping), //[1A04h.03h], Название=TPDO5-3 Mapping, ТекстПУ=TPM53, EngText=TPDO5-3 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO5_4Mapping), //[1A04h.04h], Название=TPDO5-4 Mapping, ТекстПУ=TPM54, EngText=TPDO5-4 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO6_1Mapping), //[1A05h.01h], Название=TPDO6-1 Mapping, ТекстПУ=TPM61, EngText=TPDO6-1 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO6_2Mapping), //[1A05h.02h], Название=TPDO6-2 Mapping, ТекстПУ=TPM62, EngText=TPDO6-2 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO6_3Mapping), //[1A05h.03h], Название=TPDO6-3 Mapping, ТекстПУ=TPM63, EngText=TPDO6-3 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO6_4Mapping), //[1A05h.04h], Название=TPDO6-4 Mapping, ТекстПУ=TPM64, EngText=TPDO6-4 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO7_1Mapping), //[1A06h.01h], Название=TPDO7-1 Mapping, ТекстПУ=TPM71, EngText=TPDO7-1 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO7_2Mapping), //[1A06h.02h], Название=TPDO7-2 Mapping, ТекстПУ=TPM72, EngText=TPDO7-2 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO7_3Mapping), //[1A06h.03h], Название=TPDO7-3 Mapping, ТекстПУ=TPM73, EngText=TPDO7-3 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO7_4Mapping), //[1A06h.04h], Название=TPDO7-4 Mapping, ТекстПУ=TPM74, EngText=TPDO7-4 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO8_1Mapping), //[1A07h.01h], Название=TPDO8-1 Mapping, ТекстПУ=TPM81, EngText=TPDO8-1 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO8_2Mapping), //[1A07h.02h], Название=TPDO8-2 Mapping, ТекстПУ=TPM82, EngText=TPDO8-2 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO8_3Mapping), //[1A07h.03h], Название=TPDO8-3 Mapping, ТекстПУ=TPM83, EngText=TPDO8-3 Mapping, группа=CAN
(long)(&co2_vars.co_TPDO8_4Mapping), //[1A07h.04h], Название=TPDO8-4 Mapping, ТекстПУ=TPM84, EngText=TPDO8-4 Mapping, группа=CAN
(long)(&co2_vars.co_nodeID), //[2000h.00h], Название=CAN NODE-ID, ТекстПУ=NodeID, EngText=CAN NODE-ID, группа=CAN
(long)(&co2_vars.co_bitRate), //[2001h.00h], Название=CAN Bit Rate, ТекстПУ=CANBR, EngText=CAN Bit Rate, группа=CAN
(long)(&co2_vars.co_vendorID), //[2002h.01h], Название=Код производителя, ТекстПУ=VendID, EngText=Vendor ID, группа=CAN
(long)(&co2_vars.co_productCode), //[2002h.02h], Название=Код продукта, ТекстПУ=PrCode, EngText=Product Code, группа=CAN
(long)(&co2_vars.co_revisionNumber), //[2002h.03h], Название=Номер ревизии, ТекстПУ=RevNum, EngText=Revision Number, группа=CAN
(long)(&co2_vars.co_serialNumber), //[2002h.04h], Название=Серийный номер, ТекстПУ=SerNum, EngText=Serial Number, группа=CAN
(long)(&co2_vars.co_specialData1), //[2003h.01h], Название=Special Data 1, ТекстПУ=SpDat1, EngText=Special Data 1, группа=CAN
(long)(&co2_vars.co_specialData2), //[2003h.02h], Название=Special Data 2, ТекстПУ=SpDat2, EngText=Special Data 2, группа=CAN
(long)(&co2_vars.co_specialData3), //[2003h.03h], Название=Special Data 3, ТекстПУ=SpDat3, EngText=Special Data 3, группа=CAN
(long)(&co2_vars.co_specialData4), //[2003h.04h], Название=Special Data 4, ТекстПУ=SpDat4, EngText=Special Data 4, группа=CAN
(long)(&co2_vars.co_secretCode), //[2004h.00h], Название=Secret Code, ТекстПУ=Secret, EngText=Secret Code, группа=CAN
(long)(&co2_vars.co_protectBit), //[2005h.00h], Название=Protect Bit, ТекстПУ=ProtBt, EngText=Protect Bit, группа=CAN
(long)(&co2_odHash[0]), //[2007h.01h], Название=md5_hash_0, ТекстПУ=md5_hash_0, EngText=md5_hash_0, группа=CAN
(long)(&co2_odHash[1]), //[2007h.02h], Название=md5_hash_1, ТекстПУ=md5_hash_1, EngText=md5_hash_1, группа=CAN
(long)(&co2_odHash[2]), //[2007h.03h], Название=md5_hash_2, ТекстПУ=md5_hash_2, EngText=md5_hash_2, группа=CAN
(long)(&co2_odHash[3]), //[2007h.04h], Название=md5_hash_3, ТекстПУ=md5_hash_3, EngText=md5_hash_3, группа=CAN
(long)(&co2_vars.co_heartbeatFlag0), //[2010h.01h], Название=Heartbeat Flag 0-31, ТекстПУ=HFlag0, EngText=Heartbeat Flag 0-31, группа=CAN
(long)(&co2_vars.co_heartbeatFlag1), //[2010h.02h], Название=Heartbeat Flag 32-63, ТекстПУ=HFlag1, EngText=Heartbeat Flag 32-63, группа=CAN
(long)(&co2_vars.co_heartbeatFlag2), //[2010h.03h], Название=Heartbeat Flag 64-95, ТекстПУ=HFlag2, EngText=Heartbeat Flag 64-95, группа=CAN
(long)(&co2_vars.co_heartbeatFlag3), //[2010h.04h], Название=Heartbeat Flag 96-127, ТекстПУ=HFlag3, EngText=Heartbeat Flag 96-127, группа=CAN
(long)(&co2_vars.co_heartbeatMask0), //[2011h.01h], Название=Heartbeat Mask 0-31, ТекстПУ=HMask0, EngText=Heartbeat Mask 0-31, группа=CAN
(long)(&co2_vars.co_heartbeatMask1), //[2011h.02h], Название=Heartbeat Mask 32-63, ТекстПУ=HMask1, EngText=Heartbeat Mask 32-63, группа=CAN
(long)(&co2_vars.co_heartbeatMask2), //[2011h.03h], Название=Heartbeat Mask 64-95, ТекстПУ=HMask2, EngText=Heartbeat Mask 64-95, группа=CAN
(long)(&co2_vars.co_heartbeatMask3), //[2011h.04h], Название=Heartbeat Mask 96-127, ТекстПУ=HMask3, EngText=Heartbeat Mask 96-127, группа=CAN
(long)(&co2_vars.co_heartbeatAutoStart), //[2012h.00h], Название=Heartbeat Autostart, ТекстПУ=HBASta, EngText=Heartbeat Autostart, группа=CAN
(long)(&co2_vars.co_heartbeatAutoRecovery), //[2014h.00h], Название=Heartbeat Autorecovery, ТекстПУ=HBARec, EngText=Heartbeat Autorecovery, группа=CAN
(long)(&co2_vars.co_nodeState), //[2015h.00h], Название=Состояние драйвера CAN-Open, ТекстПУ=State, EngText=Состояние драйвера CAN-Open, группа=CAN
(long)(&co2_vars.co_emergencyErrorCode), //[2016h.00h], Название=Emergency Error Code, ТекстПУ=EmErCo, EngText=Emergency Error Code, группа=CAN
(long)(&co2_vars.co_deviceErrorState), //[2017h.00h], Название=Device Error State, ТекстПУ=DeErSt, EngText=Device Error State, группа=CAN
(long)(&co2_vars.co_ODCommand), //[2080h.01h], Название=Object Dictionary Command, ТекстПУ=ODComm, EngText=Object Dictionary Command, группа=CAN
(long)(&co2_vars.co_currentODIndex), //[2080h.02h], Название=Current OD Index, ТекстПУ=ODCInd, EngText=Current OD Index, группа=CAN
(long)(&co2_vars.co_currentODSubIndex), //[2080h.03h], Название=Current OD Sub-Index, ТекстПУ=ODCSub, EngText=Current OD Sub-Index, группа=CAN
(long)(&co2_vars.co_currentODEText), //[2080h.04h], Название=Current OD Element Text, ТекстПУ=ODCTxt, EngText=Current OD Element Text, группа=CAN
(long)(&co2_vars.co_currentODEFormat), //[2080h.05h], Название=Current OD Element Format, ТекстПУ=ODCFrm, EngText=Current OD Element Format, группа=CAN
(long)(&co2_vars.co_currentODEMin), //[2080h.06h], Название=Current OD Element Min, ТекстПУ=ODCMin, EngText=Current OD Element Min, группа=CAN
(long)(&co2_vars.co_currentODEMax), //[2080h.07h], Название=Current OD Element Max, ТекстПУ=ODCMax, EngText=Current OD Element Max, группа=CAN
(long)(&co2_vars.co_currentODEDefault), //[2080h.08h], Название=Current OD Element Default, ТекстПУ=ODCDef, EngText=Current OD Element Default, группа=CAN
(long)(&co2_vars.co_currentODEMinLow), //[2080h.09h], Название=Current OD Element MinLow, ТекстПУ=ODCMiL, EngText=Current OD Element MinLow, группа=CAN
(long)(&co2_vars.co_currentODEMaxLow), //[2080h.0Ah], Название=Current OD Element MaxLow, ТекстПУ=ODCMaL, EngText=Current OD Element MaxLow, группа=CAN
(long)(&co2_vars.co_currentODEDefaultLow), //[2080h.0Bh], Название=Current OD Element Default Low, ТекстПУ=ODCDeL, EngText=Current OD Element Default Low, группа=CAN
(long)(&co2_vars.co_currentODEAddrLow), //[2080h.0Ch], Название=Current OD Element Address, ТекстПУ=ODCAdr, EngText=Current OD Element Address, группа=CAN
(long)(&co2_vars.co_currentODEAddrLow), //[2080h.0Dh], Название=Current OD Element Address, ТекстПУ=ODCAdr, EngText=Current OD Element Address, группа=CAN
(long)(&co2_vars.co_currentODEType), //[2080h.0Eh], Название=Группа параметров, ТекстПУ=Группа, EngText=Parameter Group, группа=CAN
(long)(&co2_vars.co_odIndexSize), //[2081h.00h], Название=Количество индексов, ТекстПУ=КолИнд, EngText=Number of Indexes, группа=CAN
(long)(&co2_vars.co_defaultIndex1), //[2082h.01h], Название=Default Index 1, ТекстПУ=DfInd1, EngText=Default Index 1, группа=CAN
(long)(&co2_vars.co_defaultIndex2), //[2082h.02h], Название=Default Index 2, ТекстПУ=DfInd2, EngText=Default Index 2, группа=CAN
(long)(&co2_vars.co_maskElement01), //[2083h.01h], Название=Mask Element, ТекстПУ=MskEl0, EngText=Mask Element, группа=CAN
(long)(&co2_vars.co_maskElement01), //[2083h.02h], Название=Mask Element, ТекстПУ=MskEl1, EngText=Mask Element, группа=CAN
(long)(&co2_vars.co_maskElement23), //[2083h.03h], Название=Mask Element, ТекстПУ=MskEl2, EngText=Mask Element, группа=CAN
(long)(&co2_vars.co_maskElement23), //[2083h.04h], Название=Mask Element, ТекстПУ=MskEl3, EngText=Mask Element, группа=CAN
(long)(&co2_vars.co_maskElement45), //[2083h.05h], Название=Mask Element, ТекстПУ=MskEl4, EngText=Mask Element, группа=CAN
(long)(&co2_vars.co_maskElement45), //[2083h.06h], Название=Mask Element, ТекстПУ=MskEl5, EngText=Mask Element, группа=CAN
(long)(&co2_vars.co_maskElement67), //[2083h.07h], Название=Mask Element, ТекстПУ=MskEl6, EngText=Mask Element, группа=CAN
(long)(&co2_vars.co_maskElement67), //[2083h.08h], Название=Mask Element, ТекстПУ=MskEl7, EngText=Mask Element, группа=CAN
(long)(&co2_vars.co_maskElement89), //[2083h.09h], Название=Mask Element, ТекстПУ=MskEl8, EngText=Mask Element, группа=CAN
(long)(&co2_vars.co_maskElement89), //[2083h.0Ah], Название=Mask Element, ТекстПУ=MskEl9, EngText=Mask Element, группа=CAN
(long)(&co2_vars.co_maskElementAB), //[2083h.0Bh], Название=Mask Element, ТекстПУ=MskElA, EngText=Mask Element, группа=CAN
(long)(&co2_vars.co_maskElementAB), //[2083h.0Ch], Название=Mask Element, ТекстПУ=MskElB, EngText=Mask Element, группа=CAN
(long)(&co2_vars.co_maskElementCD), //[2083h.0Dh], Название=Mask Element, ТекстПУ=MskElC, EngText=Mask Element, группа=CAN
(long)(&co2_vars.co_maskElementCD), //[2083h.0Eh], Название=Mask Element, ТекстПУ=MskElD, EngText=Mask Element, группа=CAN
(long)(&co2_vars.co_maskElementEF), //[2083h.0Fh], Название=Mask Element, ТекстПУ=MskElE, EngText=Mask Element, группа=CAN
(long)(&co2_vars.co_maskElementEF), //[2083h.10h], Название=Mask Element, ТекстПУ=MskElF, EngText=Mask Element, группа=CAN
(long)(&co2_vars.co_scaleNum0), //[2100h.01h], Название=Scale Number 0, ТекстПУ=SN0, EngText=Scale Number 0, группа=CAN
(long)(&co2_vars.co_scaleNum0Format), //[2100h.02h], Название=Scale Number 0 Format, ТекстПУ=S0Fmt, EngText=Scale Number 0 Format, группа=CAN
(long)(&co2_vars.co_scaleNum0Format), //[2100h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum0Format), //[2100h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum0Format), //[2100h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum0Format), //[2100h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum1), //[2101h.01h], Название=Scale Number 1, ТекстПУ=SN1, EngText=Scale Number 1, группа=CAN
(long)(&co2_vars.co_scaleNum1Format), //[2101h.02h], Название=Scale Number 1 Format, ТекстПУ=S1Fmt, EngText=Scale Number 1 Format, группа=CAN
(long)(&co2_vars.co_scaleNum1Format), //[2101h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum1Format), //[2101h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum1Format), //[2101h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum1Format), //[2101h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum2), //[2102h.01h], Название=Scale Number 2, ТекстПУ=SN2, EngText=Scale Number 2, группа=CAN
(long)(&co2_vars.co_scaleNum2Format), //[2102h.02h], Название=Scale Number 2 Format, ТекстПУ=S2Fmt, EngText=Scale Number 2 Format, группа=CAN
(long)(&co2_vars.co_scaleNum2Format), //[2102h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum2Format), //[2102h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum2Format), //[2102h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum2Format), //[2102h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum3), //[2103h.01h], Название=Scale Number 3, ТекстПУ=SN3, EngText=Scale Number 3, группа=CAN
(long)(&co2_vars.co_scaleNum3Format), //[2103h.02h], Название=Scale Number 3 Format, ТекстПУ=S3Fmt, EngText=Scale Number 3 Format, группа=CAN
(long)(&co2_vars.co_scaleNum3Format), //[2103h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum3Format), //[2103h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum3Format), //[2103h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum3Format), //[2103h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum4), //[2104h.01h], Название=Scale Number 4, ТекстПУ=SN4, EngText=Scale Number 4, группа=CAN
(long)(&co2_vars.co_scaleNum4Format), //[2104h.02h], Название=Scale Number 4 Format, ТекстПУ=S4Fmt, EngText=Scale Number 4 Format, группа=CAN
(long)(&co2_vars.co_scaleNum4Format), //[2104h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum4Format), //[2104h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum4Format), //[2104h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum4Format), //[2104h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum5), //[2105h.01h], Название=Scale Number 5, ТекстПУ=SN5, EngText=Scale Number 5, группа=CAN
(long)(&co2_vars.co_scaleNum5Format), //[2105h.02h], Название=Scale Number 5 Format, ТекстПУ=S5Fmt, EngText=Scale Number 5 Format, группа=CAN
(long)(&co2_vars.co_scaleNum5Format), //[2105h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum5Format), //[2105h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum5Format), //[2105h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum5Format), //[2105h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum6), //[2106h.01h], Название=Scale Number 6, ТекстПУ=SN6, EngText=Scale Number 6, группа=CAN
(long)(&co2_vars.co_scaleNum6Format), //[2106h.02h], Название=Scale Number 6 Format, ТекстПУ=S6Fmt, EngText=Scale Number 6 Format, группа=CAN
(long)(&co2_vars.co_scaleNum6Format), //[2106h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum6Format), //[2106h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum6Format), //[2106h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum6Format), //[2106h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum7), //[2107h.01h], Название=Scale Number 7, ТекстПУ=SN7, EngText=Scale Number 7, группа=CAN
(long)(&co2_vars.co_scaleNum7Format), //[2107h.02h], Название=Scale Number 7 Format, ТекстПУ=S7Fmt, EngText=Scale Number 7 Format, группа=CAN
(long)(&co2_vars.co_scaleNum7Format), //[2107h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum7Format), //[2107h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum7Format), //[2107h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum7Format), //[2107h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum8), //[2108h.01h], Название=Scale Number 8, ТекстПУ=SN8, EngText=Scale Number 8, группа=CAN
(long)(&co2_vars.co_scaleNum8Format), //[2108h.02h], Название=Scale Number 8 Format, ТекстПУ=S8Fmt, EngText=Scale Number 8 Format, группа=CAN
(long)(&co2_vars.co_scaleNum8Format), //[2108h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum8Format), //[2108h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum8Format), //[2108h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum8Format), //[2108h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum9), //[2109h.01h], Название=Scale Number 9, ТекстПУ=SN9, EngText=Scale Number 9, группа=CAN
(long)(&co2_vars.co_scaleNum9Format), //[2109h.02h], Название=Scale Number 9 Format, ТекстПУ=S9Fmt, EngText=Scale Number 9 Format, группа=CAN
(long)(&co2_vars.co_scaleNum9Format), //[2109h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum9Format), //[2109h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum9Format), //[2109h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum9Format), //[2109h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNumA), //[210Ah.01h], Название=Scale Number A, ТекстПУ=SNA, EngText=Scale Number A, группа=CAN
(long)(&co2_vars.co_scaleNumAFormat), //[210Ah.02h], Название=Scale Number A Format, ТекстПУ=SAFmt, EngText=Scale Number A Format, группа=CAN
(long)(&co2_vars.co_scaleNumAFormat), //[210Ah.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNumAFormat), //[210Ah.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNumAFormat), //[210Ah.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNumAFormat), //[210Ah.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNumB), //[210Bh.01h], Название=Scale Number B, ТекстПУ=SNB, EngText=Scale Number B, группа=CAN
(long)(&co2_vars.co_scaleNumBFormat), //[210Bh.02h], Название=Scale Number B Format, ТекстПУ=SBFmt, EngText=Scale Number B Format, группа=CAN
(long)(&co2_vars.co_scaleNumBFormat), //[210Bh.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNumBFormat), //[210Bh.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNumBFormat), //[210Bh.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNumBFormat), //[210Bh.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNumC), //[210Ch.01h], Название=Scale Number C, ТекстПУ=SNC, EngText=Scale Number C, группа=CAN
(long)(&co2_vars.co_scaleNumCFormat), //[210Ch.02h], Название=Scale Number C Format, ТекстПУ=SCFmt, EngText=Scale Number C Format, группа=CAN
(long)(&co2_vars.co_scaleNumCFormat), //[210Ch.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNumCFormat), //[210Ch.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNumCFormat), //[210Ch.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNumCFormat), //[210Ch.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNumD), //[210Dh.01h], Название=Scale Number D, ТекстПУ=SND, EngText=Scale Number D, группа=CAN
(long)(&co2_vars.co_scaleNumDFormat), //[210Dh.02h], Название=Scale Number D Format, ТекстПУ=SDFmt, EngText=Scale Number D Format, группа=CAN
(long)(&co2_vars.co_scaleNumDFormat), //[210Dh.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNumDFormat), //[210Dh.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNumDFormat), //[210Dh.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNumDFormat), //[210Dh.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNumE), //[210Eh.01h], Название=Scale Number E, ТекстПУ=SNE, EngText=Scale Number E, группа=CAN
(long)(&co2_vars.co_scaleNumEFormat), //[210Eh.02h], Название=Scale Number E Format, ТекстПУ=SEFmt, EngText=Scale Number E Format, группа=CAN
(long)(&co2_vars.co_scaleNumEFormat), //[210Eh.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNumEFormat), //[210Eh.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNumEFormat), //[210Eh.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNumEFormat), //[210Eh.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNumF), //[210Fh.01h], Название=Scale Number F, ТекстПУ=SNF, EngText=Scale Number F, группа=CAN
(long)(&co2_vars.co_scaleNumFFormat), //[210Fh.02h], Название=Scale Number F Format, ТекстПУ=SFFmt, EngText=Scale Number F Format, группа=CAN
(long)(&co2_vars.co_scaleNumFFormat), //[210Fh.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNumFFormat), //[210Fh.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNumFFormat), //[210Fh.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNumFFormat), //[210Fh.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum10), //[2110h.01h], Название=Scale Number 10, ТекстПУ=SN10, EngText=Scale Number 10, группа=CAN
(long)(&co2_vars.co_scaleNum10Format), //[2110h.02h], Название=Scale Number 10 Format, ТекстПУ=S10Fmt, EngText=Scale Number 10 Format, группа=CAN
(long)(&co2_vars.co_scaleNum10Format), //[2110h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum10Format), //[2110h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum10Format), //[2110h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum10Format), //[2110h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum11), //[2111h.01h], Название=Scale Number 11, ТекстПУ=SN11, EngText=Scale Number 11, группа=CAN
(long)(&co2_vars.co_scaleNum11Format), //[2111h.02h], Название=Scale Number 11 Format, ТекстПУ=S11Fmt, EngText=Scale Number 11 Format, группа=CAN
(long)(&co2_vars.co_scaleNum11Format), //[2111h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum11Format), //[2111h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum11Format), //[2111h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum11Format), //[2111h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum12), //[2112h.01h], Название=Scale Number 12, ТекстПУ=SN12, EngText=Scale Number 12, группа=CAN
(long)(&co2_vars.co_scaleNum12Format), //[2112h.02h], Название=Scale Number 12 Format, ТекстПУ=S12Fmt, EngText=Scale Number 12 Format, группа=CAN
(long)(&co2_vars.co_scaleNum12Format), //[2112h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum12Format), //[2112h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum12Format), //[2112h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum12Format), //[2112h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum13), //[2113h.01h], Название=Scale Number 13, ТекстПУ=SN13, EngText=Scale Number 13, группа=CAN
(long)(&co2_vars.co_scaleNum13Format), //[2113h.02h], Название=Scale Number 13 Format, ТекстПУ=S13Fmt, EngText=Scale Number 13 Format, группа=CAN
(long)(&co2_vars.co_scaleNum13Format), //[2113h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum13Format), //[2113h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum13Format), //[2113h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum13Format), //[2113h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum14), //[2114h.01h], Название=Scale Number 14, ТекстПУ=SN14, EngText=Scale Number 14, группа=CAN
(long)(&co2_vars.co_scaleNum14Format), //[2114h.02h], Название=Scale Number 14 Format, ТекстПУ=S14Fmt, EngText=Scale Number 14 Format, группа=CAN
(long)(&co2_vars.co_scaleNum14Format), //[2114h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum14Format), //[2114h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum14Format), //[2114h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum14Format), //[2114h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum15), //[2115h.01h], Название=Scale Number 15, ТекстПУ=SN15, EngText=Scale Number 15, группа=CAN
(long)(&co2_vars.co_scaleNum15Format), //[2115h.02h], Название=Scale Number 15Format, ТекстПУ=S15Fmt, EngText=Scale Number 15Format, группа=CAN
(long)(&co2_vars.co_scaleNum15Format), //[2115h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum15Format), //[2115h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum15Format), //[2115h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum15Format), //[2115h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum16), //[2116h.01h], Название=Scale Number 16, ТекстПУ=SN16, EngText=Scale Number 16, группа=CAN
(long)(&co2_vars.co_scaleNum16Format), //[2116h.02h], Название=Scale Number 16Format, ТекстПУ=S16Fmt, EngText=Scale Number 16Format, группа=CAN
(long)(&co2_vars.co_scaleNum16Format), //[2116h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum16Format), //[2116h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum16Format), //[2116h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum16Format), //[2116h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum17), //[2117h.01h], Название=Scale Number 17, ТекстПУ=SN17, EngText=Scale Number 17, группа=CAN
(long)(&co2_vars.co_scaleNum17Format), //[2117h.02h], Название=Scale Number 17Format, ТекстПУ=S17Fmt, EngText=Scale Number 17Format, группа=CAN
(long)(&co2_vars.co_scaleNum17Format), //[2117h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum17Format), //[2117h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum17Format), //[2117h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum17Format), //[2117h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum18), //[2118h.01h], Название=Scale Number 18, ТекстПУ=SN18, EngText=Scale Number 18, группа=CAN
(long)(&co2_vars.co_scaleNum18Format), //[2118h.02h], Название=Scale Number 18Format, ТекстПУ=S18Fmt, EngText=Scale Number 18Format, группа=CAN
(long)(&co2_vars.co_scaleNum18Format), //[2118h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum18Format), //[2118h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum18Format), //[2118h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum18Format), //[2118h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum19), //[2119h.01h], Название=Scale Number 19, ТекстПУ=SN19, EngText=Scale Number 19, группа=CAN
(long)(&co2_vars.co_scaleNum19Format), //[2119h.02h], Название=Scale Number 19Format, ТекстПУ=S19Fmt, EngText=Scale Number 19Format, группа=CAN
(long)(&co2_vars.co_scaleNum19Format), //[2119h.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum19Format), //[2119h.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum19Format), //[2119h.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum19Format), //[2119h.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum1A), //[211Ah.01h], Название=Scale Number 1A, ТекстПУ=SN1A, EngText=Scale Number 1A, группа=CAN
(long)(&co2_vars.co_scaleNum1AFormat), //[211Ah.02h], Название=Scale Number 1AFormat, ТекстПУ=S1AFmt, EngText=Scale Number 1AFormat, группа=CAN
(long)(&co2_vars.co_scaleNum1AFormat), //[211Ah.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum1AFormat), //[211Ah.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum1AFormat), //[211Ah.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum1AFormat), //[211Ah.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum1B), //[211Bh.01h], Название=Scale Number 1B, ТекстПУ=SN1B, EngText=Scale Number 1B, группа=CAN
(long)(&co2_vars.co_scaleNum1BFormat), //[211Bh.02h], Название=Scale Number 1BFormat, ТекстПУ=S1BFmt, EngText=Scale Number 1BFormat, группа=CAN
(long)(&co2_vars.co_scaleNum1BFormat), //[211Bh.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum1BFormat), //[211Bh.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum1BFormat), //[211Bh.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum1BFormat), //[211Bh.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum1C), //[211Ch.01h], Название=Scale Number 1C, ТекстПУ=SN1C, EngText=Scale Number 1C, группа=CAN
(long)(&co2_vars.co_scaleNum1CFormat), //[211Ch.02h], Название=Scale Number 1C Format, ТекстПУ=S1CFmt, EngText=Scale Number 1C Format, группа=CAN
(long)(&co2_vars.co_scaleNum1CFormat), //[211Ch.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum1CFormat), //[211Ch.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum1CFormat), //[211Ch.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum1CFormat), //[211Ch.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&co2_vars.co_scaleNum1D), //[211Dh.01h], Название=Scale Number 1D, ТекстПУ=SN1D, EngText=Scale Number 1D, группа=CAN
(long)(&co2_vars.co_scaleNum1DFormat), //[211Dh.02h], Название=Scale Number 1D Format, ТекстПУ=S1DFmt, EngText=Scale Number 1D Format, группа=CAN
(long)(&co2_vars.co_scaleNum1DFormat), //[211Dh.03h], Название=Scale Number Format Q, ТекстПУ=Q-type, EngText=Scale Number Format Q, группа=CAN
(long)(&co2_vars.co_scaleNum1DFormat), //[211Dh.04h], Название=Scale Number Format NumOfFloat, ТекстПУ=NFloat, EngText=Scale Number Format NumOfFloat, группа=CAN
(long)(&co2_vars.co_scaleNum1DFormat), //[211Dh.05h], Название=Scale Number Format Prefix, ТекстПУ=Prefix, EngText=Scale Number Format Prefix, группа=CAN
(long)(&co2_vars.co_scaleNum1DFormat), //[211Dh.06h], Название=Scale Number Format Units, ТекстПУ=Units, EngText=Scale Number Format Units, группа=CAN
(long)(&cmd.all), //[2502h.01h], Название=Пуск, ТекстПУ=Пуск, EngText=Command: Start, группа=Слово управления
(long)(&cmd.all), //[2502h.02h], Название=Стоп, ТекстПУ=Стоп, EngText=Command: Stop, группа=Слово управления
(long)(&cmd.all), //[2502h.03h], Название=Сбросить состояние Аварии, ТекстПУ=СбрАвр, EngText=Command: Reset fault, группа=Слово управления
(long)(&cmd.all), //[2502h.04h], Название=Слово управления CAN, ТекстПУ=КмдCAN, EngText=CAN command register, группа=Слово управления
(long)(&spiclock.packed_time), //[250Fh.00h], Название=Часы реального времени, ТекстПУ=RTC, EngText=Real time clock, группа=Наблюдаемые
(long)(&sm_prot.Main_ErrorCode), //[2532h.01h], Название=Код аварии ARM, ТекстПУ=Код аварии ARM, EngText=, группа=Рег. тока якоря
(long)(&sm_prot.Main_ErrorCode), //[2532h.02h], Название=Расширение кода аварии, ТекстПУ=Авр+, EngText=Extension of fault ID, группа=Рег. тока якоря
(long)(&sm_prot.Main_Flags), //[2532h.03h], Название=Системный объект, ТекстПУ=System, EngText=System object, группа=Рег. тока якоря
(long)(&disp_group_number), //[2534h.01h], Название=Номер груп для пульта управления, ТекстПУ=№ гр ПУ, EngText=, группа=CAN
(long)(&VendorToken), //[2600h.01h], Название=VendorToken, ТекстПУ=VendorToken, EngText=VendorToken, группа=ВНЕ ГРУПП
(long)(&drv_status.all), //[3000h.00h], Название=Статус преобразователя, ТекстПУ=Статус, EngText=Статус преобразователя, группа=ВНЕ ГРУПП
(long)(&msCounter), //[3100h.01h], Название=Счётчик прерываний 1 кГц, ТекстПУ=Сч.прерыв.1 кГц, EngText=, группа=Счетчики
(long)(&FastCounter), //[3100h.02h], Название=Счётчик прерываний 10 кГц, ТекстПУ=Сч.прерыв.10 кГц, EngText=, группа=Счетчики
(long)(&LoopCounter), //[3100h.03h], Название=BackgroundCounter, ТекстПУ=BackgroundCounter, EngText=, группа=Счетчики
(long)(&TIsr10), //[3100h.04h], Название=T_10кГц, ТекстПУ=T_10кГц, EngText=T_10кГц, группа=Счетчики
(long)(&TIsr1), //[3100h.05h], Название=T_1кГц, ТекстПУ=T_1кГц, EngText=T_1кГц, группа=Счетчики
(long)(&rmp.T), //[5000h.01h], Название=T разгона до номинала, ТекстПУ=rmpT, EngText=T разгона до номинала, группа=ЗИ
(long)(&rmp.input), //[5000h.02h], Название=Вход ЗИ, ТекстПУ=ВходЗИ, EngText=Ramp input, группа=ЗИ
(long)(&rmp.output), //[5000h.03h], Название=Выход ЗИ, ТекстПУ=ВыходЗИ, EngText=Ramp output, группа=ЗИ
(long)(&pid_id.pid_ref_reg3), //[5100h.01h], Название=Задание, ТекстПУ=Зад-е, EngText=Reference, группа=Рег. тока Id
(long)(&pid_id.pid_fdb_reg3), //[5100h.02h], Название=Обратная связь, ТекстПУ=Обр.Св, EngText=Feedback, группа=Рег. тока Id
(long)(&pid_id.Kp_reg3), //[5100h.03h], Название=Пропорц. коэфф. Kp, ТекстПУ=Kp, EngText=Kp, группа=Рег. тока Id
(long)(&pid_id.pid_out_max), //[5100h.04h], Название=Максимум выхода, ТекстПУ=Max, EngText=Maximum Output Limits, группа=Рег. тока Id
(long)(&pid_id.pid_out_min), //[5100h.05h], Название=Минимум выхода, ТекстПУ=Min, EngText=Minimum Output Limits, группа=Рег. тока Id
(long)(&pid_id.Ki_reg3), //[5100h.06h], Название=Интегральный коэфф. Ki, ТекстПУ=Ki, EngText=Ki, группа=Рег. тока Id
(long)(&pid_id.Kd_reg3), //[5100h.07h], Название=Дифф. коэфф. Kd, ТекстПУ=Kd, EngText=Kd, группа=Рег. тока Id
(long)(&pid_id.Kc_reg3), //[5100h.08h], Название=Интегр.корректир.коэффициент, ТекстПУ=Kc, EngText=Integral correction gain, группа=Рег. тока Id
(long)(&pid_id.e_reg3), //[5100h.09h], Название=Ошибка рассогласования сигналов, ТекстПУ=Ошибка, EngText=, группа=Рег. тока Id
(long)(&pid_id.pid_out_reg3), //[5100h.0Ah], Название=Выход регулятора, ТекстПУ=Выход, EngText=Controller output, группа=Рег. тока Id
(long)(&pid_id.up_reg3), //[5100h.0Bh], Название=Вых.Пропорц., ТекстПУ=Вых.Пропорц., EngText=Ud, группа=Рег. тока Id
(long)(&pid_id.ui_reg3), //[5100h.0Ch], Название=Вых.Интегр., ТекстПУ=Вых.Интегр., EngText=Integral Output, группа=Рег. тока Id
(long)(&pid_id.ud_reg3), //[5100h.0Dh], Название=Вых.Диффер., ТекстПУ=Вых.Диффер., EngText=Ud, группа=Рег. тока Id
(long)(&pid_spd.pid_ref_reg3), //[5101h.01h], Название=Задание, ТекстПУ=pisref, EngText=Задание, группа=Рег. скорости
(long)(&pid_spd.pid_fdb_reg3), //[5101h.02h], Название=Обратная связь, ТекстПУ=pisfbd, EngText=Обратная связь, группа=Рег. скорости
(long)(&pid_spd.Kp_reg3), //[5101h.03h], Название=Пропорц. коэфф. Kp, ТекстПУ=Kp, EngText=Kp, группа=Рег. скорости
(long)(&pid_spd.pid_out_max), //[5101h.04h], Название=Максимум выхода, ТекстПУ=Max, EngText=Maximum Output Limits, группа=Рег. скорости
(long)(&pid_spd.pid_out_min), //[5101h.05h], Название=Минимум выхода, ТекстПУ=Min, EngText=Minimum Output Limits, группа=Рег. скорости
(long)(&pid_spd.Ki_reg3), //[5101h.06h], Название=Интегральный коэфф. Ki, ТекстПУ=Ki, EngText=Ki, группа=Рег. скорости
(long)(&pid_spd.Kd_reg3), //[5101h.07h], Название=Дифф. коэфф. Kd, ТекстПУ=pisKd, EngText=Дифференциальный коэффициент, группа=Рег. скорости
(long)(&pid_spd.Kc_reg3), //[5101h.08h], Название=Интегр.корректир.коэффициент, ТекстПУ=Kc, EngText=Integral correction gain, группа=Рег. скорости
(long)(&pid_spd.e_reg3), //[5101h.09h], Название=Ошибка рассогласования сигналов, ТекстПУ=Ошибка, EngText=, группа=Рег. скорости
(long)(&pid_spd.pid_out_reg3), //[5101h.0Ah], Название=Выход регулятора, ТекстПУ=pisout, EngText=Выход регулятора, группа=Рег. скорости
(long)(&pid_spd.up_reg3), //[5101h.0Bh], Название=Вых.Пропорц., ТекстПУ=Вых.Пропорц., EngText=Ud, группа=Рег. скорости
(long)(&pid_spd.ui_reg3), //[5101h.0Ch], Название=Вых.Интегр., ТекстПУ=Вых.Интегр., EngText=Integral Output, группа=Рег. скорости
(long)(&pid_spd.ud_reg3), //[5101h.0Dh], Название=Вых.Диффер., ТекстПУ=Вых.Диффер., EngText=Ud, группа=Рег. скорости
(long)(&pid_spd.saterr_reg3), //[5101h.0Eh], Название=Превышение насыщения, ТекстПУ=Прев. насыщ, EngText=saterr_reg3, группа=Рег. скорости
(long)(&pid_iq.pid_ref_reg3), //[5102h.01h], Название=Задание, ТекстПУ=Зад-ие, EngText=Задание, группа=Рег. тока Iq
(long)(&pid_iq.pid_fdb_reg3), //[5102h.02h], Название=Обратная связь, ТекстПУ=Обр.Св, EngText=Обратная связь, группа=Рег. тока Iq
(long)(&pid_iq.Kp_reg3), //[5102h.03h], Название=Пропорц. коэфф. Kp, ТекстПУ=Kp, EngText=Пропорциональный коэффициент Kp, группа=Рег. тока Iq
(long)(&pid_iq.pid_out_max), //[5102h.04h], Название=Максимум выхода, ТекстПУ=Max, EngText=Максимум выхода, группа=Рег. тока Iq
(long)(&pid_iq.pid_out_min), //[5102h.05h], Название=Минимум выхода, ТекстПУ=Min, EngText=Минимум выхода, группа=Рег. тока Iq
(long)(&pid_iq.Ki_reg3), //[5102h.06h], Название=Интегральный коэфф. Ki, ТекстПУ=Ki, EngText=Интегральный коэффициент Ki, группа=Рег. тока Iq
(long)(&pid_iq.Kd_reg3), //[5102h.07h], Название=Дифф. коэфф. Kd, ТекстПУ=Kd, EngText=Дифференциальный коэффициент Kd, группа=Рег. тока Iq
(long)(&pid_iq.Kc_reg3), //[5102h.08h], Название=Интегр.корректир.коэффициент, ТекстПУ=Kc, EngText=Integral correction gain, группа=Рег. тока Iq
(long)(&pid_iq.e_reg3), //[5102h.09h], Название=Ошибка рассогласования сигналов, ТекстПУ=Ошибка, EngText=, группа=Рег. тока Iq
(long)(&pid_iq.pid_out_reg3), //[5102h.0Ah], Название=Выход регулятора, ТекстПУ=Выход, EngText=Controller Output, группа=Рег. тока Iq
(long)(&pid_iq.up_reg3), //[5102h.0Bh], Название=Вых.Пропорц., ТекстПУ=Вых.Пропорц., EngText=Ud, группа=Рег. тока Iq
(long)(&pid_iq.ui_reg3), //[5102h.0Ch], Название=Вых.Интегр., ТекстПУ=Вых.Интегр., EngText=Integral Output, группа=Рег. тока Iq
(long)(&pid_iq.ud_reg3), //[5102h.0Dh], Название=Вых.Диффер., ТекстПУ=Вых.Диффер., EngText=Ud, группа=Рег. тока Iq
(long)(&pid_pos.pid_ref_reg3), //[5103h.01h], Название=Задание, ТекстПУ=pisref, EngText=Задание, группа=Рег. положения
(long)(&pid_pos.pid_ref_reg3), //[5103h.02h], Название=Задание, ТекстПУ=pisref, EngText=Задание, группа=Рег. положения
(long)(&pid_pos.pid_fdb_reg3), //[5103h.03h], Название=Обратная связь, ТекстПУ=pisfbd, EngText=Обратная связь, группа=Рег. положения
(long)(&pid_pos.pid_fdb_reg3), //[5103h.04h], Название=Обратная связь, ТекстПУ=pisfbd, EngText=Обратная связь, группа=Рег. положения
(long)(&pid_pos.Kp_reg3), //[5103h.05h], Название=Пропорц. коэфф. Kp, ТекстПУ=Kp, EngText=Kp, группа=Рег. положения
(long)(&pid_pos.pid_out_max), //[5103h.06h], Название=Максимум выхода, ТекстПУ=Max, EngText=Maximum Output Limits, группа=Рег. положения
(long)(&pid_pos.pid_out_min), //[5103h.07h], Название=Минимум выхода, ТекстПУ=Min, EngText=Minimum Output Limits, группа=Рег. положения
(long)(&pid_pos.Ki_reg3), //[5103h.08h], Название=Интегральный коэфф. Ki, ТекстПУ=Ki, EngText=Ki, группа=Рег. положения
(long)(&pid_pos.Kd_reg3), //[5103h.09h], Название=Дифф. коэфф. Kd, ТекстПУ=pisKd, EngText=Дифференциальный коэффициент, группа=Рег. положения
(long)(&pid_pos.DiffDelim), //[5103h.0Ah], Название=Делит. дифф. части, ТекстПУ=Делит. дифф. части, EngText=DiffDelim, группа=Рег. положения
(long)(&pid_pos.Kf_d), //[5103h.0Bh], Название=Коэф. фильтра, ТекстПУ=Коэф. фильтра, EngText=Kf_d, группа=Рег. положения
(long)(&pid_pos.Kc_reg3), //[5103h.0Ch], Название=Интегр.корректир.коэффициент, ТекстПУ=Kc, EngText=Integral correction gain, группа=Рег. положения
(long)(&pid_pos.DeadZone), //[5103h.0Dh], Название=Мертвая зона, ТекстПУ=Мертвая зона, EngText=DeadZone, группа=Рег. положения
(long)(&pid_pos.e_reg3), //[5103h.0Eh], Название=Ошибка рассогласования сигналов, ТекстПУ=Ошибка, EngText=, группа=Рег. положения
(long)(&pid_spd.saterr_reg3), //[5103h.0Fh], Название=Превышение насыщения, ТекстПУ=Прев. насыщ, EngText=saterr_reg3, группа=Рег. положения
(long)(&pid_pos.pid_out_reg3), //[5103h.10h], Название=Выход регулятора, ТекстПУ=pisout, EngText=Выход регулятора, группа=Рег. положения
(long)(&pid_pos.up_reg3), //[5103h.11h], Название=Вых.Пропорц., ТекстПУ=Вых.Пропорц., EngText=Ud, группа=Рег. положения
(long)(&pid_pos.ui_reg3), //[5103h.12h], Название=Вых.Интегр., ТекстПУ=Вых.Интегр., EngText=Integral Output, группа=Рег. положения
(long)(&pid_pos.ud_reg3), //[5103h.13h], Название=Вых.Диффер., ТекстПУ=Вых.Диффер., EngText=Ud, группа=Рег. положения
(long)(&pid_pos.e_reg3_filterOut), //[5103h.14h], Название=Фильтр ошибки, ТекстПУ=Фильтр ошибки, EngText=filterOut, группа=Рег. положения
(long)(&pid_ia.pid_ref_reg3), //[5104h.01h], Название=Задание, ТекстПУ=Зад-ие, EngText=Задание, группа=Рег. тока якоря
(long)(&pid_ia.pid_fdb_reg3), //[5104h.02h], Название=Обратная связь, ТекстПУ=Обр.Св, EngText=Обратная связь, группа=Рег. тока якоря
(long)(&pid_ia.Kp_reg3), //[5104h.03h], Название=Пропорц. коэфф. Kp, ТекстПУ=Kp, EngText=Пропорциональный коэффициент Kp, группа=Рег. тока якоря
(long)(&pid_ia.pid_out_max), //[5104h.04h], Название=Максимум выхода, ТекстПУ=Max, EngText=Максимум выхода, группа=Рег. тока якоря
(long)(&pid_ia.pid_out_min), //[5104h.05h], Название=Минимум выхода, ТекстПУ=Min, EngText=Минимум выхода, группа=Рег. тока якоря
(long)(&pid_ia.Ki_reg3), //[5104h.06h], Название=Интегральный коэфф. Ki, ТекстПУ=Ki, EngText=Интегральный коэффициент Ki, группа=Рег. тока якоря
(long)(&pid_ia.Kd_reg3), //[5104h.07h], Название=Дифф. коэфф. Kd, ТекстПУ=Kd, EngText=Дифференциальный коэффициент Kd, группа=Рег. тока якоря
(long)(&pid_ia.Kc_reg3), //[5104h.08h], Название=Интегр.корректир.коэффициент, ТекстПУ=Kc, EngText=Integral correction gain, группа=Рег. тока якоря
(long)(&pid_ia.e_reg3), //[5104h.09h], Название=Ошибка рассогласования сигналов, ТекстПУ=Ошибка, EngText=, группа=Рег. тока якоря
(long)(&pid_ia.pid_out_reg3), //[5104h.0Ah], Название=Выход регулятора, ТекстПУ=Выход, EngText=Controller Output, группа=Рег. тока якоря
(long)(&pid_ia.up_reg3), //[5104h.0Bh], Название=Вых.Пропорц., ТекстПУ=Вых.Пропорц., EngText=Ud, группа=Рег. тока якоря
(long)(&pid_ia.ui_reg3), //[5104h.0Ch], Название=Вых.Интегр., ТекстПУ=Вых.Интегр., EngText=Integral Output, группа=Рег. тока якоря
(long)(&pid_ia.ud_reg3), //[5104h.0Dh], Название=Вых.Диффер., ТекстПУ=Вых.Диффер., EngText=Ud, группа=Рег. тока якоря
(long)(&pwm.UalphaRef), //[5105h.01h], Название=UaЗад, ТекстПУ=UaЗад, EngText=UaЗад, группа=ШИМ
(long)(&pwm.UbetaRef), //[5105h.02h], Название=UbЗад, ТекстПУ=UbЗад, EngText=UbЗад, группа=ШИМ
(long)(&pwm.U_lim), //[5105h.03h], Название=U_огр, ТекстПУ=U_огр, EngText=U_огр, группа=ШИМ
(long)(&pwm.GammaA), //[5105h.04h], Название=GammaA, ТекстПУ=GmA, EngText=GammaA, группа=ШИМ
(long)(&pwm.GammaB), //[5105h.05h], Название=GammaB, ТекстПУ=GmB, EngText=GammaB, группа=ШИМ
(long)(&pwm.GammaC), //[5105h.06h], Название=GammaC, ТекстПУ=GmC, EngText=GammaC, группа=ШИМ
(long)(&pwm.k_pwm), //[5105h.07h], Название=TPWM, ТекстПУ=TPWM, EngText=TPWM, группа=ШИМ
(long)(&pwm.DeadBand), //[5105h.08h], Название=Мертвое время (мкс), ТекстПУ=Мерт. время, EngText=DeadBand, группа=ШИМ
(long)(&pwm.MinGammaLimit), //[5105h.09h], Название=Ограничение мин. скважн., ТекстПУ=GamLim, EngText=GamLim, группа=ШИМ
(long)(&pwm.UdCompK), //[5105h.0Ah], Название=Коэф. компенсации Ud, ТекстПУ=UdКомпK, EngText=UdCompK, группа=ШИМ
(long)(&pwm.UdCompEnable), //[5105h.0Bh], Название=Компенсация Ud, ТекстПУ=UdКомп, EngText=UdCompEnable, группа=ШИМ
(long)(&pwm.PWM_type), //[5105h.0Ch], Название=Тип ШИМ, ТекстПУ=ТипШИМ, EngText=PWMType, группа=ШИМ
(long)(&pwm.ULimitation), //[5105h.0Dh], Название=ULimitation, ТекстПУ=ULimitation, EngText=ULimitation, группа=ШИМ
(long)(&pwm.Frequency), //[5105h.0Eh], Название=Частота ШИМ, ТекстПУ=Fшим, EngText=PWM frequency, группа=ШИМ
(long)(&pwm.sector), //[5105h.0Fh], Название=Сектор, ТекстПУ=Сектор, EngText=Sector, группа=ШИМ
(long)(&pwm.U_mag), //[5105h.10h], Название=U_ампл, ТекстПУ=U_ампл, EngText=U_ампл, группа=ШИМ
(long)(&pwm.PDP_Fault), //[5105h.11h], Название=Флаги аварий, ТекстПУ=F_bits, EngText=F_bits, группа=ШИМ
(long)(&sm_ctrl.state), //[510Bh.01h], Название=sm_state, ТекстПУ=smstat, EngText=sm_state, группа=ДА sm_ctrl
(long)(&drv_status_code), //[510Bh.02h], Название=drv_status_code, ТекстПУ=stcode, EngText=drv_status_code, группа=ДА sm_ctrl
(long)(&sm_prot.bit_fault1), //[510Eh.01h], Название=Флаги аварий 1, ТекстПУ=bitF_1, EngText=Флаги аварий 1, группа=Защиты
(long)(&sm_prot.bit_fault2), //[510Eh.02h], Название=Флаги аварий 2, ТекстПУ=bitF_2, EngText=Флаги аварий 2, группа=Защиты
(long)(&sm_prot.mask_fault1), //[510Eh.03h], Название=Маска аварий 1L, ТекстПУ=mask1L, EngText=Маска аварий 1L, группа=Защиты
(long)(&sm_prot.mask_fault1), //[510Eh.04h], Название=Маска аварий 1H, ТекстПУ=mask1H, EngText=Маска аварий 1H, группа=Защиты
(long)(&sm_prot.mask_fault2), //[510Eh.05h], Название=Маска аварий 2L, ТекстПУ=mask2L, EngText=Маска аварий 2L, группа=Защиты
(long)(&sm_prot.mask_fault2), //[510Eh.06h], Название=Маска аварий  2H, ТекстПУ=mask2H, EngText=Маска аварий  2H, группа=Защиты
(long)(&sm_prot.Imax_protect), //[510Eh.07h], Название=Максимальный ток, ТекстПУ=Макс.Ток, EngText=Максимальный ток, группа=Защиты
(long)(&sm_prot.Umax_protect), //[510Eh.08h], Название=Максимальное напряжение, ТекстПУ=Ud_max, EngText=Максимальное напряжение, группа=Защиты
(long)(&sm_prot.Umin_protect), //[510Eh.09h], Название=Минимальное напряжение, ТекстПУ=Ud_min, EngText=Минимальное напряжение, группа=Защиты
(long)(&sm_prot.speed_max), //[510Eh.0Ah], Название=Максимальная скорость, ТекстПУ=speed_max, EngText=Maximum speed, группа=Защиты
(long)(&refs.speed_ref), //[5112h.01h], Название=Скорость, ТекстПУ=Скор_з, EngText=Скорость, группа=Задания
(long)(&refs.Iq_ref), //[5112h.02h], Название=Макс. ток ВУ, ТекстПУ=ТокСТз, EngText=ТокСТз, группа=Задания
(long)(&refs.i_flux_ref), //[5112h.03h], Название=Ток удержания, ТекстПУ=ТокУПз, EngText=Ток удержания, группа=Задания
(long)(&refs.theta_elec), //[5112h.04h], Название=theta_elec, ТекстПУ=элекУг, EngText=theta_elec, группа=Задания
(long)(&refs.uf_ref), //[5112h.05h], Название=Заданное напряжение ОВ, ТекстПУ=Uf_ref, EngText=Uf_ref, группа=Задания
(long)(&drv_params.p), //[5113h.01h], Название=Число пар полюсов, ТекстПУ=ПарПол, EngText=Число пар полюсов, группа=Базов. парам
(long)(&drv_params.I_nom), //[5113h.02h], Название=Номинальный ток, ТекстПУ=IпчНом, EngText=Номинальный ток, группа=Базов. парам
(long)(&drv_params.U_nom), //[5113h.03h], Название=Номинальное напряжение фазн., ТекстПУ=Uф_ном, EngText=Номинальное напряжение фазн., группа=Базов. парам
(long)(&drv_params.speed_nom), //[5113h.04h], Название=Номинальная скорость, ТекстПУ=СкоростьНом, EngText=Номинальная скорость, группа=Базов. парам
(long)(&drv_params.Udc_nom), //[5113h.05h], Название=Номинальное напряжение ЗПТ, ТекстПУ=UdcНом, EngText=Номинальное напряжение ЗПТ, группа=Базов. парам
(long)(&drv_params.Rs), //[5113h.06h], Название=Rs, ТекстПУ=Rs, EngText=Rs, группа=Базов. парам
(long)(&drv_params.Ls), //[5113h.07h], Название=Ls, ТекстПУ=Ls, EngText=Ls, группа=Базов. парам
(long)(&drv_params.freq_nom), //[5113h.08h], Название=Номинальная частота, ТекстПУ=Ном.част., EngText=Rated frequency, группа=Базов. парам
(long)(&drv_params.sens_type), //[5113h.09h], Название=Тип датчика положения, ТекстПУ=Sens_type, EngText=Sensor type, группа=Базов. парам
(long)(&sw.HardwareType), //[5114h.01h], Название=Тип аппаратной части, ТекстПУ=HardwareType, EngText=HardwareType, группа=Настройки СУ
(long)(&sm_ctrl.run_mode), //[5114h.02h], Название=Режим работы, ТекстПУ=РежРаб, EngText=Mode of operation, группа=Настройки СУ
(long)(&sw.recuperation_ena), //[5114h.03h], Название=Рекуперация, ТекстПУ=Рекупр, EngText=Рекуперация, группа=Настройки СУ
(long)(&sw.AutoOffset), //[5114h.04h], Название=АЦП авто смещ., ТекстПУ=АЦПсмещ, EngText=AutoOffset, группа=Настройки СУ
(long)(&sw.Reboot), //[5114h.05h], Название=Перезагрузка, ТекстПУ=Перезагрузка, EngText=Reboot, группа=Настройки СУ
(long)(&sw.excitation_ena), //[5114h.06h], Название=Возбудитель, ТекстПУ=Возб, EngText=, группа=Настройки СУ
(long)(&adc.Imeas_a_gain), //[5116h.01h], Название=Коэффициент тока фазы А, ТекстПУ=IaУсил, EngText=Phase A Current Max. Scale, группа=АЦП
(long)(&adc.Imeas_a_offset), //[5116h.02h], Название=Смещение тока фазы А, ТекстПУ=IaСмещ, EngText=Phase A Current Offset, группа=АЦП
(long)(&adc.Imeas_a), //[5116h.03h], Название=Ток фазы А, ТекстПУ=Ia_мгн, EngText=Phase A Current, группа=АЦП
(long)(&adc.Imeas_b_gain), //[5116h.04h], Название=Коэффициент тока фазы В, ТекстПУ=IbУсил, EngText=Phase B Current Max. Scale, группа=АЦП
(long)(&adc.Imeas_b_offset), //[5116h.05h], Название=Смещение тока фазы В, ТекстПУ=IbСмещ, EngText=Phase B Current Offset, группа=АЦП
(long)(&adc.Imeas_b), //[5116h.06h], Название=Ток фазы В, ТекстПУ=Ib_мгн, EngText=Phase B Current, группа=АЦП
(long)(&adc.Imeas_c_gain), //[5116h.07h], Название=Коэффициент тока фазы C, ТекстПУ=IcУсил, EngText=Коэффициент тока фазы C, группа=АЦП
(long)(&adc.Imeas_c_offset), //[5116h.08h], Название=Смещение тока фазы C, ТекстПУ=IcСмещ, EngText=Смещение тока фазы C, группа=АЦП
(long)(&adc.Imeas_c), //[5116h.09h], Название=Ток фазы С, ТекстПУ=Ic_мгн, EngText=Phase C Current, группа=АЦП
(long)(&adc.Imeas_d_gain), //[5116h.0Ah], Название=Коэффициент тока фазы D, ТекстПУ=IdУсил, EngText=Phase D Current Max. Scale, группа=АЦП
(long)(&adc.Imeas_d_offset), //[5116h.0Bh], Название=Смещение тока фазы D, ТекстПУ=IdСмещ, EngText=Phase D Current Offset, группа=АЦП
(long)(&adc.Imeas_d), //[5116h.0Ch], Название=Ток фазы D, ТекстПУ=Id_мгн, EngText=Phase D Current, группа=АЦП
(long)(&adc.Udc_meas_gain), //[5116h.0Dh], Название=Коэффициент напряжения ЗПТ, ТекстПУ=UdУсил, EngText=Udc Max. Scale, группа=АЦП
(long)(&adc.Udc_meas_offset), //[5116h.0Eh], Название=Смещение напряж. ЗПТ, ТекстПУ=Udсмещ, EngText=Udc Offset, группа=АЦП
(long)(&adc.Udc_meas), //[5116h.0Fh], Название=Напряжение ЗПТ, ТекстПУ=Ud, EngText=Напряжение ЗПТ, группа=АЦП
(long)(&vhz.freq), //[511Ah.01h], Название=Выходная частота, ТекстПУ=F_вых, EngText=Выходная частота, группа=Кривая U-f
(long)(&vhz.vout), //[511Ah.02h], Название=Выходное напряжение, ТекстПУ=U_вых, EngText=Выходное напряжение, группа=Кривая U-f
(long)(&vhz.FL), //[511Ah.03h], Название=Нулевая опроная частота, ТекстПУ=f [0], EngText=Нулевая опроная частота, группа=Кривая U-f
(long)(&vhz.Vmin), //[511Ah.04h], Название=Нулевое опорное напряжение, ТекстПУ=U [0], EngText=Нулевое опорное напряжение, группа=Кривая U-f
(long)(&vhz.FH), //[511Ah.05h], Название=Первая опорная частота, ТекстПУ=f [1], EngText=Первая опорная частота, группа=Кривая U-f
(long)(&vhz.Vmax), //[511Ah.06h], Название=Первое опорное напряжение, ТекстПУ=U [1], EngText=Первое опорное напряжение, группа=Кривая U-f
(long)(&vhz.Fmax), //[511Ah.07h], Название=Максимальная частота, ТекстПУ=f max, EngText=Максимальная частота, группа=Кривая U-f
(long)(&cur_par.speed), //[5138h.01h], Название=Скорость, ТекстПУ=скор, EngText=Speed, группа=Наблюдаемые
(long)(&cur_par.power), //[5138h.02h], Название=Акт. мощность, ТекстПУ=АктМощ, EngText=main_pInd, группа=Наблюдаемые
(long)(&cur_par.Is), //[5138h.03h], Название=Ток статора, ТекстПУ=ТокСт, EngText=Ток статора, группа=Наблюдаемые
(long)(&cur_par.ThetaRefCurr), //[5138h.04h], Название=ЭлУголЗад, ТекстПУ=ЭлУголЗад, EngText=ЭлУголЗад, группа=Наблюдаемые
(long)(&cur_par.ThetaCurr), //[5138h.05h], Название=ЭлУгол, ТекстПУ=ЭлУгол, EngText=ЭлУгол, группа=Наблюдаемые
(long)(&cur_par.IsRef), //[5138h.06h], Название=Ток статора зад, ТекстПУ=ТокСтЗад, EngText=Ток статора зад, группа=Наблюдаемые
(long)(&dlog.mode_reset), //[5150h.01h], Название=dlog.mode_reset, ТекстПУ=mode, EngText=dlog.mode_reset, группа=dlog
(long)(&dlog.control), //[5150h.02h], Название=dlog.control, ТекстПУ=dlcont, EngText=dlog.control, группа=dlog
(long)(&dlog.ind_subind1), //[5150h.03h], Название=dlog.ind_subind1, ТекстПУ=dlisi1, EngText=dlog.ind_subind1, группа=dlog
(long)(&dlog.ind_subind2), //[5150h.04h], Название=dlog.ind_subind2, ТекстПУ=dlisi2, EngText=dlog.ind_subind2, группа=dlog
(long)(&dlog.ind_subind3), //[5150h.05h], Название=dlog.ind_subind3, ТекстПУ=dlisi3, EngText=dlog.ind_subind3, группа=dlog
(long)(&dlog.ind_subind4), //[5150h.06h], Название=dlog.ind_subind4, ТекстПУ=dlisi4, EngText=dlog.ind_subind4, группа=dlog
(long)(&dlog.StartBits), //[5150h.07h], Название=StartBitL, ТекстПУ=StrtBL, EngText=StartBitL, группа=dlog
(long)(&dlog.StartBits), //[5150h.08h], Название=StartBitH, ТекстПУ=StrtBH, EngText=StartBitH, группа=dlog
(long)(&dlog.OneShotOperation), //[5150h.09h], Название=OneShotOperation, ТекстПУ=OneShotOp, EngText=OneShotOperation, группа=dlog
(long)(&dlog.trig_shift), //[5150h.0Ah], Название=Количество точек предыстории, ТекстПУ=N предыст., EngText=Prehistory Points Number, группа=dlog
(long)(&dlog.next_value_var), //[5151h.00h], Название=dlog.next_value_var, ТекстПУ=dlnval, EngText=dlog.next_value_var, группа=ВНЕ ГРУПП
(long)(&posspeedEqep.resol), //[5152h.01h], Название=Кол-во периодов, ТекстПУ=Кол-во периодов, EngText=Resolution, группа=ДПР-Энкодер
(long)(&posspeedEqep.Posspeed_CTL.all), //[5152h.02h], Название=Наличие репера, ТекстПУ=Наличие репера, EngText=index_en, группа=ДПР-Энкодер
(long)(&posspeedEqep.SpeedCalcType), //[5152h.03h], Название=Рассчитывать скорость программно, ТекстПУ=SpeedCalcType, EngText=SpeedCalcType, группа=ДПР-Энкодер
(long)(&posspeedEqep.AngleOffset), //[5152h.04h], Название=СмещУгол, ТекстПУ=СмещУг, EngText=thetae_offset, группа=ДПР-Энкодер
(long)(&posspeedEqep.Poscnt_res), //[5152h.05h], Название=Механическ.угол в метках, ТекстПУ=Механическ.угол в ме, EngText=Ang_mech, группа=ДПР-Энкодер
(long)(&posspeedEqep.Poscnt_res16), //[5152h.06h], Название=Механическ.угол в метках, ТекстПУ=Механическ.угол в ме, EngText=Ang_mech, группа=ДПР-Энкодер
(long)(&posspeedEqep.speed_elec), //[5152h.07h], Название=Скорость , ТекстПУ=Скорость, EngText=Speed, группа=ДПР-Энкодер
(long)(&posspeedEqep.theta_elec), //[5152h.08h], Название=Угол. электр., ТекстПУ=Угол. электр., EngText=Electrical angle, группа=ДПР-Энкодер
(long)(&posspeedEqep.theta_mech), //[5152h.09h], Название=Угол мех., ТекстПУ=Угол мех., EngText=theta_mech_tmp, группа=ДПР-Энкодер
(long)(&posspeedEqep.Posspeed_CTL.all), //[5152h.0Ah], Название=Команда иниц., ТекстПУ=Команда иниц., EngText=Reinit Command, группа=ДПР-Энкодер
(long)(&posspeedEqep.speed_filter.T), //[5152h.0Bh], Название=К быстродействия фильтра скорости, ТекстПУ=Кф. скорости, EngText=, группа=ДПР-Энкодер
(long)(&posspeedEqep.Posspeed_FLG1.all), //[5152h.0Ch], Название=Posspeed_FLG1, ТекстПУ=Posspeed_FLG1, EngText=Posspeed_FLG1, группа=ДПР-Энкодер
(long)(&posspeedEqep.GPIOsValue), //[5152h.0Dh], Название=GPIOsValue, ТекстПУ=GPIOsValue, EngText=GPIOsValue, группа=ДПР-Энкодер
(long)(&posspeedEqep.UPPS_forWatch), //[5152h.0Eh], Название=UPPS, ТекстПУ=UPPS, EngText=UPPS, группа=ДПР-Энкодер
(long)(&posspeedEqep.Qcprdlat_tmp), //[5152h.0Fh], Название=QCPRD, ТекстПУ=QCPRD, EngText=QCPRD, группа=ДПР-Энкодер
(long)(&posspeedEqep.Qcprdlat16_tmp), //[5152h.10h], Название=QCPRD, ТекстПУ=QCPRD, EngText=QCPRD, группа=ДПР-Энкодер
(long)(&posspeedEqep.QEPSTS), //[5152h.11h], Название=QEPSTS, ТекстПУ=QEPSTS, EngText=QEPSTS, группа=ДПР-Энкодер
(long)(&posspeedEqep.RevolutionCounter), //[5152h.12h], Название=Число оборотов, ТекстПУ=revolCounter, EngText=Total Revolutions, группа=ДПР-Энкодер
(long)(&posspeedEqep.Poscnt_resContinouosInt8), //[5152h.13h], Название=Абс. положение в метках, ТекстПУ=qcAbsPos, EngText=Absolute position (qc), группа=ДПР-Энкодер
(long)(&posspeedEqep.Poscnt_resContinouosInt), //[5152h.14h], Название=Абс. положение в метках, ТекстПУ=qcAbsPos, EngText=Absolute position (qc), группа=ДПР-Энкодер
(long)(&posspeedEqep.Poscnt_resContinouosLong), //[5152h.15h], Название=Абс. положение в метках, ТекстПУ=qcAbsPos, EngText=Absolute position (qc), группа=ДПР-Энкодер
(long)(&posspeedEqep.theta_mechContinouos), //[5152h.16h], Название=Угол механический не огранич., ТекстПУ=Уг.мех.не_огр, EngText=theta_mechContinouos, группа=ДПР-Энкодер
(long)(&posspeedEqep.theta_elecContinouos), //[5152h.17h], Название=Угол электрический не огранич., ТекстПУ=Уг.эл.не_огр, EngText=theta_elecContinouos, группа=ДПР-Энкодер
(long)(&DPReCAP.Angle), //[5155h.01h], Название=ЭлУгол, ТекстПУ=ЭлУгол, EngText=ЭлУгол, группа=ДПР-Холл
(long)(&DPReCAP.speed), //[5155h.02h], Название=Скорость , ТекстПУ=Скорость, EngText=Speed, группа=ДПР-Холл
(long)(&DPReCAP.speedMIN), //[5155h.03h], Название=Уставка Скорости, ТекстПУ=Уставка Скорости, EngText=speedMIN, группа=ДПР-Холл
(long)(&DPReCAP.Angle6), //[5155h.04h], Название=ЭлУгол6, ТекстПУ=ЭлУгол6, EngText=Angle6, группа=ДПР-Холл
(long)(&DPReCAP.DPReCAP_FLG1.all), //[5155h.05h], Название=Инт-полятор вкл, ТекстПУ=Инт-полятор вкл, EngText=Инт-полятор вкл, группа=ДПР-Холл
(long)(&DPReCAP.milsecREF), //[5155h.06h], Название=Уставка обнуления скорости, ТекстПУ=Уставка обнуления ск, EngText=, группа=ДПР-Холл
(long)(&DPReCAP.speedMinREF), //[5155h.07h], Название=Скорость Мин., ТекстПУ=Скорость Мин., EngText=speed min, группа=ДПР-Холл
(long)(&DPReCAP.CAP_WrongEdgeCnt), //[5155h.08h], Название=Ложные фронты, ТекстПУ=Ложные фронты, EngText=CAP_WrongEdgeCnt, группа=ДПР-Холл
(long)(&DPReCAP.HallCode), //[5155h.09h], Название=Дискретные входы, ТекстПУ=Входы, EngText=Digital Inputs, группа=ДПР-Холл
(long)(&DPReCAP.AngleOffset), //[5155h.0Ah], Название=СмещУгол, ТекстПУ=СмещУг, EngText=thetae_offset, группа=ДПР-Холл
(long)(&DPReCAP.ErrorLevel), //[5155h.0Bh], Название=Уровень помех, ТекстПУ=Уровень помех, EngText=, группа=ДПР-Холл
(long)(&DPReCAP.CAP_WrongEdgeCnt1), //[5155h.0Ch], Название=Ошибка 1 канала, ТекстПУ=Ошиб_1_к., EngText=, группа=ДПР-Холл
(long)(&DPReCAP.CAP_WrongEdgeCnt2), //[5155h.0Dh], Название=Ошибка 2 канала, ТекстПУ=Ошиб_2_к., EngText=, группа=ДПР-Холл
(long)(&DPReCAP.CAP_WrongEdgeCnt3), //[5155h.0Eh], Название=Ошибка 3 канала, ТекстПУ=Ошиб_3_к., EngText=, группа=ДПР-Холл
(long)(&DPReCAP.DPReCAP_FLG1.all), //[5155h.0Fh], Название=Направление вращения, ТекстПУ=НапрВр, EngText=Направление вращения, группа=ДПР-Холл
(long)(&DPReCAP.WrongCodeCounterLimitPerSec), //[5155h.10h], Название=Макс.число ошибок ДПР, ТекстПУ=Макс.ош.ДПР, EngText=, группа=ДПР-Холл
(long)(&DPReCAP.WrongCodeCounter), //[5155h.11h], Название=Кол-во ошибок неверной последовательности , ТекстПУ=Ошибки последоват., EngText=, группа=ДПР-Холл
(long)(&CounterCAP_isr), //[5155h.12h], Название=Счётчик прерываний по приему, ТекстПУ=Счёт.Rx.Int., EngText=, группа=ДПР-Холл
(long)(&cap0_counter), //[5155h.13h], Название=Счётчик прерываний CAP1, ТекстПУ=Сч.прерыв.CAP1, EngText=, группа=ДПР-Холл
(long)(&cap1_counter), //[5155h.14h], Название=Счётчик прерываний CAP2, ТекстПУ=Сч.прерыв.CAP2, EngText=, группа=ДПР-Холл
(long)(&cap2_counter), //[5155h.15h], Название=Счётчик прерываний CAP3, ТекстПУ=Сч.прерыв.CAP3, EngText=, группа=ДПР-Холл
(long)(&SSI_Encoder.resol), //[5157h.01h], Название=Количество импульсов на оборот, ТекстПУ=qepTik, EngText=Количество импульсов на оборот, группа=ДПР-SSI энкодер
(long)(&SSI_Encoder.AngleOffset), //[5157h.02h], Название=СмещУгол, ТекстПУ=СмещУг, EngText=thetae_offset, группа=ДПР-SSI энкодер
(long)(&SSI_Encoder.Poscnt_res), //[5157h.03h], Название=Механическ.угол в метках, ТекстПУ=Механическ.угол в ме, EngText=Ang_mech, группа=ДПР-SSI энкодер
(long)(&SSI_Encoder.speed_elec), //[5157h.04h], Название=Скорость , ТекстПУ=Скорость, EngText=Speed, группа=ДПР-SSI энкодер
(long)(&SSI_Encoder.theta_elec), //[5157h.05h], Название=Угол. электр., ТекстПУ=Угол. электр., EngText=Electrical angle, группа=ДПР-SSI энкодер
(long)(&SSI_Encoder.theta_mech), //[5157h.06h], Название=Угол мех., ТекстПУ=Угол мех., EngText=theta_mech_tmp, группа=ДПР-SSI энкодер
(long)(&SSI_Encoder.theta_mech_filtered), //[5157h.07h], Название=Угол мех. фильтрованный, ТекстПУ=theta_mech_filtered, EngText=theta_mech_filtered, группа=ДПР-SSI энкодер
(long)(&SSI_Encoder.theta_mech_filterK), //[5157h.08h], Название=К быстродействия фильтра угла, ТекстПУ=Кф. угла, EngText=, группа=ДПР-SSI энкодер
(long)(&SSI_Encoder.RevolutionCounter), //[5157h.09h], Название=Число оборотов, ТекстПУ=revolCounter, EngText=Total Revolutions, группа=ДПР-SSI энкодер
(long)(&SSI_Encoder.Poscnt_resContinouosLong), //[5157h.0Ah], Название=Абс. положение в метках, ТекстПУ=qcAbsPos, EngText=Absolute position (qc), группа=ДПР-SSI энкодер
(long)(&SSI_Encoder.Poscnt_resContinouosInt8), //[5157h.0Bh], Название=Абс. положение в метках, ТекстПУ=qcAbsPos, EngText=Absolute position (qc), группа=ДПР-SSI энкодер
(long)(&SSI_Encoder.Poscnt_resContinouosInt), //[5157h.0Ch], Название=Абс. положение в метках, ТекстПУ=qcAbsPos, EngText=Absolute position (qc), группа=ДПР-SSI энкодер
(long)(&SSI_Encoder.theta_mechContinouos), //[5157h.0Dh], Название=Угол механический не огранич., ТекстПУ=Уг.мех.не_огр, EngText=theta_mechContinouos, группа=ДПР-SSI энкодер
(long)(&SSI_Encoder.theta_elecContinouos), //[5157h.0Eh], Название=Угол электрический не огранич., ТекстПУ=Уг.эл.не_огр, EngText=theta_elecContinouos, группа=ДПР-SSI энкодер
(long)(&Debug1), //[5174h.01h], Название=DebugL1, ТекстПУ=DebugL1, EngText=DebugL1, группа=Для отладки
(long)(&Debug1), //[5174h.02h], Название=DebugL1, ТекстПУ=DebugL1, EngText=DebugL1, группа=Для отладки
(long)(&Debug2), //[5174h.03h], Название=DebugL2, ТекстПУ=DebugL2, EngText=DebugL2, группа=Для отладки
(long)(&Debug2), //[5174h.04h], Название=DebugL2, ТекстПУ=DebugL2, EngText=DebugL2, группа=Для отладки
(long)(&Debug3), //[5174h.05h], Название=DebugI3, ТекстПУ=DebugI3, EngText=DebugI3, группа=Для отладки
(long)(&Debug3), //[5174h.06h], Название=DebugI3, ТекстПУ=DebugI3, EngText=DebugI3, группа=Для отладки
(long)(&Debug4), //[5174h.07h], Название=DebugI4, ТекстПУ=DebugI4, EngText=DebugI4, группа=Для отладки
(long)(&Debug4), //[5174h.08h], Название=DebugI4, ТекстПУ=DebugI4, EngText=DebugI4, группа=Для отладки
(long)(&DebugW1), //[5174h.09h], Название=DebugW1, ТекстПУ=DebugW1, EngText=DebugW1, группа=Для отладки
(long)(&DebugW1), //[5174h.0Ah], Название=DebugW1, ТекстПУ=DebugW1, EngText=DebugW1, группа=Для отладки
(long)(&DebugW2), //[5174h.0Bh], Название=DebugW2, ТекстПУ=DebugW2, EngText=DebugW2, группа=Для отладки
(long)(&DebugW2), //[5174h.0Ch], Название=DebugW2, ТекстПУ=DebugW2, EngText=DebugW2, группа=Для отладки
(long)(&DebugW3), //[5174h.0Dh], Название=DebugW3, ТекстПУ=DebugW3, EngText=DebugW3, группа=Для отладки
(long)(&DebugW3), //[5174h.0Eh], Название=DebugW3, ТекстПУ=DebugW3, EngText=DebugW3, группа=Для отладки
(long)(&DebugW4), //[5174h.0Fh], Название=DebugW4, ТекстПУ=DebugW4, EngText=, группа=Для отладки
(long)(&DebugW4), //[5174h.10h], Название=DebugW4, ТекстПУ=DebugW4, EngText=, группа=Для отладки
(long)(&DebugF1), //[5174h.11h], Название=DebugF1, ТекстПУ=DebugF1, EngText=, группа=Для отладки
(long)(&DebugF2), //[5174h.12h], Название=DebugF2, ТекстПУ=DebugF2, EngText=, группа=Для отладки
(long)(&DebugF3), //[5174h.13h], Название=DebugF3, ТекстПУ=DebugF3, EngText=, группа=Для отладки
(long)(&DebugF4), //[5174h.14h], Название=DebugF4, ТекстПУ=DebugF4, EngText=, группа=Для отладки
(long)(&model.MotorType), //[5181h.01h], Название=Тип двигателя модели, ТекстПУ=ModelMotorType, EngText=ModelMotorType, группа=Модель общие данные
(long)(&model.MotorParametersNum), //[5181h.02h], Название=Параметры двигателя, ТекстПУ=ПарДв, EngText=Параметры двигателя, группа=Модель общие данные
(long)(&model.load), //[5181h.03h], Название=Симулируемый момент нагрузки, ТекстПУ=Мнагр сим., EngText=Simulated load torque, группа=Модель общие данные
(long)(&model.motorInternals.udc), //[5181h.04h], Название=Напряжение ЗПТ, ТекстПУ=Ud, EngText=Напряжение ЗПТ, группа=Модель общие данные
(long)(&model.motorInternals.torque), //[5181h.05h], Название=Реализованный электрический момент, ТекстПУ=Реал.Эл.Мом, EngText=torque, группа=Модель общие данные
(long)(&model.motorInternals.omega), //[5181h.06h], Название=Скорость, ТекстПУ=qepspd, EngText=Speed, группа=Модель общие данные
(long)(&model.motorInternals.omega_rpm), //[5181h.07h], Название=Скорость, ТекстПУ=qepspd, EngText=Speed, группа=Модель общие данные
(long)(&model.motorInternals.tetaRM), //[5181h.08h], Название=Механический угол, ТекстПУ=qeptm, EngText=Механический угол, группа=Модель общие данные
(long)(&model.qepCounter), //[5181h.09h], Название=EncoderIncrements, ТекстПУ=EncoderIncrements, EngText=, группа=Модель общие данные
(long)(&model.motorInternals.isa), //[5181h.0Ah], Название=Ток статора по оси Alpha, ТекстПУ=IsAlpha, EngText=Stator current in Alpha-axis, группа=Модель общие данные
(long)(&model.motorInternals.isb), //[5181h.0Bh], Название=Ток статора по оси Beta, ТекстПУ=IsBeta, EngText=Stator current in Beta-axis, группа=Модель общие данные
(long)(&model.motorInternals.isPhaseA), //[5181h.0Ch], Название=Ток фазы А, ТекстПУ=Ia, EngText=Ток фазы А, группа=Модель общие данные
(long)(&model.motorInternals.isPhaseB), //[5181h.0Dh], Название=Ток фазы В, ТекстПУ=Ib_мгн, EngText=Phase B Current, группа=Модель общие данные
(long)(&model.motorInternals.isPhaseC), //[5181h.0Eh], Название=Ток фазы C, ТекстПУ=Ic, EngText=Ток фазы C, группа=Модель общие данные
(long)(&model.motorInternals.isPhaseD), //[5181h.0Fh], Название=Ток фазы D, ТекстПУ=Id, EngText=Ток фазы D, группа=Модель общие данные
(long)(&model.motorInternals.ua), //[5181h.10h], Название=Напряжение фазы A, ТекстПУ=Ua, EngText=, группа=Модель общие данные
(long)(&model.motorInternals.ub), //[5181h.11h], Название=Напряжение фазы B, ТекстПУ=Ub, EngText=, группа=Модель общие данные
(long)(&model.motorInternals.uc), //[5181h.12h], Название=Напряжение фазы C, ТекстПУ=Uc, EngText=, группа=Модель общие данные
(long)(&model.motorInternals.power), //[5181h.13h], Название=Мощность, ТекстПУ=Мощ-ть, EngText=, группа=Модель общие данные
(long)(&model.fault), //[5181h.14h], Название=Код аварии, ТекстПУ=КодАвр, EngText=Код аварии, группа=Модель общие данные
(long)(&model.motorInternals.lm), //[5182h.01h], Название=Взаимная индуктивность Lm, ТекстПУ=Lm, EngText=Lm, группа=Модель (асинхронный)
(long)(&model.motorInternals.ls), //[5182h.02h], Название=Индуктивность статора Ls, ТекстПУ=Ls, EngText=Ls, группа=Модель (асинхронный)
(long)(&model.motorInternals.lr), //[5182h.03h], Название=Индуктивность ротора Lr, ТекстПУ=Lr, EngText=Lr, группа=Модель (асинхронный)
(long)(&model.motorInternals.rs), //[5182h.04h], Название=Сопротивление статора Rs, ТекстПУ=Rs, EngText=Rs, группа=Модель (асинхронный)
(long)(&model.motorInternals.rr), //[5182h.05h], Название=Сопротивление ротора Rr, ТекстПУ=Rr, EngText=Rr, группа=Модель (асинхронный)
(long)(&model.motorInternals.pp), //[5182h.06h], Название=Число пар полюсов, ТекстПУ=ПарПол, EngText=Число пар полюсов, группа=Модель (асинхронный)
(long)(&model.motorInternals.j), //[5182h.07h], Название=Симулируемый момент инерции, ТекстПУ=Инерция сим., EngText=Simulated motor inertia, группа=Модель (асинхронный)
(long)(&model.motorInternals.MechLoss), //[5182h.08h], Название=Тормозной момент для выбега, ТекстПУ=Торм.Мом.Выбег, EngText=Torque ref for overrun, группа=Модель (асинхронный)
(long)(&model.motorInternals.QEPResolution), //[5182h.09h], Название=Число периодов квадратурного датчика, ТекстПУ=QEPResolution, EngText=QEPResolution, группа=Модель (асинхронный)
(long)(&model.motorInternals.RatedCurrent), //[5182h.0Ah], Название=Номинальный действующий ток, ТекстПУ=RatedCurrent, EngText=RatedCurrent, группа=Модель (асинхронный)
(long)(&model.motorInternals.RatedPower), //[5182h.0Bh], Название=Номинальная мощность, ТекстПУ=RatedPower, EngText=RatedPower, группа=Модель (асинхронный)
(long)(&model.motorInternals.RatedSpeed), //[5182h.0Ch], Название=Номинальная скорость, ТекстПУ=СкоростьНом, EngText=Номинальная скорость, группа=Модель (асинхронный)
(long)(&model.motorInternals.iADCk), //[5182h.0Dh], Название=Коэффициент АЦП тока фаз, ТекстПУ=Коэф. АЦП тока, EngText=ADC_k_i, группа=Модель (асинхронный)
(long)(&model.motorInternals.isa), //[5182h.0Eh], Название=Ток статора по оси Alpha, ТекстПУ=IsAlpha, EngText=Stator current in Alpha-axis, группа=Модель (асинхронный)
(long)(&model.motorInternals.isb), //[5182h.0Fh], Название=Ток статора по оси Beta, ТекстПУ=IsBeta, EngText=Stator current in Beta-axis, группа=Модель (асинхронный)
(long)(&model.motorInternals.prd), //[5182h.10h], Название=Поток ротора по оси D, ТекстПУ=PsiRD, EngText=Rotor flux D-axis, группа=Модель (асинхронный)
(long)(&model.motorInternals.prq), //[5182h.11h], Название=Поток ротора по оси Q, ТекстПУ=PsiRQ, EngText=Rotor flux Q-axis, группа=Модель (асинхронный)
(long)(&model.motorInternals.pra), //[5182h.12h], Название=Поток ротора по оси Alpha, ТекстПУ=PsiRAlfa, EngText=Rotor flux Alpha-axis, группа=Модель (асинхронный)
(long)(&model.motorInternals.prb), //[5182h.13h], Название=Поток ротора по оси Beta, ТекстПУ=PsiRBeta, EngText=Rotor flux Beta-axis, группа=Модель (асинхронный)
(long)(&model.motorInternals.m), //[5183h.01h], Название=Потокосцепление магнитов, ТекстПУ=m, EngText=m, группа=Модель (синхронный)
(long)(&model.motorInternals.lsd), //[5183h.02h], Название=Индуктивность статора Lsd, ТекстПУ=Lsd, EngText=Lsd, группа=Модель (синхронный)
(long)(&model.motorInternals.lsq), //[5183h.03h], Название=Индуктивность статора Lsq, ТекстПУ=Lsq, EngText=Lsq, группа=Модель (синхронный)
(long)(&model.motorInternals.rs), //[5183h.04h], Название=Сопротивление статора Rs, ТекстПУ=Rs, EngText=Rs, группа=Модель (синхронный)
(long)(&model.motorInternals.pp), //[5183h.05h], Название=Число пар полюсов, ТекстПУ=ПарПол, EngText=Число пар полюсов, группа=Модель (синхронный)
(long)(&model.motorInternals.j), //[5183h.06h], Название=Симулируемый момент инерции, ТекстПУ=Инерция сим., EngText=Simulated motor inertia, группа=Модель (синхронный)
(long)(&model.motorInternals.MechLoss), //[5183h.07h], Название=Тормозной момент для выбега, ТекстПУ=Торм.Мом.Выбег, EngText=Torque ref for overrun, группа=Модель (синхронный)
(long)(&model.motorInternals.QEPResolution), //[5183h.08h], Название=Число периодов квадратурного датчика, ТекстПУ=QEPResolution, EngText=QEPResolution, группа=Модель (синхронный)
(long)(&model.motorInternals.iADCk), //[5183h.09h], Название=Коэффициент АЦП тока фаз, ТекстПУ=Коэф. АЦП тока, EngText=ADC_k_i, группа=Модель (синхронный)
(long)(&model.motorInternals.ifADCk), //[5183h.0Ah], Название=Коэффициент АЦП тока ОВ, ТекстПУ=Коэф. АЦП ток ОВ, EngText=ADC_k_if, группа=Модель (синхронный)
(long)(&model.motorInternals.RatedCurrent), //[5183h.0Bh], Название=Номинальный действующий ток, ТекстПУ=RatedCurrent, EngText=RatedCurrent, группа=Модель (синхронный)
(long)(&model.motorInternals.RatedPower), //[5183h.0Ch], Название=Номинальная мощность, ТекстПУ=RatedPower, EngText=RatedPower, группа=Модель (синхронный)
(long)(&model.motorInternals.RatedSpeed), //[5183h.0Dh], Название=Номинальная скорость, ТекстПУ=СкоростьНом, EngText=Номинальная скорость, группа=Модель (синхронный)
(long)(&model.motorInternals.RatedFluxCurrent), //[5183h.0Eh], Название=Номинальный ток ОВ, ТекстПУ=RatedFluxCurrent, EngText=RatedFluxCurrent, группа=Модель (синхронный)
(long)(&model.motorInternals.u_f), //[5183h.0Fh], Название=Разность потенциалов ОВ, ТекстПУ=Uf, EngText=Uf, группа=Модель (синхронный)
(long)(&model.motorInternals.i_f), //[5183h.10h], Название=Ток ОВ, ТекстПУ=Ток ОВ, EngText=Exciter current, группа=Модель (синхронный)
(long)(&model.motorInternals.m), //[5183h.11h], Название=Поток возбуждения, ТекстПУ=Psi возб, EngText=, группа=Модель (синхронный)
(long)(&model.motorInternals.l_a), //[5184h.01h], Название=Индуктивность якоря, ТекстПУ=La, EngText=La, группа=Модель (ДПТ)
(long)(&model.motorInternals.l_f), //[5184h.02h], Название=Индуктивность ОВ, ТекстПУ=Lf, EngText=Lf, группа=Модель (ДПТ)
(long)(&model.motorInternals.r_a), //[5184h.03h], Название=Сопротивление якоря, ТекстПУ=Ra, EngText=Ra, группа=Модель (ДПТ)
(long)(&model.motorInternals.r_f), //[5184h.04h], Название=Сопротивление ОВ, ТекстПУ=Rf, EngText=Rf, группа=Модель (ДПТ)
(long)(&model.motorInternals.j), //[5184h.05h], Название=Симулируемый момент инерции, ТекстПУ=Инерция сим., EngText=Simulated motor inertia, группа=Модель (ДПТ)
(long)(&model.motorInternals.MechLoss), //[5184h.06h], Название=Тормозной момент для выбега, ТекстПУ=Торм.Мом.Выбег, EngText=Torque ref for overrun, группа=Модель (ДПТ)
(long)(&model.motorInternals.QEPResolution), //[5184h.07h], Название=Число периодов квадратурного датчика, ТекстПУ=QEPResolution, EngText=QEPResolution, группа=Модель (ДПТ)
(long)(&model.motorInternals.RatedCurrent), //[5184h.08h], Название=Номинальный ток якоря, ТекстПУ=RatedCurrent, EngText=RatedCurrent, группа=Модель (ДПТ)
(long)(&model.motorInternals.RatedFluxCurrent), //[5184h.09h], Название=Номинальный ток ОВ, ТекстПУ=RatedFluxCurrent, EngText=RatedFluxCurrent, группа=Модель (ДПТ)
(long)(&model.motorInternals.iADCk), //[5184h.0Ah], Название=Коэффициент АЦП тока ОЯ, ТекстПУ=Коэф. АЦП ток ОЯ, EngText=ADC_k_ia, группа=Модель (ДПТ)
(long)(&model.motorInternals.ifADCk), //[5184h.0Bh], Название=Коэффициент АЦП тока ОВ, ТекстПУ=Коэф. АЦП ток ОВ, EngText=ADC_k_if, группа=Модель (ДПТ)
(long)(&model.motorInternals.RatedPower), //[5184h.0Ch], Название=Номинальная мощность, ТекстПУ=RatedPower, EngText=RatedPower, группа=Модель (ДПТ)
(long)(&model.motorInternals.RatedSpeed), //[5184h.0Dh], Название=Номинальная скорость, ТекстПУ=СкоростьНом, EngText=Номинальная скорость, группа=Модель (ДПТ)
(long)(&model.motorInternals.r_ad), //[5184h.0Eh], Название=Добавочное сопротивление, Rad, ТекстПУ=Доб.сопротив., EngText=Rad, группа=Модель (ДПТ)
(long)(&model.motorInternals.u_a), //[5184h.0Fh], Название=Разность потенциалов ОЯ, ТекстПУ=Ua, EngText=Ua, группа=Модель (ДПТ)
(long)(&model.motorInternals.u_f), //[5184h.10h], Название=Разность потенциалов ОВ, ТекстПУ=Uf, EngText=Uf, группа=Модель (ДПТ)
(long)(&model.motorInternals.i_a), //[5184h.11h], Название=Ток ОЯ, ТекстПУ=Ia, EngText=Ia, группа=Модель (ДПТ)
(long)(&model.motorInternals.i_f), //[5184h.12h], Название=Ток ОВ, ТекстПУ=Ток ОВ, EngText=Exciter current, группа=Модель (ДПТ)
(long)(&refs.DCMspeed_ref), //[5185h.01h], Название=Задание скорости (Подч. Рег.), ТекстПУ=Зад. Скор., EngText=SpeeD Ref, группа=ДПТ Сист. Упр.
(long)(&refs.ua_ref), //[5185h.02h], Название=Заданное напряжение якоря, ТекстПУ=Ua_ref, EngText=Ua_ref, группа=ДПТ Сист. Упр.
(long)(&refs.uf_ref), //[5185h.03h], Название=Заданное напряжение ОВ, ТекстПУ=Uf_ref, EngText=Uf_ref, группа=ДПТ Сист. Упр.
(long)(&model.motorInternals.j), //[5186h.01h], Название=Симулируемый момент инерции, ТекстПУ=Инерция сим., EngText=Simulated motor inertia, группа=Модель (ВИД)
(long)(&model.motorInternals.MechLoss), //[5186h.02h], Название=Тормозной момент для выбега, ТекстПУ=Торм.Мом.Выбег, EngText=Torque ref for overrun, группа=Модель (ВИД)
(long)(&model.motorInternals.rs), //[5186h.03h], Название=Сопротивление статора Rs, ТекстПУ=Rs, EngText=Rs, группа=Модель (ВИД)
(long)(&model.motorInternals.pp), //[5186h.04h], Название=Число пар полюсов, ТекстПУ=ПарПол, EngText=Число пар полюсов, группа=Модель (ВИД)
(long)(&model.motorInternals.QEPResolution), //[5186h.05h], Название=Число периодов квадратурного датчика, ТекстПУ=QEPResolution, EngText=QEPResolution, группа=Модель (ВИД)
(long)(&model.motorInternals.iADCk), //[5186h.06h], Название=Коэффициент АЦП тока фаз, ТекстПУ=Коэф. АЦП тока, EngText=ADC_k_i, группа=Модель (ВИД)
(long)(&model.motorInternals.RatedCurrent), //[5186h.07h], Название=Номинальный действующий ток, ТекстПУ=RatedCurrent, EngText=RatedCurrent, группа=Модель (ВИД)
(long)(&model.motorInternals.RatedPower), //[5186h.08h], Название=Номинальная мощность, ТекстПУ=RatedPower, EngText=RatedPower, группа=Модель (ВИД)
(long)(&model.motorInternals.RatedSpeed), //[5186h.09h], Название=Номинальная скорость, ТекстПУ=СкоростьНом, EngText=Номинальная скорость, группа=Модель (ВИД)
(long)(&model.motorInternals.psa), //[5186h.0Ah], Название=Поток статора фазы A, ТекстПУ=Поток A, EngText=, группа=Модель (ВИД)
(long)(&model.motorInternals.psb), //[5186h.0Bh], Название=Поток статора фазы B, ТекстПУ=Поток B, EngText=, группа=Модель (ВИД)
(long)(&model.motorInternals.psc), //[5186h.0Ch], Название=Поток статора фазы C, ТекстПУ=Поток C, EngText=, группа=Модель (ВИД)
(long)(&model.motorInternals.torque_a), //[5186h.0Dh], Название=Момент фазы A, ТекстПУ=Момент A, EngText=, группа=Модель (ВИД)
(long)(&model.motorInternals.torque_b), //[5186h.0Eh], Название=Момент фазы B, ТекстПУ=Момент B, EngText=, группа=Модель (ВИД)
(long)(&model.motorInternals.torque_c), //[5186h.0Fh], Название=Момент фазы C, ТекстПУ=Момент C, EngText=, группа=Модель (ВИД)
(long)(&ModBus.Enabled), //[5A20h.01h], Название=Включен, ТекстПУ=Вкл, EngText=Enabled, группа=MODBUS
(long)(&ModBus.RSNodeID), //[5A20h.02h], Название=Номер в сети ModBus, ТекстПУ=ModBus_ID, EngText=ModBus node ID, группа=MODBUS
(long)(&ModBus.BaudRate), //[5A20h.03h], Название=Скорость, ТекстПУ=скор, EngText=Speed, группа=MODBUS
(long)(&ModBus.OfflineCounterMax), //[5A20h.04h], Название=Таймаут потери связи, ТекстПУ=OfflineCounterMax, EngText=OfflineCounterMax, группа=MODBUS
(long)(&ModBus.AutoRestart), //[5A20h.05h], Название=Перезапуск при сбоях на линии, ТекстПУ=auto_res, EngText=auto_res, группа=MODBUS
(long)(&ModBus.OfflineCounter), //[5A20h.06h], Название=Счетчик потери связи, ТекстПУ=OfflineCounter, EngText=OfflineCounter, группа=MODBUS
(long)(&ModBus.isOnline), //[5A20h.07h], Название=MODBUS online, ТекстПУ=MODBUS online, EngText=MODBUS online, группа=MODBUS
(long)(&ModBus.received_data), //[5A20h.08h], Название=rs485_last_received_data, ТекстПУ=485_RD, EngText=rs485_last_received_data, группа=MODBUS
(long)(&ModBus.MBInternals.TxState), //[5A20h.09h], Название=Состояние передачи посылки, ТекстПУ=TXState, EngText=TXState, группа=MODBUS
(long)(&ModBus.GPIOsValue), //[5A20h.0Ah], Название=GPIOsValue, ТекстПУ=GPIOsValue, EngText=GPIOsValue, группа=MODBUS
(long)(&ModBus.ReceivePacketCounter), //[5A20h.0Bh], Название=Пакетов принято, ТекстПУ=ReceivePacketCounte, EngText=ReceivePacketCounter, группа=MODBUS
(long)(&ModBus.ReceiveByteCounter), //[5A20h.0Ch], Название=Байт принято, ТекстПУ=ReceiveByteCounter, EngText=ReceiveByteCounter, группа=MODBUS
(long)(&RotorObserver.Tr), //[5B04h.01h], Название=Пост. времени ротора, ТекстПУ=Tr, EngText=Tr, группа=Наблюдатель ротора АД
(long)(&RotorObserver.FluxCurrentRatio), //[5B04h.02h], Название=Соотношение тока возб. X к току Y, ТекстПУ=FluxCurrentRatio, EngText=FluxCurrentRatio, группа=Наблюдатель ротора АД
(long)(&RotorObserver.FluxCurrentMin), //[5B04h.03h], Название=Минимальный ток возбуждения, ТекстПУ=FluxCurrentMin, EngText=FluxCurrentMin, группа=Наблюдатель ротора АД
(long)(&RotorObserver.id), //[5B04h.04h], Название=Ток статора по оси D, ТекстПУ=IsD, EngText=Stator current D-Axis, группа=Наблюдатель ротора АД
(long)(&RotorObserver.iq), //[5B04h.05h], Название=Ток статора по оси Q, ТекстПУ=IsQ, EngText=Stator current Q-Axis, группа=Наблюдатель ротора АД
(long)(&RotorObserver.psi_d), //[5B04h.06h], Название=Поток ротора по оси D, ТекстПУ=PsiRD, EngText=Rotor flux D-axis, группа=Наблюдатель ротора АД
(long)(&RotorObserver.psi_q), //[5B04h.07h], Название=Поток ротора по оси Q, ТекстПУ=PsiRQ, EngText=Rotor flux Q-axis, группа=Наблюдатель ротора АД
(long)(&RotorObserver.theta_psi_elec), //[5B04h.08h], Название=Угол потока ротора, ТекстПУ=psiRalpha, EngText=Rotor flux angle, группа=Наблюдатель ротора АД
};

 //Таблица с форматами, дефолтами, и другими параметрами элементов 

Uint16 const CO2_OD_TBL2[] = {
 //[1000h.00h]
32045, //формат
1,//номер текста, ТекстПУ=Device Type
0,//смещение длЯ таблицы с адресами

 //[1001h.00h]
31021, //формат
2,//номер текста, ТекстПУ=ErrReg
1,//смещение длЯ таблицы с адресами

 //[1002h.00h]
32041, //формат
3,//номер текста, ТекстПУ=DevSta
2,//смещение длЯ таблицы с адресами

 //[1014h.00h]
32013, //формат
4,//номер текста, ТекстПУ=eCOBID
3,//смещение длЯ таблицы с адресами

 // Корневой под-индекс 1016h, ТекстПУ=CHBT
28814,//кол-во подиндексов
5, // номер текста
3, //такой длиной

 //[1016h.01h]
64769, //формат
5,//номер текста, ТекстПУ=CHBT
4,//смещение длЯ таблицы с адресами
(Uint16)(2000),//знач по умолч.
(((Uint32)2000)>>16),//знач по умолч.

 //[1017h.00h]
64257, //формат
6,//номер текста, ТекстПУ=PHBT
5,//смещение длЯ таблицы с адресами
(Uint16)(1000),//знач по умолч.

 // Корневой под-индекс 1018h, ТекстПУ=IdObj
28801,//кол-во подиндексов
7, // номер текста
1, //такой длиной

 //[1018h.01h]
32013, //формат
8,//номер текста, ТекстПУ=VendID
6,//смещение длЯ таблицы с адресами

 //[1018h.02h]
32013, //формат
9,//номер текста, ТекстПУ=PrCode
7,//смещение длЯ таблицы с адресами

 //[1018h.03h]
32013, //формат
10,//номер текста, ТекстПУ=RevNum
8,//смещение длЯ таблицы с адресами

 //[1018h.04h]
32013, //формат
11,//номер текста, ТекстПУ=SerNum
9,//смещение длЯ таблицы с адресами

 // Корневой под-индекс 1200h, ТекстПУ=SSDOP
19631,//кол-во подиндексов
12, // номер текста
1, //такой длиной

 //[1200h.01h]
31021, //формат
13,//номер текста, ТекстПУ=CSRXID
10,//смещение длЯ таблицы с адресами

 //[1200h.02h]
31021, //формат
14,//номер текста, ТекстПУ=SCTXID
11,//смещение длЯ таблицы с адресами

 // Корневой под-индекс 1280h, ТекстПУ=CSDOP
19632,//кол-во подиндексов
15, // номер текста
1, //такой длиной

 //[1280h.01h]
30765, //формат
16,//номер текста, ТекстПУ=CSTXID
12,//смещение длЯ таблицы с адресами

 //[1280h.02h]
30765, //формат
17,//номер текста, ТекстПУ=SCRXID
13,//смещение длЯ таблицы с адресами

 //[1280h.03h]
29741, //формат
18,//номер текста, ТекстПУ=SDOSID
14,//смещение длЯ таблицы с адресами

 // Корневой под-индекс 1400h, ТекстПУ=RPDO1P
18607,//кол-во подиндексов
19, // номер текста
3, //такой длиной

 //[1400h.01h]
64813, //формат
20,//номер текста, ТекстПУ=IRPDO1
15,//смещение длЯ таблицы с адресами
(Uint16)(0x180),//знач по умолч.
(((Uint32)0x180)>>16),//знач по умолч.

 //[1400h.02h]
28973, //формат
21,//номер текста, ТекстПУ=TransT
16,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 1401h, ТекстПУ=RPDO2P
18607,//кол-во подиндексов
22, // номер текста
3, //такой длиной

 //[1401h.01h]
64813, //формат
23,//номер текста, ТекстПУ=IRPDO2
17,//смещение длЯ таблицы с адресами
(Uint16)(0x280),//знач по умолч.
(((Uint32)0x280)>>16),//знач по умолч.

 //[1401h.02h]
28973, //формат
21,//номер текста, ТекстПУ=TransT
18,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 1402h, ТекстПУ=RPDO3P
18607,//кол-во подиндексов
24, // номер текста
3, //такой длиной

 //[1402h.01h]
64809, //формат
25,//номер текста, ТекстПУ=IRPDO3
19,//смещение длЯ таблицы с адресами
(Uint16)(0x0401),//знач по умолч.
(((Uint32)0x0401)>>16),//знач по умолч.

 //[1402h.02h]
28969, //формат
21,//номер текста, ТекстПУ=TransT
20,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 1403h, ТекстПУ=RPDO4P
18607,//кол-во подиндексов
26, // номер текста
3, //такой длиной

 //[1403h.01h]
64809, //формат
27,//номер текста, ТекстПУ=IRPDO4
21,//смещение длЯ таблицы с адресами
(Uint16)(0x0501),//знач по умолч.
(((Uint32)0x0501)>>16),//знач по умолч.

 //[1403h.02h]
28969, //формат
21,//номер текста, ТекстПУ=TransT
22,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 1404h, ТекстПУ=RPDO5P
18607,//кол-во подиндексов
28, // номер текста
3, //такой длиной

 //[1404h.01h]
64813, //формат
29,//номер текста, ТекстПУ=IRPDO5
23,//смещение длЯ таблицы с адресами
(Uint16)(0x0501),//знач по умолч.
(((Uint32)0x0501)>>16),//знач по умолч.

 //[1404h.02h]
28973, //формат
21,//номер текста, ТекстПУ=TransT
24,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 1405h, ТекстПУ=RPDO6P
18607,//кол-во подиндексов
30, // номер текста
3, //такой длиной

 //[1405h.01h]
64813, //формат
31,//номер текста, ТекстПУ=IRPDO6
25,//смещение длЯ таблицы с адресами
(Uint16)(0x0501),//знач по умолч.
(((Uint32)0x0501)>>16),//знач по умолч.

 //[1405h.02h]
28973, //формат
21,//номер текста, ТекстПУ=TransT
26,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 1406h, ТекстПУ=RPDO7P
18607,//кол-во подиндексов
32, // номер текста
3, //такой длиной

 //[1406h.01h]
64801, //формат
33,//номер текста, ТекстПУ=IRPDO7
27,//смещение длЯ таблицы с адресами
(Uint16)(0x0501),//знач по умолч.
(((Uint32)0x0501)>>16),//знач по умолч.

 //[1406h.02h]
28961, //формат
21,//номер текста, ТекстПУ=TransT
28,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 1407h, ТекстПУ=RPDO8P
18607,//кол-во подиндексов
34, // номер текста
3, //такой длиной

 //[1407h.01h]
64801, //формат
35,//номер текста, ТекстПУ=IRPDO8
29,//смещение длЯ таблицы с адресами
(Uint16)(0x0501),//знач по умолч.
(((Uint32)0x0501)>>16),//знач по умолч.

 //[1407h.02h]
28961, //формат
21,//номер текста, ТекстПУ=TransT
30,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 1600h, ТекстПУ=RPDO1M
19633,//кол-во подиндексов
36, // номер текста
3, //такой длиной

 //[1600h.01h]
61741, //формат
37,//номер текста, ТекстПУ=RPM11
31,//смещение длЯ таблицы с адресами
(Uint16)(0x511E0500),//знач по умолч.
(((Uint32)0x511E0500)>>16),//знач по умолч.

 //[1600h.02h]
61741, //формат
38,//номер текста, ТекстПУ=RPM12
32,//смещение длЯ таблицы с адресами
(Uint16)(0x511E0600),//знач по умолч.
(((Uint32)0x511E0600)>>16),//знач по умолч.

 //[1600h.03h]
61741, //формат
39,//номер текста, ТекстПУ=RPM13
33,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1600h.04h]
61741, //формат
40,//номер текста, ТекстПУ=RPM14
34,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 // Корневой под-индекс 1601h, ТекстПУ=RPDO2M
19633,//кол-во подиндексов
41, // номер текста
3, //такой длиной

 //[1601h.01h]
61741, //формат
42,//номер текста, ТекстПУ=RPM21
35,//смещение длЯ таблицы с адресами
(Uint16)(0x511E0700),//знач по умолч.
(((Uint32)0x511E0700)>>16),//знач по умолч.

 //[1601h.02h]
61741, //формат
43,//номер текста, ТекстПУ=RPM22
36,//смещение длЯ таблицы с адресами
(Uint16)(0x511E0800),//знач по умолч.
(((Uint32)0x511E0800)>>16),//знач по умолч.

 //[1601h.03h]
61741, //формат
44,//номер текста, ТекстПУ=RPM23
37,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1601h.04h]
61741, //формат
45,//номер текста, ТекстПУ=RPM24
38,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 // Корневой под-индекс 1602h, ТекстПУ=RPDO3M
19629,//кол-во подиндексов
46, // номер текста
3, //такой длиной

 //[1602h.01h]
61737, //формат
47,//номер текста, ТекстПУ=RPM31
39,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1602h.02h]
61737, //формат
48,//номер текста, ТекстПУ=RPM32
40,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1602h.03h]
61737, //формат
49,//номер текста, ТекстПУ=RPM33
41,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1602h.04h]
61737, //формат
50,//номер текста, ТекстПУ=RPM34
42,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 // Корневой под-индекс 1603h, ТекстПУ=RPDO4M
19629,//кол-во подиндексов
51, // номер текста
3, //такой длиной

 //[1603h.01h]
61737, //формат
52,//номер текста, ТекстПУ=RPM41
43,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1603h.02h]
61737, //формат
53,//номер текста, ТекстПУ=RPM42
44,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1603h.03h]
61737, //формат
54,//номер текста, ТекстПУ=RPM43
45,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1603h.04h]
61737, //формат
55,//номер текста, ТекстПУ=RPM44
46,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 // Корневой под-индекс 1604h, ТекстПУ=RPDO5M
19625,//кол-во подиндексов
56, // номер текста
3, //такой длиной

 //[1604h.01h]
61741, //формат
57,//номер текста, ТекстПУ=RPM51
47,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1604h.02h]
61741, //формат
58,//номер текста, ТекстПУ=RPM52
48,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1604h.03h]
61741, //формат
59,//номер текста, ТекстПУ=RPM53
49,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1604h.04h]
61741, //формат
60,//номер текста, ТекстПУ=RPM54
50,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 // Корневой под-индекс 1605h, ТекстПУ=RPDO6M
19625,//кол-во подиндексов
61, // номер текста
3, //такой длиной

 //[1605h.01h]
61741, //формат
62,//номер текста, ТекстПУ=RPM61
51,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1605h.02h]
61741, //формат
63,//номер текста, ТекстПУ=RPM62
52,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1605h.03h]
61741, //формат
64,//номер текста, ТекстПУ=RPM63
53,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1605h.04h]
61741, //формат
65,//номер текста, ТекстПУ=RPM64
54,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 // Корневой под-индекс 1606h, ТекстПУ=RPDO7M
19629,//кол-во подиндексов
66, // номер текста
3, //такой длиной

 //[1606h.01h]
61729, //формат
67,//номер текста, ТекстПУ=RPM71
55,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1606h.02h]
61729, //формат
68,//номер текста, ТекстПУ=RPM72
56,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1606h.03h]
61729, //формат
69,//номер текста, ТекстПУ=RPM73
57,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1606h.04h]
61729, //формат
70,//номер текста, ТекстПУ=RPM74
58,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 // Корневой под-индекс 1607h, ТекстПУ=RPDO8M
19629,//кол-во подиндексов
71, // номер текста
3, //такой длиной

 //[1607h.01h]
61729, //формат
72,//номер текста, ТекстПУ=RPM81
59,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1607h.02h]
61729, //формат
73,//номер текста, ТекстПУ=RPM82
60,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1607h.03h]
61729, //формат
74,//номер текста, ТекстПУ=RPM83
61,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1607h.04h]
61729, //формат
75,//номер текста, ТекстПУ=RPM84
62,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 // Корневой под-индекс 1800h, ТекстПУ=TPDO1P
16562,//кол-во подиндексов
76, // номер текста
2, //такой длиной

 //[1800h.01h]
32045, //формат
77,//номер текста, ТекстПУ=ITPDO1
63,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[1800h.02h]
31021, //формат
21,//номер текста, ТекстПУ=TransT
64,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[1800h.03h]
64301, //формат
78,//номер текста, ТекстПУ=ITime1
65,//смещение длЯ таблицы с адресами
(Uint16)(100),//знач по умолч.

 //[1800h.04h]
47405, //формат
79,//номер текста, ТекстПУ=CompEn
66,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[1800h.05h]
64301, //формат
80,//номер текста, ТекстПУ=EvTmr1
67,//смещение длЯ таблицы с адресами
(Uint16)(100),//знач по умолч.

 // Корневой под-индекс 1801h, ТекстПУ=TPDO2P
16560,//кол-во подиндексов
81, // номер текста
2, //такой длиной

 //[1801h.01h]
32045, //формат
82,//номер текста, ТекстПУ=ITPDO2
68,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[1801h.02h]
31021, //формат
21,//номер текста, ТекстПУ=TransT
69,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[1801h.03h]
64301, //формат
83,//номер текста, ТекстПУ=ITime2
70,//смещение длЯ таблицы с адресами
(Uint16)(100),//знач по умолч.

 //[1801h.04h]
47405, //формат
79,//номер текста, ТекстПУ=CompEn
71,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[1801h.05h]
64301, //формат
84,//номер текста, ТекстПУ=EvTmr2
72,//смещение длЯ таблицы с адресами
(Uint16)(1000),//знач по умолч.

 // Корневой под-индекс 1802h, ТекстПУ=TPDO3P
16562,//кол-во подиндексов
85, // номер текста
2, //такой длиной

 //[1802h.01h]
32041, //формат
86,//номер текста, ТекстПУ=ITPDO3
73,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[1802h.02h]
31017, //формат
21,//номер текста, ТекстПУ=TransT
74,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[1802h.03h]
64297, //формат
87,//номер текста, ТекстПУ=ITime3
75,//смещение длЯ таблицы с адресами
(Uint16)(100),//знач по умолч.

 //[1802h.04h]
47401, //формат
79,//номер текста, ТекстПУ=CompEn
76,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[1802h.05h]
64297, //формат
88,//номер текста, ТекстПУ=EvTmr3
77,//смещение длЯ таблицы с адресами
(Uint16)(1000),//знач по умолч.

 // Корневой под-индекс 1803h, ТекстПУ=TPDO4P
16556,//кол-во подиндексов
89, // номер текста
2, //такой длиной

 //[1803h.01h]
32041, //формат
90,//номер текста, ТекстПУ=ITPDO4
78,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[1803h.02h]
31017, //формат
21,//номер текста, ТекстПУ=TransT
79,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[1803h.03h]
64297, //формат
91,//номер текста, ТекстПУ=ITime4
80,//смещение длЯ таблицы с адресами
(Uint16)(100),//знач по умолч.

 //[1803h.04h]
47401, //формат
79,//номер текста, ТекстПУ=CompEn
81,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[1803h.05h]
64297, //формат
92,//номер текста, ТекстПУ=EvTmr4
82,//смещение длЯ таблицы с адресами
(Uint16)(1000),//знач по умолч.

 // Корневой под-индекс 1804h, ТекстПУ=TPDO5P
16554,//кол-во подиндексов
93, // номер текста
2, //такой длиной

 //[1804h.01h]
32045, //формат
94,//номер текста, ТекстПУ=ITPDO5
83,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[1804h.02h]
31021, //формат
21,//номер текста, ТекстПУ=TransT
84,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[1804h.03h]
64301, //формат
95,//номер текста, ТекстПУ=ITime5
85,//смещение длЯ таблицы с адресами
(Uint16)(100),//знач по умолч.

 //[1804h.04h]
47405, //формат
79,//номер текста, ТекстПУ=CompEn
86,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[1804h.05h]
64301, //формат
96,//номер текста, ТекстПУ=EvTmr5
87,//смещение длЯ таблицы с адресами
(Uint16)(1000),//знач по умолч.

 // Корневой под-индекс 1805h, ТекстПУ=TPDO6P
16552,//кол-во подиндексов
97, // номер текста
2, //такой длиной

 //[1805h.01h]
32045, //формат
98,//номер текста, ТекстПУ=ITPDO6
88,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[1805h.02h]
31021, //формат
21,//номер текста, ТекстПУ=TransT
89,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[1805h.03h]
64301, //формат
99,//номер текста, ТекстПУ=ITime6
90,//смещение длЯ таблицы с адресами
(Uint16)(100),//знач по умолч.

 //[1805h.04h]
47405, //формат
79,//номер текста, ТекстПУ=CompEn
91,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[1805h.05h]
64301, //формат
100,//номер текста, ТекстПУ=EvTmr6
92,//смещение длЯ таблицы с адресами
(Uint16)(1000),//знач по умолч.

 // Корневой под-индекс 1806h, ТекстПУ=TPDO7P
16554,//кол-во подиндексов
101, // номер текста
2, //такой длиной

 //[1806h.01h]
32033, //формат
102,//номер текста, ТекстПУ=ITPDO7
93,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[1806h.02h]
31009, //формат
21,//номер текста, ТекстПУ=TransT
94,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[1806h.03h]
64289, //формат
103,//номер текста, ТекстПУ=ITime7
95,//смещение длЯ таблицы с адресами
(Uint16)(100),//знач по умолч.

 //[1806h.04h]
47393, //формат
79,//номер текста, ТекстПУ=CompEn
96,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[1806h.05h]
64289, //формат
104,//номер текста, ТекстПУ=EvTmr7
97,//смещение длЯ таблицы с адресами
(Uint16)(1000),//знач по умолч.

 // Корневой под-индекс 1807h, ТекстПУ=TPDO8P
16556,//кол-во подиндексов
105, // номер текста
2, //такой длиной

 //[1807h.01h]
32033, //формат
106,//номер текста, ТекстПУ=ITPDO8
98,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[1807h.02h]
31009, //формат
21,//номер текста, ТекстПУ=TransT
99,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[1807h.03h]
64289, //формат
107,//номер текста, ТекстПУ=ITime8
100,//смещение длЯ таблицы с адресами
(Uint16)(100),//знач по умолч.

 //[1807h.04h]
47393, //формат
79,//номер текста, ТекстПУ=CompEn
101,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[1807h.05h]
64289, //формат
108,//номер текста, ТекстПУ=EvTmr8
102,//смещение длЯ таблицы с адресами
(Uint16)(1000),//знач по умолч.

 // Корневой под-индекс 1A00h, ТекстПУ=TPDO1M
19633,//кол-во подиндексов
109, // номер текста
3, //такой длиной

 //[1A00h.01h]
63789, //формат
110,//номер текста, ТекстПУ=TPM11
103,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A00h.02h]
63789, //формат
111,//номер текста, ТекстПУ=TPM12
104,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A00h.03h]
63789, //формат
112,//номер текста, ТекстПУ=TPM13
105,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A00h.04h]
63789, //формат
113,//номер текста, ТекстПУ=TPM14
106,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 // Корневой под-индекс 1A01h, ТекстПУ=TPDO2M
19633,//кол-во подиндексов
114, // номер текста
3, //такой длиной

 //[1A01h.01h]
63789, //формат
115,//номер текста, ТекстПУ=TPM21
107,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A01h.02h]
63789, //формат
116,//номер текста, ТекстПУ=TPM22
108,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A01h.03h]
63789, //формат
117,//номер текста, ТекстПУ=TPM23
109,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A01h.04h]
63789, //формат
118,//номер текста, ТекстПУ=TPM24
110,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 // Корневой под-индекс 1A02h, ТекстПУ=TPDO3M
19629,//кол-во подиндексов
119, // номер текста
3, //такой длиной

 //[1A02h.01h]
63785, //формат
120,//номер текста, ТекстПУ=TPM31
111,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A02h.02h]
63785, //формат
121,//номер текста, ТекстПУ=TPM32
112,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A02h.03h]
63785, //формат
122,//номер текста, ТекстПУ=TPM33
113,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A02h.04h]
63785, //формат
123,//номер текста, ТекстПУ=TPM34
114,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 // Корневой под-индекс 1A03h, ТекстПУ=TPDO4M
19629,//кол-во подиндексов
124, // номер текста
3, //такой длиной

 //[1A03h.01h]
63785, //формат
125,//номер текста, ТекстПУ=TPM41
115,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A03h.02h]
63785, //формат
126,//номер текста, ТекстПУ=TPM42
116,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A03h.03h]
63785, //формат
127,//номер текста, ТекстПУ=TPM43
117,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A03h.04h]
63785, //формат
128,//номер текста, ТекстПУ=TPM44
118,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 // Корневой под-индекс 1A04h, ТекстПУ=TPDO5M
19625,//кол-во подиндексов
129, // номер текста
3, //такой длиной

 //[1A04h.01h]
63789, //формат
130,//номер текста, ТекстПУ=TPM51
119,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A04h.02h]
63789, //формат
131,//номер текста, ТекстПУ=TPM52
120,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A04h.03h]
63789, //формат
132,//номер текста, ТекстПУ=TPM53
121,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A04h.04h]
63789, //формат
133,//номер текста, ТекстПУ=TPM54
122,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 // Корневой под-индекс 1A05h, ТекстПУ=TPDO6M
19625,//кол-во подиндексов
134, // номер текста
3, //такой длиной

 //[1A05h.01h]
63789, //формат
135,//номер текста, ТекстПУ=TPM61
123,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A05h.02h]
63789, //формат
136,//номер текста, ТекстПУ=TPM62
124,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A05h.03h]
63789, //формат
137,//номер текста, ТекстПУ=TPM63
125,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A05h.04h]
63789, //формат
138,//номер текста, ТекстПУ=TPM64
126,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 // Корневой под-индекс 1A06h, ТекстПУ=TPDO7M
19629,//кол-во подиндексов
139, // номер текста
3, //такой длиной

 //[1A06h.01h]
63777, //формат
140,//номер текста, ТекстПУ=TPM71
127,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A06h.02h]
63777, //формат
141,//номер текста, ТекстПУ=TPM72
128,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A06h.03h]
63777, //формат
142,//номер текста, ТекстПУ=TPM73
129,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A06h.04h]
63777, //формат
143,//номер текста, ТекстПУ=TPM74
130,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 // Корневой под-индекс 1A07h, ТекстПУ=TPDO8M
19629,//кол-во подиндексов
144, // номер текста
3, //такой длиной

 //[1A07h.01h]
63777, //формат
145,//номер текста, ТекстПУ=TPM81
131,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A07h.02h]
63777, //формат
146,//номер текста, ТекстПУ=TPM82
132,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A07h.03h]
63777, //формат
147,//номер текста, ТекстПУ=TPM83
133,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[1A07h.04h]
63777, //формат
148,//номер текста, ТекстПУ=TPM84
134,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[2000h.00h]
64173, //формат
149,//номер текста, ТекстПУ=NodeID
135,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.
1,//минимум
127,//максимум

 //[2001h.00h]
53469, //формат
150,//номер текста, ТекстПУ=CANBR
136,//смещение длЯ таблицы с адресами
(Uint16)(3),//знач по умолч.

 // Корневой под-индекс 2002h, ТекстПУ=IdObj
28845,//кол-во подиндексов
7, // номер текста
1, //такой длиной

 //[2002h.01h]
32041, //формат
8,//номер текста, ТекстПУ=VendID
137,//смещение длЯ таблицы с адресами

 //[2002h.02h]
32041, //формат
9,//номер текста, ТекстПУ=PrCode
138,//смещение длЯ таблицы с адресами

 //[2002h.03h]
32041, //формат
10,//номер текста, ТекстПУ=RevNum
139,//смещение длЯ таблицы с адресами

 //[2002h.04h]
32041, //формат
11,//номер текста, ТекстПУ=SerNum
140,//смещение длЯ таблицы с адресами

 // Корневой под-индекс 2003h, ТекстПУ=SpData
28845,//кол-во подиндексов
151, // номер текста
1, //такой длиной

 //[2003h.01h]
31529, //формат
152,//номер текста, ТекстПУ=SpDat1
141,//смещение длЯ таблицы с адресами

 //[2003h.02h]
31529, //формат
153,//номер текста, ТекстПУ=SpDat2
142,//смещение длЯ таблицы с адресами

 //[2003h.03h]
31529, //формат
154,//номер текста, ТекстПУ=SpDat3
143,//смещение длЯ таблицы с адресами

 //[2003h.04h]
31529, //формат
155,//номер текста, ТекстПУ=SpDat4
144,//смещение длЯ таблицы с адресами

 //[2004h.00h]
64813, //формат
156,//номер текста, ТекстПУ=Secret
145,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[2005h.00h]
20669, //формат
157,//номер текста, ТекстПУ=ProtBt
146,//смещение длЯ таблицы с адресами

 // Корневой под-индекс 2007h, ТекстПУ=OD_md5_hash
28845,//кол-во подиндексов
158, // номер текста
1, //такой длиной

 //[2007h.01h]
32033, //формат
159,//номер текста, ТекстПУ=md5_hash_0
147,//смещение длЯ таблицы с адресами

 //[2007h.02h]
32033, //формат
160,//номер текста, ТекстПУ=md5_hash_1
148,//смещение длЯ таблицы с адресами

 //[2007h.03h]
32033, //формат
161,//номер текста, ТекстПУ=md5_hash_2
149,//смещение длЯ таблицы с адресами

 //[2007h.04h]
32033, //формат
162,//номер текста, ТекстПУ=md5_hash_3
150,//смещение длЯ таблицы с адресами

 // Корневой под-индекс 2010h, ТекстПУ=HBFlag
28849,//кол-во подиндексов
163, // номер текста
1, //такой длиной

 //[2010h.01h]
32013, //формат
164,//номер текста, ТекстПУ=HFlag0
151,//смещение длЯ таблицы с адресами

 //[2010h.02h]
32013, //формат
165,//номер текста, ТекстПУ=HFlag1
152,//смещение длЯ таблицы с адресами

 //[2010h.03h]
32013, //формат
166,//номер текста, ТекстПУ=HFlag2
153,//смещение длЯ таблицы с адресами

 //[2010h.04h]
32013, //формат
167,//номер текста, ТекстПУ=HFlag3
154,//смещение длЯ таблицы с адресами

 // Корневой под-индекс 2011h, ТекстПУ=HBMask
28849,//кол-во подиндексов
168, // номер текста
3, //такой длиной

 //[2011h.01h]
64781, //формат
169,//номер текста, ТекстПУ=HMask0
155,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[2011h.02h]
64781, //формат
170,//номер текста, ТекстПУ=HMask1
156,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[2011h.03h]
64781, //формат
171,//номер текста, ТекстПУ=HMask2
157,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[2011h.04h]
64781, //формат
172,//номер текста, ТекстПУ=HMask3
158,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.

 //[2012h.00h]
53433, //формат
173,//номер текста, ТекстПУ=HBASta
159,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.

 //[2014h.00h]
53437, //формат
174,//номер текста, ТекстПУ=HBARec
160,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.

 //[2015h.00h]
30989, //формат
175,//номер текста, ТекстПУ=State
161,//смещение длЯ таблицы с адресами

 //[2016h.00h]
31489, //формат
176,//номер текста, ТекстПУ=EmErCo
162,//смещение длЯ таблицы с адресами

 //[2017h.00h]
32001, //формат
177,//номер текста, ТекстПУ=DeErSt
163,//смещение длЯ таблицы с адресами

 // Корневой под-индекс 2080h, ТекстПУ=ODComW
28859,//кол-во подиндексов
178, // номер текста
2, //такой длиной

 //[2080h.01h]
47661, //формат
179,//номер текста, ТекстПУ=ODComm
164,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2080h.02h]
47661, //формат
180,//номер текста, ТекстПУ=ODCInd
165,//смещение длЯ таблицы с адресами
(Uint16)(0x2000),//знач по умолч.

 //[2080h.03h]
47661, //формат
181,//номер текста, ТекстПУ=ODCSub
166,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2080h.04h]
31277, //формат
182,//номер текста, ТекстПУ=ODCTxt
167,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2080h.05h]
31277, //формат
183,//номер текста, ТекстПУ=ODCFrm
168,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2080h.06h]
31277, //формат
184,//номер текста, ТекстПУ=ODCMin
169,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2080h.07h]
31277, //формат
185,//номер текста, ТекстПУ=ODCMax
170,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2080h.08h]
31277, //формат
186,//номер текста, ТекстПУ=ODCDef
171,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2080h.09h]
31277, //формат
187,//номер текста, ТекстПУ=ODCMiL
172,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2080h.0Ah]
31277, //формат
188,//номер текста, ТекстПУ=ODCMaL
173,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2080h.0Bh]
31277, //формат
189,//номер текста, ТекстПУ=ODCDeL
174,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2080h.0Ch]
31277, //формат
190,//номер текста, ТекстПУ=ODCAdr
175,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2080h.0Dh]
31277, //формат
190,//номер текста, ТекстПУ=ODCAdr
176,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2080h.0Eh]
31277, //формат
191,//номер текста, ТекстПУ=Группа
177,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2081h.00h]
31277, //формат
192,//номер текста, ТекстПУ=КолИнд
178,//смещение длЯ таблицы с адресами

 // Корневой под-индекс 2082h, ТекстПУ=DefInd
28847,//кол-во подиндексов
193, // номер текста
3, //такой длиной

 //[2082h.01h]
48169, //формат
194,//номер текста, ТекстПУ=DfInd1
179,//смещение длЯ таблицы с адресами
(Uint16)(0x30000000),//знач по умолч.
(((Uint32)0x30000000)>>16),//знач по умолч.

 //[2082h.02h]
48169, //формат
195,//номер текста, ТекстПУ=DfInd2
180,//смещение длЯ таблицы с адресами
(Uint16)(0x30000000),//знач по умолч.
(((Uint32)0x30000000)>>16),//знач по умолч.

 // Корневой под-индекс 2083h, ТекстПУ=ProfML
28857,//кол-во подиндексов
196, // номер текста
2, //такой длиной

 //[2083h.01h]
36905, //формат
197,//номер текста, ТекстПУ=MskEl0
181,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2083h.02h]
36913, //формат
198,//номер текста, ТекстПУ=MskEl1
182,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2083h.03h]
36905, //формат
199,//номер текста, ТекстПУ=MskEl2
183,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2083h.04h]
36913, //формат
200,//номер текста, ТекстПУ=MskEl3
184,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2083h.05h]
36905, //формат
201,//номер текста, ТекстПУ=MskEl4
185,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2083h.06h]
36913, //формат
202,//номер текста, ТекстПУ=MskEl5
186,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2083h.07h]
36905, //формат
203,//номер текста, ТекстПУ=MskEl6
187,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2083h.08h]
36913, //формат
204,//номер текста, ТекстПУ=MskEl7
188,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2083h.09h]
36905, //формат
205,//номер текста, ТекстПУ=MskEl8
189,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2083h.0Ah]
36913, //формат
206,//номер текста, ТекстПУ=MskEl9
190,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2083h.0Bh]
36905, //формат
207,//номер текста, ТекстПУ=MskElA
191,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2083h.0Ch]
36913, //формат
208,//номер текста, ТекстПУ=MskElB
192,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2083h.0Dh]
36905, //формат
209,//номер текста, ТекстПУ=MskElC
193,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2083h.0Eh]
36913, //формат
210,//номер текста, ТекстПУ=MskElD
194,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2083h.0Fh]
36905, //формат
211,//номер текста, ТекстПУ=MskElE
195,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2083h.10h]
36913, //формат
212,//номер текста, ТекстПУ=MskElF
196,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 // Корневой под-индекс 2100h, ТекстПУ=SNR0
28339,//кол-во подиндексов
213, // номер текста
2, //такой длиной

 //[2100h.01h]
31533, //формат
214,//номер текста, ТекстПУ=SN0
197,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2100h.02h]
47917, //формат
215,//номер текста, ТекстПУ=S0Fmt
198,//смещение длЯ таблицы с адресами
(Uint16)(32),//знач по умолч.

 //[2100h.03h]
37117, //формат
216,//номер текста, ТекстПУ=Q-type
199,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2100h.04h]
37074, //формат
217,//номер текста, ТекстПУ=NFloat
200,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.

 //[2100h.05h]
36580, //формат
218,//номер текста, ТекстПУ=Prefix
201,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2100h.06h]
37143, //формат
219,//номер текста, ТекстПУ=Units
202,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 // Корневой под-индекс 2101h, ТекстПУ=SNR1
28339,//кол-во подиндексов
220, // номер текста
2, //такой длиной

 //[2101h.01h]
31533, //формат
221,//номер текста, ТекстПУ=SN1
203,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2101h.02h]
47917, //формат
222,//номер текста, ТекстПУ=S1Fmt
204,//смещение длЯ таблицы с адресами
(Uint16)(4128),//знач по умолч.

 //[2101h.03h]
37117, //формат
216,//номер текста, ТекстПУ=Q-type
205,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2101h.04h]
37072, //формат
217,//номер текста, ТекстПУ=NFloat
206,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.

 //[2101h.05h]
36578, //формат
218,//номер текста, ТекстПУ=Prefix
207,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2101h.06h]
37143, //формат
219,//номер текста, ТекстПУ=Units
208,//смещение длЯ таблицы с адресами
(Uint16)(4),//знач по умолч.

 // Корневой под-индекс 2102h, ТекстПУ=SNR2
28339,//кол-во подиндексов
223, // номер текста
2, //такой длиной

 //[2102h.01h]
31529, //формат
224,//номер текста, ТекстПУ=SN2
209,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2102h.02h]
47913, //формат
225,//номер текста, ТекстПУ=S2Fmt
210,//смещение длЯ таблицы с адресами
(Uint16)(3136),//знач по умолч.

 //[2102h.03h]
37113, //формат
216,//номер текста, ТекстПУ=Q-type
211,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2102h.04h]
37074, //формат
217,//номер текста, ТекстПУ=NFloat
212,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.

 //[2102h.05h]
36576, //формат
218,//номер текста, ТекстПУ=Prefix
213,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2102h.06h]
37143, //формат
219,//номер текста, ТекстПУ=Units
214,//смещение длЯ таблицы с адресами
(Uint16)(3),//знач по умолч.

 // Корневой под-индекс 2103h, ТекстПУ=SNR3
28339,//кол-во подиндексов
226, // номер текста
2, //такой длиной

 //[2103h.01h]
31529, //формат
227,//номер текста, ТекстПУ=SN3
215,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2103h.02h]
47913, //формат
228,//номер текста, ТекстПУ=S3Fmt
216,//смещение длЯ таблицы с адресами
(Uint16)(2112),//знач по умолч.

 //[2103h.03h]
37113, //формат
216,//номер текста, ТекстПУ=Q-type
217,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2103h.04h]
37068, //формат
217,//номер текста, ТекстПУ=NFloat
218,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.

 //[2103h.05h]
36578, //формат
218,//номер текста, ТекстПУ=Prefix
219,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2103h.06h]
37143, //формат
219,//номер текста, ТекстПУ=Units
220,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.

 // Корневой под-индекс 2104h, ТекстПУ=SNR4
28331,//кол-во подиндексов
229, // номер текста
2, //такой длиной

 //[2104h.01h]
31533, //формат
230,//номер текста, ТекстПУ=SN4
221,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2104h.02h]
47917, //формат
231,//номер текста, ТекстПУ=S4Fmt
222,//смещение длЯ таблицы с адресами
(Uint16)(1056),//знач по умолч.

 //[2104h.03h]
37117, //формат
216,//номер текста, ТекстПУ=Q-type
223,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2104h.04h]
37066, //формат
217,//номер текста, ТекстПУ=NFloat
224,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.

 //[2104h.05h]
36580, //формат
218,//номер текста, ТекстПУ=Prefix
225,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2104h.06h]
37143, //формат
219,//номер текста, ТекстПУ=Units
226,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.

 // Корневой под-индекс 2105h, ТекстПУ=SNR5
28331,//кол-во подиндексов
232, // номер текста
2, //такой длиной

 //[2105h.01h]
31533, //формат
233,//номер текста, ТекстПУ=SN5
227,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2105h.02h]
47917, //формат
234,//номер текста, ТекстПУ=S5Fmt
228,//смещение длЯ таблицы с адресами
(Uint16)(1056),//знач по умолч.

 //[2105h.03h]
37117, //формат
216,//номер текста, ТекстПУ=Q-type
229,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2105h.04h]
37064, //формат
217,//номер текста, ТекстПУ=NFloat
230,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.

 //[2105h.05h]
36570, //формат
218,//номер текста, ТекстПУ=Prefix
231,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2105h.06h]
37143, //формат
219,//номер текста, ТекстПУ=Units
232,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.

 // Корневой под-индекс 2106h, ТекстПУ=SNR6
28331,//кол-во подиндексов
235, // номер текста
2, //такой длиной

 //[2106h.01h]
31521, //формат
236,//номер текста, ТекстПУ=SN6
233,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2106h.02h]
47905, //формат
237,//номер текста, ТекстПУ=S6Fmt
234,//смещение длЯ таблицы с адресами
(Uint16)(1088),//знач по умолч.

 //[2106h.03h]
37105, //формат
216,//номер текста, ТекстПУ=Q-type
235,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2106h.04h]
37066, //формат
217,//номер текста, ТекстПУ=NFloat
236,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.

 //[2106h.05h]
36568, //формат
218,//номер текста, ТекстПУ=Prefix
237,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2106h.06h]
37143, //формат
219,//номер текста, ТекстПУ=Units
238,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.

 // Корневой под-индекс 2107h, ТекстПУ=SNR7
28331,//кол-во подиндексов
238, // номер текста
2, //такой длиной

 //[2107h.01h]
31521, //формат
239,//номер текста, ТекстПУ=SN7
239,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2107h.02h]
47905, //формат
240,//номер текста, ТекстПУ=S7Fmt
240,//смещение длЯ таблицы с адресами
(Uint16)(2112),//знач по умолч.

 //[2107h.03h]
37105, //формат
216,//номер текста, ТекстПУ=Q-type
241,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2107h.04h]
37068, //формат
217,//номер текста, ТекстПУ=NFloat
242,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.

 //[2107h.05h]
36570, //формат
218,//номер текста, ТекстПУ=Prefix
243,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2107h.06h]
37143, //формат
219,//номер текста, ТекстПУ=Units
244,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.

 // Корневой под-индекс 2108h, ТекстПУ=SNR8
28323,//кол-во подиндексов
241, // номер текста
2, //такой длиной

 //[2108h.01h]
31533, //формат
242,//номер текста, ТекстПУ=SN8
245,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2108h.02h]
47917, //формат
243,//номер текста, ТекстПУ=S8Fmt
246,//смещение длЯ таблицы с адресами
(Uint16)(9286),//знач по умолч.

 //[2108h.03h]
37117, //формат
216,//номер текста, ТекстПУ=Q-type
247,//смещение длЯ таблицы с адресами
(Uint16)(6),//знач по умолч.

 //[2108h.04h]
37058, //формат
217,//номер текста, ТекстПУ=NFloat
248,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.

 //[2108h.05h]
36564, //формат
218,//номер текста, ТекстПУ=Prefix
249,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2108h.06h]
37127, //формат
219,//номер текста, ТекстПУ=Units
250,//смещение длЯ таблицы с адресами
(Uint16)(9),//знач по умолч.

 // Корневой под-индекс 2109h, ТекстПУ=SNR9
28323,//кол-во подиндексов
244, // номер текста
2, //такой длиной

 //[2109h.01h]
31533, //формат
245,//номер текста, ТекстПУ=SN9
251,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2109h.02h]
47917, //формат
246,//номер текста, ТекстПУ=S9Fmt
252,//смещение длЯ таблицы с адресами
(Uint16)(5184),//знач по умолч.

 //[2109h.03h]
37117, //формат
216,//номер текста, ТекстПУ=Q-type
253,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2109h.04h]
37056, //формат
217,//номер текста, ТекстПУ=NFloat
254,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.

 //[2109h.05h]
36562, //формат
218,//номер текста, ТекстПУ=Prefix
255,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2109h.06h]
37127, //формат
219,//номер текста, ТекстПУ=Units
256,//смещение длЯ таблицы с адресами
(Uint16)(5),//знач по умолч.

 // Корневой под-индекс 210Ah, ТекстПУ=SNRA
28323,//кол-во подиндексов
247, // номер текста
2, //такой длиной

 //[210Ah.01h]
31529, //формат
248,//номер текста, ТекстПУ=SNA
257,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[210Ah.02h]
47913, //формат
249,//номер текста, ТекстПУ=SAFmt
258,//смещение длЯ таблицы с адресами
(Uint16)(12320),//знач по умолч.

 //[210Ah.03h]
37113, //формат
216,//номер текста, ТекстПУ=Q-type
259,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[210Ah.04h]
37058, //формат
217,//номер текста, ТекстПУ=NFloat
260,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.

 //[210Ah.05h]
36560, //формат
218,//номер текста, ТекстПУ=Prefix
261,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[210Ah.06h]
37127, //формат
219,//номер текста, ТекстПУ=Units
262,//смещение длЯ таблицы с адресами
(Uint16)(12),//знач по умолч.

 // Корневой под-индекс 210Bh, ТекстПУ=SNRB
28323,//кол-во подиндексов
250, // номер текста
2, //такой длиной

 //[210Bh.01h]
31529, //формат
251,//номер текста, ТекстПУ=SNB
263,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[210Bh.02h]
47913, //формат
252,//номер текста, ТекстПУ=SBFmt
264,//смещение длЯ таблицы с адресами
(Uint16)(6144),//знач по умолч.

 //[210Bh.03h]
37113, //формат
216,//номер текста, ТекстПУ=Q-type
265,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[210Bh.04h]
37068, //формат
217,//номер текста, ТекстПУ=NFloat
266,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[210Bh.05h]
36562, //формат
218,//номер текста, ТекстПУ=Prefix
267,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[210Bh.06h]
37127, //формат
219,//номер текста, ТекстПУ=Units
268,//смещение длЯ таблицы с адресами
(Uint16)(6),//знач по умолч.

 // Корневой под-индекс 210Ch, ТекстПУ=SNRC
28331,//кол-во подиндексов
253, // номер текста
2, //такой длиной

 //[210Ch.01h]
31533, //формат
254,//номер текста, ТекстПУ=SNC
269,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[210Ch.02h]
47917, //формат
255,//номер текста, ТекстПУ=SCFmt
270,//смещение длЯ таблицы с адресами
(Uint16)(7168),//знач по умолч.

 //[210Ch.03h]
37117, //формат
216,//номер текста, ТекстПУ=Q-type
271,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[210Ch.04h]
37066, //формат
217,//номер текста, ТекстПУ=NFloat
272,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[210Ch.05h]
36564, //формат
218,//номер текста, ТекстПУ=Prefix
273,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[210Ch.06h]
37127, //формат
219,//номер текста, ТекстПУ=Units
274,//смещение длЯ таблицы с адресами
(Uint16)(7),//знач по умолч.

 // Корневой под-индекс 210Dh, ТекстПУ=SNRD
28331,//кол-во подиндексов
256, // номер текста
2, //такой длиной

 //[210Dh.01h]
31533, //формат
257,//номер текста, ТекстПУ=SND
275,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[210Dh.02h]
47917, //формат
258,//номер текста, ТекстПУ=SDFmt
276,//смещение длЯ таблицы с адресами
(Uint16)(2048),//знач по умолч.

 //[210Dh.03h]
37117, //формат
216,//номер текста, ТекстПУ=Q-type
277,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[210Dh.04h]
37064, //формат
217,//номер текста, ТекстПУ=NFloat
278,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[210Dh.05h]
36570, //формат
218,//номер текста, ТекстПУ=Prefix
279,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[210Dh.06h]
37127, //формат
219,//номер текста, ТекстПУ=Units
280,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.

 // Корневой под-индекс 210Eh, ТекстПУ=SNRE
28331,//кол-во подиндексов
259, // номер текста
2, //такой длиной

 //[210Eh.01h]
31505, //формат
260,//номер текста, ТекстПУ=SNE
281,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[210Eh.02h]
47889, //формат
261,//номер текста, ТекстПУ=SEFmt
282,//смещение длЯ таблицы с адресами
(Uint16)(1024),//знач по умолч.

 //[210Eh.03h]
37089, //формат
216,//номер текста, ТекстПУ=Q-type
283,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[210Eh.04h]
37066, //формат
217,//номер текста, ТекстПУ=NFloat
284,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[210Eh.05h]
36568, //формат
218,//номер текста, ТекстПУ=Prefix
285,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[210Eh.06h]
37127, //формат
219,//номер текста, ТекстПУ=Units
286,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.

 // Корневой под-индекс 210Fh, ТекстПУ=SNRF
28331,//кол-во подиндексов
262, // номер текста
2, //такой длиной

 //[210Fh.01h]
31505, //формат
263,//номер текста, ТекстПУ=SNF
287,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[210Fh.02h]
47889, //формат
264,//номер текста, ТекстПУ=SFFmt
288,//смещение длЯ таблицы с адресами
(Uint16)(96),//знач по умолч.

 //[210Fh.03h]
37089, //формат
216,//номер текста, ТекстПУ=Q-type
289,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[210Fh.04h]
37068, //формат
217,//номер текста, ТекстПУ=NFloat
290,//смещение длЯ таблицы с адресами
(Uint16)(3),//знач по умолч.

 //[210Fh.05h]
36570, //формат
218,//номер текста, ТекстПУ=Prefix
291,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[210Fh.06h]
37127, //формат
219,//номер текста, ТекстПУ=Units
292,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 // Корневой под-индекс 2110h, ТекстПУ=SNR10
28339,//кол-во подиндексов
265, // номер текста
2, //такой длиной

 //[2110h.01h]
31501, //формат
266,//номер текста, ТекстПУ=SN10
293,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2110h.02h]
47885, //формат
267,//номер текста, ТекстПУ=S10Fmt
294,//смещение длЯ таблицы с адресами
(Uint16)(3136),//знач по умолч.

 //[2110h.03h]
37117, //формат
216,//номер текста, ТекстПУ=Q-type
295,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2110h.04h]
37074, //формат
217,//номер текста, ТекстПУ=NFloat
296,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.

 //[2110h.05h]
36548, //формат
218,//номер текста, ТекстПУ=Prefix
297,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2110h.06h]
37143, //формат
219,//номер текста, ТекстПУ=Units
298,//смещение длЯ таблицы с адресами
(Uint16)(3),//знач по умолч.

 // Корневой под-индекс 2111h, ТекстПУ=SNR11
28339,//кол-во подиндексов
268, // номер текста
2, //такой длиной

 //[2111h.01h]
31501, //формат
269,//номер текста, ТекстПУ=SN11
299,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2111h.02h]
47885, //формат
270,//номер текста, ТекстПУ=S11Fmt
300,//смещение длЯ таблицы с адресами
(Uint16)(9408),//знач по умолч.

 //[2111h.03h]
37117, //формат
216,//номер текста, ТекстПУ=Q-type
301,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2111h.04h]
37072, //формат
217,//номер текста, ТекстПУ=NFloat
302,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.

 //[2111h.05h]
36546, //формат
218,//номер текста, ТекстПУ=Prefix
303,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.

 //[2111h.06h]
37143, //формат
219,//номер текста, ТекстПУ=Units
304,//смещение длЯ таблицы с адресами
(Uint16)(9),//знач по умолч.

 // Корневой под-индекс 2112h, ТекстПУ=SNR12
28339,//кол-во подиндексов
271, // номер текста
2, //такой длиной

 //[2112h.01h]
31497, //формат
272,//номер текста, ТекстПУ=SN12
305,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2112h.02h]
47881, //формат
273,//номер текста, ТекстПУ=S12Fmt
306,//смещение длЯ таблицы с адресами
(Uint16)(9414),//знач по умолч.

 //[2112h.03h]
37113, //формат
216,//номер текста, ТекстПУ=Q-type
307,//смещение длЯ таблицы с адресами
(Uint16)(6),//знач по умолч.

 //[2112h.04h]
37074, //формат
217,//номер текста, ТекстПУ=NFloat
308,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.

 //[2112h.05h]
36544, //формат
218,//номер текста, ТекстПУ=Prefix
309,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.

 //[2112h.06h]
37143, //формат
219,//номер текста, ТекстПУ=Units
310,//смещение длЯ таблицы с адресами
(Uint16)(9),//знач по умолч.

 // Корневой под-индекс 2113h, ТекстПУ=SNR13
28339,//кол-во подиндексов
274, // номер текста
2, //такой длиной

 //[2113h.01h]
31497, //формат
275,//номер текста, ТекстПУ=SN13
311,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2113h.02h]
47881, //формат
276,//номер текста, ТекстПУ=S13Fmt
312,//смещение длЯ таблицы с адресами
(Uint16)(12352),//знач по умолч.

 //[2113h.03h]
37113, //формат
216,//номер текста, ТекстПУ=Q-type
313,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2113h.04h]
37036, //формат
217,//номер текста, ТекстПУ=NFloat
314,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.

 //[2113h.05h]
36546, //формат
218,//номер текста, ТекстПУ=Prefix
315,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2113h.06h]
37143, //формат
219,//номер текста, ТекстПУ=Units
316,//смещение длЯ таблицы с адресами
(Uint16)(12),//знач по умолч.

 // Корневой под-индекс 2114h, ТекстПУ=SNR14
28299,//кол-во подиндексов
277, // номер текста
2, //такой длиной

 //[2114h.01h]
31501, //формат
278,//номер текста, ТекстПУ=SN14
317,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2114h.02h]
47885, //формат
279,//номер текста, ТекстПУ=S14Fmt
318,//смещение длЯ таблицы с адресами
(Uint16)(13344),//знач по умолч.

 //[2114h.03h]
37117, //формат
216,//номер текста, ТекстПУ=Q-type
319,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2114h.04h]
37034, //формат
217,//номер текста, ТекстПУ=NFloat
320,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.

 //[2114h.05h]
36548, //формат
218,//номер текста, ТекстПУ=Prefix
321,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2114h.06h]
37143, //формат
219,//номер текста, ТекстПУ=Units
322,//смещение длЯ таблицы с адресами
(Uint16)(13),//знач по умолч.

 // Корневой под-индекс 2115h, ТекстПУ=SNR15
28299,//кол-во подиндексов
280, // номер текста
2, //такой длиной

 //[2115h.01h]
31501, //формат
281,//номер текста, ТекстПУ=SN15
323,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2115h.02h]
47885, //формат
282,//номер текста, ТекстПУ=S15Fmt
324,//смещение длЯ таблицы с адресами
(Uint16)(10240),//знач по умолч.

 //[2115h.03h]
37117, //формат
216,//номер текста, ТекстПУ=Q-type
325,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2115h.04h]
37032, //формат
217,//номер текста, ТекстПУ=NFloat
326,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2115h.05h]
36570, //формат
218,//номер текста, ТекстПУ=Prefix
327,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2115h.06h]
37143, //формат
219,//номер текста, ТекстПУ=Units
328,//смещение длЯ таблицы с адресами
(Uint16)(10),//знач по умолч.

 // Корневой под-индекс 2116h, ТекстПУ=SNR16
28299,//кол-во подиндексов
283, // номер текста
2, //такой длиной

 //[2116h.01h]
31489, //формат
284,//номер текста, ТекстПУ=SN16
329,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2116h.02h]
47873, //формат
285,//номер текста, ТекстПУ=S16Fmt
330,//смещение длЯ таблицы с адресами
(Uint16)(14400),//знач по умолч.

 //[2116h.03h]
37105, //формат
216,//номер текста, ТекстПУ=Q-type
331,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2116h.04h]
37034, //формат
217,//номер текста, ТекстПУ=NFloat
332,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.

 //[2116h.05h]
36568, //формат
218,//номер текста, ТекстПУ=Prefix
333,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2116h.06h]
37143, //формат
219,//номер текста, ТекстПУ=Units
334,//смещение длЯ таблицы с адресами
(Uint16)(14),//знач по умолч.

 // Корневой под-индекс 2117h, ТекстПУ=SNR17
28299,//кол-во подиндексов
286, // номер текста
2, //такой длиной

 //[2117h.01h]
31489, //формат
287,//номер текста, ТекстПУ=SN17
335,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2117h.02h]
47873, //формат
288,//номер текста, ТекстПУ=S17Fmt
336,//смещение длЯ таблицы с адресами
(Uint16)(15430),//знач по умолч.

 //[2117h.03h]
37105, //формат
216,//номер текста, ТекстПУ=Q-type
337,//смещение длЯ таблицы с адресами
(Uint16)(6),//знач по умолч.

 //[2117h.04h]
37036, //формат
217,//номер текста, ТекстПУ=NFloat
338,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.

 //[2117h.05h]
36570, //формат
218,//номер текста, ТекстПУ=Prefix
339,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2117h.06h]
37143, //формат
219,//номер текста, ТекстПУ=Units
340,//смещение длЯ таблицы с адресами
(Uint16)(15),//знач по умолч.

 // Корневой под-индекс 2118h, ТекстПУ=SNR18
28291,//кол-во подиндексов
289, // номер текста
2, //такой длиной

 //[2118h.01h]
31501, //формат
290,//номер текста, ТекстПУ=SN18
341,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2118h.02h]
47885, //формат
291,//номер текста, ТекстПУ=S18Fmt
342,//смещение длЯ таблицы с адресами
(Uint16)(16448),//знач по умолч.

 //[2118h.03h]
37117, //формат
216,//номер текста, ТекстПУ=Q-type
343,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2118h.04h]
37026, //формат
217,//номер текста, ТекстПУ=NFloat
344,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.

 //[2118h.05h]
36564, //формат
218,//номер текста, ТекстПУ=Prefix
345,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2118h.06h]
36583, //формат
219,//номер текста, ТекстПУ=Units
346,//смещение длЯ таблицы с адресами
(Uint16)(16),//знач по умолч.

 // Корневой под-индекс 2119h, ТекстПУ=SNR19
28291,//кол-во подиндексов
292, // номер текста
2, //такой длиной

 //[2119h.01h]
31501, //формат
293,//номер текста, ТекстПУ=SN19
347,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[2119h.02h]
47885, //формат
294,//номер текста, ТекстПУ=S19Fmt
348,//смещение длЯ таблицы с адресами
(Uint16)(13312),//знач по умолч.

 //[2119h.03h]
37117, //формат
216,//номер текста, ТекстПУ=Q-type
349,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2119h.04h]
37024, //формат
217,//номер текста, ТекстПУ=NFloat
350,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2119h.05h]
36562, //формат
218,//номер текста, ТекстПУ=Prefix
351,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[2119h.06h]
36583, //формат
219,//номер текста, ТекстПУ=Units
352,//смещение длЯ таблицы с адресами
(Uint16)(13),//знач по умолч.

 // Корневой под-индекс 211Ah, ТекстПУ=SNR1A
28291,//кол-во подиндексов
295, // номер текста
2, //такой длиной

 //[211Ah.01h]
31497, //формат
296,//номер текста, ТекстПУ=SN1A
353,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[211Ah.02h]
47881, //формат
297,//номер текста, ТекстПУ=S1AFmt
354,//смещение длЯ таблицы с адресами
(Uint16)(14336),//знач по умолч.

 //[211Ah.03h]
37113, //формат
216,//номер текста, ТекстПУ=Q-type
355,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[211Ah.04h]
37026, //формат
217,//номер текста, ТекстПУ=NFloat
356,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[211Ah.05h]
36560, //формат
218,//номер текста, ТекстПУ=Prefix
357,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[211Ah.06h]
36583, //формат
219,//номер текста, ТекстПУ=Units
358,//смещение длЯ таблицы с адресами
(Uint16)(14),//знач по умолч.

 // Корневой под-индекс 211Bh, ТекстПУ=SNR1B
28291,//кол-во подиндексов
298, // номер текста
2, //такой длиной

 //[211Bh.01h]
31497, //формат
299,//номер текста, ТекстПУ=SN1B
359,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[211Bh.02h]
47881, //формат
300,//номер текста, ТекстПУ=S1BFmt
360,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[211Bh.03h]
37113, //формат
216,//номер текста, ТекстПУ=Q-type
361,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[211Bh.04h]
37036, //формат
217,//номер текста, ТекстПУ=NFloat
362,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[211Bh.05h]
36562, //формат
218,//номер текста, ТекстПУ=Prefix
363,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[211Bh.06h]
36583, //формат
219,//номер текста, ТекстПУ=Units
364,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 // Корневой под-индекс 211Ch, ТекстПУ=SNR1C
28299,//кол-во подиндексов
301, // номер текста
2, //такой длиной

 //[211Ch.01h]
31501, //формат
302,//номер текста, ТекстПУ=SN1C
365,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[211Ch.02h]
47885, //формат
303,//номер текста, ТекстПУ=S1CFmt
366,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[211Ch.03h]
37117, //формат
216,//номер текста, ТекстПУ=Q-type
367,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[211Ch.04h]
37034, //формат
217,//номер текста, ТекстПУ=NFloat
368,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[211Ch.05h]
36564, //формат
218,//номер текста, ТекстПУ=Prefix
369,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[211Ch.06h]
36583, //формат
219,//номер текста, ТекстПУ=Units
370,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 // Корневой под-индекс 211Dh, ТекстПУ=SNR1D
28299,//кол-во подиндексов
304, // номер текста
2, //такой длиной

 //[211Dh.01h]
31501, //формат
305,//номер текста, ТекстПУ=SN1D
371,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[211Dh.02h]
47885, //формат
306,//номер текста, ТекстПУ=S1DFmt
372,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[211Dh.03h]
37117, //формат
216,//номер текста, ТекстПУ=Q-type
373,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[211Dh.04h]
37032, //формат
217,//номер текста, ТекстПУ=NFloat
374,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[211Dh.05h]
36570, //формат
218,//номер текста, ТекстПУ=Prefix
375,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 //[211Dh.06h]
36583, //формат
219,//номер текста, ТекстПУ=Units
376,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.

 // Корневой под-индекс 2502h, ТекстПУ=КмдCAN
28333,//кол-во подиндексов
307, // номер текста
4, //такой длиной

 //[2502h.01h]
36537, //формат
308,//номер текста, ТекстПУ=Пуск
377,//смещение длЯ таблицы с адресами
(Uint16)(256),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[2502h.02h]
36542, //формат
309,//номер текста, ТекстПУ=Стоп
378,//смещение длЯ таблицы с адресами
(Uint16)(256),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[2502h.03h]
36543, //формат
310,//номер текста, ТекстПУ=СбрАвр
379,//смещение длЯ таблицы с адресами
(Uint16)(256),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[2502h.04h]
45225, //формат
307,//номер текста, ТекстПУ=КмдCAN
380,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
0,//минимум
1,//максимум

 //[250Fh.00h]
48401, //формат
311,//номер текста, ТекстПУ=RTC
381,//смещение длЯ таблицы с адресами
(Uint16)(438392299),//знач по умолч.
(((Uint32)438392299)>>16),//знач по умолч.

 // Корневой под-индекс 2532h, ТекстПУ=System
28236,//кол-во подиндексов
312, // номер текста
1, //такой длиной

 //[2532h.01h]
20169, //формат
313,//номер текста, ТекстПУ=Код аварии ARM
382,//смещение длЯ таблицы с адресами

 //[2532h.02h]
18737, //формат
314,//номер текста, ТекстПУ=Авр+
383,//смещение длЯ таблицы с адресами

 //[2532h.03h]
20153, //формат
312,//номер текста, ТекстПУ=System
384,//смещение длЯ таблицы с адресами

 // Корневой под-индекс 2534h, ТекстПУ=Конфиг. ПУ
28238,//кол-во подиндексов
315, // номер текста
2, //такой длиной

 //[2534h.01h]
45261, //формат
316,//номер текста, ТекстПУ=№ гр ПУ
385,//смещение длЯ таблицы с адресами
(Uint16)(15),//знач по умолч.

 // Корневой под-индекс 2600h, ТекстПУ=VendorToken
27822,//кол-во подиндексов
577, // номер текста
3, //такой длиной

 //[2600h.01h]
45357, //формат
577,//номер текста, ТекстПУ=VendorToken
386,//смещение длЯ таблицы с адресами
(Uint16)(0x11111111),//знач по умолч.
(((Uint32)0x11111111)>>16),//знач по умолч.

 //[3000h.00h]
20749, //формат
317,//номер текста, ТекстПУ=Статус
387,//смещение длЯ таблицы с адресами

 // Корневой под-индекс 3100h, ТекстПУ=System
3762,//кол-во подиндексов
312, // номер текста
2, //такой длиной

 //[3100h.01h]
47917, //формат
318,//номер текста, ТекстПУ=Сч.прерыв.1 кГц
388,//смещение длЯ таблицы с адресами
(Uint16)(58002),//знач по умолч.

 //[3100h.02h]
47917, //формат
319,//номер текста, ТекстПУ=Сч.прерыв.10 кГц
389,//смещение длЯ таблицы с адресами
(Uint16)(14349),//знач по умолч.

 //[3100h.03h]
47917, //формат
320,//номер текста, ТекстПУ=BackgroundCounter
390,//смещение длЯ таблицы с адресами
(Uint16)(64364),//знач по умолч.

 //[3100h.04h]
47917, //формат
321,//номер текста, ТекстПУ=T_10кГц
391,//смещение длЯ таблицы с адресами
(Uint16)(3745),//знач по умолч.

 //[3100h.05h]
47917, //формат
322,//номер текста, ТекстПУ=T_1кГц
392,//смещение длЯ таблицы с адресами
(Uint16)(1832),//знач по умолч.

 // Корневой под-индекс 5000h, ТекстПУ=ЗИ
28848,//кол-во подиндексов
323, // номер текста
7, //такой длиной

 //[5000h.01h]
58853, //формат
324,//номер текста, ТекстПУ=rmpT
393,//смещение длЯ таблицы с адресами
(Uint16)(16777216),//знач по умолч.
(((Uint32)16777216)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(_IQ(120.0)),//максимум
(((Uint32)_IQ(120.0))>>16),//максимум

 //[5000h.02h]
26437, //формат
325,//номер текста, ТекстПУ=ВходЗИ
394,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5000h.03h]
26437, //формат
326,//номер текста, ТекстПУ=ВыходЗИ
395,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5100h, ТекстПУ=Рег_id
20154,//кол-во подиндексов
327, // номер текста
7, //такой длиной

 //[5100h.01h]
25925, //формат
328,//номер текста, ТекстПУ=Зад-е
396,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5100h.02h]
25925, //формат
329,//номер текста, ТекстПУ=Обр.Св
397,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5100h.03h]
9381, //формат
330,//номер текста, ТекстПУ=Kp
398,//смещение длЯ таблицы с адресами
(Uint16)(335544),//знач по умолч.
(((Uint32)335544)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5100h.04h]
9509, //формат
331,//номер текста, ТекстПУ=Max
399,//смещение длЯ таблицы с адресами
(Uint16)(16777216),//знач по умолч.
(((Uint32)16777216)>>16),//знач по умолч.
(Uint16)(_IQ(-1.0)),//минимум
(((Uint32)_IQ(-1.0))>>16),//минимум
(Uint16)(_IQ(1.0)),//максимум
(((Uint32)_IQ(1.0))>>16),//максимум

 //[5100h.05h]
9509, //формат
332,//номер текста, ТекстПУ=Min
400,//смещение длЯ таблицы с адресами
(Uint16)(-16777216),//знач по умолч.
(((Uint32)-16777216)>>16),//знач по умолч.
(Uint16)(_IQ(-1.0)),//минимум
(((Uint32)_IQ(-1.0))>>16),//минимум
(Uint16)(_IQ(1.0)),//максимум
(((Uint32)_IQ(1.0))>>16),//максимум

 //[5100h.06h]
9381, //формат
333,//номер текста, ТекстПУ=Ki
401,//смещение длЯ таблицы с адресами
(Uint16)(2684354),//знач по умолч.
(((Uint32)2684354)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5100h.07h]
9381, //формат
334,//номер текста, ТекстПУ=Kd
402,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5100h.08h]
9381, //формат
335,//номер текста, ТекстПУ=Kc
403,//смещение длЯ таблицы с адресами
(Uint16)(8388608),//знач по умолч.
(((Uint32)8388608)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5100h.09h]
25925, //формат
336,//номер текста, ТекстПУ=Ошибка
404,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5100h.0Ah]
25893, //формат
337,//номер текста, ТекстПУ=Выход
405,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5100h.0Bh]
25893, //формат
338,//номер текста, ТекстПУ=Вых.Пропорц.
406,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5100h.0Ch]
25893, //формат
339,//номер текста, ТекстПУ=Вых.Интегр.
407,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5100h.0Dh]
25893, //формат
340,//номер текста, ТекстПУ=Вых.Диффер.
408,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5101h, ТекстПУ=РегСкр
20155,//кол-во подиндексов
341, // номер текста
7, //такой длиной

 //[5101h.01h]
26437, //формат
342,//номер текста, ТекстПУ=pisref
409,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5101h.02h]
26437, //формат
343,//номер текста, ТекстПУ=pisfbd
410,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5101h.03h]
9381, //формат
330,//номер текста, ТекстПУ=Kp
411,//смещение длЯ таблицы с адресами
(Uint16)(3355443),//знач по умолч.
(((Uint32)3355443)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5101h.04h]
9541, //формат
331,//номер текста, ТекстПУ=Max
412,//смещение длЯ таблицы с адресами
(Uint16)(13421772),//знач по умолч.
(((Uint32)13421772)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5101h.05h]
9541, //формат
332,//номер текста, ТекстПУ=Min
413,//смещение длЯ таблицы с адресами
(Uint16)(-13421772),//знач по умолч.
(((Uint32)-13421772)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5101h.06h]
9381, //формат
333,//номер текста, ТекстПУ=Ki
414,//смещение длЯ таблицы с адресами
(Uint16)(16777),//знач по умолч.
(((Uint32)16777)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5101h.07h]
9381, //формат
344,//номер текста, ТекстПУ=pisKd
415,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5101h.08h]
9381, //формат
335,//номер текста, ТекстПУ=Kc
416,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5101h.09h]
26437, //формат
336,//номер текста, ТекстПУ=Ошибка
417,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5101h.0Ah]
25925, //формат
345,//номер текста, ТекстПУ=pisout
418,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5101h.0Bh]
25925, //формат
338,//номер текста, ТекстПУ=Вых.Пропорц.
419,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5101h.0Ch]
25925, //формат
339,//номер текста, ТекстПУ=Вых.Интегр.
420,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5101h.0Dh]
25925, //формат
340,//номер текста, ТекстПУ=Вых.Диффер.
421,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5101h.0Eh]
25925, //формат
346,//номер текста, ТекстПУ=Прев. насыщ
422,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5102h, ТекстПУ=Рег_iq
20154,//кол-во подиндексов
347, // номер текста
7, //такой длиной

 //[5102h.01h]
25921, //формат
348,//номер текста, ТекстПУ=Зад-ие
423,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5102h.02h]
25921, //формат
349,//номер текста, ТекстПУ=Обр.Св
424,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5102h.03h]
9377, //формат
350,//номер текста, ТекстПУ=Kp
425,//смещение длЯ таблицы с адресами
(Uint16)(335544),//знач по умолч.
(((Uint32)335544)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5102h.04h]
9505, //формат
351,//номер текста, ТекстПУ=Max
426,//смещение длЯ таблицы с адресами
(Uint16)(16777216),//знач по умолч.
(((Uint32)16777216)>>16),//знач по умолч.
(Uint16)(_IQ(-1.0)),//минимум
(((Uint32)_IQ(-1.0))>>16),//минимум
(Uint16)(_IQ(1.0)),//максимум
(((Uint32)_IQ(1.0))>>16),//максимум

 //[5102h.05h]
9505, //формат
352,//номер текста, ТекстПУ=Min
427,//смещение длЯ таблицы с адресами
(Uint16)(-16777216),//знач по умолч.
(((Uint32)-16777216)>>16),//знач по умолч.
(Uint16)(_IQ(-1.0)),//минимум
(((Uint32)_IQ(-1.0))>>16),//минимум
(Uint16)(_IQ(1.0)),//максимум
(((Uint32)_IQ(1.0))>>16),//максимум

 //[5102h.06h]
9377, //формат
353,//номер текста, ТекстПУ=Ki
428,//смещение длЯ таблицы с адресами
(Uint16)(2684354),//знач по умолч.
(((Uint32)2684354)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5102h.07h]
9377, //формат
354,//номер текста, ТекстПУ=Kd
429,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5102h.08h]
9377, //формат
335,//номер текста, ТекстПУ=Kc
430,//смещение длЯ таблицы с адресами
(Uint16)(8388608),//знач по умолч.
(((Uint32)8388608)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5102h.09h]
25921, //формат
336,//номер текста, ТекстПУ=Ошибка
431,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5102h.0Ah]
25889, //формат
355,//номер текста, ТекстПУ=Выход
432,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5102h.0Bh]
25889, //формат
338,//номер текста, ТекстПУ=Вых.Пропорц.
433,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5102h.0Ch]
25889, //формат
339,//номер текста, ТекстПУ=Вых.Интегр.
434,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5102h.0Dh]
25889, //формат
340,//номер текста, ТекстПУ=Вых.Диффер.
435,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5103h, ТекстПУ=pos_reg
20157,//кол-во подиндексов
356, // номер текста
7, //такой длиной

 //[5103h.01h]
15401, //формат
342,//номер текста, ТекстПУ=pisref
436,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5103h.02h]
10017, //формат
342,//номер текста, ТекстПУ=pisref
437,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5103h.03h]
26401, //формат
343,//номер текста, ТекстПУ=pisfbd
438,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5103h.04h]
31785, //формат
343,//номер текста, ТекстПУ=pisfbd
439,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5103h.05h]
9377, //формат
330,//номер текста, ТекстПУ=Kp
440,//смещение длЯ таблицы с адресами
(Uint16)(1174405120),//знач по умолч.
(((Uint32)1174405120)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5103h.06h]
10049, //формат
331,//номер текста, ТекстПУ=Max
441,//смещение длЯ таблицы с адресами
(Uint16)(100663296),//знач по умолч.
(((Uint32)100663296)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5103h.07h]
10049, //формат
332,//номер текста, ТекстПУ=Min
442,//смещение длЯ таблицы с адресами
(Uint16)(-100663296),//знач по умолч.
(((Uint32)-100663296)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5103h.08h]
9377, //формат
333,//номер текста, ТекстПУ=Ki
443,//смещение длЯ таблицы с адресами
(Uint16)(83886),//знач по умолч.
(((Uint32)83886)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5103h.09h]
9377, //формат
344,//номер текста, ТекстПУ=pisKd
444,//смещение длЯ таблицы с адресами
(Uint16)(167772160),//знач по умолч.
(((Uint32)167772160)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5103h.0Ah]
15401, //формат
357,//номер текста, ТекстПУ=Делит. дифф. части
445,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5103h.0Bh]
9377, //формат
358,//номер текста, ТекстПУ=Коэф. фильтра
446,//смещение длЯ таблицы с адресами
(Uint16)(8388608),//знач по умолч.
(((Uint32)8388608)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5103h.0Ch]
9377, //формат
335,//номер текста, ТекстПУ=Kc
447,//смещение длЯ таблицы с адресами
(Uint16)(8388608),//знач по умолч.
(((Uint32)8388608)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5103h.0Dh]
10017, //формат
359,//номер текста, ТекстПУ=Мертвая зона
448,//смещение длЯ таблицы с адресами
(Uint16)(13981),//знач по умолч.
(((Uint32)13981)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5103h.0Eh]
26401, //формат
336,//номер текста, ТекстПУ=Ошибка
449,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5103h.0Fh]
25761, //формат
346,//номер текста, ТекстПУ=Прев. насыщ
450,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5103h.10h]
26433, //формат
345,//номер текста, ТекстПУ=pisout
451,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5103h.11h]
26433, //формат
338,//номер текста, ТекстПУ=Вых.Пропорц.
452,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5103h.12h]
26433, //формат
339,//номер текста, ТекстПУ=Вых.Интегр.
453,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5103h.13h]
26433, //формат
340,//номер текста, ТекстПУ=Вых.Диффер.
454,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5103h.14h]
26401, //формат
360,//номер текста, ТекстПУ=Фильтр ошибки
455,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5104h, ТекстПУ=ПИД рег. Ia
20146,//кол-во подиндексов
361, // номер текста
7, //такой длиной

 //[5104h.01h]
25925, //формат
348,//номер текста, ТекстПУ=Зад-ие
456,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5104h.02h]
25925, //формат
349,//номер текста, ТекстПУ=Обр.Св
457,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5104h.03h]
9381, //формат
350,//номер текста, ТекстПУ=Kp
458,//смещение длЯ таблицы с адресами
(Uint16)(335544),//знач по умолч.
(((Uint32)335544)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5104h.04h]
9509, //формат
351,//номер текста, ТекстПУ=Max
459,//смещение длЯ таблицы с адресами
(Uint16)(16777216),//знач по умолч.
(((Uint32)16777216)>>16),//знач по умолч.
(Uint16)(_IQ(-1.0)),//минимум
(((Uint32)_IQ(-1.0))>>16),//минимум
(Uint16)(_IQ(1.0)),//максимум
(((Uint32)_IQ(1.0))>>16),//максимум

 //[5104h.05h]
9509, //формат
352,//номер текста, ТекстПУ=Min
460,//смещение длЯ таблицы с адресами
(Uint16)(-16777216),//знач по умолч.
(((Uint32)-16777216)>>16),//знач по умолч.
(Uint16)(_IQ(-1.0)),//минимум
(((Uint32)_IQ(-1.0))>>16),//минимум
(Uint16)(_IQ(1.0)),//максимум
(((Uint32)_IQ(1.0))>>16),//максимум

 //[5104h.06h]
9381, //формат
353,//номер текста, ТекстПУ=Ki
461,//смещение длЯ таблицы с адресами
(Uint16)(2684354),//знач по умолч.
(((Uint32)2684354)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5104h.07h]
9381, //формат
354,//номер текста, ТекстПУ=Kd
462,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5104h.08h]
9381, //формат
335,//номер текста, ТекстПУ=Kc
463,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5104h.09h]
25925, //формат
336,//номер текста, ТекстПУ=Ошибка
464,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5104h.0Ah]
25893, //формат
355,//номер текста, ТекстПУ=Выход
465,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5104h.0Bh]
25893, //формат
338,//номер текста, ТекстПУ=Вых.Пропорц.
466,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5104h.0Ch]
25893, //формат
339,//номер текста, ТекстПУ=Вых.Интегр.
467,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5104h.0Dh]
25893, //формат
340,//номер текста, ТекстПУ=Вых.Диффер.
468,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5105h, ТекстПУ=ШИМ
20156,//кол-во подиндексов
362, // номер текста
7, //такой длиной

 //[5105h.01h]
25893, //формат
363,//номер текста, ТекстПУ=UaЗад
469,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5105h.02h]
25893, //формат
364,//номер текста, ТекстПУ=UbЗад
470,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5105h.03h]
8901, //формат
365,//номер текста, ТекстПУ=U_огр
471,//смещение длЯ таблицы с адресами
(Uint16)(14529069),//знач по умолч.
(((Uint32)14529069)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0x01000000),//максимум
(((Uint32)0x01000000)>>16),//максимум

 //[5105h.04h]
31533, //формат
366,//номер текста, ТекстПУ=GmA
472,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5105h.05h]
31533, //формат
367,//номер текста, ТекстПУ=GmB
473,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5105h.06h]
31533, //формат
368,//номер текста, ТекстПУ=GmC
474,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5105h.07h]
31533, //формат
369,//номер текста, ТекстПУ=TPWM
475,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5105h.08h]
11429, //формат
370,//номер текста, ТекстПУ=Мерт. время
476,//смещение длЯ таблицы с адресами
(Uint16)(3355443),//знач по умолч.
(((Uint32)3355443)>>16),//знач по умолч.
(Uint16)(_IQ(2.0)),//минимум
(((Uint32)_IQ(2.0))>>16),//минимум
(Uint16)(_IQ(10.0)),//максимум
(((Uint32)_IQ(10.0))>>16),//максимум

 //[5105h.09h]
11429, //формат
371,//номер текста, ТекстПУ=GamLim
477,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(_IQ(10)),//максимум
(((Uint32)_IQ(10))>>16),//максимум

 //[5105h.0Ah]
8901, //формат
372,//номер текста, ТекстПУ=UdКомпK
478,//смещение длЯ таблицы с адресами
(Uint16)(16777216),//знач по умолч.
(((Uint32)16777216)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5105h.0Bh]
11965, //формат
373,//номер текста, ТекстПУ=UdКомп
479,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5105h.0Ch]
15149, //формат
374,//номер текста, ТекстПУ=ТипШИМ
480,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5105h.0Dh]
31661, //формат
375,//номер текста, ТекстПУ=ULimitation
481,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5105h.0Eh]
9367, //формат
376,//номер текста, ТекстПУ=Fшим
482,//смещение длЯ таблицы с адресами
(Uint16)(20480),//знач по умолч.
(((Uint32)20480)>>16),//знач по умолч.
(Uint16)(2048),//минимум
(((Uint32)2048)>>16),//минимум
(Uint16)(10240),//максимум
(((Uint32)10240)>>16),//максимум

 //[5105h.0Fh]
31277, //формат
377,//номер текста, ТекстПУ=Сектор
483,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5105h.10h]
25893, //формат
378,//номер текста, ТекстПУ=U_ампл
484,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5105h.11h]
31533, //формат
379,//номер текста, ТекстПУ=F_bits
485,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 510Bh, ТекстПУ=smctrl
20143,//кол-во подиндексов
380, // номер текста
2, //такой длиной

 //[510Bh.01h]
31273, //формат
381,//номер текста, ТекстПУ=smstat
486,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.

 //[510Bh.02h]
53545, //формат
382,//номер текста, ТекстПУ=stcode
487,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.

 // Корневой под-индекс 510Eh, ТекстПУ=Защиты
20135,//кол-во подиндексов
383, // номер текста
7, //такой длиной

 //[510Eh.01h]
15121, //формат
384,//номер текста, ТекстПУ=bitF_1
488,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[510Eh.02h]
15121, //формат
385,//номер текста, ТекстПУ=bitF_2
489,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[510Eh.03h]
53265, //формат
386,//номер текста, ТекстПУ=mask1L
490,//смещение длЯ таблицы с адресами
(Uint16)(255),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[510Eh.04h]
53289, //формат
387,//номер текста, ТекстПУ=mask1H
491,//смещение длЯ таблицы с адресами
(Uint16)(255),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[510Eh.05h]
53265, //формат
388,//номер текста, ТекстПУ=mask2L
492,//смещение длЯ таблицы с адресами
(Uint16)(255),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[510Eh.06h]
53289, //формат
389,//номер текста, ТекстПУ=mask2H
493,//смещение длЯ таблицы с адресами
(Uint16)(255),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[510Eh.07h]
9529, //формат
390,//номер текста, ТекстПУ=Макс.Ток
494,//смещение длЯ таблицы с адресами
(Uint16)(16777216),//знач по умолч.
(((Uint32)16777216)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[510Eh.08h]
9625, //формат
391,//номер текста, ТекстПУ=Ud_max
495,//смещение длЯ таблицы с адресами
(Uint16)(18454938),//знач по умолч.
(((Uint32)18454938)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[510Eh.09h]
9625, //формат
392,//номер текста, ТекстПУ=Ud_min
496,//смещение длЯ таблицы с адресами
(Uint16)(6990506),//знач по умолч.
(((Uint32)6990506)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[510Eh.0Ah]
10041, //формат
393,//номер текста, ТекстПУ=speed_max
497,//смещение длЯ таблицы с адресами
(Uint16)(100663296),//знач по умолч.
(((Uint32)100663296)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 // Корневой под-индекс 5112h, ТекстПУ=ЗадПар
20146,//кол-во подиндексов
394, // номер текста
7, //такой длиной

 //[5112h.01h]
10017, //формат
395,//номер текста, ТекстПУ=Скор_з
498,//смещение длЯ таблицы с адресами
(Uint16)(335544),//знач по умолч.
(((Uint32)335544)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5112h.02h]
9505, //формат
396,//номер текста, ТекстПУ=ТокСТз
499,//смещение длЯ таблицы с адресами
(Uint16)(3355443),//знач по умолч.
(((Uint32)3355443)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5112h.03h]
9505, //формат
397,//номер текста, ТекстПУ=ТокУПз
500,//смещение длЯ таблицы с адресами
(Uint16)(3355443),//знач по умолч.
(((Uint32)3355443)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5112h.04h]
9985, //формат
398,//номер текста, ТекстПУ=элекУг
501,//смещение длЯ таблицы с адресами
(Uint16)(720992),//знач по умолч.
(((Uint32)720992)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5112h.05h]
9601, //формат
559,//номер текста, ТекстПУ=Uf_ref
502,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 // Корневой под-индекс 5113h, ТекстПУ=ПарДв
20144,//кол-во подиндексов
399, // номер текста
7, //такой длиной

 //[5113h.01h]
15241, //формат
400,//номер текста, ТекстПУ=ПарПол
503,//смещение длЯ таблицы с адресами
(Uint16)(4),//знач по умолч.
1,//минимум
50,//максимум
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5113h.02h]
8265, //формат
401,//номер текста, ТекстПУ=IпчНом
504,//смещение длЯ таблицы с адресами
(Uint16)(5),//знач по умолч.
0,//минимум
0,//максимум
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5113h.03h]
24617, //формат
402,//номер текста, ТекстПУ=Uф_ном
505,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5113h.04h]
9129, //формат
403,//номер текста, ТекстПУ=СкоростьНом
506,//смещение длЯ таблицы с адресами
(Uint16)(500),//знач по умолч.
0,//минимум
0,//максимум
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5113h.05h]
8233, //формат
404,//номер текста, ТекстПУ=UdcНом
507,//смещение длЯ таблицы с адресами
(Uint16)(36),//знач по умолч.
0,//минимум
1000,//максимум
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5113h.06h]
10113, //формат
405,//номер текста, ТекстПУ=Rs
508,//смещение длЯ таблицы с адресами
(Uint16)(1048576),//знач по умолч.
(((Uint32)1048576)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5113h.07h]
10145, //формат
406,//номер текста, ТекстПУ=Ls
509,//смещение длЯ таблицы с адресами
(Uint16)(16777),//знач по умолч.
(((Uint32)16777)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5113h.08h]
24713, //формат
407,//номер текста, ТекстПУ=Ном.част.
510,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5113h.09h]
11993, //формат
408,//номер текста, ТекстПУ=Sens_type
511,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5114h, ТекстПУ=СистПр
20107,//кол-во подиндексов
409, // номер текста
4, //такой длиной

 //[5114h.01h]
4365, //формат
410,//номер текста, ТекстПУ=HardwareType
512,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5114h.02h]
15117, //формат
411,//номер текста, ТекстПУ=РежРаб
513,//смещение длЯ таблицы с адресами
(Uint16)(4),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5114h.03h]
15117, //формат
412,//номер текста, ТекстПУ=Рекупр
514,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5114h.04h]
15117, //формат
413,//номер текста, ТекстПУ=АЦПсмещ
515,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5114h.05h]
14477, //формат
414,//номер текста, ТекстПУ=Перезагрузка
516,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
0,//минимум
1,//максимум

 //[5114h.06h]
15117, //формат
590,//номер текста, ТекстПУ=Возб
517,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5116h, ТекстПУ=АЦП
20144,//кол-во подиндексов
415, // номер текста
4, //такой длиной

 //[5116h.01h]
8257, //формат
416,//номер текста, ТекстПУ=IaУсил
518,//смещение длЯ таблицы с адресами
(Uint16)(40),//знач по умолч.
0,//минимум
0,//максимум

 //[5116h.02h]
16001, //формат
417,//номер текста, ТекстПУ=IaСмещ
519,//смещение длЯ таблицы с адресами
(Uint16)(62706),//знач по умолч.
0,//минимум
0,//максимум

 //[5116h.03h]
25913, //формат
418,//номер текста, ТекстПУ=Ia_мгн
520,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5116h.04h]
8257, //формат
419,//номер текста, ТекстПУ=IbУсил
521,//смещение длЯ таблицы с адресами
(Uint16)(40),//знач по умолч.
0,//минимум
0,//максимум

 //[5116h.05h]
16001, //формат
420,//номер текста, ТекстПУ=IbСмещ
522,//смещение длЯ таблицы с адресами
(Uint16)(62251),//знач по умолч.
0,//минимум
0,//максимум

 //[5116h.06h]
25913, //формат
421,//номер текста, ТекстПУ=Ib_мгн
523,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5116h.07h]
8257, //формат
422,//номер текста, ТекстПУ=IcУсил
524,//смещение длЯ таблицы с адресами
(Uint16)(40),//знач по умолч.
0,//минимум
0,//максимум

 //[5116h.08h]
16001, //формат
423,//номер текста, ТекстПУ=IcСмещ
525,//смещение длЯ таблицы с адресами
(Uint16)(62836),//знач по умолч.
0,//минимум
0,//максимум

 //[5116h.09h]
25913, //формат
424,//номер текста, ТекстПУ=Ic_мгн
526,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5116h.0Ah]
8257, //формат
585,//номер текста, ТекстПУ=IdУсил
527,//смещение длЯ таблицы с адресами
(Uint16)(40),//знач по умолч.
0,//минимум
0,//максимум

 //[5116h.0Bh]
16001, //формат
586,//номер текста, ТекстПУ=IdСмещ
528,//смещение длЯ таблицы с адресами
(Uint16)(62836),//знач по умолч.
0,//минимум
0,//максимум

 //[5116h.0Ch]
25913, //формат
587,//номер текста, ТекстПУ=Id_мгн
529,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5116h.0Dh]
8225, //формат
425,//номер текста, ТекстПУ=UdУсил
530,//смещение длЯ таблицы с адресами
(Uint16)(66),//знач по умолч.
0,//минимум
0,//максимум

 //[5116h.0Eh]
16001, //формат
426,//номер текста, ТекстПУ=Udсмещ
531,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
0,//минимум
0,//максимум

 //[5116h.0Fh]
26009, //формат
427,//номер текста, ТекстПУ=Ud
532,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 511Ah, ТекстПУ=Закон U(f) (АД)
20096,//кол-во подиндексов
428, // номер текста
7, //такой длиной

 //[511Ah.01h]
26417, //формат
429,//номер текста, ТекстПУ=F_вых
533,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[511Ah.02h]
25873, //формат
430,//номер текста, ТекстПУ=U_вых
534,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[511Ah.03h]
10033, //формат
431,//номер текста, ТекстПУ=f [0]
535,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[511Ah.04h]
9489, //формат
432,//номер текста, ТекстПУ=U [0]
536,//смещение длЯ таблицы с адресами
(Uint16)(419430),//знач по умолч.
(((Uint32)419430)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[511Ah.05h]
10033, //формат
433,//номер текста, ТекстПУ=f [1]
537,//смещение длЯ таблицы с адресами
(Uint16)(100663296),//знач по умолч.
(((Uint32)100663296)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[511Ah.06h]
9489, //формат
434,//номер текста, ТекстПУ=U [1]
538,//смещение длЯ таблицы с адресами
(Uint16)(12582912),//знач по умолч.
(((Uint32)12582912)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[511Ah.07h]
10033, //формат
435,//номер текста, ТекстПУ=f max
539,//смещение длЯ таблицы с адресами
(Uint16)(100663296),//знач по умолч.
(((Uint32)100663296)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 // Корневой под-индекс 5138h, ТекстПУ=текЗнч
20035,//кол-во подиндексов
436, // номер текста
1, //такой длиной

 //[5138h.01h]
26421, //формат
437,//номер текста, ТекстПУ=скор
540,//смещение длЯ таблицы с адресами

 //[5138h.02h]
26037, //формат
438,//номер текста, ТекстПУ=АктМощ
541,//смещение длЯ таблицы с адресами

 //[5138h.03h]
25909, //формат
439,//номер текста, ТекстПУ=ТокСт
542,//смещение длЯ таблицы с адресами

 //[5138h.04h]
25813, //формат
440,//номер текста, ТекстПУ=ЭлУголЗад
543,//смещение длЯ таблицы с адресами

 //[5138h.05h]
25813, //формат
441,//номер текста, ТекстПУ=ЭлУгол
544,//смещение длЯ таблицы с адресами

 //[5138h.06h]
25909, //формат
442,//номер текста, ТекстПУ=ТокСтЗад
545,//смещение длЯ таблицы с адресами

 // Корневой под-индекс 5150h, ТекстПУ=dlog
20023,//кол-во подиндексов
443, // номер текста
7, //такой длиной

 //[5150h.01h]
14477, //формат
444,//номер текста, ТекстПУ=mode
546,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5150h.02h]
14989, //формат
445,//номер текста, ТекстПУ=dlcont
547,//смещение длЯ таблицы с адресами
(Uint16)(192),//знач по умолч.
(((Uint32)192)>>16),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5150h.03h]
14989, //формат
446,//номер текста, ТекстПУ=dlisi1
548,//смещение длЯ таблицы с адресами
(Uint16)(1359151123),//знач по умолч.
(((Uint32)1359151123)>>16),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5150h.04h]
14989, //формат
447,//номер текста, ТекстПУ=dlisi2
549,//смещение длЯ таблицы с адресами
(Uint16)(1359085569),//знач по умолч.
(((Uint32)1359085569)>>16),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5150h.05h]
14989, //формат
448,//номер текста, ТекстПУ=dlisi3
550,//смещение длЯ таблицы с адресами
(Uint16)(1359151122),//знач по умолч.
(((Uint32)1359151122)>>16),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5150h.06h]
14989, //формат
449,//номер текста, ТекстПУ=dlisi4
551,//смещение длЯ таблицы с адресами
(Uint16)(1359020033),//знач по умолч.
(((Uint32)1359020033)>>16),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5150h.07h]
53645, //формат
450,//номер текста, ТекстПУ=StrtBL
552,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5150h.08h]
53685, //формат
451,//номер текста, ТекстПУ=StrtBH
553,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5150h.09h]
11837, //формат
452,//номер текста, ТекстПУ=OneShotOp
554,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5150h.0Ah]
10949, //формат
453,//номер текста, ТекстПУ=N предыст.
555,//смещение длЯ таблицы с адресами
(Uint16)(6710886),//знач по умолч.
(((Uint32)6710886)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5151h.00h]
15757, //формат
454,//номер текста, ТекстПУ=dlnval
556,//смещение длЯ таблицы с адресами
(Uint16)(50461787),//знач по умолч.
(((Uint32)50461787)>>16),//знач по умолч.

 // Корневой под-индекс 5152h, ТекстПУ=Энкодер
20000,//кол-во подиндексов
455, // номер текста
7, //такой длиной

 //[5152h.01h]
14985, //формат
456,//номер текста, ТекстПУ=Кол-во периодов
557,//смещение длЯ таблицы с адресами
(Uint16)(1000),//знач по умолч.
(((Uint32)1000)>>16),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.02h]
11839, //формат
457,//номер текста, ТекстПУ=Наличие репера
558,//смещение длЯ таблицы с адресами
(Uint16)(2),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.03h]
14473, //формат
458,//номер текста, ТекстПУ=SpeedCalcType
559,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.04h]
9345, //формат
459,//номер текста, ТекстПУ=СмещУг
560,//смещение длЯ таблицы с адресами
(Uint16)(9298387),//знач по умолч.
(((Uint32)9298387)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5152h.05h]
31369, //формат
460,//номер текста, ТекстПУ=Механическ.угол в ме
561,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.06h]
30857, //формат
460,//номер текста, ТекстПУ=Механическ.угол в ме
562,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.07h]
25761, //формат
461,//номер текста, ТекстПУ=Скорость
563,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.08h]
25729, //формат
462,//номер текста, ТекстПУ=Угол. электр.
564,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.09h]
25729, //формат
463,//номер текста, ТекстПУ=Угол мех.
565,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.0Ah]
53565, //формат
464,//номер текста, ТекстПУ=Команда иниц.
566,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.0Bh]
9217, //формат
465,//номер текста, ТекстПУ=Кф. скорости
567,//смещение длЯ таблицы с адресами
(Uint16)(838860),//знач по умолч.
(((Uint32)838860)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(65535),//максимум
(((Uint32)65535)>>16),//максимум

 //[5152h.0Ch]
30857, //формат
466,//номер текста, ТекстПУ=Posspeed_FLG1
568,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.0Dh]
30857, //формат
467,//номер текста, ТекстПУ=GPIOsValue
569,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.0Eh]
30857, //формат
468,//номер текста, ТекстПУ=UPPS
570,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.0Fh]
31369, //формат
469,//номер текста, ТекстПУ=QCPRD
571,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.10h]
30857, //формат
469,//номер текста, ТекстПУ=QCPRD
572,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.11h]
30857, //формат
470,//номер текста, ТекстПУ=QEPSTS
573,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.12h]
15241, //формат
471,//номер текста, ТекстПУ=revolCounter
574,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.13h]
15241, //формат
472,//номер текста, ТекстПУ=qcAbsPos
575,//смещение длЯ таблицы с адресами
(Uint16)(7),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.14h]
15241, //формат
472,//номер текста, ТекстПУ=qcAbsPos
576,//смещение длЯ таблицы с адресами
(Uint16)(7335),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.15h]
15753, //формат
472,//номер текста, ТекстПУ=qcAbsPos
577,//смещение длЯ таблицы с адресами
(Uint16)(1973415),//знач по умолч.
(((Uint32)1973415)>>16),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.16h]
25729, //формат
473,//номер текста, ТекстПУ=Уг.мех.не_огр
578,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5152h.17h]
25729, //формат
474,//номер текста, ТекстПУ=Уг.эл.не_огр
579,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5155h, ТекстПУ=Датчик Холла
20024,//кол-во подиндексов
475, // номер текста
7, //такой длиной

 //[5155h.01h]
25733, //формат
441,//номер текста, ТекстПУ=ЭлУгол
580,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.02h]
25765, //формат
461,//номер текста, ТекстПУ=Скорость
581,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.03h]
9381, //формат
476,//номер текста, ТекстПУ=Уставка Скорости
582,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5155h.04h]
25733, //формат
477,//номер текста, ТекстПУ=ЭлУгол6
583,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.05h]
11837, //формат
478,//номер текста, ТекстПУ=Инт-полятор вкл
584,//смещение длЯ таблицы с адресами
(Uint16)(81),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.06h]
14989, //формат
479,//номер текста, ТекстПУ=Уставка обнуления ск
585,//смещение длЯ таблицы с адресами
(Uint16)(500),//знач по умолч.
(((Uint32)500)>>16),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.07h]
25765, //формат
480,//номер текста, ТекстПУ=Скорость Мин.
586,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.08h]
30861, //формат
481,//номер текста, ТекстПУ=Ложные фронты
587,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.09h]
4573, //формат
482,//номер текста, ТекстПУ=Входы
588,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.0Ah]
9349, //формат
459,//номер текста, ТекстПУ=СмещУг
589,//смещение длЯ таблицы с адресами
(Uint16)(4869970),//знач по умолч.
(((Uint32)4869970)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5155h.0Bh]
30861, //формат
483,//номер текста, ТекстПУ=Уровень помех
590,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.0Ch]
30861, //формат
484,//номер текста, ТекстПУ=Ошиб_1_к.
591,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.0Dh]
30861, //формат
485,//номер текста, ТекстПУ=Ошиб_2_к.
592,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.0Eh]
30861, //формат
486,//номер текста, ТекстПУ=Ошиб_3_к.
593,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.0Fh]
53567, //формат
487,//номер текста, ТекстПУ=НапрВр
594,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.10h]
14477, //формат
488,//номер текста, ТекстПУ=Макс.ош.ДПР
595,//смещение длЯ таблицы с адресами
(Uint16)(10),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.11h]
30861, //формат
489,//номер текста, ТекстПУ=Ошибки последоват.
596,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.12h]
30861, //формат
490,//номер текста, ТекстПУ=Счёт.Rx.Int.
597,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.13h]
30861, //формат
491,//номер текста, ТекстПУ=Сч.прерыв.CAP1
598,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.14h]
30861, //формат
492,//номер текста, ТекстПУ=Сч.прерыв.CAP2
599,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5155h.15h]
30861, //формат
493,//номер текста, ТекстПУ=Сч.прерыв.CAP3
600,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5157h, ТекстПУ=Энкодер
20019,//кол-во подиндексов
455, // номер текста
7, //такой длиной

 //[5157h.01h]
14977, //формат
494,//номер текста, ТекстПУ=qepTik
601,//смещение длЯ таблицы с адресами
(Uint16)(4096),//знач по умолч.
(((Uint32)4096)>>16),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5157h.02h]
9369, //формат
459,//номер текста, ТекстПУ=СмещУг
602,//смещение длЯ таблицы с адресами
(Uint16)(9298387),//знач по умолч.
(((Uint32)9298387)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5157h.03h]
30849, //формат
460,//номер текста, ТекстПУ=Механическ.угол в ме
603,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5157h.04h]
25785, //формат
461,//номер текста, ТекстПУ=Скорость
604,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5157h.05h]
25753, //формат
462,//номер текста, ТекстПУ=Угол. электр.
605,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5157h.06h]
25753, //формат
463,//номер текста, ТекстПУ=Угол мех.
606,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5157h.07h]
25753, //формат
495,//номер текста, ТекстПУ=theta_mech_filtered
607,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5157h.08h]
9241, //формат
496,//номер текста, ТекстПУ=Кф. угла
608,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5157h.09h]
15233, //формат
471,//номер текста, ТекстПУ=revolCounter
609,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5157h.0Ah]
15745, //формат
472,//номер текста, ТекстПУ=qcAbsPos
610,//смещение длЯ таблицы с адресами
(Uint16)(1973415),//знач по умолч.
(((Uint32)1973415)>>16),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5157h.0Bh]
15233, //формат
472,//номер текста, ТекстПУ=qcAbsPos
611,//смещение длЯ таблицы с адресами
(Uint16)(7),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5157h.0Ch]
15233, //формат
472,//номер текста, ТекстПУ=qcAbsPos
612,//смещение длЯ таблицы с адресами
(Uint16)(7335),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5157h.0Dh]
25753, //формат
473,//номер текста, ТекстПУ=Уг.мех.не_огр
613,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5157h.0Eh]
25753, //формат
474,//номер текста, ТекстПУ=Уг.эл.не_огр
614,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5174h, ТекстПУ=Для отладки
20025,//кол-во подиндексов
497, // номер текста
7, //такой длиной

 //[5174h.01h]
25669, //формат
498,//номер текста, ТекстПУ=DebugL1
615,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5174h.02h]
25693, //формат
498,//номер текста, ТекстПУ=DebugL1
616,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5174h.03h]
25669, //формат
499,//номер текста, ТекстПУ=DebugL2
617,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5174h.04h]
25693, //формат
499,//номер текста, ТекстПУ=DebugL2
618,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5174h.05h]
30925, //формат
500,//номер текста, ТекстПУ=DebugI3
619,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5174h.06h]
31693, //формат
500,//номер текста, ТекстПУ=DebugI3
620,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5174h.07h]
30925, //формат
501,//номер текста, ТекстПУ=DebugI4
621,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5174h.08h]
31693, //формат
501,//номер текста, ТекстПУ=DebugI4
622,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5174h.09h]
9285, //формат
502,//номер текста, ТекстПУ=DebugW1
623,//смещение длЯ таблицы с адресами
(Uint16)(16777),//знач по умолч.
(((Uint32)16777)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5174h.0Ah]
15821, //формат
502,//номер текста, ТекстПУ=DebugW1
624,//смещение длЯ таблицы с адресами
(Uint16)(16777),//знач по умолч.
(((Uint32)16777)>>16),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5174h.0Bh]
9285, //формат
503,//номер текста, ТекстПУ=DebugW2
625,//смещение длЯ таблицы с адресами
(Uint16)(-190048),//знач по умолч.
(((Uint32)-190048)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5174h.0Ch]
15821, //формат
503,//номер текста, ТекстПУ=DebugW2
626,//смещение длЯ таблицы с адресами
(Uint16)(-190048),//знач по умолч.
(((Uint32)-190048)>>16),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5174h.0Dh]
9285, //формат
504,//номер текста, ТекстПУ=DebugW3
627,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5174h.0Eh]
15821, //формат
504,//номер текста, ТекстПУ=DebugW3
628,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5174h.0Fh]
9285, //формат
505,//номер текста, ТекстПУ=DebugW4
629,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5174h.10h]
15821, //формат
505,//номер текста, ТекстПУ=DebugW4
630,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5174h.11h]
50253, //формат
506,//номер текста, ТекстПУ=DebugF1
631,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5174h.12h]
50253, //формат
507,//номер текста, ТекстПУ=DebugF2
632,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5174h.13h]
50253, //формат
508,//номер текста, ТекстПУ=DebugF3
633,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5174h.14h]
50253, //формат
509,//номер текста, ТекстПУ=DebugF4
634,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 // Корневой под-индекс 5181h, ТекстПУ=Симулятор
20161,//кол-во подиндексов
510, // номер текста
7, //такой длиной

 //[5181h.01h]
11821, //формат
511,//номер текста, ТекстПУ=ModelMotorType
635,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.02h]
14381, //формат
399,//номер текста, ТекстПУ=ПарДв
636,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.03h]
50363, //формат
512,//номер текста, ТекстПУ=Мнагр сим.
637,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5181h.04h]
49327, //формат
427,//номер текста, ТекстПУ=Ud
638,//смещение длЯ таблицы с адресами
(Uint16)(1103101952),//знач по умолч.
(((Uint32)1103101952)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5181h.05h]
1211, //формат
513,//номер текста, ТекстПУ=Реал.Эл.Мом
639,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.06h]
1225, //формат
514,//номер текста, ТекстПУ=qepspd
640,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.07h]
1208, //формат
514,//номер текста, ТекстПУ=qepspd
641,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.08h]
173, //формат
515,//номер текста, ТекстПУ=qeptm
642,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.09h]
30765, //формат
516,//номер текста, ТекстПУ=EncoderIncrements
643,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.0Ah]
172, //формат
517,//номер текста, ТекстПУ=IsAlpha
644,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.0Bh]
172, //формат
518,//номер текста, ТекстПУ=IsBeta
645,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.0Ch]
172, //формат
519,//номер текста, ТекстПУ=Ia
646,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.0Dh]
172, //формат
421,//номер текста, ТекстПУ=Ib_мгн
647,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.0Eh]
172, //формат
520,//номер текста, ТекстПУ=Ic
648,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.0Fh]
172, //формат
584,//номер текста, ТекстПУ=Id
649,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.10h]
175, //формат
521,//номер текста, ТекстПУ=Ua
650,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.11h]
175, //формат
522,//номер текста, ТекстПУ=Ub
651,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.12h]
175, //формат
523,//номер текста, ТекстПУ=Uc
652,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.13h]
244, //формат
524,//номер текста, ТекстПУ=Мощ-ть
653,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5181h.14h]
30765, //формат
525,//номер текста, ТекстПУ=КодАвр
654,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5182h, ТекстПУ=Симулятор
20156,//кол-во подиндексов
510, // номер текста
7, //такой длиной

 //[5182h.01h]
51065, //формат
526,//номер текста, ТекстПУ=Lm
655,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5182h.02h]
51065, //формат
527,//номер текста, ТекстПУ=Ls
656,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5182h.03h]
51065, //формат
528,//номер текста, ТекстПУ=Lr
657,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5182h.04h]
50360, //формат
529,//номер текста, ТекстПУ=Rs
658,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5182h.05h]
50360, //формат
530,//номер текста, ТекстПУ=Rr
659,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5182h.06h]
57001, //формат
400,//номер текста, ТекстПУ=ПарПол
660,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5182h.07h]
50345, //формат
531,//номер текста, ТекстПУ=Инерция сим.
661,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5182h.08h]
50363, //формат
532,//номер текста, ТекстПУ=Торм.Мом.Выбег
662,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5182h.09h]
57001, //формат
533,//номер текста, ТекстПУ=QEPResolution
663,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5182h.0Ah]
49838, //формат
592,//номер текста, ТекстПУ=RatedCurrent
664,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5182h.0Bh]
49910, //формат
591,//номер текста, ТекстПУ=RatedPower
665,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5182h.0Ch]
57018, //формат
403,//номер текста, ТекстПУ=СкоростьНом
666,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5182h.0Dh]
31529, //формат
535,//номер текста, ТекстПУ=Коэф. АЦП тока
667,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5182h.0Eh]
1198, //формат
517,//номер текста, ТекстПУ=IsAlpha
668,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5182h.0Fh]
1198, //формат
518,//номер текста, ТекстПУ=IsBeta
669,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5182h.10h]
1193, //формат
536,//номер текста, ТекстПУ=PsiRD
670,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5182h.11h]
1193, //формат
537,//номер текста, ТекстПУ=PsiRQ
671,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5182h.12h]
1193, //формат
538,//номер текста, ТекстПУ=PsiRAlfa
672,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5182h.13h]
1193, //формат
539,//номер текста, ТекстПУ=PsiRBeta
673,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5183h, ТекстПУ=Симулятор
20152,//кол-во подиндексов
510, // номер текста
7, //такой длиной

 //[5183h.01h]
50345, //формат
540,//номер текста, ТекстПУ=m
674,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5183h.02h]
51065, //формат
541,//номер текста, ТекстПУ=Lsd
675,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5183h.03h]
51065, //формат
542,//номер текста, ТекстПУ=Lsq
676,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5183h.04h]
50362, //формат
529,//номер текста, ТекстПУ=Rs
677,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5183h.05h]
57001, //формат
400,//номер текста, ТекстПУ=ПарПол
678,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5183h.06h]
50345, //формат
531,//номер текста, ТекстПУ=Инерция сим.
679,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5183h.07h]
50363, //формат
532,//номер текста, ТекстПУ=Торм.Мом.Выбег
680,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5183h.08h]
57001, //формат
533,//номер текста, ТекстПУ=QEPResolution
681,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5183h.09h]
31529, //формат
535,//номер текста, ТекстПУ=Коэф. АЦП тока
682,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5183h.0Ah]
31529, //формат
550,//номер текста, ТекстПУ=Коэф. АЦП ток ОВ
683,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5183h.0Bh]
49832, //формат
592,//номер текста, ТекстПУ=RatedCurrent
684,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5183h.0Ch]
49904, //формат
591,//номер текста, ТекстПУ=RatedPower
685,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5183h.0Dh]
57012, //формат
403,//номер текста, ТекстПУ=СкоростьНом
686,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5183h.0Eh]
49832, //формат
593,//номер текста, ТекстПУ=RatedFluxCurrent
687,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5183h.0Fh]
175, //формат
553,//номер текста, ТекстПУ=Uf
688,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5183h.10h]
1192, //формат
555,//номер текста, ТекстПУ=Ток ОВ
689,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5183h.11h]
1193, //формат
588,//номер текста, ТекстПУ=Psi возб
690,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5184h, ТекстПУ=Симулятор
20159,//кол-во подиндексов
510, // номер текста
7, //такой длиной

 //[5184h.01h]
51069, //формат
543,//номер текста, ТекстПУ=La
691,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5184h.02h]
51069, //формат
544,//номер текста, ТекстПУ=Lf
692,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5184h.03h]
50364, //формат
545,//номер текста, ТекстПУ=Ra
693,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5184h.04h]
50364, //формат
546,//номер текста, ТекстПУ=Rf
694,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5184h.05h]
50349, //формат
531,//номер текста, ТекстПУ=Инерция сим.
695,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5184h.06h]
50355, //формат
532,//номер текста, ТекстПУ=Торм.Мом.Выбег
696,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5184h.07h]
57005, //формат
533,//номер текста, ТекстПУ=QEPResolution
697,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5184h.08h]
49838, //формат
594,//номер текста, ТекстПУ=RatedCurrent
698,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5184h.09h]
49838, //формат
593,//номер текста, ТекстПУ=RatedFluxCurrent
699,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5184h.0Ah]
31533, //формат
549,//номер текста, ТекстПУ=Коэф. АЦП ток ОЯ
700,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5184h.0Bh]
31533, //формат
550,//номер текста, ТекстПУ=Коэф. АЦП ток ОВ
701,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5184h.0Ch]
49910, //формат
591,//номер текста, ТекстПУ=RatedPower
702,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5184h.0Dh]
57010, //формат
403,//номер текста, ТекстПУ=СкоростьНом
703,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5184h.0Eh]
50364, //формат
551,//номер текста, ТекстПУ=Доб.сопротив.
704,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5184h.0Fh]
175, //формат
552,//номер текста, ТекстПУ=Ua
705,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5184h.10h]
175, //формат
553,//номер текста, ТекстПУ=Uf
706,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5184h.11h]
1198, //формат
554,//номер текста, ТекстПУ=Ia
707,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5184h.12h]
1198, //формат
555,//номер текста, ТекстПУ=Ток ОВ
708,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5185h, ТекстПУ=ДПТ СУ
20142,//кол-во подиндексов
556, // номер текста
7, //такой длиной

 //[5185h.01h]
9285, //формат
557,//номер текста, ТекстПУ=Зад. Скор.
709,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5185h.02h]
9637, //формат
558,//номер текста, ТекстПУ=Ua_ref
710,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5185h.03h]
9637, //формат
559,//номер текста, ТекстПУ=Uf_ref
711,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 // Корневой под-индекс 5186h, ТекстПУ=Симулятор
20144,//кол-во подиндексов
510, // номер текста
7, //такой длиной

 //[5186h.01h]
50337, //формат
531,//номер текста, ТекстПУ=Инерция сим.
712,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5186h.02h]
50355, //формат
532,//номер текста, ТекстПУ=Торм.Мом.Выбег
713,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5186h.03h]
50352, //формат
529,//номер текста, ТекстПУ=Rs
714,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5186h.04h]
56993, //формат
400,//номер текста, ТекстПУ=ПарПол
715,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5186h.05h]
56993, //формат
533,//номер текста, ТекстПУ=QEPResolution
716,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5186h.06h]
31521, //формат
535,//номер текста, ТекстПУ=Коэф. АЦП тока
717,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5186h.07h]
49838, //формат
592,//номер текста, ТекстПУ=RatedCurrent
718,//смещение длЯ таблицы с адресами
(Uint16)(0),//знач по умолч.
(((Uint32)0)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5186h.08h]
758, //формат
591,//номер текста, ТекстПУ=RatedPower
719,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5186h.09h]
7858, //формат
403,//номер текста, ТекстПУ=СкоростьНом
720,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5186h.0Ah]
1185, //формат
560,//номер текста, ТекстПУ=Поток A
721,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5186h.0Bh]
1185, //формат
561,//номер текста, ТекстПУ=Поток B
722,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5186h.0Ch]
1185, //формат
562,//номер текста, ТекстПУ=Поток C
723,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5186h.0Dh]
1203, //формат
563,//номер текста, ТекстПУ=Момент A
724,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5186h.0Eh]
1203, //формат
564,//номер текста, ТекстПУ=Момент B
725,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5186h.0Fh]
1203, //формат
565,//номер текста, ТекстПУ=Момент C
726,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5A20h, ТекстПУ=MODBUS
19641,//кол-во подиндексов
566, // номер текста
3, //такой длиной

 //[5A20h.01h]
15149, //формат
589,//номер текста, ТекстПУ=Вкл
727,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.

 //[5A20h.02h]
15149, //формат
567,//номер текста, ТекстПУ=ModBus_ID
728,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.

 //[5A20h.03h]
14637, //формат
437,//номер текста, ТекстПУ=скор
729,//смещение длЯ таблицы с адресами
(Uint16)(9600),//знач по умолч.
(((Uint32)9600)>>16),//знач по умолч.

 //[5A20h.04h]
14637, //формат
581,//номер текста, ТекстПУ=OfflineCounterMax
730,//смещение длЯ таблицы с адресами
(Uint16)(20000),//знач по умолч.
(((Uint32)20000)>>16),//знач по умолч.

 //[5A20h.05h]
15149, //формат
583,//номер текста, ТекстПУ=auto_res
731,//смещение длЯ таблицы с адресами
(Uint16)(1),//знач по умолч.
0,// заполнитель места под одинак. размер. подинд.

 //[5A20h.06h]
31021, //формат
582,//номер текста, ТекстПУ=OfflineCounter
732,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5A20h.07h]
31533, //формат
568,//номер текста, ТекстПУ=MODBUS online
733,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5A20h.08h]
31533, //формат
569,//номер текста, ТекстПУ=485_RD
734,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5A20h.09h]
31533, //формат
578,//номер текста, ТекстПУ=TXState
735,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5A20h.0Ah]
31533, //формат
467,//номер текста, ТекстПУ=GPIOsValue
736,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5A20h.0Bh]
31533, //формат
579,//номер текста, ТекстПУ=ReceivePacketCounte
737,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5A20h.0Ch]
31533, //формат
580,//номер текста, ТекстПУ=ReceiveByteCounter
738,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 // Корневой под-индекс 5B04h, ТекстПУ=ПарДв
20149,//кол-во подиндексов
399, // номер текста
7, //такой длиной

 //[5B04h.01h]
12453, //формат
571,//номер текста, ТекстПУ=Tr
739,//смещение длЯ таблицы с адресами
(Uint16)(16777216),//знач по умолч.
(((Uint32)16777216)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5B04h.02h]
12453, //формат
572,//номер текста, ТекстПУ=FluxCurrentRatio
740,//смещение длЯ таблицы с адресами
(Uint16)(5033165),//знач по умолч.
(((Uint32)5033165)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5B04h.03h]
12613, //формат
573,//номер текста, ТекстПУ=FluxCurrentMin
741,//смещение длЯ таблицы с адресами
(Uint16)(5033165),//знач по умолч.
(((Uint32)5033165)>>16),//знач по умолч.
(Uint16)(0),//минимум
(((Uint32)0)>>16),//минимум
(Uint16)(0),//максимум
(((Uint32)0)>>16),//максимум

 //[5B04h.04h]
28997, //формат
574,//номер текста, ТекстПУ=IsD
742,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5B04h.05h]
28997, //формат
575,//номер текста, ТекстПУ=IsQ
743,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5B04h.06h]
28837, //формат
536,//номер текста, ТекстПУ=PsiRD
744,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5B04h.07h]
28837, //формат
537,//номер текста, ТекстПУ=PsiRQ
745,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.

 //[5B04h.08h]
30501, //формат
576,//номер текста, ТекстПУ=psiRalpha
746,//смещение длЯ таблицы с адресами
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
0,// заполнитель места под одинак. размер. подинд.
};
//Таблица соответствиЯ индекса и смещениЯ длЯ таблицы с адресами

int16 const CO2_OD_TBL1[] = { 4096, //индекс
0,//смещение длЯ таблицы с адресами
4097, //индекс
3,//смещение длЯ таблицы с адресами
4098, //индекс
6,//смещение длЯ таблицы с адресами
4116, //индекс
9,//смещение длЯ таблицы с адресами
4118, //индекс
12,//смещение длЯ таблицы с адресами
4119, //индекс
20,//смещение длЯ таблицы с адресами
4120, //индекс
24,//смещение длЯ таблицы с адресами
4608, //индекс
39,//смещение длЯ таблицы с адресами
4736, //индекс
48,//смещение длЯ таблицы с адресами
5120, //индекс
60,//смещение длЯ таблицы с адресами
5121, //индекс
73,//смещение длЯ таблицы с адресами
5122, //индекс
86,//смещение длЯ таблицы с адресами
5123, //индекс
99,//смещение длЯ таблицы с адресами
5124, //индекс
112,//смещение длЯ таблицы с адресами
5125, //индекс
125,//смещение длЯ таблицы с адресами
5126, //индекс
138,//смещение длЯ таблицы с адресами
5127, //индекс
151,//смещение длЯ таблицы с адресами
5632, //индекс
164,//смещение длЯ таблицы с адресами
5633, //индекс
187,//смещение длЯ таблицы с адресами
5634, //индекс
210,//смещение длЯ таблицы с адресами
5635, //индекс
233,//смещение длЯ таблицы с адресами
5636, //индекс
256,//смещение длЯ таблицы с адресами
5637, //индекс
279,//смещение длЯ таблицы с адресами
5638, //индекс
302,//смещение длЯ таблицы с адресами
5639, //индекс
325,//смещение длЯ таблицы с адресами
6144, //индекс
348,//смещение длЯ таблицы с адресами
6145, //индекс
371,//смещение длЯ таблицы с адресами
6146, //индекс
394,//смещение длЯ таблицы с адресами
6147, //индекс
417,//смещение длЯ таблицы с адресами
6148, //индекс
440,//смещение длЯ таблицы с адресами
6149, //индекс
463,//смещение длЯ таблицы с адресами
6150, //индекс
486,//смещение длЯ таблицы с адресами
6151, //индекс
509,//смещение длЯ таблицы с адресами
6656, //индекс
532,//смещение длЯ таблицы с адресами
6657, //индекс
555,//смещение длЯ таблицы с адресами
6658, //индекс
578,//смещение длЯ таблицы с адресами
6659, //индекс
601,//смещение длЯ таблицы с адресами
6660, //индекс
624,//смещение длЯ таблицы с адресами
6661, //индекс
647,//смещение длЯ таблицы с адресами
6662, //индекс
670,//смещение длЯ таблицы с адресами
6663, //индекс
693,//смещение длЯ таблицы с адресами
8192, //индекс
716,//смещение длЯ таблицы с адресами
8193, //индекс
722,//смещение длЯ таблицы с адресами
8194, //индекс
726,//смещение длЯ таблицы с адресами
8195, //индекс
741,//смещение длЯ таблицы с адресами
8196, //индекс
756,//смещение длЯ таблицы с адресами
8197, //индекс
761,//смещение длЯ таблицы с адресами
8199, //индекс
764,//смещение длЯ таблицы с адресами
8208, //индекс
779,//смещение длЯ таблицы с адресами
8209, //индекс
794,//смещение длЯ таблицы с адресами
8210, //индекс
817,//смещение длЯ таблицы с адресами
8212, //индекс
821,//смещение длЯ таблицы с адресами
8213, //индекс
825,//смещение длЯ таблицы с адресами
8214, //индекс
828,//смещение длЯ таблицы с адресами
8215, //индекс
831,//смещение длЯ таблицы с адресами
8320, //индекс
834,//смещение длЯ таблицы с адресами
8321, //индекс
893,//смещение длЯ таблицы с адресами
8322, //индекс
896,//смещение длЯ таблицы с адресами
8323, //индекс
909,//смещение длЯ таблицы с адресами
8448, //индекс
976,//смещение длЯ таблицы с адресами
8449, //индекс
1003,//смещение длЯ таблицы с адресами
8450, //индекс
1030,//смещение длЯ таблицы с адресами
8451, //индекс
1057,//смещение длЯ таблицы с адресами
8452, //индекс
1084,//смещение длЯ таблицы с адресами
8453, //индекс
1111,//смещение длЯ таблицы с адресами
8454, //индекс
1138,//смещение длЯ таблицы с адресами
8455, //индекс
1165,//смещение длЯ таблицы с адресами
8456, //индекс
1192,//смещение длЯ таблицы с адресами
8457, //индекс
1219,//смещение длЯ таблицы с адресами
8458, //индекс
1246,//смещение длЯ таблицы с адресами
8459, //индекс
1273,//смещение длЯ таблицы с адресами
8460, //индекс
1300,//смещение длЯ таблицы с адресами
8461, //индекс
1327,//смещение длЯ таблицы с адресами
8462, //индекс
1354,//смещение длЯ таблицы с адресами
8463, //индекс
1381,//смещение длЯ таблицы с адресами
8464, //индекс
1408,//смещение длЯ таблицы с адресами
8465, //индекс
1435,//смещение длЯ таблицы с адресами
8466, //индекс
1462,//смещение длЯ таблицы с адресами
8467, //индекс
1489,//смещение длЯ таблицы с адресами
8468, //индекс
1516,//смещение длЯ таблицы с адресами
8469, //индекс
1543,//смещение длЯ таблицы с адресами
8470, //индекс
1570,//смещение длЯ таблицы с адресами
8471, //индекс
1597,//смещение длЯ таблицы с адресами
8472, //индекс
1624,//смещение длЯ таблицы с адресами
8473, //индекс
1651,//смещение длЯ таблицы с адресами
8474, //индекс
1678,//смещение длЯ таблицы с адресами
8475, //индекс
1705,//смещение длЯ таблицы с адресами
8476, //индекс
1732,//смещение длЯ таблицы с адресами
8477, //индекс
1759,//смещение длЯ таблицы с адресами
9474, //индекс
1786,//смещение длЯ таблицы с адресами
9487, //индекс
1813,//смещение длЯ таблицы с адресами
9522, //индекс
1818,//смещение длЯ таблицы с адресами
9524, //индекс
1830,//смещение длЯ таблицы с адресами
9728, //индекс
1837,//смещение длЯ таблицы с адресами
12288, //индекс
1845,//смещение длЯ таблицы с адресами
12544, //индекс
1848,//смещение длЯ таблицы с адресами
20480, //индекс
1871,//смещение длЯ таблицы с адресами
20736, //индекс
1901,//смещение длЯ таблицы с адресами
20737, //индекс
2021,//смещение длЯ таблицы с адресами
20738, //индекс
2150,//смещение длЯ таблицы с адресами
20739, //индекс
2270,//смещение длЯ таблицы с адресами
20740, //индекс
2453,//смещение длЯ таблицы с адресами
20741, //индекс
2573,//смещение длЯ таблицы с адресами
20747, //индекс
2729,//смещение длЯ таблицы с адресами
20750, //индекс
2740,//смещение длЯ таблицы с адресами
20754, //индекс
2833,//смещение длЯ таблицы с адресами
20755, //индекс
2881,//смещение длЯ таблицы с адресами
20756, //индекс
2965,//смещение длЯ таблицы с адресами
20758, //индекс
3004,//смещение длЯ таблицы с адресами
20762, //индекс
3097,//смещение длЯ таблицы с адресами
20792, //индекс
3163,//смещение длЯ таблицы с адресами
20816, //индекс
3184,//смещение длЯ таблицы с адресами
20817, //индекс
3277,//смещение длЯ таблицы с адресами
20818, //индекс
3282,//смещение длЯ таблицы с адресами
20821, //индекс
3492,//смещение длЯ таблицы с адресами
20823, //индекс
3684,//смещение длЯ таблицы с адресами
20852, //индекс
3813,//смещение длЯ таблицы с адресами
20865, //индекс
3996,//смещение длЯ таблицы с адресами
20866, //индекс
4179,//смещение длЯ таблицы с адресами
20867, //индекс
4353,//смещение длЯ таблицы с адресами
20868, //индекс
4509,//смещение длЯ таблицы с адресами
20869, //индекс
4674,//смещение длЯ таблицы с адресами
20870, //индекс
4704,//смещение длЯ таблицы с адресами
23072, //индекс
4842,//смещение длЯ таблицы с адресами
23300, //индекс
4905,//смещение длЯ таблицы с адресами
};
Uint16 const co2_SPIrange1rw = CO2_SPI_START_ADDR+0;
Uint16 const co2_SPIrange1rwCRC = CO2_SPI_START_ADDR+8;
Uint16 const co2_SPIrange1rwp = CO2_SPI_START_ADDR+10;
Uint16 const co2_SPIrange1rwpCRC = CO2_SPI_START_ADDR+336;
Uint16 const co2_SPIrange1rwps = CO2_SPI_START_ADDR+338;
Uint16 const co2_SPIrange1rwpsCRC = CO2_SPI_START_ADDR+338;
Uint16 const co2_SPIrange2rw = CO2_SPI_START_ADDR+340;
Uint16 const co2_SPIrange2rwCRC = CO2_SPI_START_ADDR+565;
Uint16 const co2_SPIrange2rwp = CO2_SPI_START_ADDR+567;
Uint16 const co2_SPIrange2rwpCRC = CO2_SPI_START_ADDR+592;
Uint16 const co2_SPIrange2rwps = CO2_SPI_START_ADDR+594;
Uint16 const co2_SPIrange2rwpsCRC = CO2_SPI_START_ADDR+594;
Uint16 const co2_SPIrange3rw = CO1_SPI_START_ADDR+596;
Uint16 const co2_SPIrange3rwCRC = CO1_SPI_START_ADDR+606;
Uint16 const co2_SPIrange3rwp = CO1_SPI_START_ADDR+608;
Uint16 const co2_SPIrange3rwpCRC = CO1_SPI_START_ADDR+608;
Uint16 const co2_SPIrange3rwps = CO1_SPI_START_ADDR+610;
Uint16 const co2_SPIrange3rwpsCRC = CO1_SPI_START_ADDR+610;
Uint16 const co2_SPIrange4rw = CO1_SPI_START_ADDR+612;
Uint16 const co2_SPIrange4rwCRC = CO1_SPI_START_ADDR+612;
Uint16 const co2_SPIrange4rwp = CO1_SPI_START_ADDR+614;
Uint16 const co2_SPIrange4rwpCRC = CO1_SPI_START_ADDR+614;
Uint16 const co2_SPIrange4rwps = CO1_SPI_START_ADDR+616;
Uint16 const co2_SPIrange4rwpsCRC = CO1_SPI_START_ADDR+616;
Uint16 const co2_SPIrange5rw = CO1_SPI_START_ADDR+618;
Uint16 const co2_SPIrange5rwCRC = CO1_SPI_START_ADDR+1255;
Uint16 const co2_SPIrange5rwp = CO1_SPI_START_ADDR+1257;
Uint16 const co2_SPIrange5rwpCRC = CO1_SPI_START_ADDR+1261;
Uint16 const co2_SPIrange5rwps = CO1_SPI_START_ADDR+1263;
Uint16 const co2_SPIrange5rwpsCRC = CO1_SPI_START_ADDR+1263;
Uint16 const co2_first1000 = 0;
Uint16 const co2_first2000 = 82;
Uint16 const co2_first3000 = 188;
Uint16 const co2_first4000 = 192;
Uint16 const co2_first5000 = 192;
//Таблица групп

Uint16 const CO2_TYPE_DEF_TABLE[] = {    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)4,\
    (Uint16)5,\
    (Uint16)-1,\
    (Uint16)10,\
    (Uint16)4,\
    (Uint16)0,\
    (Uint16)0,\
    (Uint16)40,\
    (Uint16)17,\
    (Uint16)6,\
    (Uint16)8,\
    (Uint16)7,\
    (Uint16)9,\
    (Uint16)10,\
    (Uint16)15,\
    (Uint16)36,\
    (Uint16)16,\
    (Uint16)1,\
    (Uint16)13,\
    (Uint16)12,\
    (Uint16)14,\
    (Uint16)18,\
    (Uint16)-1,\
    (Uint16)28,\
    (Uint16)0,\
    (Uint16)24,\
    (Uint16)23,\
    (Uint16)25,\
    (Uint16)27,\
    (Uint16)47,\
    (Uint16)48,\
    (Uint16)49,\
    (Uint16)50,\
    (Uint16)19,\
    (Uint16)51,\
    (Uint16)3,\
    (Uint16)20,\
};
//Таблица CALLBACK-функций

//**************************************************************
long const CO2_OD_CALLBACK_TBL[]={0,//1000h.00h
0,//1001h.00h
0,//1002h.00h
0,//1014h.00h
0,//1016h.00h
0,//1017h.00h
0,//1018h.00h
0,//1200h.00h
0,//1280h.00h
0,//1400h.00h
0,//1401h.00h
0,//1402h.00h
0,//1403h.00h
0,//1404h.00h
0,//1405h.00h
0,//1406h.00h
0,//1407h.00h
0,//1600h.00h
0,//1601h.00h
0,//1602h.00h
0,//1603h.00h
0,//1604h.00h
0,//1605h.00h
0,//1606h.00h
0,//1607h.00h
0,//1800h.00h
0,//1801h.00h
0,//1802h.00h
0,//1803h.00h
0,//1804h.00h
0,//1805h.00h
0,//1806h.00h
0,//1807h.00h
0,//1A00h.00h
0,//1A01h.00h
0,//1A02h.00h
0,//1A03h.00h
0,//1A04h.00h
0,//1A05h.00h
0,//1A06h.00h
0,//1A07h.00h
0,//2000h.00h
0,//2001h.00h
0,//2002h.00h
0,//2003h.00h
0,//2004h.00h
0,//2005h.00h
0,//2007h.00h
0,//2010h.00h
0,//2011h.00h
0,//2012h.00h
0,//2014h.00h
0,//2015h.00h
0,//2016h.00h
0,//2017h.00h
0,//2080h.00h
0,//2081h.00h
0,//2082h.00h
0,//2083h.00h
0,//2100h.00h
0,//2101h.00h
0,//2102h.00h
0,//2103h.00h
0,//2104h.00h
0,//2105h.00h
0,//2106h.00h
0,//2107h.00h
0,//2108h.00h
0,//2109h.00h
0,//210Ah.00h
0,//210Bh.00h
0,//210Ch.00h
0,//210Dh.00h
0,//210Eh.00h
0,//210Fh.00h
0,//2110h.00h
0,//2111h.00h
0,//2112h.00h
0,//2113h.00h
0,//2114h.00h
0,//2115h.00h
0,//2116h.00h
0,//2117h.00h
0,//2118h.00h
0,//2119h.00h
0,//211Ah.00h
0,//211Bh.00h
0,//211Ch.00h
0,//211Dh.00h
0,//2502h.00h
(long)((void(*)(long))callback_RTC),//250Fh.00h
0,//2532h.00h
0,//2534h.00h
0,//2600h.00h
0,//3000h.00h
0,//3100h.00h
0,//5000h.00h
0,//5100h.00h
0,//5101h.00h
0,//5102h.00h
0,//5103h.00h
0,//5104h.00h
0,//5105h.00h
0,//510Bh.00h
0,//510Eh.00h
0,//5112h.00h
0,//5113h.00h
0,//5114h.00h
0,//5116h.00h
0,//511Ah.00h
0,//5138h.00h
0,//5150h.00h
(long)((void(*)(long))callback_dlog),//5151h.00h
0,//5152h.00h
0,//5155h.00h
0,//5157h.00h
0,//5174h.00h
0,//5181h.00h
0,//5182h.00h
0,//5183h.00h
0,//5184h.00h
0,//5185h.00h
0,//5186h.00h
0,//5A20h.00h
0,//5B04h.00h
};

