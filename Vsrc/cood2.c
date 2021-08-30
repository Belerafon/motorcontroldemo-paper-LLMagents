// Файл: cood2.c
// Профиль: D:\Eclipse_GCC_ARM_wks\motorcontroldemo\cood.xml
// Конфигурация: Базовый профиль
// Дата создания: 30.08.2021 18:46:11
// Пользователь: default
// COODEdit Basic v.7.3.8
#include "DSP.h"
#include "main.h"
#include "cood2.h"
#include "co_ODvars.h"

// всего 130 индексов в словаре
// всего 897 элементов в словаре
Uint16 const co2_numOfInd = 130;
// Номер перечесления для групп
Uint16 const co2_groupEnum = 15;
// Хэш-сумма:
Uint32 const co2_odHash[4] = {0xB9C45032, 0xCBA147EB, 0xC14657A4, 0x98DECC5B};
// Таблица с адресами переменных
long const CO2_OD_TBL3[] = {
(long)(&co2_vars.co_deviceType),	//[1000h.00h], Название: Тип устройства, ТекстПУ: Device Type, Группа: CAN
(long)(&co2_vars.co_errorRegister),	//[1001h.00h], Название: Регистр ошибок, ТекстПУ: ErrReg, Группа: CAN
(long)(&co2_vars.co_deviceState),	//[1002h.00h], Название: Manufacturer Status Register, ТекстПУ: DevSta, Группа: CAN
(long)(&co2_vars.co_emcyCOBID),	//[1014h.00h], Название: COB-ID Emergency Object, ТекстПУ: eCOBID, Группа: CAN
(long)(&co2_vars.co_CHBT),	//[1016h.01h], Название: Время ожидания сообщения Heartbeat, ТекстПУ: CHBT, Группа: CAN
(long)(&co2_vars.co_PHBT),	//[1017h.00h], Название: Период генерации сообщения Heartbeat, ТекстПУ: PHBT, Группа: CAN
(long)(&co2_vars.co_vendorID),	//[1018h.01h], Название: Код производителя, ТекстПУ: VendID, Группа: CAN
(long)(&co2_vars.co_productCode),	//[1018h.02h], Название: Код продукта, ТекстПУ: PrCode, Группа: CAN
(long)(&co2_vars.co_revisionNumber),	//[1018h.03h], Название: Номер ревизии, ТекстПУ: RevNum, Группа: CAN
(long)(&co2_vars.co_serialNumber),	//[1018h.04h], Название: Серийный номер, ТекстПУ: SerNum, Группа: CAN
(long)(&co2_vars.co_csrxCOBID),	//[1200h.01h], Название: COB-ID Client->Server(rx), ТекстПУ: CSRXID, Группа: CAN
(long)(&co2_vars.co_sctxCOBID),	//[1200h.02h], Название: COB-ID Server->Client(tx), ТекстПУ: SCTXID, Группа: CAN
(long)(&co2_vars.co_cstxCOBID),	//[1280h.01h], Название: COB-ID Client->Server(tx), ТекстПУ: CSTXID, Группа: CAN
(long)(&co2_vars.co_scrxCOBID),	//[1280h.02h], Название: COB-ID Server->Client(rx), ТекстПУ: SCRXID, Группа: CAN
(long)(&co2_vars.co_ssNODEID),	//[1280h.03h], Название: NODE-ID on SDO server, ТекстПУ: SDOSID, Группа: CAN
(long)(&co2_vars.co_RPDO1COBID),	//[1400h.01h], Название: COB-ID for RPDO1, ТекстПУ: IRPDO1, Группа: CAN
(long)(&co2_vars.co_transType),	//[1400h.02h], Название: Тип передачи, ТекстПУ: TransT, Группа: CAN
(long)(&co2_vars.co_RPDO2COBID),	//[1401h.01h], Название: COB-ID for RPDO2, ТекстПУ: IRPDO2, Группа: CAN
(long)(&co2_vars.co_transType),	//[1401h.02h], Название: Тип передачи, ТекстПУ: TransT, Группа: CAN
(long)(&co2_vars.co_RPDO3COBID),	//[1402h.01h], Название: COB-ID for RPDO3, ТекстПУ: IRPDO3, Группа: CAN
(long)(&co2_vars.co_transType),	//[1402h.02h], Название: Тип передачи, ТекстПУ: TransT, Группа: CAN
(long)(&co2_vars.co_RPDO4COBID),	//[1403h.01h], Название: COB-ID for RPDO4, ТекстПУ: IRPDO4, Группа: CAN
(long)(&co2_vars.co_transType),	//[1403h.02h], Название: Тип передачи, ТекстПУ: TransT, Группа: CAN
(long)(&co2_vars.co_RPDO5COBID),	//[1404h.01h], Название: COB-ID for RPDO5, ТекстПУ: IRPDO5, Группа: CAN
(long)(&co2_vars.co_transType),	//[1404h.02h], Название: Тип передачи, ТекстПУ: TransT, Группа: CAN
(long)(&co2_vars.co_RPDO6COBID),	//[1405h.01h], Название: COB-ID for RPDO6, ТекстПУ: IRPDO6, Группа: CAN
(long)(&co2_vars.co_transType),	//[1405h.02h], Название: Тип передачи, ТекстПУ: TransT, Группа: CAN
(long)(&co2_vars.co_RPDO7COBID),	//[1406h.01h], Название: COB-ID for RPDO7, ТекстПУ: IRPDO7, Группа: CAN
(long)(&co2_vars.co_transType),	//[1406h.02h], Название: Тип передачи, ТекстПУ: TransT, Группа: CAN
(long)(&co2_vars.co_RPDO8COBID),	//[1407h.01h], Название: COB-ID for RPDO8, ТекстПУ: IRPDO8, Группа: CAN
(long)(&co2_vars.co_transType),	//[1407h.02h], Название: Тип передачи, ТекстПУ: TransT, Группа: CAN
(long)(&co2_vars.co_RPDO1_1Mapping),	//[1600h.01h], Название: RPDO1-1 Mapping, ТекстПУ: RPM11, Группа: CAN
(long)(&co2_vars.co_RPDO1_2Mapping),	//[1600h.02h], Название: RPDO1-2 Mapping, ТекстПУ: RPM12, Группа: CAN
(long)(&co2_vars.co_RPDO1_3Mapping),	//[1600h.03h], Название: RPDO1-3 Mapping, ТекстПУ: RPM13, Группа: CAN
(long)(&co2_vars.co_RPDO1_4Mapping),	//[1600h.04h], Название: RPDO1-4 Mapping, ТекстПУ: RPM14, Группа: CAN
(long)(&co2_vars.co_RPDO2_1Mapping),	//[1601h.01h], Название: RPDO2-1 Mapping, ТекстПУ: RPM21, Группа: CAN
(long)(&co2_vars.co_RPDO2_2Mapping),	//[1601h.02h], Название: RPDO2-2 Mapping, ТекстПУ: RPM22, Группа: CAN
(long)(&co2_vars.co_RPDO2_3Mapping),	//[1601h.03h], Название: RPDO2-3 Mapping, ТекстПУ: RPM23, Группа: CAN
(long)(&co2_vars.co_RPDO2_4Mapping),	//[1601h.04h], Название: RPDO2-4 Mapping, ТекстПУ: RPM24, Группа: CAN
(long)(&co2_vars.co_RPDO3_1Mapping),	//[1602h.01h], Название: RPDO3-1 Mapping, ТекстПУ: RPM31, Группа: CAN
(long)(&co2_vars.co_RPDO3_2Mapping),	//[1602h.02h], Название: RPDO3-2 Mapping, ТекстПУ: RPM32, Группа: CAN
(long)(&co2_vars.co_RPDO3_3Mapping),	//[1602h.03h], Название: RPDO3-3 Mapping, ТекстПУ: RPM33, Группа: CAN
(long)(&co2_vars.co_RPDO3_4Mapping),	//[1602h.04h], Название: RPDO3-4 Mapping, ТекстПУ: RPM34, Группа: CAN
(long)(&co2_vars.co_RPDO4_1Mapping),	//[1603h.01h], Название: RPDO4-1 Mapping, ТекстПУ: RPM41, Группа: CAN
(long)(&co2_vars.co_RPDO4_2Mapping),	//[1603h.02h], Название: RPDO4-2 Mapping, ТекстПУ: RPM42, Группа: CAN
(long)(&co2_vars.co_RPDO4_3Mapping),	//[1603h.03h], Название: RPDO4-3 Mapping, ТекстПУ: RPM43, Группа: CAN
(long)(&co2_vars.co_RPDO4_4Mapping),	//[1603h.04h], Название: RPDO4-4 Mapping, ТекстПУ: RPM44, Группа: CAN
(long)(&co2_vars.co_RPDO5_1Mapping),	//[1604h.01h], Название: RPDO5-1 Mapping, ТекстПУ: RPM51, Группа: CAN
(long)(&co2_vars.co_RPDO5_2Mapping),	//[1604h.02h], Название: RPDO5-2 Mapping, ТекстПУ: RPM52, Группа: CAN
(long)(&co2_vars.co_RPDO5_3Mapping),	//[1604h.03h], Название: RPDO5-3 Mapping, ТекстПУ: RPM53, Группа: CAN
(long)(&co2_vars.co_RPDO5_4Mapping),	//[1604h.04h], Название: RPDO5-4 Mapping, ТекстПУ: RPM54, Группа: CAN
(long)(&co2_vars.co_RPDO6_1Mapping),	//[1605h.01h], Название: RPDO6-1 Mapping, ТекстПУ: RPM61, Группа: CAN
(long)(&co2_vars.co_RPDO6_2Mapping),	//[1605h.02h], Название: RPDO6-2 Mapping, ТекстПУ: RPM62, Группа: CAN
(long)(&co2_vars.co_RPDO6_3Mapping),	//[1605h.03h], Название: RPDO6-3 Mapping, ТекстПУ: RPM63, Группа: CAN
(long)(&co2_vars.co_RPDO6_4Mapping),	//[1605h.04h], Название: RPDO6-4 Mapping, ТекстПУ: RPM64, Группа: CAN
(long)(&co2_vars.co_RPDO7_1Mapping),	//[1606h.01h], Название: RPDO7-1 Mapping, ТекстПУ: RPM71, Группа: CAN
(long)(&co2_vars.co_RPDO7_2Mapping),	//[1606h.02h], Название: RPDO7-2 Mapping, ТекстПУ: RPM72, Группа: CAN
(long)(&co2_vars.co_RPDO7_3Mapping),	//[1606h.03h], Название: RPDO7-3 Mapping, ТекстПУ: RPM73, Группа: CAN
(long)(&co2_vars.co_RPDO7_4Mapping),	//[1606h.04h], Название: RPDO7-4 Mapping, ТекстПУ: RPM74, Группа: CAN
(long)(&co2_vars.co_RPDO8_1Mapping),	//[1607h.01h], Название: RPDO8-1 Mapping, ТекстПУ: RPM81, Группа: CAN
(long)(&co2_vars.co_RPDO8_2Mapping),	//[1607h.02h], Название: RPDO8-2 Mapping, ТекстПУ: RPM82, Группа: CAN
(long)(&co2_vars.co_RPDO8_3Mapping),	//[1607h.03h], Название: RPDO8-3 Mapping, ТекстПУ: RPM83, Группа: CAN
(long)(&co2_vars.co_RPDO8_4Mapping),	//[1607h.04h], Название: RPDO8-4 Mapping, ТекстПУ: RPM84, Группа: CAN
(long)(&co2_vars.co_TPDO1COBID),	//[1800h.01h], Название: COB-ID for TPDO1, ТекстПУ: ITPDO1, Группа: CAN
(long)(&co2_vars.co_transType),	//[1800h.02h], Название: Тип передачи, ТекстПУ: TransT, Группа: CAN
(long)(&co2_vars.co_TPDO1ITime),	//[1800h.03h], Название: Inhibit Time 1, ТекстПУ: ITime1, Группа: CAN
(long)(&co2_vars.co_compatEntry),	//[1800h.04h], Название: Compatibility Entry, ТекстПУ: CompEn, Группа: CAN
(long)(&co2_vars.co_TPDO1EventTimer),	//[1800h.05h], Название: Event Timer 1, ТекстПУ: EvTmr1, Группа: CAN
(long)(&co2_vars.co_TPDO2COBID),	//[1801h.01h], Название: COB-ID for TPDO2, ТекстПУ: ITPDO2, Группа: CAN
(long)(&co2_vars.co_transType),	//[1801h.02h], Название: Тип передачи, ТекстПУ: TransT, Группа: CAN
(long)(&co2_vars.co_TPDO2ITime),	//[1801h.03h], Название: Inhibit Time 2, ТекстПУ: ITime2, Группа: CAN
(long)(&co2_vars.co_compatEntry),	//[1801h.04h], Название: Compatibility Entry, ТекстПУ: CompEn, Группа: CAN
(long)(&co2_vars.co_TPDO2EventTimer),	//[1801h.05h], Название: Event Timer 2, ТекстПУ: EvTmr2, Группа: CAN
(long)(&co2_vars.co_TPDO3COBID),	//[1802h.01h], Название: COB-ID for TPDO3, ТекстПУ: ITPDO3, Группа: CAN
(long)(&co2_vars.co_transType),	//[1802h.02h], Название: Тип передачи, ТекстПУ: TransT, Группа: CAN
(long)(&co2_vars.co_TPDO3ITime),	//[1802h.03h], Название: Inhibit Time 3, ТекстПУ: ITime3, Группа: CAN
(long)(&co2_vars.co_compatEntry),	//[1802h.04h], Название: Compatibility Entry, ТекстПУ: CompEn, Группа: CAN
(long)(&co2_vars.co_TPDO3EventTimer),	//[1802h.05h], Название: Event Timer 3, ТекстПУ: EvTmr3, Группа: CAN
(long)(&co2_vars.co_TPDO4COBID),	//[1803h.01h], Название: COB-ID for TPDO4, ТекстПУ: ITPDO4, Группа: CAN
(long)(&co2_vars.co_transType),	//[1803h.02h], Название: Тип передачи, ТекстПУ: TransT, Группа: CAN
(long)(&co2_vars.co_TPDO4ITime),	//[1803h.03h], Название: Inhibit Time 4, ТекстПУ: ITime4, Группа: CAN
(long)(&co2_vars.co_compatEntry),	//[1803h.04h], Название: Compatibility Entry, ТекстПУ: CompEn, Группа: CAN
(long)(&co2_vars.co_TPDO4EventTimer),	//[1803h.05h], Название: Event Timer 4, ТекстПУ: EvTmr4, Группа: CAN
(long)(&co2_vars.co_TPDO5COBID),	//[1804h.01h], Название: COB-ID for TPDO5, ТекстПУ: ITPDO5, Группа: CAN
(long)(&co2_vars.co_transType),	//[1804h.02h], Название: Тип передачи, ТекстПУ: TransT, Группа: CAN
(long)(&co2_vars.co_TPDO5ITime),	//[1804h.03h], Название: Inhibit Time 5, ТекстПУ: ITime5, Группа: CAN
(long)(&co2_vars.co_compatEntry),	//[1804h.04h], Название: Compatibility Entry, ТекстПУ: CompEn, Группа: CAN
(long)(&co2_vars.co_TPDO5EventTimer),	//[1804h.05h], Название: Event Timer 5, ТекстПУ: EvTmr5, Группа: CAN
(long)(&co2_vars.co_TPDO6COBID),	//[1805h.01h], Название: COB-ID for TPDO6, ТекстПУ: ITPDO6, Группа: CAN
(long)(&co2_vars.co_transType),	//[1805h.02h], Название: Тип передачи, ТекстПУ: TransT, Группа: CAN
(long)(&co2_vars.co_TPDO6ITime),	//[1805h.03h], Название: Inhibit Time 6, ТекстПУ: ITime6, Группа: CAN
(long)(&co2_vars.co_compatEntry),	//[1805h.04h], Название: Compatibility Entry, ТекстПУ: CompEn, Группа: CAN
(long)(&co2_vars.co_TPDO6EventTimer),	//[1805h.05h], Название: Event Timer 6, ТекстПУ: EvTmr6, Группа: CAN
(long)(&co2_vars.co_TPDO7COBID),	//[1806h.01h], Название: COB-ID for TPDO7, ТекстПУ: ITPDO7, Группа: CAN
(long)(&co2_vars.co_transType),	//[1806h.02h], Название: Тип передачи, ТекстПУ: TransT, Группа: CAN
(long)(&co2_vars.co_TPDO7ITime),	//[1806h.03h], Название: Inhibit Time 7, ТекстПУ: ITime7, Группа: CAN
(long)(&co2_vars.co_compatEntry),	//[1806h.04h], Название: Compatibility Entry, ТекстПУ: CompEn, Группа: CAN
(long)(&co2_vars.co_TPDO7EventTimer),	//[1806h.05h], Название: Event Timer 7, ТекстПУ: EvTmr7, Группа: CAN
(long)(&co2_vars.co_TPDO8COBID),	//[1807h.01h], Название: COB-ID for TPDO8, ТекстПУ: ITPDO8, Группа: CAN
(long)(&co2_vars.co_transType),	//[1807h.02h], Название: Тип передачи, ТекстПУ: TransT, Группа: CAN
(long)(&co2_vars.co_TPDO8ITime),	//[1807h.03h], Название: Inhibit Time 8, ТекстПУ: ITime8, Группа: CAN
(long)(&co2_vars.co_compatEntry),	//[1807h.04h], Название: Compatibility Entry, ТекстПУ: CompEn, Группа: CAN
(long)(&co2_vars.co_TPDO8EventTimer),	//[1807h.05h], Название: Event Timer 8, ТекстПУ: EvTmr8, Группа: CAN
(long)(&co2_vars.co_TPDO1_1Mapping),	//[1A00h.01h], Название: TPDO1-1 Mapping, ТекстПУ: TPM11, Группа: CAN
(long)(&co2_vars.co_TPDO1_2Mapping),	//[1A00h.02h], Название: TPDO1-2 Mapping, ТекстПУ: TPM12, Группа: CAN
(long)(&co2_vars.co_TPDO1_3Mapping),	//[1A00h.03h], Название: TPDO1-3 Mapping, ТекстПУ: TPM13, Группа: CAN
(long)(&co2_vars.co_TPDO1_4Mapping),	//[1A00h.04h], Название: TPDO1-4 Mapping, ТекстПУ: TPM14, Группа: CAN
(long)(&co2_vars.co_TPDO2_1Mapping),	//[1A01h.01h], Название: TPDO2-1 Mapping, ТекстПУ: TPM21, Группа: CAN
(long)(&co2_vars.co_TPDO2_2Mapping),	//[1A01h.02h], Название: TPDO2-2 Mapping, ТекстПУ: TPM22, Группа: CAN
(long)(&co2_vars.co_TPDO2_3Mapping),	//[1A01h.03h], Название: TPDO2-3 Mapping, ТекстПУ: TPM23, Группа: CAN
(long)(&co2_vars.co_TPDO2_4Mapping),	//[1A01h.04h], Название: TPDO2-4 Mapping, ТекстПУ: TPM24, Группа: CAN
(long)(&co2_vars.co_TPDO3_1Mapping),	//[1A02h.01h], Название: TPDO3-1 Mapping, ТекстПУ: TPM31, Группа: CAN
(long)(&co2_vars.co_TPDO3_2Mapping),	//[1A02h.02h], Название: TPDO3-2 Mapping, ТекстПУ: TPM32, Группа: CAN
(long)(&co2_vars.co_TPDO3_3Mapping),	//[1A02h.03h], Название: TPDO3-3 Mapping, ТекстПУ: TPM33, Группа: CAN
(long)(&co2_vars.co_TPDO3_4Mapping),	//[1A02h.04h], Название: TPDO3-4 Mapping, ТекстПУ: TPM34, Группа: CAN
(long)(&co2_vars.co_TPDO4_1Mapping),	//[1A03h.01h], Название: TPDO4-1 Mapping, ТекстПУ: TPM41, Группа: CAN
(long)(&co2_vars.co_TPDO4_2Mapping),	//[1A03h.02h], Название: TPDO4-2 Mapping, ТекстПУ: TPM42, Группа: CAN
(long)(&co2_vars.co_TPDO4_3Mapping),	//[1A03h.03h], Название: TPDO4-3 Mapping, ТекстПУ: TPM43, Группа: CAN
(long)(&co2_vars.co_TPDO4_4Mapping),	//[1A03h.04h], Название: TPDO4-4 Mapping, ТекстПУ: TPM44, Группа: CAN
(long)(&co2_vars.co_TPDO5_1Mapping),	//[1A04h.01h], Название: TPDO5-1 Mapping, ТекстПУ: TPM51, Группа: CAN
(long)(&co2_vars.co_TPDO5_2Mapping),	//[1A04h.02h], Название: TPDO5-2 Mapping, ТекстПУ: TPM52, Группа: CAN
(long)(&co2_vars.co_TPDO5_3Mapping),	//[1A04h.03h], Название: TPDO5-3 Mapping, ТекстПУ: TPM53, Группа: CAN
(long)(&co2_vars.co_TPDO5_4Mapping),	//[1A04h.04h], Название: TPDO5-4 Mapping, ТекстПУ: TPM54, Группа: CAN
(long)(&co2_vars.co_TPDO6_1Mapping),	//[1A05h.01h], Название: TPDO6-1 Mapping, ТекстПУ: TPM61, Группа: CAN
(long)(&co2_vars.co_TPDO6_2Mapping),	//[1A05h.02h], Название: TPDO6-2 Mapping, ТекстПУ: TPM62, Группа: CAN
(long)(&co2_vars.co_TPDO6_3Mapping),	//[1A05h.03h], Название: TPDO6-3 Mapping, ТекстПУ: TPM63, Группа: CAN
(long)(&co2_vars.co_TPDO6_4Mapping),	//[1A05h.04h], Название: TPDO6-4 Mapping, ТекстПУ: TPM64, Группа: CAN
(long)(&co2_vars.co_TPDO7_1Mapping),	//[1A06h.01h], Название: TPDO7-1 Mapping, ТекстПУ: TPM71, Группа: CAN
(long)(&co2_vars.co_TPDO7_2Mapping),	//[1A06h.02h], Название: TPDO7-2 Mapping, ТекстПУ: TPM72, Группа: CAN
(long)(&co2_vars.co_TPDO7_3Mapping),	//[1A06h.03h], Название: TPDO7-3 Mapping, ТекстПУ: TPM73, Группа: CAN
(long)(&co2_vars.co_TPDO7_4Mapping),	//[1A06h.04h], Название: TPDO7-4 Mapping, ТекстПУ: TPM74, Группа: CAN
(long)(&co2_vars.co_TPDO8_1Mapping),	//[1A07h.01h], Название: TPDO8-1 Mapping, ТекстПУ: TPM81, Группа: CAN
(long)(&co2_vars.co_TPDO8_2Mapping),	//[1A07h.02h], Название: TPDO8-2 Mapping, ТекстПУ: TPM82, Группа: CAN
(long)(&co2_vars.co_TPDO8_3Mapping),	//[1A07h.03h], Название: TPDO8-3 Mapping, ТекстПУ: TPM83, Группа: CAN
(long)(&co2_vars.co_TPDO8_4Mapping),	//[1A07h.04h], Название: TPDO8-4 Mapping, ТекстПУ: TPM84, Группа: CAN
(long)(&co2_vars.co_nodeID),	//[2000h.00h], Название: CAN NODE-ID, ТекстПУ: NodeID, Группа: CAN
(long)(&co2_vars.co_bitRate),	//[2001h.00h], Название: CAN Bit Rate, ТекстПУ: CANBR, Группа: CAN, биты с 0 по 2: 0000000000000111b
(long)(&co2_vars.co_vendorID),	//[2002h.01h], Название: Код производителя, ТекстПУ: VendID, Группа: CAN
(long)(&co2_vars.co_productCode),	//[2002h.02h], Название: Код продукта, ТекстПУ: PrCode, Группа: CAN
(long)(&co2_vars.co_revisionNumber),	//[2002h.03h], Название: Номер ревизии, ТекстПУ: RevNum, Группа: CAN
(long)(&co2_vars.co_serialNumber),	//[2002h.04h], Название: Серийный номер, ТекстПУ: SerNum, Группа: CAN
(long)(&co2_vars.co_specialData1),	//[2003h.01h], Название: Special Data 1, ТекстПУ: SpDat1, Группа: CAN
(long)(&co2_vars.co_specialData2),	//[2003h.02h], Название: Special Data 2, ТекстПУ: SpDat2, Группа: CAN
(long)(&co2_vars.co_specialData3),	//[2003h.03h], Название: Special Data 3, ТекстПУ: SpDat3, Группа: CAN
(long)(&co2_vars.co_specialData4),	//[2003h.04h], Название: Special Data 4, ТекстПУ: SpDat4, Группа: CAN
(long)(&co2_vars.co_secretCode),	//[2004h.00h], Название: Secret Code, ТекстПУ: Secret, Группа: CAN
(long)(&co2_vars.co_protectBit),	//[2005h.00h], Название: Protect Bit, ТекстПУ: ProtBt, Группа: CAN, биты с 0 по 0: 0000000000000001b
(long)(&co2_odHash[0]),	//[2007h.01h], Название: md5_hash_0, ТекстПУ: md5_hash_0, Группа: CAN
(long)(&co2_odHash[1]),	//[2007h.02h], Название: md5_hash_1, ТекстПУ: md5_hash_1, Группа: CAN
(long)(&co2_odHash[2]),	//[2007h.03h], Название: md5_hash_2, ТекстПУ: md5_hash_2, Группа: CAN
(long)(&co2_odHash[3]),	//[2007h.04h], Название: md5_hash_3, ТекстПУ: md5_hash_3, Группа: CAN
(long)(&co2_vars.co_heartbeatFlag0),	//[2010h.01h], Название: Heartbeat Flag 0-31, ТекстПУ: HFlag0, Группа: CAN
(long)(&co2_vars.co_heartbeatFlag1),	//[2010h.02h], Название: Heartbeat Flag 32-63, ТекстПУ: HFlag1, Группа: CAN
(long)(&co2_vars.co_heartbeatFlag2),	//[2010h.03h], Название: Heartbeat Flag 64-95, ТекстПУ: HFlag2, Группа: CAN
(long)(&co2_vars.co_heartbeatFlag3),	//[2010h.04h], Название: Heartbeat Flag 96-127, ТекстПУ: HFlag3, Группа: CAN
(long)(&co2_vars.co_heartbeatMask0),	//[2011h.01h], Название: Heartbeat Mask 0-31, ТекстПУ: HMask0, Группа: CAN
(long)(&co2_vars.co_heartbeatMask1),	//[2011h.02h], Название: Heartbeat Mask 32-63, ТекстПУ: HMask1, Группа: CAN
(long)(&co2_vars.co_heartbeatMask2),	//[2011h.03h], Название: Heartbeat Mask 64-95, ТекстПУ: HMask2, Группа: CAN
(long)(&co2_vars.co_heartbeatMask3),	//[2011h.04h], Название: Heartbeat Mask 96-127, ТекстПУ: HMask3, Группа: CAN
(long)(&co2_vars.co_heartbeatAutoStart),	//[2012h.00h], Название: Heartbeat Autostart, ТекстПУ: HBASta, Группа: CAN, биты с 0 по 0: 0000000000000001b
(long)(&co2_vars.co_heartbeatAutoRecovery),	//[2014h.00h], Название: Heartbeat Autorecovery, ТекстПУ: HBARec, Группа: CAN, биты с 0 по 0: 0000000000000001b
(long)(&co2_vars.co_nodeState),	//[2015h.00h], Название: Состояние драйвера CAN-Open, ТекстПУ: State, Группа: CAN
(long)(&co2_vars.co_emergencyErrorCode),	//[2016h.00h], Название: Emergency Error Code, ТекстПУ: EmErCo, Группа: CAN
(long)(&co2_vars.co_deviceErrorState),	//[2017h.00h], Название: Device Error State, ТекстПУ: DeErSt, Группа: CAN
(long)(&co2_vars.co_ODCommand),	//[2080h.01h], Название: Object Dictionary Command, ТекстПУ: ODComm, Группа: CAN
(long)(&co2_vars.co_currentODIndex),	//[2080h.02h], Название: Current OD Index, ТекстПУ: ODCInd, Группа: CAN
(long)(&co2_vars.co_currentODSubIndex),	//[2080h.03h], Название: Current OD Sub-Index, ТекстПУ: ODCSub, Группа: CAN
(long)(&co2_vars.co_currentODEText),	//[2080h.04h], Название: Current OD Element Text, ТекстПУ: ODCTxt, Группа: CAN
(long)(&co2_vars.co_currentODEFormat),	//[2080h.05h], Название: Current OD Element Format, ТекстПУ: ODCFrm, Группа: CAN
(long)(&co2_vars.co_currentODEMin),	//[2080h.06h], Название: Current OD Element Min, ТекстПУ: ODCMin, Группа: CAN
(long)(&co2_vars.co_currentODEMax),	//[2080h.07h], Название: Current OD Element Max, ТекстПУ: ODCMax, Группа: CAN
(long)(&co2_vars.co_currentODEDefault),	//[2080h.08h], Название: Current OD Element Default, ТекстПУ: ODCDef, Группа: CAN
(long)(&co2_vars.co_currentODEMinLow),	//[2080h.09h], Название: Current OD Element MinLow, ТекстПУ: ODCMiL, Группа: CAN
(long)(&co2_vars.co_currentODEMaxLow),	//[2080h.0Ah], Название: Current OD Element MaxLow, ТекстПУ: ODCMaL, Группа: CAN
(long)(&co2_vars.co_currentODEDefaultLow),	//[2080h.0Bh], Название: Current OD Element Default Low, ТекстПУ: ODCDeL, Группа: CAN
(long)(&co2_vars.co_currentODEAddrLow),	//[2080h.0Ch], Название: Current OD Element Address, ТекстПУ: ODCAdr, Группа: CAN
(long)(&co2_vars.co_currentODEAddrLow),	//[2080h.0Dh], Название: Current OD Element Address, ТекстПУ: ODCAdr, Группа: CAN
(long)(&co2_vars.co_currentODEType),	//[2080h.0Eh], Название: Группа параметров, ТекстПУ: Группа, Группа: CAN
(long)(&co2_vars.co_odIndexSize),	//[2081h.00h], Название: Количество индексов, ТекстПУ: КолИнд, Группа: CAN
(long)(&co2_vars.co_defaultIndex1),	//[2082h.01h], Название: Default Index 1, ТекстПУ: DfInd1, Группа: CAN
(long)(&co2_vars.co_defaultIndex2),	//[2082h.02h], Название: Default Index 2, ТекстПУ: DfInd2, Группа: CAN
(long)(&co2_vars.co_maskElement01),	//[2083h.01h], Название: Mask Element, ТекстПУ: MskEl0, Группа: CAN, биты с 0 по 7: 0000000011111111b
(long)(&co2_vars.co_maskElement01),	//[2083h.02h], Название: Mask Element, ТекстПУ: MskEl1, Группа: CAN, биты с 8 по 15: 1111111100000000b
(long)(&co2_vars.co_maskElement23),	//[2083h.03h], Название: Mask Element, ТекстПУ: MskEl2, Группа: CAN, биты с 0 по 7: 0000000011111111b
(long)(&co2_vars.co_maskElement23),	//[2083h.04h], Название: Mask Element, ТекстПУ: MskEl3, Группа: CAN, биты с 8 по 15: 1111111100000000b
(long)(&co2_vars.co_maskElement45),	//[2083h.05h], Название: Mask Element, ТекстПУ: MskEl4, Группа: CAN, биты с 0 по 7: 0000000011111111b
(long)(&co2_vars.co_maskElement45),	//[2083h.06h], Название: Mask Element, ТекстПУ: MskEl5, Группа: CAN, биты с 8 по 15: 1111111100000000b
(long)(&co2_vars.co_maskElement67),	//[2083h.07h], Название: Mask Element, ТекстПУ: MskEl6, Группа: CAN, биты с 0 по 7: 0000000011111111b
(long)(&co2_vars.co_maskElement67),	//[2083h.08h], Название: Mask Element, ТекстПУ: MskEl7, Группа: CAN, биты с 8 по 15: 1111111100000000b
(long)(&co2_vars.co_maskElement89),	//[2083h.09h], Название: Mask Element, ТекстПУ: MskEl8, Группа: CAN, биты с 0 по 7: 0000000011111111b
(long)(&co2_vars.co_maskElement89),	//[2083h.0Ah], Название: Mask Element, ТекстПУ: MskEl9, Группа: CAN, биты с 8 по 15: 1111111100000000b
(long)(&co2_vars.co_maskElementAB),	//[2083h.0Bh], Название: Mask Element, ТекстПУ: MskElA, Группа: CAN, биты с 0 по 7: 0000000011111111b
(long)(&co2_vars.co_maskElementAB),	//[2083h.0Ch], Название: Mask Element, ТекстПУ: MskElB, Группа: CAN, биты с 8 по 15: 1111111100000000b
(long)(&co2_vars.co_maskElementCD),	//[2083h.0Dh], Название: Mask Element, ТекстПУ: MskElC, Группа: CAN, биты с 0 по 7: 0000000011111111b
(long)(&co2_vars.co_maskElementCD),	//[2083h.0Eh], Название: Mask Element, ТекстПУ: MskElD, Группа: CAN, биты с 8 по 15: 1111111100000000b
(long)(&co2_vars.co_maskElementEF),	//[2083h.0Fh], Название: Mask Element, ТекстПУ: MskElE, Группа: CAN, биты с 0 по 7: 0000000011111111b
(long)(&co2_vars.co_maskElementEF),	//[2083h.10h], Название: Mask Element, ТекстПУ: MskElF, Группа: CAN, биты с 8 по 15: 1111111100000000b
(long)(&co2_vars.co_scaleNum0),	//[2100h.01h], Название: Scale Number 0, ТекстПУ: SN0, Группа: CAN
(long)(&co2_vars.co_scaleNum0Format),	//[2100h.02h], Название: Scale Number 0 Format, ТекстПУ: S0Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum0Format),	//[2100h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum0Format),	//[2100h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum0Format),	//[2100h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum0Format),	//[2100h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum1),	//[2101h.01h], Название: Scale Number 1, ТекстПУ: SN1, Группа: CAN
(long)(&co2_vars.co_scaleNum1Format),	//[2101h.02h], Название: Scale Number 1 Format, ТекстПУ: S1Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum1Format),	//[2101h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum1Format),	//[2101h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum1Format),	//[2101h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum1Format),	//[2101h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum2),	//[2102h.01h], Название: Scale Number 2, ТекстПУ: SN2, Группа: CAN
(long)(&co2_vars.co_scaleNum2Format),	//[2102h.02h], Название: Scale Number 2 Format, ТекстПУ: S2Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum2Format),	//[2102h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum2Format),	//[2102h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum2Format),	//[2102h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum2Format),	//[2102h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum3),	//[2103h.01h], Название: Scale Number 3, ТекстПУ: SN3, Группа: CAN
(long)(&co2_vars.co_scaleNum3Format),	//[2103h.02h], Название: Scale Number 3 Format, ТекстПУ: S3Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum3Format),	//[2103h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum3Format),	//[2103h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum3Format),	//[2103h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum3Format),	//[2103h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum4),	//[2104h.01h], Название: Scale Number 4, ТекстПУ: SN4, Группа: CAN
(long)(&co2_vars.co_scaleNum4Format),	//[2104h.02h], Название: Scale Number 4 Format, ТекстПУ: S4Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum4Format),	//[2104h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum4Format),	//[2104h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum4Format),	//[2104h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum4Format),	//[2104h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum5),	//[2105h.01h], Название: Scale Number 5, ТекстПУ: SN5, Группа: CAN
(long)(&co2_vars.co_scaleNum5Format),	//[2105h.02h], Название: Scale Number 5 Format, ТекстПУ: S5Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum5Format),	//[2105h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum5Format),	//[2105h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum5Format),	//[2105h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum5Format),	//[2105h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum6),	//[2106h.01h], Название: Scale Number 6, ТекстПУ: SN6, Группа: CAN
(long)(&co2_vars.co_scaleNum6Format),	//[2106h.02h], Название: Scale Number 6 Format, ТекстПУ: S6Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum6Format),	//[2106h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum6Format),	//[2106h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum6Format),	//[2106h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum6Format),	//[2106h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum7),	//[2107h.01h], Название: Scale Number 7, ТекстПУ: SN7, Группа: CAN
(long)(&co2_vars.co_scaleNum7Format),	//[2107h.02h], Название: Scale Number 7 Format, ТекстПУ: S7Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum7Format),	//[2107h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum7Format),	//[2107h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum7Format),	//[2107h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum7Format),	//[2107h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum8),	//[2108h.01h], Название: Scale Number 8, ТекстПУ: SN8, Группа: CAN
(long)(&co2_vars.co_scaleNum8Format),	//[2108h.02h], Название: Scale Number 8 Format, ТекстПУ: S8Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum8Format),	//[2108h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum8Format),	//[2108h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum8Format),	//[2108h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum8Format),	//[2108h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum9),	//[2109h.01h], Название: Scale Number 9, ТекстПУ: SN9, Группа: CAN
(long)(&co2_vars.co_scaleNum9Format),	//[2109h.02h], Название: Scale Number 9 Format, ТекстПУ: S9Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum9Format),	//[2109h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum9Format),	//[2109h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum9Format),	//[2109h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum9Format),	//[2109h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNumA),	//[210Ah.01h], Название: Scale Number A, ТекстПУ: SNA, Группа: CAN
(long)(&co2_vars.co_scaleNumAFormat),	//[210Ah.02h], Название: Scale Number A Format, ТекстПУ: SAFmt, Группа: CAN
(long)(&co2_vars.co_scaleNumAFormat),	//[210Ah.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNumAFormat),	//[210Ah.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNumAFormat),	//[210Ah.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNumAFormat),	//[210Ah.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNumB),	//[210Bh.01h], Название: Scale Number B, ТекстПУ: SNB, Группа: CAN
(long)(&co2_vars.co_scaleNumBFormat),	//[210Bh.02h], Название: Scale Number B Format, ТекстПУ: SBFmt, Группа: CAN
(long)(&co2_vars.co_scaleNumBFormat),	//[210Bh.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNumBFormat),	//[210Bh.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNumBFormat),	//[210Bh.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNumBFormat),	//[210Bh.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNumC),	//[210Ch.01h], Название: Scale Number C, ТекстПУ: SNC, Группа: CAN
(long)(&co2_vars.co_scaleNumCFormat),	//[210Ch.02h], Название: Scale Number C Format, ТекстПУ: SCFmt, Группа: CAN
(long)(&co2_vars.co_scaleNumCFormat),	//[210Ch.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNumCFormat),	//[210Ch.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNumCFormat),	//[210Ch.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNumCFormat),	//[210Ch.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNumD),	//[210Dh.01h], Название: Scale Number D, ТекстПУ: SND, Группа: CAN
(long)(&co2_vars.co_scaleNumDFormat),	//[210Dh.02h], Название: Scale Number D Format, ТекстПУ: SDFmt, Группа: CAN
(long)(&co2_vars.co_scaleNumDFormat),	//[210Dh.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNumDFormat),	//[210Dh.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNumDFormat),	//[210Dh.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNumDFormat),	//[210Dh.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNumE),	//[210Eh.01h], Название: Scale Number E, ТекстПУ: SNE, Группа: CAN
(long)(&co2_vars.co_scaleNumEFormat),	//[210Eh.02h], Название: Scale Number E Format, ТекстПУ: SEFmt, Группа: CAN
(long)(&co2_vars.co_scaleNumEFormat),	//[210Eh.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNumEFormat),	//[210Eh.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNumEFormat),	//[210Eh.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNumEFormat),	//[210Eh.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNumF),	//[210Fh.01h], Название: Scale Number F, ТекстПУ: SNF, Группа: CAN
(long)(&co2_vars.co_scaleNumFFormat),	//[210Fh.02h], Название: Scale Number F Format, ТекстПУ: SFFmt, Группа: CAN
(long)(&co2_vars.co_scaleNumFFormat),	//[210Fh.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNumFFormat),	//[210Fh.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNumFFormat),	//[210Fh.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNumFFormat),	//[210Fh.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum10),	//[2110h.01h], Название: Scale Number 10, ТекстПУ: SN10, Группа: CAN
(long)(&co2_vars.co_scaleNum10Format),	//[2110h.02h], Название: Scale Number 10 Format, ТекстПУ: S10Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum10Format),	//[2110h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum10Format),	//[2110h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum10Format),	//[2110h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum10Format),	//[2110h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum11),	//[2111h.01h], Название: Scale Number 11, ТекстПУ: SN11, Группа: CAN
(long)(&co2_vars.co_scaleNum11Format),	//[2111h.02h], Название: Scale Number 11 Format, ТекстПУ: S11Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum11Format),	//[2111h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum11Format),	//[2111h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum11Format),	//[2111h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum11Format),	//[2111h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum12),	//[2112h.01h], Название: Scale Number 12, ТекстПУ: SN12, Группа: CAN
(long)(&co2_vars.co_scaleNum12Format),	//[2112h.02h], Название: Scale Number 12 Format, ТекстПУ: S12Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum12Format),	//[2112h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum12Format),	//[2112h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum12Format),	//[2112h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum12Format),	//[2112h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum13),	//[2113h.01h], Название: Scale Number 13, ТекстПУ: SN13, Группа: CAN
(long)(&co2_vars.co_scaleNum13Format),	//[2113h.02h], Название: Scale Number 13 Format, ТекстПУ: S13Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum13Format),	//[2113h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum13Format),	//[2113h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum13Format),	//[2113h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum13Format),	//[2113h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum14),	//[2114h.01h], Название: Scale Number 14, ТекстПУ: SN14, Группа: CAN
(long)(&co2_vars.co_scaleNum14Format),	//[2114h.02h], Название: Scale Number 14 Format, ТекстПУ: S14Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum14Format),	//[2114h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum14Format),	//[2114h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum14Format),	//[2114h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum14Format),	//[2114h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum15),	//[2115h.01h], Название: Scale Number 15, ТекстПУ: SN15, Группа: CAN
(long)(&co2_vars.co_scaleNum15Format),	//[2115h.02h], Название: Scale Number 15Format, ТекстПУ: S15Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum15Format),	//[2115h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum15Format),	//[2115h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum15Format),	//[2115h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum15Format),	//[2115h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum16),	//[2116h.01h], Название: Scale Number 16, ТекстПУ: SN16, Группа: CAN
(long)(&co2_vars.co_scaleNum16Format),	//[2116h.02h], Название: Scale Number 16Format, ТекстПУ: S16Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum16Format),	//[2116h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum16Format),	//[2116h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum16Format),	//[2116h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum16Format),	//[2116h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum17),	//[2117h.01h], Название: Scale Number 17, ТекстПУ: SN17, Группа: CAN
(long)(&co2_vars.co_scaleNum17Format),	//[2117h.02h], Название: Scale Number 17Format, ТекстПУ: S17Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum17Format),	//[2117h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum17Format),	//[2117h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum17Format),	//[2117h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum17Format),	//[2117h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum18),	//[2118h.01h], Название: Scale Number 18, ТекстПУ: SN18, Группа: CAN
(long)(&co2_vars.co_scaleNum18Format),	//[2118h.02h], Название: Scale Number 18Format, ТекстПУ: S18Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum18Format),	//[2118h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum18Format),	//[2118h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum18Format),	//[2118h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum18Format),	//[2118h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum19),	//[2119h.01h], Название: Scale Number 19, ТекстПУ: SN19, Группа: CAN
(long)(&co2_vars.co_scaleNum19Format),	//[2119h.02h], Название: Scale Number 19Format, ТекстПУ: S19Fmt, Группа: CAN
(long)(&co2_vars.co_scaleNum19Format),	//[2119h.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum19Format),	//[2119h.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum19Format),	//[2119h.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum19Format),	//[2119h.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum1A),	//[211Ah.01h], Название: Scale Number 1A, ТекстПУ: SN1A, Группа: CAN
(long)(&co2_vars.co_scaleNum1AFormat),	//[211Ah.02h], Название: Scale Number 1AFormat, ТекстПУ: S1AFmt, Группа: CAN
(long)(&co2_vars.co_scaleNum1AFormat),	//[211Ah.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum1AFormat),	//[211Ah.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum1AFormat),	//[211Ah.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum1AFormat),	//[211Ah.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum1B),	//[211Bh.01h], Название: Scale Number 1B, ТекстПУ: SN1B, Группа: CAN
(long)(&co2_vars.co_scaleNum1BFormat),	//[211Bh.02h], Название: Scale Number 1BFormat, ТекстПУ: S1BFmt, Группа: CAN
(long)(&co2_vars.co_scaleNum1BFormat),	//[211Bh.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum1BFormat),	//[211Bh.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum1BFormat),	//[211Bh.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum1BFormat),	//[211Bh.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum1C),	//[211Ch.01h], Название: Scale Number 1C, ТекстПУ: SN1C, Группа: CAN
(long)(&co2_vars.co_scaleNum1CFormat),	//[211Ch.02h], Название: Scale Number 1C Format, ТекстПУ: S1CFmt, Группа: CAN
(long)(&co2_vars.co_scaleNum1CFormat),	//[211Ch.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum1CFormat),	//[211Ch.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum1CFormat),	//[211Ch.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum1CFormat),	//[211Ch.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&co2_vars.co_scaleNum1D),	//[211Dh.01h], Название: Scale Number 1D, ТекстПУ: SN1D, Группа: CAN
(long)(&co2_vars.co_scaleNum1DFormat),	//[211Dh.02h], Название: Scale Number 1D Format, ТекстПУ: S1DFmt, Группа: CAN
(long)(&co2_vars.co_scaleNum1DFormat),	//[211Dh.03h], Название: Scale Number Format Q, ТекстПУ: Q-type, Группа: CAN, биты с 0 по 4: 0000000000011111b
(long)(&co2_vars.co_scaleNum1DFormat),	//[211Dh.04h], Название: Scale Number Format NumOfFloat, ТекстПУ: NFloat, Группа: CAN, биты с 5 по 6: 0000000001100000b
(long)(&co2_vars.co_scaleNum1DFormat),	//[211Dh.05h], Название: Scale Number Format Prefix, ТекстПУ: Prefix, Группа: CAN, биты с 7 по 9: 0000001110000000b
(long)(&co2_vars.co_scaleNum1DFormat),	//[211Dh.06h], Название: Scale Number Format Units, ТекстПУ: Units, Группа: CAN, биты с 10 по 15: 1111110000000000b
(long)(&cmd.all),	//[2502h.01h], Название: Пуск, ТекстПУ: Пуск, Группа: Слово управления, биты с 0 по 0: 0000000000000001b
(long)(&cmd.all),	//[2502h.02h], Название: Стоп, ТекстПУ: Стоп, Группа: Слово управления, биты с 1 по 1: 0000000000000010b
(long)(&cmd.all),	//[2502h.03h], Название: Сбросить состояние Аварии, ТекстПУ: СбрАвр, Группа: Слово управления, биты с 2 по 2: 0000000000000100b
(long)(&cmd.all),	//[2502h.04h], Название: Слово управления CAN, ТекстПУ: КмдCAN, Группа: Слово управления
(long)(&drv_interface.req_data),	//[2506h.01h], Название: Таймаут потери связи, ТекстПУ: OfflineCounterMax, Группа: ВНЕ ГРУПП
(long)(&drv_interface.ans_data),	//[2506h.02h], Название: Счетчик потери связи, ТекстПУ: OfflineCounter, Группа: ВНЕ ГРУПП
(long)(&drv_interface.data_Low),	//[2506h.03h], Название: data_Low, ТекстПУ: data_L, Группа: ВНЕ ГРУПП
(long)(&drv_interface.data_High),	//[2506h.04h], Название: data_High, ТекстПУ: data_H, Группа: ВНЕ ГРУПП
(long)(&RTCclock.packed_time),	//[250Fh.00h], Название: Часы реального времени, ТекстПУ: RTC, Группа: Наблюдаемые
(long)(&sm_prot.Main_ErrorCode),	//[2532h.01h], Название: Код аварии ARM, ТекстПУ: Код аварии ARM, Группа: Рег. тока якоря, биты с 0 по 7: 0000000011111111b
(long)(&sm_prot.Main_ErrorCode),	//[2532h.02h], Название: Расширение кода аварии, ТекстПУ: Авр+, Группа: Рег. тока якоря, биты с 8 по 15: 1111111100000000b
(long)(&sm_prot.Main_Flags),	//[2532h.03h], Название: Системный объект, ТекстПУ: System, Группа: Рег. тока якоря, биты с 0 по 0: 0000000000000001b
(long)(&disp_group_number),	//[2534h.01h], Название: Номер груп для пульта управления, ТекстПУ: № гр ПУ, Группа: CAN
(long)(&VendorToken),	//[2600h.01h], Название: VendorToken, ТекстПУ: VendorToken, Группа: ВНЕ ГРУПП
(long)(&co2_vars.co_blockTransferCommand),	//[2700h.00h], Название: Block Transfer Interpreter, ТекстПУ: BT Interpreter, Группа: Слово управления
(long)(&swu_vars),	//[2701h.01h], Название: cmd, ТекстПУ: cmd, Группа: Слово управления
(long)(&swu_vars),	//[2701h.02h], Название: status, ТекстПУ: status, Группа: Слово управления
(long)(&swu_vars),	//[2701h.03h], Название: quest1, ТекстПУ: quest1, Группа: Слово управления
(long)(&swu_vars),	//[2701h.04h], Название: quest2, ТекстПУ: quest2, Группа: Слово управления
(long)(&swu_vars),	//[2701h.05h], Название: quest3, ТекстПУ: quest3, Группа: Слово управления
(long)(&swu_vars),	//[2701h.06h], Название: quest4, ТекстПУ: quest4, Группа: Слово управления
(long)(&swu_vars),	//[2701h.07h], Название: ans1, ТекстПУ: ans1, Группа: Слово управления
(long)(&swu_vars),	//[2701h.08h], Название: ans2, ТекстПУ: ans2, Группа: Слово управления
(long)(&swu_vars),	//[2701h.09h], Название: ans3, ТекстПУ: ans3, Группа: Слово управления
(long)(&swu_vars),	//[2701h.0Ah], Название: ans4, ТекстПУ: ans4, Группа: Слово управления
(long)(&swu_vars),	//[2701h.0Bh], Название: addr, ТекстПУ: addr, Группа: Слово управления
(long)(&swu_vars),	//[2701h.0Ch], Название: CPU_ID, ТекстПУ: CPU_ID, Группа: Слово управления
(long)(&swu_vars),	//[2701h.0Dh], Название: sector_mask, ТекстПУ: sector_mask, Группа: Слово управления
(long)(&swu_vars),	//[2701h.0Eh], Название: version, ТекстПУ: version, Группа: Слово управления
(long)(&swu_vars),	//[2701h.0Fh], Название: CRC, ТекстПУ: CRC, Группа: Слово управления
(long)(&drv_status.all),	//[3000h.00h], Название: Статус преобразователя, ТекстПУ: Статус, Группа: ВНЕ ГРУПП, биты с 0 по 5: 0000000000111111b
(long)(&msCounter),	//[3100h.01h], Название: Счётчик прерываний 1 кГц, ТекстПУ: Сч.прерыв.1 кГц, Группа: Счетчики
(long)(&FastCounter),	//[3100h.02h], Название: Счётчик прерываний 10 кГц, ТекстПУ: Сч.прерыв.10 кГц, Группа: Счетчики
(long)(&LoopCounter),	//[3100h.03h], Название: BackgroundCounter, ТекстПУ: BackgroundCounter, Группа: Счетчики
(long)(&TIsr10),	//[3100h.04h], Название: T_10кГц, ТекстПУ: T_10кГц, Группа: Счетчики
(long)(&TIsr1),	//[3100h.05h], Название: T_1кГц, ТекстПУ: T_1кГц, Группа: Счетчики
(long)(&rmp.T),	//[5000h.01h], Название: T разгона до номинала, ТекстПУ: rmpT, Группа: ЗИ
(long)(&rmp.input),	//[5000h.02h], Название: Вход ЗИ, ТекстПУ: ВходЗИ, Группа: ЗИ
(long)(&rmp.output),	//[5000h.03h], Название: Выход ЗИ, ТекстПУ: ВыходЗИ, Группа: ЗИ
(long)(&pid_id.pid_ref_reg3),	//[5100h.01h], Название: Задание, ТекстПУ: Зад-е, Группа: Рег. тока Id
(long)(&pid_id.pid_fdb_reg3),	//[5100h.02h], Название: Обратная связь, ТекстПУ: Обр.Св, Группа: Рег. тока Id
(long)(&pid_id.Kp_reg3),	//[5100h.03h], Название: Пропорц. коэфф. Kp, ТекстПУ: Kp, Группа: Рег. тока Id
(long)(&pid_id.pid_out_max),	//[5100h.04h], Название: Максимум выхода, ТекстПУ: Max, Группа: Рег. тока Id
(long)(&pid_id.pid_out_min),	//[5100h.05h], Название: Минимум выхода, ТекстПУ: Min, Группа: Рег. тока Id
(long)(&pid_id.Ki_reg3),	//[5100h.06h], Название: Интегральный коэфф. Ki, ТекстПУ: Ki, Группа: Рег. тока Id
(long)(&pid_id.Kd_reg3),	//[5100h.07h], Название: Дифф. коэфф. Kd, ТекстПУ: Kd, Группа: Рег. тока Id
(long)(&pid_id.Kc_reg3),	//[5100h.08h], Название: Интегр.корректир.коэффициент, ТекстПУ: Kc, Группа: Рег. тока Id
(long)(&pid_id.e_reg3),	//[5100h.09h], Название: Ошибка рассогласования сигналов, ТекстПУ: Ошибка, Группа: Рег. тока Id
(long)(&pid_id.pid_out_reg3),	//[5100h.0Ah], Название: Выход регулятора, ТекстПУ: Выход, Группа: Рег. тока Id
(long)(&pid_id.up_reg3),	//[5100h.0Bh], Название: Вых.Пропорц., ТекстПУ: Вых.Пропорц., Группа: Рег. тока Id
(long)(&pid_id.ui_reg3),	//[5100h.0Ch], Название: Вых.Интегр., ТекстПУ: Вых.Интегр., Группа: Рег. тока Id
(long)(&pid_id.ud_reg3),	//[5100h.0Dh], Название: Вых.Диффер., ТекстПУ: Вых.Диффер., Группа: Рег. тока Id
(long)(&pid_spd.pid_ref_reg3),	//[5101h.01h], Название: Задание, ТекстПУ: pisref, Группа: Рег. скорости
(long)(&pid_spd.pid_fdb_reg3),	//[5101h.02h], Название: Обратная связь, ТекстПУ: pisfbd, Группа: Рег. скорости
(long)(&pid_spd.Kp_reg3),	//[5101h.03h], Название: Пропорц. коэфф. Kp, ТекстПУ: Kp, Группа: Рег. скорости
(long)(&pid_spd.pid_out_max),	//[5101h.04h], Название: Максимум выхода, ТекстПУ: Max, Группа: Рег. скорости
(long)(&pid_spd.pid_out_min),	//[5101h.05h], Название: Минимум выхода, ТекстПУ: Min, Группа: Рег. скорости
(long)(&pid_spd.Ki_reg3),	//[5101h.06h], Название: Интегральный коэфф. Ki, ТекстПУ: Ki, Группа: Рег. скорости
(long)(&pid_spd.Kd_reg3),	//[5101h.07h], Название: Дифф. коэфф. Kd, ТекстПУ: pisKd, Группа: Рег. скорости
(long)(&pid_spd.Kc_reg3),	//[5101h.08h], Название: Интегр.корректир.коэффициент, ТекстПУ: Kc, Группа: Рег. скорости
(long)(&pid_spd.e_reg3),	//[5101h.09h], Название: Ошибка рассогласования сигналов, ТекстПУ: Ошибка, Группа: Рег. скорости
(long)(&pid_spd.pid_out_reg3),	//[5101h.0Ah], Название: Выход регулятора, ТекстПУ: pisout, Группа: Рег. скорости
(long)(&pid_spd.up_reg3),	//[5101h.0Bh], Название: Вых.Пропорц., ТекстПУ: Вых.Пропорц., Группа: Рег. скорости
(long)(&pid_spd.ui_reg3),	//[5101h.0Ch], Название: Вых.Интегр., ТекстПУ: Вых.Интегр., Группа: Рег. скорости
(long)(&pid_spd.ud_reg3),	//[5101h.0Dh], Название: Вых.Диффер., ТекстПУ: Вых.Диффер., Группа: Рег. скорости
(long)(&pid_spd.saterr_reg3),	//[5101h.0Eh], Название: Превышение насыщения, ТекстПУ: Прев. насыщ, Группа: Рег. скорости
(long)(&pid_iq.pid_ref_reg3),	//[5102h.01h], Название: Задание, ТекстПУ: Зад-ие, Группа: Рег. тока Iq
(long)(&pid_iq.pid_fdb_reg3),	//[5102h.02h], Название: Обратная связь, ТекстПУ: Обр.Св, Группа: Рег. тока Iq
(long)(&pid_iq.Kp_reg3),	//[5102h.03h], Название: Пропорц. коэфф. Kp, ТекстПУ: Kp, Группа: Рег. тока Iq
(long)(&pid_iq.pid_out_max),	//[5102h.04h], Название: Максимум выхода, ТекстПУ: Max, Группа: Рег. тока Iq
(long)(&pid_iq.pid_out_min),	//[5102h.05h], Название: Минимум выхода, ТекстПУ: Min, Группа: Рег. тока Iq
(long)(&pid_iq.Ki_reg3),	//[5102h.06h], Название: Интегральный коэфф. Ki, ТекстПУ: Ki, Группа: Рег. тока Iq
(long)(&pid_iq.Kd_reg3),	//[5102h.07h], Название: Дифф. коэфф. Kd, ТекстПУ: Kd, Группа: Рег. тока Iq
(long)(&pid_iq.Kc_reg3),	//[5102h.08h], Название: Интегр.корректир.коэффициент, ТекстПУ: Kc, Группа: Рег. тока Iq
(long)(&pid_iq.e_reg3),	//[5102h.09h], Название: Ошибка рассогласования сигналов, ТекстПУ: Ошибка, Группа: Рег. тока Iq
(long)(&pid_iq.pid_out_reg3),	//[5102h.0Ah], Название: Выход регулятора, ТекстПУ: Выход, Группа: Рег. тока Iq
(long)(&pid_iq.up_reg3),	//[5102h.0Bh], Название: Вых.Пропорц., ТекстПУ: Вых.Пропорц., Группа: Рег. тока Iq
(long)(&pid_iq.ui_reg3),	//[5102h.0Ch], Название: Вых.Интегр., ТекстПУ: Вых.Интегр., Группа: Рег. тока Iq
(long)(&pid_iq.ud_reg3),	//[5102h.0Dh], Название: Вых.Диффер., ТекстПУ: Вых.Диффер., Группа: Рег. тока Iq
(long)(&pid_pos.pid_ref_reg3),	//[5103h.01h], Название: Задание, ТекстПУ: pisref, Группа: Рег. положения
(long)(&pid_pos.pid_ref_reg3),	//[5103h.02h], Название: Задание, ТекстПУ: pisref, Группа: Рег. положения
(long)(&pid_pos.pid_fdb_reg3),	//[5103h.03h], Название: Обратная связь, ТекстПУ: pisfbd, Группа: Рег. положения
(long)(&pid_pos.pid_fdb_reg3),	//[5103h.04h], Название: Обратная связь, ТекстПУ: pisfbd, Группа: Рег. положения
(long)(&pid_pos.Kp_reg3),	//[5103h.05h], Название: Пропорц. коэфф. Kp, ТекстПУ: Kp, Группа: Рег. положения
(long)(&pid_pos.pid_out_max),	//[5103h.06h], Название: Максимум выхода, ТекстПУ: Max, Группа: Рег. положения
(long)(&pid_pos.pid_out_min),	//[5103h.07h], Название: Минимум выхода, ТекстПУ: Min, Группа: Рег. положения
(long)(&pid_pos.Ki_reg3),	//[5103h.08h], Название: Интегральный коэфф. Ki, ТекстПУ: Ki, Группа: Рег. положения
(long)(&pid_pos.Kd_reg3),	//[5103h.09h], Название: Дифф. коэфф. Kd, ТекстПУ: pisKd, Группа: Рег. положения
(long)(&pid_pos.DiffDelim),	//[5103h.0Ah], Название: Делит. дифф. части, ТекстПУ: Делит. дифф. части, Группа: Рег. положения
(long)(&pid_pos.Kf_d),	//[5103h.0Bh], Название: Коэф. фильтра, ТекстПУ: Коэф. фильтра, Группа: Рег. положения
(long)(&pid_pos.Kc_reg3),	//[5103h.0Ch], Название: Интегр.корректир.коэффициент, ТекстПУ: Kc, Группа: Рег. положения
(long)(&pid_pos.DeadZone),	//[5103h.0Dh], Название: Мертвая зона, ТекстПУ: Мертвая зона, Группа: Рег. положения
(long)(&pid_pos.e_reg3),	//[5103h.0Eh], Название: Ошибка рассогласования сигналов, ТекстПУ: Ошибка, Группа: Рег. положения
(long)(&pid_spd.saterr_reg3),	//[5103h.0Fh], Название: Превышение насыщения, ТекстПУ: Прев. насыщ, Группа: Рег. положения
(long)(&pid_pos.pid_out_reg3),	//[5103h.10h], Название: Выход регулятора, ТекстПУ: pisout, Группа: Рег. положения
(long)(&pid_pos.up_reg3),	//[5103h.11h], Название: Вых.Пропорц., ТекстПУ: Вых.Пропорц., Группа: Рег. положения
(long)(&pid_pos.ui_reg3),	//[5103h.12h], Название: Вых.Интегр., ТекстПУ: Вых.Интегр., Группа: Рег. положения
(long)(&pid_pos.ud_reg3),	//[5103h.13h], Название: Вых.Диффер., ТекстПУ: Вых.Диффер., Группа: Рег. положения
(long)(&pid_pos.e_reg3_filterOut),	//[5103h.14h], Название: Фильтр ошибки, ТекстПУ: Фильтр ошибки, Группа: Рег. положения
(long)(&pid_ia.pid_ref_reg3),	//[5104h.01h], Название: Задание, ТекстПУ: Зад-ие, Группа: Рег. тока якоря
(long)(&pid_ia.pid_fdb_reg3),	//[5104h.02h], Название: Обратная связь, ТекстПУ: Обр.Св, Группа: Рег. тока якоря
(long)(&pid_ia.Kp_reg3),	//[5104h.03h], Название: Пропорц. коэфф. Kp, ТекстПУ: Kp, Группа: Рег. тока якоря
(long)(&pid_ia.pid_out_max),	//[5104h.04h], Название: Максимум выхода, ТекстПУ: Max, Группа: Рег. тока якоря
(long)(&pid_ia.pid_out_min),	//[5104h.05h], Название: Минимум выхода, ТекстПУ: Min, Группа: Рег. тока якоря
(long)(&pid_ia.Ki_reg3),	//[5104h.06h], Название: Интегральный коэфф. Ki, ТекстПУ: Ki, Группа: Рег. тока якоря
(long)(&pid_ia.Kd_reg3),	//[5104h.07h], Название: Дифф. коэфф. Kd, ТекстПУ: Kd, Группа: Рег. тока якоря
(long)(&pid_ia.Kc_reg3),	//[5104h.08h], Название: Интегр.корректир.коэффициент, ТекстПУ: Kc, Группа: Рег. тока якоря
(long)(&pid_ia.e_reg3),	//[5104h.09h], Название: Ошибка рассогласования сигналов, ТекстПУ: Ошибка, Группа: Рег. тока якоря
(long)(&pid_ia.pid_out_reg3),	//[5104h.0Ah], Название: Выход регулятора, ТекстПУ: Выход, Группа: Рег. тока якоря
(long)(&pid_ia.up_reg3),	//[5104h.0Bh], Название: Вых.Пропорц., ТекстПУ: Вых.Пропорц., Группа: Рег. тока якоря
(long)(&pid_ia.ui_reg3),	//[5104h.0Ch], Название: Вых.Интегр., ТекстПУ: Вых.Интегр., Группа: Рег. тока якоря
(long)(&pid_ia.ud_reg3),	//[5104h.0Dh], Название: Вых.Диффер., ТекстПУ: Вых.Диффер., Группа: Рег. тока якоря
(long)(&pwm.UalphaRef),	//[5105h.01h], Название: UaЗад, ТекстПУ: UaЗад, Группа: ШИМ
(long)(&pwm.UbetaRef),	//[5105h.02h], Название: UbЗад, ТекстПУ: UbЗад, Группа: ШИМ
(long)(&pwm.U_lim),	//[5105h.03h], Название: U_огр, ТекстПУ: U_огр, Группа: ШИМ
(long)(&pwm.GammaA),	//[5105h.04h], Название: GammaA, ТекстПУ: GmA, Группа: ШИМ
(long)(&pwm.GammaB),	//[5105h.05h], Название: GammaB, ТекстПУ: GmB, Группа: ШИМ
(long)(&pwm.GammaC),	//[5105h.06h], Название: GammaC, ТекстПУ: GmC, Группа: ШИМ
(long)(&pwm.k_pwm),	//[5105h.07h], Название: TPWM, ТекстПУ: TPWM, Группа: ШИМ
(long)(&pwm.DeadBand),	//[5105h.08h], Название: Мертвое время (мкс), ТекстПУ: Мерт. время, Группа: ШИМ
(long)(&pwm.MinGammaLimit),	//[5105h.09h], Название: Ограничение мин. скважн., ТекстПУ: GamLim, Группа: ШИМ
(long)(&pwm.UdCompK),	//[5105h.0Ah], Название: Коэф. компенсации Ud, ТекстПУ: UdКомпK, Группа: ШИМ
(long)(&pwm.UdCompEnable),	//[5105h.0Bh], Название: Компенсация Ud, ТекстПУ: UdКомп, Группа: ШИМ, биты с 0 по 0: 0000000000000001b
(long)(&pwm.PWM_type),	//[5105h.0Ch], Название: Тип ШИМ, ТекстПУ: ТипШИМ, Группа: ШИМ
(long)(&pwm.ULimitation),	//[5105h.0Dh], Название: ULimitation, ТекстПУ: ULimitation, Группа: ШИМ
(long)(&pwm.Frequency),	//[5105h.0Eh], Название: Частота ШИМ, ТекстПУ: Fшим, Группа: ШИМ
(long)(&pwm.sector),	//[5105h.0Fh], Название: Сектор, ТекстПУ: Сектор, Группа: ШИМ
(long)(&pwm.U_mag),	//[5105h.10h], Название: U_ампл, ТекстПУ: U_ампл, Группа: ШИМ
(long)(&pwm.PDP_Fault),	//[5105h.11h], Название: Флаги аварий, ТекстПУ: F_bits, Группа: ШИМ
(long)(&sm_ctrl.state),	//[510Bh.01h], Название: sm_state, ТекстПУ: smstat, Группа: ДА sm_ctrl
(long)(&drv_status_code),	//[510Bh.02h], Название: drv_status_code, ТекстПУ: stcode, Группа: ДА sm_ctrl, биты с 0 по 7: 0000000011111111b
(long)(&sm_prot.bit_fault1),	//[510Eh.01h], Название: Флаги аварий 1, ТекстПУ: bitF_1, Группа: Защиты
(long)(&sm_prot.bit_fault2),	//[510Eh.02h], Название: Флаги аварий 2, ТекстПУ: bitF_2, Группа: Защиты
(long)(&sm_prot.mask_fault1),	//[510Eh.03h], Название: Маска аварий 1L, ТекстПУ: mask1L, Группа: Защиты, биты с 0 по 7: 0000000011111111b
(long)(&sm_prot.mask_fault1),	//[510Eh.04h], Название: Маска аварий 1H, ТекстПУ: mask1H, Группа: Защиты, биты с 8 по 15: 1111111100000000b
(long)(&sm_prot.mask_fault2),	//[510Eh.05h], Название: Маска аварий 2L, ТекстПУ: mask2L, Группа: Защиты, биты с 0 по 7: 0000000011111111b
(long)(&sm_prot.mask_fault2),	//[510Eh.06h], Название: Маска аварий  2H, ТекстПУ: mask2H, Группа: Защиты, биты с 8 по 15: 1111111100000000b
(long)(&sm_prot.Imax_protect),	//[510Eh.07h], Название: Максимальный ток, ТекстПУ: Макс.Ток, Группа: Защиты
(long)(&sm_prot.Umax_protect),	//[510Eh.08h], Название: Максимальное напряжение, ТекстПУ: Ud_max, Группа: Защиты
(long)(&sm_prot.Umin_protect),	//[510Eh.09h], Название: Минимальное напряжение, ТекстПУ: Ud_min, Группа: Защиты
(long)(&sm_prot.speed_max),	//[510Eh.0Ah], Название: Максимальная скорость, ТекстПУ: speed_max, Группа: Защиты
(long)(&sm_prot.T_max),	//[510Eh.0Bh], Название: Максимальная температура, ТекстПУ: t_max, Группа: Защиты
(long)(&refs.speed_ref),	//[5112h.01h], Название: Скорость, ТекстПУ: Скор_з, Группа: Задания
(long)(&refs.Iq_ref),	//[5112h.02h], Название: Макс. ток ВУ, ТекстПУ: ТокСТз, Группа: Задания
(long)(&refs.i_flux_ref),	//[5112h.03h], Название: Ток удержания, ТекстПУ: ТокУПз, Группа: Задания
(long)(&refs.theta_elec),	//[5112h.04h], Название: theta_elec, ТекстПУ: элекУг, Группа: Задания
(long)(&refs.uf_ref),	//[5112h.05h], Название: Заданное напряжение ОВ, ТекстПУ: Uf_ref, Группа: Задания
(long)(&drv_params.p),	//[5113h.01h], Название: Число пар полюсов, ТекстПУ: ПарПол, Группа: Базов. парам
(long)(&drv_params.I_nom),	//[5113h.02h], Название: Номинальный ток, ТекстПУ: IпчНом, Группа: Базов. парам
(long)(&drv_params.U_nom),	//[5113h.03h], Название: Номинальное напряжение фазн., ТекстПУ: Uф_ном, Группа: Базов. парам
(long)(&drv_params.speed_nom),	//[5113h.04h], Название: Номинальная скорость, ТекстПУ: СкоростьНом, Группа: Базов. парам
(long)(&drv_params.Udc_nom),	//[5113h.05h], Название: Номинальное напряжение ЗПТ, ТекстПУ: UdcНом, Группа: Базов. парам
(long)(&drv_params.Rs),	//[5113h.06h], Название: Rs, ТекстПУ: Rs, Группа: Базов. парам
(long)(&drv_params.Ls),	//[5113h.07h], Название: Ls, ТекстПУ: Ls, Группа: Базов. парам
(long)(&drv_params.freq_nom),	//[5113h.08h], Название: Номинальная частота, ТекстПУ: Ном.част., Группа: Базов. парам
(long)(&drv_params.sens_type),	//[5113h.09h], Название: Тип датчика положения, ТекстПУ: Sens_type, Группа: Базов. парам, биты с 0 по 2: 0000000000000111b
(long)(&sw.HardwareType),	//[5114h.01h], Название: Тип аппаратной части, ТекстПУ: HardwareType, Группа: Настройки СУ, биты с 0 по 7: 0000000011111111b
(long)(&sm_ctrl.run_mode),	//[5114h.02h], Название: Режим работы, ТекстПУ: РежРаб, Группа: Настройки СУ
(long)(&sw.recuperation_ena),	//[5114h.03h], Название: Рекуперация, ТекстПУ: Рекупр, Группа: Настройки СУ
(long)(&sw.AutoOffset),	//[5114h.04h], Название: АЦП авто смещ., ТекстПУ: АЦПсмещ, Группа: Настройки СУ
(long)(&sw.Reboot),	//[5114h.05h], Название: Перезагрузка, ТекстПУ: Перезагрузка, Группа: Настройки СУ
(long)(&sw.excitation_ena),	//[5114h.06h], Название: Возбудитель, ТекстПУ: Возб, Группа: Настройки СУ
(long)(&adc.Imeas_a_gain),	//[5116h.01h], Название: Коэффициент тока фазы А, ТекстПУ: IaУсил, Группа: АЦП
(long)(&adc.Imeas_a_offset),	//[5116h.02h], Название: Смещение тока фазы А, ТекстПУ: IaСмещ, Группа: АЦП
(long)(&adc.Imeas_a),	//[5116h.03h], Название: Ток фазы А, ТекстПУ: Ia_мгн, Группа: АЦП
(long)(&adc.Imeas_b_gain),	//[5116h.04h], Название: Коэффициент тока фазы В, ТекстПУ: IbУсил, Группа: АЦП
(long)(&adc.Imeas_b_offset),	//[5116h.05h], Название: Смещение тока фазы В, ТекстПУ: IbСмещ, Группа: АЦП
(long)(&adc.Imeas_b),	//[5116h.06h], Название: Ток фазы В, ТекстПУ: Ib_мгн, Группа: АЦП
(long)(&adc.Imeas_c_gain),	//[5116h.07h], Название: Коэффициент тока фазы C, ТекстПУ: IcУсил, Группа: АЦП
(long)(&adc.Imeas_c_offset),	//[5116h.08h], Название: Смещение тока фазы C, ТекстПУ: IcСмещ, Группа: АЦП
(long)(&adc.Imeas_c),	//[5116h.09h], Название: Ток фазы С, ТекстПУ: Ic_мгн, Группа: АЦП
(long)(&adc.Imeas_d_gain),	//[5116h.0Ah], Название: Коэффициент тока фазы D, ТекстПУ: IdУсил, Группа: АЦП
(long)(&adc.Imeas_d_offset),	//[5116h.0Bh], Название: Смещение тока фазы D, ТекстПУ: IdСмещ, Группа: АЦП
(long)(&adc.Imeas_d),	//[5116h.0Ch], Название: Ток фазы D, ТекстПУ: Id_мгн, Группа: АЦП
(long)(&adc.Udc_meas_gain),	//[5116h.0Dh], Название: Коэффициент напряжения ЗПТ, ТекстПУ: UdУсил, Группа: АЦП
(long)(&adc.Udc_meas_offset),	//[5116h.0Eh], Название: Смещение напряж. ЗПТ, ТекстПУ: Udсмещ, Группа: АЦП
(long)(&adc.Udc_meas),	//[5116h.0Fh], Название: Напряжение ЗПТ, ТекстПУ: Ud, Группа: АЦП
(long)(&adc.T_meas_gain),	//[5116h.10h], Название: Коэффициент температуры, ТекстПУ: t_gain, Группа: АЦП
(long)(&adc.T_meas),	//[5116h.11h], Название: Температура, ТекстПУ: t, Группа: АЦП
(long)(&adc.ref_meas_gain),	//[5116h.12h], Название: Коэффициент внешнего входа, ТекстПУ: ADC_ext_gain, Группа: АЦП
(long)(&adc.ref_meas_offset),	//[5116h.13h], Название: Смещение внешнего входа, ТекстПУ: ADC_ext_offset, Группа: АЦП
(long)(&adc.ref_meas),	//[5116h.14h], Название: Внешний аналоговый вход 1, ТекстПУ: ADC_ext1, Группа: АЦП
(long)(&vhz.freq),	//[511Ah.01h], Название: Выходная частота, ТекстПУ: F_вых, Группа: Кривая U-f
(long)(&vhz.vout),	//[511Ah.02h], Название: Выходное напряжение, ТекстПУ: U_вых, Группа: Кривая U-f
(long)(&vhz.FL),	//[511Ah.03h], Название: Нулевая опроная частота, ТекстПУ: f [0], Группа: Кривая U-f
(long)(&vhz.Vmin),	//[511Ah.04h], Название: Нулевое опорное напряжение, ТекстПУ: U [0], Группа: Кривая U-f
(long)(&vhz.FH),	//[511Ah.05h], Название: Первая опорная частота, ТекстПУ: f [1], Группа: Кривая U-f
(long)(&vhz.Vmax),	//[511Ah.06h], Название: Первое опорное напряжение, ТекстПУ: U [1], Группа: Кривая U-f
(long)(&vhz.Fmax),	//[511Ah.07h], Название: Максимальная частота, ТекстПУ: f max, Группа: Кривая U-f
(long)(&cur_par.speed),	//[5138h.01h], Название: Скорость, ТекстПУ: скор, Группа: Наблюдаемые
(long)(&cur_par.power),	//[5138h.02h], Название: Акт. мощность, ТекстПУ: АктМощ, Группа: Наблюдаемые
(long)(&cur_par.Is),	//[5138h.03h], Название: Ток статора, ТекстПУ: ТокСт, Группа: Наблюдаемые
(long)(&cur_par.ThetaRefCurr),	//[5138h.04h], Название: ЭлУголЗад, ТекстПУ: ЭлУголЗад, Группа: Наблюдаемые
(long)(&cur_par.ThetaCurr),	//[5138h.05h], Название: ЭлУгол, ТекстПУ: ЭлУгол, Группа: Наблюдаемые
(long)(&cur_par.IsRef),	//[5138h.06h], Название: Ток статора зад, ТекстПУ: ТокСтЗад, Группа: Наблюдаемые
(long)(&global_time.PowerOn_time_min),	//[5138h.07h], Название: Время запитанного состояния, мин, ТекстПУ: Power On Time, Группа: Наблюдаемые
(long)(&global_time.operational_time_min),	//[5138h.08h], Название: Вреря работы привода, мин, ТекстПУ: Work Time, Группа: Наблюдаемые
(long)(&dlog.mode_reset),	//[5150h.01h], Название: dlog.mode_reset, ТекстПУ: mode, Группа: dlog
(long)(&dlog.control),	//[5150h.02h], Название: dlog.control, ТекстПУ: dlcont, Группа: dlog
(long)(&dlog.ind_subind1),	//[5150h.03h], Название: dlog.ind_subind1, ТекстПУ: dlisi1, Группа: dlog
(long)(&dlog.ind_subind2),	//[5150h.04h], Название: dlog.ind_subind2, ТекстПУ: dlisi2, Группа: dlog
(long)(&dlog.ind_subind3),	//[5150h.05h], Название: dlog.ind_subind3, ТекстПУ: dlisi3, Группа: dlog
(long)(&dlog.ind_subind4),	//[5150h.06h], Название: dlog.ind_subind4, ТекстПУ: dlisi4, Группа: dlog
(long)(&dlog.StartBits),	//[5150h.07h], Название: StartBitL, ТекстПУ: StrtBL, Группа: dlog, биты с 0 по 7: 0000000011111111b
(long)(&dlog.StartBits),	//[5150h.08h], Название: StartBitH, ТекстПУ: StrtBH, Группа: dlog, биты с 8 по 15: 1111111100000000b
(long)(&dlog.OneShotOperation),	//[5150h.09h], Название: OneShotOperation, ТекстПУ: OneShotOp, Группа: dlog, биты с 0 по 0: 0000000000000001b
(long)(&dlog.trig_shift),	//[5150h.0Ah], Название: Количество точек предыстории, ТекстПУ: N предыст., Группа: dlog
(long)(&dlog.sampling_rate),	//[5150h.0Bh], Название: Частота даталогера, ТекстПУ: dlogFreq, Группа: dlog
(long)(&dlog.next_value_var),	//[5151h.00h], Название: dlog.next_value_var, ТекстПУ: dlnval, Группа: ВНЕ ГРУПП
(long)(&posspeedEqep.resol),	//[5152h.01h], Название: Кол-во периодов, ТекстПУ: Кол-во периодов, Группа: ДПР-Энкодер
(long)(&posspeedEqep.Posspeed_CTL.all),	//[5152h.02h], Название: Наличие репера, ТекстПУ: Наличие репера, Группа: ДПР-Энкодер, биты с 2 по 2: 0000000000000100b
(long)(&posspeedEqep.SpeedCalcType),	//[5152h.03h], Название: Рассчитывать скорость программно, ТекстПУ: SpeedCalcType, Группа: ДПР-Энкодер
(long)(&posspeedEqep.AngleOffset),	//[5152h.04h], Название: СмещУгол, ТекстПУ: СмещУг, Группа: ДПР-Энкодер
(long)(&posspeedEqep.Poscnt_res),	//[5152h.05h], Название: Механическ.угол в метках, ТекстПУ: Механическ.угол в ме, Группа: ДПР-Энкодер
(long)(&posspeedEqep.Poscnt_res16),	//[5152h.06h], Название: Механическ.угол в метках, ТекстПУ: Механическ.угол в ме, Группа: ДПР-Энкодер
(long)(&posspeedEqep.speed_elec),	//[5152h.07h], Название: Скорость , ТекстПУ: Скорость, Группа: ДПР-Энкодер
(long)(&posspeedEqep.theta_elec),	//[5152h.08h], Название: Угол. электр., ТекстПУ: Угол. электр., Группа: ДПР-Энкодер
(long)(&posspeedEqep.theta_mech),	//[5152h.09h], Название: Угол мех., ТекстПУ: Угол мех., Группа: ДПР-Энкодер
(long)(&posspeedEqep.Posspeed_CTL.all),	//[5152h.0Ah], Название: Команда иниц., ТекстПУ: Команда иниц., Группа: ДПР-Энкодер, биты с 4 по 4: 0000000000010000b
(long)(&posspeedEqep.speed_filter.T),	//[5152h.0Bh], Название: К быстродействия фильтра скорости, ТекстПУ: Кф. скорости, Группа: ДПР-Энкодер
(long)(&posspeedEqep.Posspeed_FLG1.all),	//[5152h.0Ch], Название: Posspeed_FLG1, ТекстПУ: Posspeed_FLG1, Группа: ДПР-Энкодер
(long)(&posspeedEqep.GPIOsValue),	//[5152h.0Dh], Название: GPIOsValue, ТекстПУ: GPIOsValue, Группа: ДПР-Энкодер
(long)(&posspeedEqep.UPPS_forWatch),	//[5152h.0Eh], Название: UPPS, ТекстПУ: UPPS, Группа: ДПР-Энкодер
(long)(&posspeedEqep.Qcprdlat_tmp),	//[5152h.0Fh], Название: QCPRD, ТекстПУ: QCPRD, Группа: ДПР-Энкодер
(long)(&posspeedEqep.Qcprdlat16_tmp),	//[5152h.10h], Название: QCPRD, ТекстПУ: QCPRD, Группа: ДПР-Энкодер
(long)(&posspeedEqep.QEPSTS),	//[5152h.11h], Название: QEPSTS, ТекстПУ: QEPSTS, Группа: ДПР-Энкодер
(long)(&posspeedEqep.RevolutionCounter),	//[5152h.12h], Название: Число оборотов, ТекстПУ: revolCounter, Группа: ДПР-Энкодер
(long)(&posspeedEqep.Poscnt_resContinouosInt8),	//[5152h.13h], Название: Абс. положение в метках, ТекстПУ: qcAbsPos, Группа: ДПР-Энкодер
(long)(&posspeedEqep.Poscnt_resContinouosInt),	//[5152h.14h], Название: Абс. положение в метках, ТекстПУ: qcAbsPos, Группа: ДПР-Энкодер
(long)(&posspeedEqep.Poscnt_resContinouosLong),	//[5152h.15h], Название: Абс. положение в метках, ТекстПУ: qcAbsPos, Группа: ДПР-Энкодер
(long)(&posspeedEqep.theta_mechContinouos),	//[5152h.16h], Название: Угол механический не огранич., ТекстПУ: Уг.мех.не_огр, Группа: ДПР-Энкодер
(long)(&posspeedEqep.theta_elecContinouos),	//[5152h.17h], Название: Угол электрический не огранич., ТекстПУ: Уг.эл.не_огр, Группа: ДПР-Энкодер
(long)(&DPReCAP.Angle),	//[5155h.01h], Название: ЭлУгол, ТекстПУ: ЭлУгол, Группа: ДПР-Холл
(long)(&DPReCAP.speed),	//[5155h.02h], Название: Скорость , ТекстПУ: Скорость, Группа: ДПР-Холл
(long)(&DPReCAP.speedMIN),	//[5155h.03h], Название: Уставка Скорости, ТекстПУ: Уставка Скорости, Группа: ДПР-Холл
(long)(&DPReCAP.Angle6),	//[5155h.04h], Название: ЭлУгол6, ТекстПУ: ЭлУгол6, Группа: ДПР-Холл
(long)(&DPReCAP.DPReCAP_FLG1.all),	//[5155h.05h], Название: Инт-полятор вкл, ТекстПУ: Инт-полятор вкл, Группа: ДПР-Холл, биты с 0 по 0: 0000000000000001b
(long)(&DPReCAP.milsecREF),	//[5155h.06h], Название: Уставка обнуления скорости, ТекстПУ: Уставка обнуления ск, Группа: ДПР-Холл
(long)(&DPReCAP.speedMinREF),	//[5155h.07h], Название: Скорость Мин., ТекстПУ: Скорость Мин., Группа: ДПР-Холл
(long)(&DPReCAP.CAP_WrongEdgeCnt),	//[5155h.08h], Название: Ложные фронты, ТекстПУ: Ложные фронты, Группа: ДПР-Холл
(long)(&DPReCAP.HallCode),	//[5155h.09h], Название: Дискретные входы, ТекстПУ: Входы, Группа: ДПР-Холл, биты с 0 по 2: 0000000000000111b
(long)(&DPReCAP.AngleOffset),	//[5155h.0Ah], Название: СмещУгол, ТекстПУ: СмещУг, Группа: ДПР-Холл
(long)(&DPReCAP.ErrorLevel),	//[5155h.0Bh], Название: Уровень помех, ТекстПУ: Уровень помех, Группа: ДПР-Холл
(long)(&DPReCAP.CAP_WrongEdgeCnt1),	//[5155h.0Ch], Название: Ошибка 1 канала, ТекстПУ: Ошиб_1_к., Группа: ДПР-Холл
(long)(&DPReCAP.CAP_WrongEdgeCnt2),	//[5155h.0Dh], Название: Ошибка 2 канала, ТекстПУ: Ошиб_2_к., Группа: ДПР-Холл
(long)(&DPReCAP.CAP_WrongEdgeCnt3),	//[5155h.0Eh], Название: Ошибка 3 канала, ТекстПУ: Ошиб_3_к., Группа: ДПР-Холл
(long)(&DPReCAP.UserDirection),	//[5155h.0Fh], Название: Направление вращения, ТекстПУ: НапрВр, Группа: ДПР-Холл, биты с 2 по 2: 0000000000000100b
(long)(&DPReCAP.WrongCodeCounterLimitPerSec),	//[5155h.10h], Название: Макс.число ошибок ДПР, ТекстПУ: Макс.ош.ДПР, Группа: ДПР-Холл
(long)(&DPReCAP.WrongCodeCounter),	//[5155h.11h], Название: Кол-во ошибок неверной последовательности , ТекстПУ: Ошибки последоват., Группа: ДПР-Холл
(long)(&CounterCAP_isr),	//[5155h.12h], Название: Счётчик прерываний по приему, ТекстПУ: Счёт.Rx.Int., Группа: ДПР-Холл
(long)(&cap0_counter),	//[5155h.13h], Название: Счётчик прерываний CAP1, ТекстПУ: Сч.прерыв.CAP1, Группа: ДПР-Холл
(long)(&cap1_counter),	//[5155h.14h], Название: Счётчик прерываний CAP2, ТекстПУ: Сч.прерыв.CAP2, Группа: ДПР-Холл
(long)(&cap2_counter),	//[5155h.15h], Название: Счётчик прерываний CAP3, ТекстПУ: Сч.прерыв.CAP3, Группа: ДПР-Холл
(long)(&SSI_Encoder.resol),	//[5157h.01h], Название: Количество импульсов на оборот, ТекстПУ: qepTik, Группа: ДПР-SSI энкодер
(long)(&SSI_Encoder.AngleOffset),	//[5157h.02h], Название: СмещУгол, ТекстПУ: СмещУг, Группа: ДПР-SSI энкодер
(long)(&SSI_Encoder.Poscnt_res),	//[5157h.03h], Название: Механическ.угол в метках, ТекстПУ: Механическ.угол в ме, Группа: ДПР-SSI энкодер
(long)(&SSI_Encoder.speed_elec),	//[5157h.04h], Название: Скорость , ТекстПУ: Скорость, Группа: ДПР-SSI энкодер
(long)(&SSI_Encoder.theta_elec),	//[5157h.05h], Название: Угол. электр., ТекстПУ: Угол. электр., Группа: ДПР-SSI энкодер
(long)(&SSI_Encoder.theta_mech),	//[5157h.06h], Название: Угол мех., ТекстПУ: Угол мех., Группа: ДПР-SSI энкодер
(long)(&SSI_Encoder.theta_mech_filtered),	//[5157h.07h], Название: Угол мех. фильтрованный, ТекстПУ: theta_mech_filtered, Группа: ДПР-SSI энкодер
(long)(&SSI_Encoder.theta_mech_filterK),	//[5157h.08h], Название: К быстродействия фильтра угла, ТекстПУ: Кф. угла, Группа: ДПР-SSI энкодер
(long)(&SSI_Encoder.RevolutionCounter),	//[5157h.09h], Название: Число оборотов, ТекстПУ: revolCounter, Группа: ДПР-SSI энкодер
(long)(&SSI_Encoder.Poscnt_resContinouosLong),	//[5157h.0Ah], Название: Абс. положение в метках, ТекстПУ: qcAbsPos, Группа: ДПР-SSI энкодер
(long)(&SSI_Encoder.Poscnt_resContinouosInt8),	//[5157h.0Bh], Название: Абс. положение в метках, ТекстПУ: qcAbsPos, Группа: ДПР-SSI энкодер
(long)(&SSI_Encoder.Poscnt_resContinouosInt),	//[5157h.0Ch], Название: Абс. положение в метках, ТекстПУ: qcAbsPos, Группа: ДПР-SSI энкодер
(long)(&SSI_Encoder.theta_mechContinouos),	//[5157h.0Dh], Название: Угол механический не огранич., ТекстПУ: Уг.мех.не_огр, Группа: ДПР-SSI энкодер
(long)(&SSI_Encoder.theta_elecContinouos),	//[5157h.0Eh], Название: Угол электрический не огранич., ТекстПУ: Уг.эл.не_огр, Группа: ДПР-SSI энкодер
(long)(&Debug1),	//[5174h.01h], Название: DebugL1, ТекстПУ: DebugL1, Группа: Для отладки
(long)(&Debug1),	//[5174h.02h], Название: DebugL1, ТекстПУ: DebugL1, Группа: Для отладки
(long)(&Debug2),	//[5174h.03h], Название: DebugL2, ТекстПУ: DebugL2, Группа: Для отладки
(long)(&Debug2),	//[5174h.04h], Название: DebugL2, ТекстПУ: DebugL2, Группа: Для отладки
(long)(&Debug3),	//[5174h.05h], Название: DebugI3, ТекстПУ: DebugI3, Группа: Для отладки
(long)(&Debug3),	//[5174h.06h], Название: DebugI3, ТекстПУ: DebugI3, Группа: Для отладки
(long)(&Debug4),	//[5174h.07h], Название: DebugI4, ТекстПУ: DebugI4, Группа: Для отладки
(long)(&Debug4),	//[5174h.08h], Название: DebugI4, ТекстПУ: DebugI4, Группа: Для отладки
(long)(&DebugW1),	//[5174h.09h], Название: DebugW1, ТекстПУ: DebugW1, Группа: Для отладки
(long)(&DebugW1),	//[5174h.0Ah], Название: DebugW1, ТекстПУ: DebugW1, Группа: Для отладки
(long)(&DebugW2),	//[5174h.0Bh], Название: DebugW2, ТекстПУ: DebugW2, Группа: Для отладки
(long)(&DebugW2),	//[5174h.0Ch], Название: DebugW2, ТекстПУ: DebugW2, Группа: Для отладки
(long)(&DebugW3),	//[5174h.0Dh], Название: DebugW3, ТекстПУ: DebugW3, Группа: Для отладки
(long)(&DebugW3),	//[5174h.0Eh], Название: DebugW3, ТекстПУ: DebugW3, Группа: Для отладки
(long)(&DebugW4),	//[5174h.0Fh], Название: DebugW4, ТекстПУ: DebugW4, Группа: Для отладки
(long)(&DebugW4),	//[5174h.10h], Название: DebugW4, ТекстПУ: DebugW4, Группа: Для отладки
(long)(&DebugF1),	//[5174h.11h], Название: DebugF1, ТекстПУ: DebugF1, Группа: Для отладки
(long)(&DebugF2),	//[5174h.12h], Название: DebugF2, ТекстПУ: DebugF2, Группа: Для отладки
(long)(&DebugF3),	//[5174h.13h], Название: DebugF3, ТекстПУ: DebugF3, Группа: Для отладки
(long)(&DebugF4),	//[5174h.14h], Название: DebugF4, ТекстПУ: DebugF4, Группа: Для отладки
(long)(&model.MotorType),	//[5181h.01h], Название: Тип двигателя модели, ТекстПУ: ModelMotorType, Группа: Модель общие данные, биты с 0 по 7: 0000000011111111b
(long)(&model.MotorParametersNum),	//[5181h.02h], Название: Параметры двигателя, ТекстПУ: ПарДв, Группа: Модель общие данные
(long)(&model.load),	//[5181h.03h], Название: Симулируемый момент нагрузки, ТекстПУ: Мнагр сим., Группа: Модель общие данные
(long)(&model.motorInternals.udc),	//[5181h.04h], Название: Напряжение ЗПТ, ТекстПУ: Ud, Группа: Модель общие данные
(long)(&model.motorInternals.torque),	//[5181h.05h], Название: Реализованный электрический момент, ТекстПУ: Реал.Эл.Мом, Группа: Модель общие данные
(long)(&model.motorInternals.omega),	//[5181h.06h], Название: Скорость, ТекстПУ: qepspd, Группа: Модель общие данные
(long)(&model.motorInternals.omega_rpm),	//[5181h.07h], Название: Скорость, ТекстПУ: qepspd, Группа: Модель общие данные
(long)(&model.motorInternals.tetaRM),	//[5181h.08h], Название: Механический угол, ТекстПУ: qeptm, Группа: Модель общие данные
(long)(&model.qepCounter),	//[5181h.09h], Название: EncoderIncrements, ТекстПУ: EncoderIncrements, Группа: Модель общие данные
(long)(&model.motorInternals.isa),	//[5181h.0Ah], Название: Ток статора по оси Alpha, ТекстПУ: IsAlpha, Группа: Модель общие данные
(long)(&model.motorInternals.isb),	//[5181h.0Bh], Название: Ток статора по оси Beta, ТекстПУ: IsBeta, Группа: Модель общие данные
(long)(&model.motorInternals.isPhaseA),	//[5181h.0Ch], Название: Ток фазы А, ТекстПУ: Ia, Группа: Модель общие данные
(long)(&model.motorInternals.isPhaseB),	//[5181h.0Dh], Название: Ток фазы В, ТекстПУ: Ib_мгн, Группа: Модель общие данные
(long)(&model.motorInternals.isPhaseC),	//[5181h.0Eh], Название: Ток фазы C, ТекстПУ: Ic, Группа: Модель общие данные
(long)(&model.motorInternals.isPhaseD),	//[5181h.0Fh], Название: Ток фазы D, ТекстПУ: Id, Группа: Модель общие данные
(long)(&model.motorInternals.ua),	//[5181h.10h], Название: Напряжение фазы A, ТекстПУ: Ua, Группа: Модель общие данные
(long)(&model.motorInternals.ub),	//[5181h.11h], Название: Напряжение фазы B, ТекстПУ: Ub, Группа: Модель общие данные
(long)(&model.motorInternals.uc),	//[5181h.12h], Название: Напряжение фазы C, ТекстПУ: Uc, Группа: Модель общие данные
(long)(&model.motorInternals.power),	//[5181h.13h], Название: Мощность, ТекстПУ: Мощ-ть, Группа: Модель общие данные
(long)(&model.fault),	//[5181h.14h], Название: Код аварии, ТекстПУ: КодАвр, Группа: Модель общие данные
(long)(&model.motorInternals.lm),	//[5182h.01h], Название: Взаимная индуктивность Lm, ТекстПУ: Lm, Группа: Модель (асинхронный)
(long)(&model.motorInternals.ls),	//[5182h.02h], Название: Индуктивность статора Ls, ТекстПУ: Ls, Группа: Модель (асинхронный)
(long)(&model.motorInternals.lr),	//[5182h.03h], Название: Индуктивность ротора Lr, ТекстПУ: Lr, Группа: Модель (асинхронный)
(long)(&model.motorInternals.rs),	//[5182h.04h], Название: Сопротивление статора Rs, ТекстПУ: Rs, Группа: Модель (асинхронный)
(long)(&model.motorInternals.rr),	//[5182h.05h], Название: Сопротивление ротора Rr, ТекстПУ: Rr, Группа: Модель (асинхронный)
(long)(&model.motorInternals.pp),	//[5182h.06h], Название: Число пар полюсов, ТекстПУ: ПарПол, Группа: Модель (асинхронный)
(long)(&model.motorInternals.j),	//[5182h.07h], Название: Симулируемый момент инерции, ТекстПУ: Инерция сим., Группа: Модель (асинхронный)
(long)(&model.motorInternals.MechLoss),	//[5182h.08h], Название: Тормозной момент для выбега, ТекстПУ: Торм.Мом.Выбег, Группа: Модель (асинхронный)
(long)(&model.motorInternals.QEPResolution),	//[5182h.09h], Название: Число периодов квадратурного датчика, ТекстПУ: QEPResolution, Группа: Модель (асинхронный)
(long)(&model.motorInternals.RatedCurrent),	//[5182h.0Ah], Название: Номинальный действующий ток, ТекстПУ: RatedCurrent, Группа: Модель (асинхронный)
(long)(&model.motorInternals.RatedPower),	//[5182h.0Bh], Название: Номинальная мощность, ТекстПУ: RatedPower, Группа: Модель (асинхронный)
(long)(&model.motorInternals.RatedSpeed),	//[5182h.0Ch], Название: Номинальная скорость, ТекстПУ: СкоростьНом, Группа: Модель (асинхронный)
(long)(&model.motorInternals.iADCk),	//[5182h.0Dh], Название: Коэффициент АЦП тока фаз, ТекстПУ: Коэф. АЦП тока, Группа: Модель (асинхронный)
(long)(&model.motorInternals.isa),	//[5182h.0Eh], Название: Ток статора по оси Alpha, ТекстПУ: IsAlpha, Группа: Модель (асинхронный)
(long)(&model.motorInternals.isb),	//[5182h.0Fh], Название: Ток статора по оси Beta, ТекстПУ: IsBeta, Группа: Модель (асинхронный)
(long)(&model.motorInternals.prd),	//[5182h.10h], Название: Поток ротора по оси D, ТекстПУ: PsiRD, Группа: Модель (асинхронный)
(long)(&model.motorInternals.prq),	//[5182h.11h], Название: Поток ротора по оси Q, ТекстПУ: PsiRQ, Группа: Модель (асинхронный)
(long)(&model.motorInternals.pra),	//[5182h.12h], Название: Поток ротора по оси Alpha, ТекстПУ: PsiRAlfa, Группа: Модель (асинхронный)
(long)(&model.motorInternals.prb),	//[5182h.13h], Название: Поток ротора по оси Beta, ТекстПУ: PsiRBeta, Группа: Модель (асинхронный)
(long)(&model.motorInternals.m),	//[5183h.01h], Название: Потокосцепление магнитов, ТекстПУ: m, Группа: Модель (синхронный)
(long)(&model.motorInternals.lsd),	//[5183h.02h], Название: Индуктивность статора Lsd, ТекстПУ: Lsd, Группа: Модель (синхронный)
(long)(&model.motorInternals.lsq),	//[5183h.03h], Название: Индуктивность статора Lsq, ТекстПУ: Lsq, Группа: Модель (синхронный)
(long)(&model.motorInternals.rs),	//[5183h.04h], Название: Сопротивление статора Rs, ТекстПУ: Rs, Группа: Модель (синхронный)
(long)(&model.motorInternals.pp),	//[5183h.05h], Название: Число пар полюсов, ТекстПУ: ПарПол, Группа: Модель (синхронный)
(long)(&model.motorInternals.j),	//[5183h.06h], Название: Симулируемый момент инерции, ТекстПУ: Инерция сим., Группа: Модель (синхронный)
(long)(&model.motorInternals.MechLoss),	//[5183h.07h], Название: Тормозной момент для выбега, ТекстПУ: Торм.Мом.Выбег, Группа: Модель (синхронный)
(long)(&model.motorInternals.QEPResolution),	//[5183h.08h], Название: Число периодов квадратурного датчика, ТекстПУ: QEPResolution, Группа: Модель (синхронный)
(long)(&model.motorInternals.iADCk),	//[5183h.09h], Название: Коэффициент АЦП тока фаз, ТекстПУ: Коэф. АЦП тока, Группа: Модель (синхронный)
(long)(&model.motorInternals.ifADCk),	//[5183h.0Ah], Название: Коэффициент АЦП тока ОВ, ТекстПУ: Коэф. АЦП ток ОВ, Группа: Модель (синхронный)
(long)(&model.motorInternals.RatedCurrent),	//[5183h.0Bh], Название: Номинальный действующий ток, ТекстПУ: RatedCurrent, Группа: Модель (синхронный)
(long)(&model.motorInternals.RatedPower),	//[5183h.0Ch], Название: Номинальная мощность, ТекстПУ: RatedPower, Группа: Модель (синхронный)
(long)(&model.motorInternals.RatedSpeed),	//[5183h.0Dh], Название: Номинальная скорость, ТекстПУ: СкоростьНом, Группа: Модель (синхронный)
(long)(&model.motorInternals.RatedFluxCurrent),	//[5183h.0Eh], Название: Номинальный ток ОВ, ТекстПУ: RatedFluxCurrent, Группа: Модель (синхронный)
(long)(&model.motorInternals.u_f),	//[5183h.0Fh], Название: Разность потенциалов ОВ, ТекстПУ: Uf, Группа: Модель (синхронный)
(long)(&model.motorInternals.i_f),	//[5183h.10h], Название: Ток ОВ, ТекстПУ: Ток ОВ, Группа: Модель (синхронный)
(long)(&model.motorInternals.m),	//[5183h.11h], Название: Поток возбуждения, ТекстПУ: Psi возб, Группа: Модель (синхронный)
(long)(&model.motorInternals.l_a),	//[5184h.01h], Название: Индуктивность якоря, ТекстПУ: La, Группа: Модель (ДПТ)
(long)(&model.motorInternals.l_f),	//[5184h.02h], Название: Индуктивность ОВ, ТекстПУ: Lf, Группа: Модель (ДПТ)
(long)(&model.motorInternals.r_a),	//[5184h.03h], Название: Сопротивление якоря, ТекстПУ: Ra, Группа: Модель (ДПТ)
(long)(&model.motorInternals.r_f),	//[5184h.04h], Название: Сопротивление ОВ, ТекстПУ: Rf, Группа: Модель (ДПТ)
(long)(&model.motorInternals.j),	//[5184h.05h], Название: Симулируемый момент инерции, ТекстПУ: Инерция сим., Группа: Модель (ДПТ)
(long)(&model.motorInternals.MechLoss),	//[5184h.06h], Название: Тормозной момент для выбега, ТекстПУ: Торм.Мом.Выбег, Группа: Модель (ДПТ)
(long)(&model.motorInternals.QEPResolution),	//[5184h.07h], Название: Число периодов квадратурного датчика, ТекстПУ: QEPResolution, Группа: Модель (ДПТ)
(long)(&model.motorInternals.RatedCurrent),	//[5184h.08h], Название: Номинальный ток якоря, ТекстПУ: RatedCurrent, Группа: Модель (ДПТ)
(long)(&model.motorInternals.RatedFluxCurrent),	//[5184h.09h], Название: Номинальный ток ОВ, ТекстПУ: RatedFluxCurrent, Группа: Модель (ДПТ)
(long)(&model.motorInternals.iADCk),	//[5184h.0Ah], Название: Коэффициент АЦП тока ОЯ, ТекстПУ: Коэф. АЦП ток ОЯ, Группа: Модель (ДПТ)
(long)(&model.motorInternals.ifADCk),	//[5184h.0Bh], Название: Коэффициент АЦП тока ОВ, ТекстПУ: Коэф. АЦП ток ОВ, Группа: Модель (ДПТ)
(long)(&model.motorInternals.RatedPower),	//[5184h.0Ch], Название: Номинальная мощность, ТекстПУ: RatedPower, Группа: Модель (ДПТ)
(long)(&model.motorInternals.RatedSpeed),	//[5184h.0Dh], Название: Номинальная скорость, ТекстПУ: СкоростьНом, Группа: Модель (ДПТ)
(long)(&model.motorInternals.r_ad),	//[5184h.0Eh], Название: Добавочное сопротивление, Rad, ТекстПУ: Доб.сопротив., Группа: Модель (ДПТ)
(long)(&model.motorInternals.u_a),	//[5184h.0Fh], Название: Разность потенциалов ОЯ, ТекстПУ: Ua, Группа: Модель (ДПТ)
(long)(&model.motorInternals.u_f),	//[5184h.10h], Название: Разность потенциалов ОВ, ТекстПУ: Uf, Группа: Модель (ДПТ)
(long)(&model.motorInternals.i_a),	//[5184h.11h], Название: Ток ОЯ, ТекстПУ: Ia, Группа: Модель (ДПТ)
(long)(&model.motorInternals.i_f),	//[5184h.12h], Название: Ток ОВ, ТекстПУ: Ток ОВ, Группа: Модель (ДПТ)
(long)(&refs.DCMspeed_ref),	//[5185h.01h], Название: Задание скорости (Подч. Рег.), ТекстПУ: Зад. Скор., Группа: ДПТ Сист. Упр.
(long)(&refs.ua_ref),	//[5185h.02h], Название: Заданное напряжение якоря, ТекстПУ: Ua_ref, Группа: ДПТ Сист. Упр.
(long)(&refs.uf_ref),	//[5185h.03h], Название: Заданное напряжение ОВ, ТекстПУ: Uf_ref, Группа: ДПТ Сист. Упр.
(long)(&model.motorInternals.j),	//[5186h.01h], Название: Симулируемый момент инерции, ТекстПУ: Инерция сим., Группа: Модель (ВИД)
(long)(&model.motorInternals.MechLoss),	//[5186h.02h], Название: Тормозной момент для выбега, ТекстПУ: Торм.Мом.Выбег, Группа: Модель (ВИД)
(long)(&model.motorInternals.rs),	//[5186h.03h], Название: Сопротивление статора Rs, ТекстПУ: Rs, Группа: Модель (ВИД)
(long)(&model.motorInternals.pp),	//[5186h.04h], Название: Число пар полюсов, ТекстПУ: ПарПол, Группа: Модель (ВИД)
(long)(&model.motorInternals.QEPResolution),	//[5186h.05h], Название: Число периодов квадратурного датчика, ТекстПУ: QEPResolution, Группа: Модель (ВИД)
(long)(&model.motorInternals.iADCk),	//[5186h.06h], Название: Коэффициент АЦП тока фаз, ТекстПУ: Коэф. АЦП тока, Группа: Модель (ВИД)
(long)(&model.motorInternals.RatedCurrent),	//[5186h.07h], Название: Номинальный действующий ток, ТекстПУ: RatedCurrent, Группа: Модель (ВИД)
(long)(&model.motorInternals.RatedPower),	//[5186h.08h], Название: Номинальная мощность, ТекстПУ: RatedPower, Группа: Модель (ВИД)
(long)(&model.motorInternals.RatedSpeed),	//[5186h.09h], Название: Номинальная скорость, ТекстПУ: СкоростьНом, Группа: Модель (ВИД)
(long)(&model.motorInternals.psa),	//[5186h.0Ah], Название: Поток статора фазы A, ТекстПУ: Поток A, Группа: Модель (ВИД)
(long)(&model.motorInternals.psb),	//[5186h.0Bh], Название: Поток статора фазы B, ТекстПУ: Поток B, Группа: Модель (ВИД)
(long)(&model.motorInternals.psc),	//[5186h.0Ch], Название: Поток статора фазы C, ТекстПУ: Поток C, Группа: Модель (ВИД)
(long)(&model.motorInternals.torque_a),	//[5186h.0Dh], Название: Момент фазы A, ТекстПУ: Момент A, Группа: Модель (ВИД)
(long)(&model.motorInternals.torque_b),	//[5186h.0Eh], Название: Момент фазы B, ТекстПУ: Момент B, Группа: Модель (ВИД)
(long)(&model.motorInternals.torque_c),	//[5186h.0Fh], Название: Момент фазы C, ТекстПУ: Момент C, Группа: Модель (ВИД)
(long)(&ModBus.Enabled),	//[5A20h.01h], Название: Включен, ТекстПУ: Вкл, Группа: MODBUS
(long)(&ModBus.RSNodeID),	//[5A20h.02h], Название: Номер в сети ModBus, ТекстПУ: ModBus_ID, Группа: MODBUS
(long)(&ModBus.BaudRate),	//[5A20h.03h], Название: Скорость, ТекстПУ: скор, Группа: MODBUS
(long)(&ModBus.OfflineCounterMax),	//[5A20h.04h], Название: Таймаут потери связи, ТекстПУ: OfflineCounterMax, Группа: MODBUS
(long)(&ModBus.AutoRestart),	//[5A20h.05h], Название: Перезапуск при сбоях на линии, ТекстПУ: auto_res, Группа: MODBUS
(long)(&ModBus.OfflineCounter),	//[5A20h.06h], Название: Счетчик потери связи, ТекстПУ: OfflineCounter, Группа: MODBUS
(long)(&ModBus.isOnline),	//[5A20h.07h], Название: MODBUS online, ТекстПУ: MODBUS online, Группа: MODBUS
(long)(&ModBus.received_data),	//[5A20h.08h], Название: rs485_last_received_data, ТекстПУ: 485_RD, Группа: MODBUS
(long)(&ModBus.MBInternals.TxState),	//[5A20h.09h], Название: Состояние передачи посылки, ТекстПУ: TXState, Группа: MODBUS
(long)(&ModBus.GPIOsValue),	//[5A20h.0Ah], Название: GPIOsValue, ТекстПУ: GPIOsValue, Группа: MODBUS
(long)(&ModBus.ReceivePacketCounter),	//[5A20h.0Bh], Название: Пакетов принято, ТекстПУ: ReceivePacketCounte, Группа: MODBUS
(long)(&ModBus.ReceiveByteCounter),	//[5A20h.0Ch], Название: Байт принято, ТекстПУ: ReceiveByteCounter, Группа: MODBUS
(long)(&RotorObserver.Tr),	//[5B04h.01h], Название: Пост. времени ротора, ТекстПУ: Tr, Группа: Наблюдатель ротора АД
(long)(&RotorObserver.FluxCurrentRatio),	//[5B04h.02h], Название: Соотношение тока возб. X к току Y, ТекстПУ: FluxCurrentRatio, Группа: Наблюдатель ротора АД
(long)(&RotorObserver.FluxCurrentMin),	//[5B04h.03h], Название: Минимальный ток возбуждения, ТекстПУ: FluxCurrentMin, Группа: Наблюдатель ротора АД
(long)(&RotorObserver.id),	//[5B04h.04h], Название: Ток статора по оси D, ТекстПУ: IsD, Группа: Наблюдатель ротора АД
(long)(&RotorObserver.iq),	//[5B04h.05h], Название: Ток статора по оси Q, ТекстПУ: IsQ, Группа: Наблюдатель ротора АД
(long)(&RotorObserver.psi_d),	//[5B04h.06h], Название: Поток ротора по оси D, ТекстПУ: PsiRD, Группа: Наблюдатель ротора АД
(long)(&RotorObserver.psi_q),	//[5B04h.07h], Название: Поток ротора по оси Q, ТекстПУ: PsiRQ, Группа: Наблюдатель ротора АД
(long)(&RotorObserver.theta_psi_elec),	//[5B04h.08h], Название: Угол потока ротора, ТекстПУ: psiRalpha, Группа: Наблюдатель ротора АД
(long)(&udControl.StateOn),	//[5B05h.01h], Название: Текущее состояние реле, ТекстПУ: Relay_state, Группа: Плавный заряд ЗПТ
(long)(&udControl.Timeout_on),	//[5B05h.02h], Название: Задежка включения реле, мс, ТекстПУ: relay on-delay, Группа: Плавный заряд ЗПТ
(long)(&udControl.U_on),	//[5B05h.03h], Название: Напряжение включения, В, ТекстПУ: on-voltage, Группа: Плавный заряд ЗПТ
(long)(&udControl.U_off),	//[5B05h.04h], Название: Напряжение выключения, В, ТекстПУ: off-voltage, Группа: Плавный заряд ЗПТ
(long)(&udControl.deriv_time_ZPT),	//[5B05h.05h], Название: Производная напряжения ЗПТ, ТекстПУ: derivative_voltage, Группа: Плавный заряд ЗПТ
(long)(&udControl.deriv_const),	//[5B05h.06h], Название: Уставка по производной ЗПТ, ТекстПУ: derivative_const, Группа: Плавный заряд ЗПТ
(long)(&FanControl.StateOn),	//[5B06h.01h], Название: Текущее состояние вентилятора, ТекстПУ: fan_state, Группа: Система охлаждения
(long)(&FanControl.T_on),	//[5B06h.02h], Название: Температура включения вентилятора, ТекстПУ: t_on, Группа: Система охлаждения
(long)(&FanControl.T_off),	//[5B06h.03h], Название: Температура выключения вентилятора, ТекстПУ: t_off, Группа: Система охлаждения
(long)(&FanControl.T_alarm),	//[5B06h.04h], Название: Температура вывода предупреждения о перегреве, ТекстПУ: t_alarm, Группа: Система охлаждения
};
Uint16 const CO2_OD_TBL2[] = {
32045, 1, 0,
31021, 2, 1,
32041, 3, 2,
32013, 4, 3,

28814, 
5, 
3,
64769, 5, 4,
(Uint16)(2000), (((Uint32)2000)>>16),
64257, 6, 5,
(Uint16)(1000),

28801, 
7, 
1,
32013, 8, 6,
32013, 9, 7,
32013, 10, 8,
32013, 11, 9,

19631, 
12, 
1,
31021, 13, 10,
31021, 14, 11,

19632, 
15, 
1,
30765, 16, 12,
30765, 17, 13,
29741, 18, 14,

18607, 
19, 
3,
64813, 20, 15,
(Uint16)(0x180), (((Uint32)0x180)>>16),
28973, 21, 16,
0,
0,

18607, 
22, 
3,
64813, 23, 17,
(Uint16)(0x280), (((Uint32)0x280)>>16),
28973, 21, 18,
0,
0,

18607, 
24, 
3,
64809, 25, 19,
(Uint16)(0x0401), (((Uint32)0x0401)>>16),
28969, 21, 20,
0,
0,

18607, 
26, 
3,
64809, 27, 21,
(Uint16)(0x0501), (((Uint32)0x0501)>>16),
28969, 21, 22,
0,
0,

18607, 
28, 
3,
64813, 29, 23,
(Uint16)(0x0501), (((Uint32)0x0501)>>16),
28973, 21, 24,
0,
0,

18607, 
30, 
3,
64813, 31, 25,
(Uint16)(0x0501), (((Uint32)0x0501)>>16),
28973, 21, 26,
0,
0,

18607, 
32, 
3,
64801, 33, 27,
(Uint16)(0x0501), (((Uint32)0x0501)>>16),
28961, 21, 28,
0,
0,

18607, 
34, 
3,
64801, 35, 29,
(Uint16)(0x0501), (((Uint32)0x0501)>>16),
28961, 21, 30,
0,
0,

19633, 
36, 
3,
61741, 37, 31,
(Uint16)(0x511E0500), (((Uint32)0x511E0500)>>16),
61741, 38, 32,
(Uint16)(0x511E0600), (((Uint32)0x511E0600)>>16),
61741, 39, 33,
(Uint16)(0), (((Uint32)0)>>16),
61741, 40, 34,
(Uint16)(0), (((Uint32)0)>>16),

19633, 
41, 
3,
61741, 42, 35,
(Uint16)(0x511E0700), (((Uint32)0x511E0700)>>16),
61741, 43, 36,
(Uint16)(0x511E0800), (((Uint32)0x511E0800)>>16),
61741, 44, 37,
(Uint16)(0), (((Uint32)0)>>16),
61741, 45, 38,
(Uint16)(0), (((Uint32)0)>>16),

19629, 
46, 
3,
61737, 47, 39,
(Uint16)(0), (((Uint32)0)>>16),
61737, 48, 40,
(Uint16)(0), (((Uint32)0)>>16),
61737, 49, 41,
(Uint16)(0), (((Uint32)0)>>16),
61737, 50, 42,
(Uint16)(0), (((Uint32)0)>>16),

19629, 
51, 
3,
61737, 52, 43,
(Uint16)(0), (((Uint32)0)>>16),
61737, 53, 44,
(Uint16)(0), (((Uint32)0)>>16),
61737, 54, 45,
(Uint16)(0), (((Uint32)0)>>16),
61737, 55, 46,
(Uint16)(0), (((Uint32)0)>>16),

19625, 
56, 
3,
61741, 57, 47,
(Uint16)(0), (((Uint32)0)>>16),
61741, 58, 48,
(Uint16)(0), (((Uint32)0)>>16),
61741, 59, 49,
(Uint16)(0), (((Uint32)0)>>16),
61741, 60, 50,
(Uint16)(0), (((Uint32)0)>>16),

19625, 
61, 
3,
61741, 62, 51,
(Uint16)(0), (((Uint32)0)>>16),
61741, 63, 52,
(Uint16)(0), (((Uint32)0)>>16),
61741, 64, 53,
(Uint16)(0), (((Uint32)0)>>16),
61741, 65, 54,
(Uint16)(0), (((Uint32)0)>>16),

19629, 
66, 
3,
61729, 67, 55,
(Uint16)(0), (((Uint32)0)>>16),
61729, 68, 56,
(Uint16)(0), (((Uint32)0)>>16),
61729, 69, 57,
(Uint16)(0), (((Uint32)0)>>16),
61729, 70, 58,
(Uint16)(0), (((Uint32)0)>>16),

19629, 
71, 
3,
61729, 72, 59,
(Uint16)(0), (((Uint32)0)>>16),
61729, 73, 60,
(Uint16)(0), (((Uint32)0)>>16),
61729, 74, 61,
(Uint16)(0), (((Uint32)0)>>16),
61729, 75, 62,
(Uint16)(0), (((Uint32)0)>>16),

16562, 
76, 
2,
32045, 77, 63,
0,
31021, 21, 64,
0,
64301, 78, 65,
(Uint16)(100),
47405, 79, 66,
(Uint16)(0),
64301, 80, 67,
(Uint16)(100),

16560, 
81, 
2,
32045, 82, 68,
0,
31021, 21, 69,
0,
64301, 83, 70,
(Uint16)(100),
47405, 79, 71,
(Uint16)(0),
64301, 84, 72,
(Uint16)(1000),

16562, 
85, 
2,
32041, 86, 73,
0,
31017, 21, 74,
0,
64297, 87, 75,
(Uint16)(100),
47401, 79, 76,
(Uint16)(0),
64297, 88, 77,
(Uint16)(1000),

16556, 
89, 
2,
32041, 90, 78,
0,
31017, 21, 79,
0,
64297, 91, 80,
(Uint16)(100),
47401, 79, 81,
(Uint16)(0),
64297, 92, 82,
(Uint16)(1000),

16554, 
93, 
2,
32045, 94, 83,
0,
31021, 21, 84,
0,
64301, 95, 85,
(Uint16)(100),
47405, 79, 86,
(Uint16)(0),
64301, 96, 87,
(Uint16)(1000),

16552, 
97, 
2,
32045, 98, 88,
0,
31021, 21, 89,
0,
64301, 99, 90,
(Uint16)(100),
47405, 79, 91,
(Uint16)(0),
64301, 100, 92,
(Uint16)(1000),

16554, 
101, 
2,
32033, 102, 93,
0,
31009, 21, 94,
0,
64289, 103, 95,
(Uint16)(100),
47393, 79, 96,
(Uint16)(0),
64289, 104, 97,
(Uint16)(1000),

16556, 
105, 
2,
32033, 106, 98,
0,
31009, 21, 99,
0,
64289, 107, 100,
(Uint16)(100),
47393, 79, 101,
(Uint16)(0),
64289, 108, 102,
(Uint16)(1000),

19633, 
109, 
3,
63789, 110, 103,
(Uint16)(0), (((Uint32)0)>>16),
63789, 111, 104,
(Uint16)(0), (((Uint32)0)>>16),
63789, 112, 105,
(Uint16)(0), (((Uint32)0)>>16),
63789, 113, 106,
(Uint16)(0), (((Uint32)0)>>16),

19633, 
114, 
3,
63789, 115, 107,
(Uint16)(0), (((Uint32)0)>>16),
63789, 116, 108,
(Uint16)(0), (((Uint32)0)>>16),
63789, 117, 109,
(Uint16)(0), (((Uint32)0)>>16),
63789, 118, 110,
(Uint16)(0), (((Uint32)0)>>16),

19629, 
119, 
3,
63785, 120, 111,
(Uint16)(0), (((Uint32)0)>>16),
63785, 121, 112,
(Uint16)(0), (((Uint32)0)>>16),
63785, 122, 113,
(Uint16)(0), (((Uint32)0)>>16),
63785, 123, 114,
(Uint16)(0), (((Uint32)0)>>16),

19629, 
124, 
3,
63785, 125, 115,
(Uint16)(0), (((Uint32)0)>>16),
63785, 126, 116,
(Uint16)(0), (((Uint32)0)>>16),
63785, 127, 117,
(Uint16)(0), (((Uint32)0)>>16),
63785, 128, 118,
(Uint16)(0), (((Uint32)0)>>16),

19625, 
129, 
3,
63789, 130, 119,
(Uint16)(0), (((Uint32)0)>>16),
63789, 131, 120,
(Uint16)(0), (((Uint32)0)>>16),
63789, 132, 121,
(Uint16)(0), (((Uint32)0)>>16),
63789, 133, 122,
(Uint16)(0), (((Uint32)0)>>16),

19625, 
134, 
3,
63789, 135, 123,
(Uint16)(0), (((Uint32)0)>>16),
63789, 136, 124,
(Uint16)(0), (((Uint32)0)>>16),
63789, 137, 125,
(Uint16)(0), (((Uint32)0)>>16),
63789, 138, 126,
(Uint16)(0), (((Uint32)0)>>16),

19629, 
139, 
3,
63777, 140, 127,
(Uint16)(0), (((Uint32)0)>>16),
63777, 141, 128,
(Uint16)(0), (((Uint32)0)>>16),
63777, 142, 129,
(Uint16)(0), (((Uint32)0)>>16),
63777, 143, 130,
(Uint16)(0), (((Uint32)0)>>16),

19629, 
144, 
3,
63777, 145, 131,
(Uint16)(0), (((Uint32)0)>>16),
63777, 146, 132,
(Uint16)(0), (((Uint32)0)>>16),
63777, 147, 133,
(Uint16)(0), (((Uint32)0)>>16),
63777, 148, 134,
(Uint16)(0), (((Uint32)0)>>16),
64173, 149, 135,
(Uint16)(1),
(Uint16)(1), (Uint16)(127),
53469, 150, 136,
(Uint16)(3),

28845, 
7, 
1,
32041, 8, 137,
32041, 9, 138,
32041, 10, 139,
32041, 11, 140,

28845, 
151, 
1,
31529, 152, 141,
31529, 153, 142,
31529, 154, 143,
31529, 155, 144,
64813, 156, 145,
(Uint16)(0), (((Uint32)0)>>16),
20669, 157, 146,

28845, 
158, 
1,
32033, 159, 147,
32033, 160, 148,
32033, 161, 149,
32033, 162, 150,

28849, 
163, 
1,
32013, 164, 151,
32013, 165, 152,
32013, 166, 153,
32013, 167, 154,

28849, 
168, 
3,
64781, 169, 155,
(Uint16)(0), (((Uint32)0)>>16),
64781, 170, 156,
(Uint16)(0), (((Uint32)0)>>16),
64781, 171, 157,
(Uint16)(0), (((Uint32)0)>>16),
64781, 172, 158,
(Uint16)(0), (((Uint32)0)>>16),
53433, 173, 159,
(Uint16)(1),
53437, 174, 160,
(Uint16)(1),
30989, 175, 161,
31489, 176, 162,
32001, 177, 163,

28859, 
178, 
2,
47661, 179, 164,
(Uint16)(0),
47661, 180, 165,
(Uint16)(0x2000),
47661, 181, 166,
(Uint16)(0),
31277, 182, 167,
0,
31277, 183, 168,
0,
31277, 184, 169,
0,
31277, 185, 170,
0,
31277, 186, 171,
0,
31277, 187, 172,
0,
31277, 188, 173,
0,
31277, 189, 174,
0,
31277, 190, 175,
0,
31277, 190, 176,
0,
31277, 191, 177,
0,
31277, 192, 178,

28847, 
193, 
3,
48169, 194, 179,
(Uint16)(0x30000000), (((Uint32)0x30000000)>>16),
48169, 195, 180,
(Uint16)(0x30000000), (((Uint32)0x30000000)>>16),

28857, 
196, 
2,
36905, 197, 181,
(Uint16)(0),
36913, 198, 182,
(Uint16)(0),
36905, 199, 183,
(Uint16)(0),
36913, 200, 184,
(Uint16)(0),
36905, 201, 185,
(Uint16)(0),
36913, 202, 186,
(Uint16)(0),
36905, 203, 187,
(Uint16)(0),
36913, 204, 188,
(Uint16)(0),
36905, 205, 189,
(Uint16)(0),
36913, 206, 190,
(Uint16)(0),
36905, 207, 191,
(Uint16)(0),
36913, 208, 192,
(Uint16)(0),
36905, 209, 193,
(Uint16)(0),
36913, 210, 194,
(Uint16)(0),
36905, 211, 195,
(Uint16)(0),
36913, 212, 196,
(Uint16)(0),

28339, 
213, 
2,
31533, 214, 197,
0,
47917, 215, 198,
(Uint16)(32),
37117, 216, 199,
(Uint16)(0),
37074, 217, 200,
(Uint16)(1),
36580, 218, 201,
(Uint16)(0),
37143, 219, 202,
(Uint16)(0),

28339, 
220, 
2,
31533, 221, 203,
0,
47917, 222, 204,
(Uint16)(4128),
37117, 216, 205,
(Uint16)(0),
37072, 217, 206,
(Uint16)(1),
36578, 218, 207,
(Uint16)(0),
37143, 219, 208,
(Uint16)(4),

28339, 
223, 
2,
31529, 224, 209,
0,
47913, 225, 210,
(Uint16)(3136),
37113, 216, 211,
(Uint16)(0),
37074, 217, 212,
(Uint16)(2),
36576, 218, 213,
(Uint16)(0),
37143, 219, 214,
(Uint16)(3),

28339, 
226, 
2,
31529, 227, 215,
0,
47913, 228, 216,
(Uint16)(2112),
37113, 216, 217,
(Uint16)(0),
37068, 217, 218,
(Uint16)(2),
36578, 218, 219,
(Uint16)(0),
37143, 219, 220,
(Uint16)(2),

28331, 
229, 
2,
31533, 230, 221,
0,
47917, 231, 222,
(Uint16)(1056),
37117, 216, 223,
(Uint16)(0),
37066, 217, 224,
(Uint16)(1),
36580, 218, 225,
(Uint16)(0),
37143, 219, 226,
(Uint16)(1),

28331, 
232, 
2,
31533, 233, 227,
0,
47917, 234, 228,
(Uint16)(1056),
37117, 216, 229,
(Uint16)(0),
37064, 217, 230,
(Uint16)(1),
36570, 218, 231,
(Uint16)(0),
37143, 219, 232,
(Uint16)(1),

28331, 
235, 
2,
31521, 236, 233,
0,
47905, 237, 234,
(Uint16)(1088),
37105, 216, 235,
(Uint16)(0),
37066, 217, 236,
(Uint16)(2),
36568, 218, 237,
(Uint16)(0),
37143, 219, 238,
(Uint16)(1),

28331, 
238, 
2,
31521, 239, 239,
0,
47905, 240, 240,
(Uint16)(2112),
37105, 216, 241,
(Uint16)(0),
37068, 217, 242,
(Uint16)(2),
36570, 218, 243,
(Uint16)(0),
37143, 219, 244,
(Uint16)(2),

28323, 
241, 
2,
31533, 242, 245,
0,
47917, 243, 246,
(Uint16)(9286),
37117, 216, 247,
(Uint16)(6),
37058, 217, 248,
(Uint16)(2),
36564, 218, 249,
(Uint16)(0),
37127, 219, 250,
(Uint16)(9),

28323, 
244, 
2,
31533, 245, 251,
0,
47917, 246, 252,
(Uint16)(5184),
37117, 216, 253,
(Uint16)(0),
37056, 217, 254,
(Uint16)(2),
36562, 218, 255,
(Uint16)(0),
37127, 219, 256,
(Uint16)(5),

28323, 
247, 
2,
31529, 248, 257,
0,
47913, 249, 258,
(Uint16)(12320),
37113, 216, 259,
(Uint16)(0),
37058, 217, 260,
(Uint16)(1),
36560, 218, 261,
(Uint16)(0),
37127, 219, 262,
(Uint16)(12),

28323, 
250, 
2,
31529, 251, 263,
0,
47913, 252, 264,
(Uint16)(6144),
37113, 216, 265,
(Uint16)(0),
37068, 217, 266,
(Uint16)(0),
36562, 218, 267,
(Uint16)(0),
37127, 219, 268,
(Uint16)(6),

28331, 
253, 
2,
31533, 254, 269,
0,
47917, 255, 270,
(Uint16)(7168),
37117, 216, 271,
(Uint16)(0),
37066, 217, 272,
(Uint16)(0),
36564, 218, 273,
(Uint16)(0),
37127, 219, 274,
(Uint16)(7),

28331, 
256, 
2,
31533, 257, 275,
0,
47917, 258, 276,
(Uint16)(2048),
37117, 216, 277,
(Uint16)(0),
37064, 217, 278,
(Uint16)(0),
36570, 218, 279,
(Uint16)(0),
37127, 219, 280,
(Uint16)(2),

28331, 
259, 
2,
31505, 260, 281,
0,
47889, 261, 282,
(Uint16)(1024),
37089, 216, 283,
(Uint16)(0),
37066, 217, 284,
(Uint16)(0),
36568, 218, 285,
(Uint16)(0),
37127, 219, 286,
(Uint16)(1),

28331, 
262, 
2,
31505, 263, 287,
0,
47889, 264, 288,
(Uint16)(96),
37089, 216, 289,
(Uint16)(0),
37068, 217, 290,
(Uint16)(3),
36570, 218, 291,
(Uint16)(0),
37127, 219, 292,
(Uint16)(0),

28339, 
265, 
2,
31501, 266, 293,
0,
47885, 267, 294,
(Uint16)(3136),
37117, 216, 295,
(Uint16)(0),
37074, 217, 296,
(Uint16)(2),
36548, 218, 297,
(Uint16)(0),
37143, 219, 298,
(Uint16)(3),

28339, 
268, 
2,
31501, 269, 299,
0,
47885, 270, 300,
(Uint16)(9408),
37117, 216, 301,
(Uint16)(0),
37072, 217, 302,
(Uint16)(2),
36546, 218, 303,
(Uint16)(1),
37143, 219, 304,
(Uint16)(9),

28339, 
271, 
2,
31497, 272, 305,
0,
47881, 273, 306,
(Uint16)(9414),
37113, 216, 307,
(Uint16)(6),
37074, 217, 308,
(Uint16)(2),
36544, 218, 309,
(Uint16)(1),
37143, 219, 310,
(Uint16)(9),

28339, 
274, 
2,
31497, 275, 311,
0,
47881, 276, 312,
(Uint16)(12352),
37113, 216, 313,
(Uint16)(0),
37036, 217, 314,
(Uint16)(2),
36546, 218, 315,
(Uint16)(0),
37143, 219, 316,
(Uint16)(12),

28299, 
277, 
2,
31501, 278, 317,
0,
47885, 279, 318,
(Uint16)(13344),
37117, 216, 319,
(Uint16)(0),
37034, 217, 320,
(Uint16)(1),
36548, 218, 321,
(Uint16)(0),
37143, 219, 322,
(Uint16)(13),

28299, 
280, 
2,
31501, 281, 323,
0,
47885, 282, 324,
(Uint16)(10240),
37117, 216, 325,
(Uint16)(0),
37032, 217, 326,
(Uint16)(0),
36570, 218, 327,
(Uint16)(0),
37143, 219, 328,
(Uint16)(10),

28299, 
283, 
2,
31489, 284, 329,
0,
47873, 285, 330,
(Uint16)(14400),
37105, 216, 331,
(Uint16)(0),
37034, 217, 332,
(Uint16)(2),
36568, 218, 333,
(Uint16)(0),
37143, 219, 334,
(Uint16)(14),

28299, 
286, 
2,
31489, 287, 335,
0,
47873, 288, 336,
(Uint16)(15430),
37105, 216, 337,
(Uint16)(6),
37036, 217, 338,
(Uint16)(2),
36570, 218, 339,
(Uint16)(0),
37143, 219, 340,
(Uint16)(15),

28291, 
289, 
2,
31501, 290, 341,
0,
47885, 291, 342,
(Uint16)(16448),
37117, 216, 343,
(Uint16)(0),
37026, 217, 344,
(Uint16)(2),
36564, 218, 345,
(Uint16)(0),
36583, 219, 346,
(Uint16)(16),

28291, 
292, 
2,
31501, 293, 347,
0,
47885, 294, 348,
(Uint16)(13312),
37117, 216, 349,
(Uint16)(0),
37024, 217, 350,
(Uint16)(0),
36562, 218, 351,
(Uint16)(0),
36583, 219, 352,
(Uint16)(13),

28291, 
295, 
2,
31497, 296, 353,
0,
47881, 297, 354,
(Uint16)(14336),
37113, 216, 355,
(Uint16)(0),
37026, 217, 356,
(Uint16)(0),
36560, 218, 357,
(Uint16)(0),
36583, 219, 358,
(Uint16)(14),

28291, 
298, 
2,
31497, 299, 359,
0,
47881, 300, 360,
(Uint16)(12384),
37113, 216, 361,
(Uint16)(0),
37036, 217, 362,
(Uint16)(3),
36562, 218, 363,
(Uint16)(0),
36583, 219, 364,
(Uint16)(12),

28299, 
301, 
2,
31501, 302, 365,
0,
47885, 303, 366,
(Uint16)(0),
37117, 216, 367,
(Uint16)(0),
37034, 217, 368,
(Uint16)(0),
36564, 218, 369,
(Uint16)(0),
36583, 219, 370,
(Uint16)(0),

28299, 
304, 
2,
31501, 305, 371,
0,
47885, 306, 372,
(Uint16)(0),
37117, 216, 373,
(Uint16)(0),
37032, 217, 374,
(Uint16)(0),
36570, 218, 375,
(Uint16)(0),
36583, 219, 376,
(Uint16)(0),

28333, 
307, 
4,
36537, 308, 377,
(Uint16)(256),
0,
0,
36542, 309, 378,
(Uint16)(256),
0,
0,
36543, 310, 379,
(Uint16)(256),
0,
0,
45225, 307, 380,
(Uint16)(0),
(Uint16)(0), (Uint16)(1),

28333, 
756, 
3,
48417, 581, 381,
(Uint16)(0), (((Uint32)0)>>16),
48417, 582, 382,
(Uint16)(0), (((Uint32)0)>>16),
48417, 754, 383,
(Uint16)(0), (((Uint32)0)>>16),
48417, 753, 384,
(Uint16)(0), (((Uint32)0)>>16),
48401, 311, 385,
(Uint16)(438392299), (((Uint32)438392299)>>16),

28236, 
312, 
1,
20169, 313, 386,
18737, 314, 387,
20153, 312, 388,

28238, 
315, 
2,
45261, 316, 389,
(Uint16)(15),

27822, 
577, 
3,
45357, 577, 390,
(Uint16)(0x11111111), (((Uint32)0x11111111)>>16),
45869, 596, 391,
(Uint16)(0),

28346, 
597, 
7,
45741, 598, 392,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
45741, 599, 393,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
45741, 600, 394,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
45741, 601, 395,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
45741, 602, 396,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
45741, 603, 397,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
45741, 604, 398,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
45741, 605, 399,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
45741, 606, 400,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
45741, 607, 401,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
45741, 608, 402,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
45741, 609, 403,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
45741, 610, 404,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
45741, 611, 405,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
45357, 612, 406,
(Uint16)(0), (((Uint32)0)>>16),
0,
0,
0,
0,
20749, 317, 407,

3762, 
312, 
3,
47917, 318, 408,
(Uint16)(0),
0,
47917, 319, 409,
(Uint16)(0),
0,
47917, 320, 410,
(Uint16)(0),
0,
48429, 321, 411,
(Uint16)(0), (((Uint32)0)>>16),
48429, 322, 412,
(Uint16)(0), (((Uint32)0)>>16),

28848, 
323, 
7,
58853, 324, 413,
(Uint16)(16777216), (((Uint32)16777216)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(_IQ(120.0)), (((Uint32)_IQ(120.0))>>16),
26437, 325, 414,
0,
0,
0,
0,
0,
0,
26437, 326, 415,
0,
0,
0,
0,
0,
0,

20154, 
327, 
7,
25925, 328, 416,
0,
0,
0,
0,
0,
0,
25925, 329, 417,
0,
0,
0,
0,
0,
0,
9381, 330, 418,
(Uint16)(335544), (((Uint32)335544)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9509, 331, 419,
(Uint16)(16777216), (((Uint32)16777216)>>16),
(Uint16)(_IQ(-1.0)), (((Uint32)_IQ(-1.0))>>16), (Uint16)(_IQ(1.0)), (((Uint32)_IQ(1.0))>>16),
9509, 332, 420,
(Uint16)(-16777216), (((Uint32)-16777216)>>16),
(Uint16)(_IQ(-1.0)), (((Uint32)_IQ(-1.0))>>16), (Uint16)(_IQ(1.0)), (((Uint32)_IQ(1.0))>>16),
9381, 333, 421,
(Uint16)(2684354), (((Uint32)2684354)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9381, 334, 422,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9381, 335, 423,
(Uint16)(8388608), (((Uint32)8388608)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
25925, 336, 424,
0,
0,
0,
0,
0,
0,
25893, 337, 425,
0,
0,
0,
0,
0,
0,
25893, 338, 426,
0,
0,
0,
0,
0,
0,
25893, 339, 427,
0,
0,
0,
0,
0,
0,
25893, 340, 428,
0,
0,
0,
0,
0,
0,

20155, 
341, 
7,
26437, 342, 429,
0,
0,
0,
0,
0,
0,
26437, 343, 430,
0,
0,
0,
0,
0,
0,
9381, 330, 431,
(Uint16)(3355443), (((Uint32)3355443)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9541, 331, 432,
(Uint16)(13421772), (((Uint32)13421772)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9541, 332, 433,
(Uint16)(-13421772), (((Uint32)-13421772)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9381, 333, 434,
(Uint16)(16777), (((Uint32)16777)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9381, 344, 435,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9381, 335, 436,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
26437, 336, 437,
0,
0,
0,
0,
0,
0,
25925, 345, 438,
0,
0,
0,
0,
0,
0,
25925, 338, 439,
0,
0,
0,
0,
0,
0,
25925, 339, 440,
0,
0,
0,
0,
0,
0,
25925, 340, 441,
0,
0,
0,
0,
0,
0,
25925, 346, 442,
0,
0,
0,
0,
0,
0,

20154, 
347, 
7,
25921, 348, 443,
0,
0,
0,
0,
0,
0,
25921, 349, 444,
0,
0,
0,
0,
0,
0,
9377, 350, 445,
(Uint16)(335544), (((Uint32)335544)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9505, 351, 446,
(Uint16)(16777216), (((Uint32)16777216)>>16),
(Uint16)(_IQ(-1.0)), (((Uint32)_IQ(-1.0))>>16), (Uint16)(_IQ(1.0)), (((Uint32)_IQ(1.0))>>16),
9505, 352, 447,
(Uint16)(-16777216), (((Uint32)-16777216)>>16),
(Uint16)(_IQ(-1.0)), (((Uint32)_IQ(-1.0))>>16), (Uint16)(_IQ(1.0)), (((Uint32)_IQ(1.0))>>16),
9377, 353, 448,
(Uint16)(2684354), (((Uint32)2684354)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9377, 354, 449,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9377, 335, 450,
(Uint16)(8388608), (((Uint32)8388608)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
25921, 336, 451,
0,
0,
0,
0,
0,
0,
25889, 355, 452,
0,
0,
0,
0,
0,
0,
25889, 338, 453,
0,
0,
0,
0,
0,
0,
25889, 339, 454,
0,
0,
0,
0,
0,
0,
25889, 340, 455,
0,
0,
0,
0,
0,
0,

20157, 
356, 
7,
15401, 342, 456,
(Uint16)(0), (((Uint32)0)>>16),
0,
0,
0,
0,
10017, 342, 457,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
26401, 343, 458,
0,
0,
0,
0,
0,
0,
31785, 343, 459,
0,
0,
0,
0,
0,
0,
9377, 330, 460,
(Uint16)(1174405120), (((Uint32)1174405120)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
10049, 331, 461,
(Uint16)(100663296), (((Uint32)100663296)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
10049, 332, 462,
(Uint16)(-100663296), (((Uint32)-100663296)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9377, 333, 463,
(Uint16)(83886), (((Uint32)83886)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9377, 344, 464,
(Uint16)(167772160), (((Uint32)167772160)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
15401, 357, 465,
(Uint16)(0), (((Uint32)0)>>16),
0,
0,
0,
0,
9377, 358, 466,
(Uint16)(8388608), (((Uint32)8388608)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9377, 335, 467,
(Uint16)(8388608), (((Uint32)8388608)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
10017, 359, 468,
(Uint16)(13981), (((Uint32)13981)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
26401, 336, 469,
0,
0,
0,
0,
0,
0,
25761, 346, 470,
0,
0,
0,
0,
0,
0,
26433, 345, 471,
0,
0,
0,
0,
0,
0,
26433, 338, 472,
0,
0,
0,
0,
0,
0,
26433, 339, 473,
0,
0,
0,
0,
0,
0,
26433, 340, 474,
0,
0,
0,
0,
0,
0,
26401, 360, 475,
0,
0,
0,
0,
0,
0,

20146, 
361, 
7,
25925, 348, 476,
0,
0,
0,
0,
0,
0,
25925, 349, 477,
0,
0,
0,
0,
0,
0,
9381, 350, 478,
(Uint16)(335544), (((Uint32)335544)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9509, 351, 479,
(Uint16)(16777216), (((Uint32)16777216)>>16),
(Uint16)(_IQ(-1.0)), (((Uint32)_IQ(-1.0))>>16), (Uint16)(_IQ(1.0)), (((Uint32)_IQ(1.0))>>16),
9509, 352, 480,
(Uint16)(-16777216), (((Uint32)-16777216)>>16),
(Uint16)(_IQ(-1.0)), (((Uint32)_IQ(-1.0))>>16), (Uint16)(_IQ(1.0)), (((Uint32)_IQ(1.0))>>16),
9381, 353, 481,
(Uint16)(2684354), (((Uint32)2684354)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9381, 354, 482,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9381, 335, 483,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
25925, 336, 484,
0,
0,
0,
0,
0,
0,
25893, 355, 485,
0,
0,
0,
0,
0,
0,
25893, 338, 486,
0,
0,
0,
0,
0,
0,
25893, 339, 487,
0,
0,
0,
0,
0,
0,
25893, 340, 488,
0,
0,
0,
0,
0,
0,

20156, 
362, 
7,
25893, 363, 489,
0,
0,
0,
0,
0,
0,
25893, 364, 490,
0,
0,
0,
0,
0,
0,
8901, 365, 491,
(Uint16)(14529069), (((Uint32)14529069)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0x01000000), (((Uint32)0x01000000)>>16),
31533, 366, 492,
0,
0,
0,
0,
0,
0,
31533, 367, 493,
0,
0,
0,
0,
0,
0,
31533, 368, 494,
0,
0,
0,
0,
0,
0,
31533, 369, 495,
0,
0,
0,
0,
0,
0,
11429, 370, 496,
(Uint16)(3355443), (((Uint32)3355443)>>16),
(Uint16)(_IQ(2.0)), (((Uint32)_IQ(2.0))>>16), (Uint16)(_IQ(10.0)), (((Uint32)_IQ(10.0))>>16),
11429, 371, 497,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(_IQ(10)), (((Uint32)_IQ(10))>>16),
8901, 372, 498,
(Uint16)(16777216), (((Uint32)16777216)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
11965, 373, 499,
(Uint16)(1),
0,
0,
0,
0,
0,
15149, 374, 500,
(Uint16)(1),
0,
0,
0,
0,
0,
31661, 375, 501,
0,
0,
0,
0,
0,
0,
9367, 376, 502,
(Uint16)(20480), (((Uint32)20480)>>16),
(Uint16)(2048), (((Uint32)2048)>>16), (Uint16)(10240), (((Uint32)10240)>>16),
31277, 377, 503,
0,
0,
0,
0,
0,
0,
25893, 378, 504,
0,
0,
0,
0,
0,
0,
31533, 379, 505,
0,
0,
0,
0,
0,
0,

20143, 
380, 
2,
31273, 381, 506,
0,
53545, 382, 507,
(Uint16)(1),

20132, 
383, 
7,
15121, 384, 508,
(Uint16)(0),
0,
0,
0,
0,
0,
15121, 385, 509,
(Uint16)(0),
0,
0,
0,
0,
0,
53265, 386, 510,
(Uint16)(255),
0,
0,
0,
0,
0,
53289, 387, 511,
(Uint16)(255),
0,
0,
0,
0,
0,
53265, 388, 512,
(Uint16)(255),
0,
0,
0,
0,
0,
53289, 389, 513,
(Uint16)(255),
0,
0,
0,
0,
0,
9529, 390, 514,
(Uint16)(16777216), (((Uint32)16777216)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9625, 391, 515,
(Uint16)(18454938), (((Uint32)18454938)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9625, 392, 516,
(Uint16)(6990506), (((Uint32)6990506)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
10041, 393, 517,
(Uint16)(100663296), (((Uint32)100663296)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9721, 639, 518,
(Uint16)(75497472), (((Uint32)75497472)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),

20146, 
394, 
7,
10017, 395, 519,
(Uint16)(335544), (((Uint32)335544)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9505, 396, 520,
(Uint16)(3355443), (((Uint32)3355443)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9505, 397, 521,
(Uint16)(3355443), (((Uint32)3355443)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9985, 398, 522,
(Uint16)(720992), (((Uint32)720992)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9601, 559, 523,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),

20144, 
399, 
7,
15241, 400, 524,
(Uint16)(4),
(Uint16)(1), (Uint16)(50),
0,
0,
0,
8265, 401, 525,
(Uint16)(5),
(Uint16)(0), (Uint16)(0),
0,
0,
0,
24617, 402, 526,
0,
0,
0,
0,
0,
0,
9129, 403, 527,
(Uint16)(500),
(Uint16)(0), (Uint16)(0),
0,
0,
0,
8233, 404, 528,
(Uint16)(36),
(Uint16)(0), (Uint16)(1000),
0,
0,
0,
10113, 405, 529,
(Uint16)(1048576), (((Uint32)1048576)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
10145, 406, 530,
(Uint16)(16777), (((Uint32)16777)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
24713, 407, 531,
0,
0,
0,
0,
0,
0,
11993, 408, 532,
(Uint16)(1),
0,
0,
0,
0,
0,

20107, 
409, 
4,
4365, 410, 533,
0,
0,
0,
15117, 411, 534,
(Uint16)(4),
0,
0,
15117, 412, 535,
(Uint16)(0),
0,
0,
15117, 413, 536,
(Uint16)(1),
0,
0,
14477, 414, 537,
(Uint16)(0),
(Uint16)(0), (Uint16)(1),
15117, 590, 538,
(Uint16)(0),
0,
0,

20157, 
415, 
4,
8257, 416, 539,
(Uint16)(40),
(Uint16)(0), (Uint16)(0),
16001, 417, 540,
(Uint16)(62706),
(Uint16)(0), (Uint16)(0),
25913, 418, 541,
0,
0,
0,
8257, 419, 542,
(Uint16)(40),
(Uint16)(0), (Uint16)(0),
16001, 420, 543,
(Uint16)(62251),
(Uint16)(0), (Uint16)(0),
25913, 421, 544,
0,
0,
0,
8257, 422, 545,
(Uint16)(40),
(Uint16)(0), (Uint16)(0),
16001, 423, 546,
(Uint16)(62836),
(Uint16)(0), (Uint16)(0),
25913, 424, 547,
0,
0,
0,
8257, 585, 548,
(Uint16)(40),
(Uint16)(0), (Uint16)(0),
16001, 586, 549,
(Uint16)(62836),
(Uint16)(0), (Uint16)(0),
25913, 587, 550,
0,
0,
0,
8225, 425, 551,
(Uint16)(66),
(Uint16)(0), (Uint16)(0),
16001, 426, 552,
(Uint16)(0),
(Uint16)(0), (Uint16)(0),
26009, 427, 553,
0,
0,
0,
9185, 633, 554,
(Uint16)(0),
(Uint16)(0), (Uint16)(0),
26105, 634, 555,
0,
0,
0,
16001, 642, 556,
(Uint16)(100),
(Uint16)(0), (Uint16)(0),
16001, 643, 557,
(Uint16)(0),
(Uint16)(0), (Uint16)(0),
25305, 641, 558,
0,
0,
0,

20096, 
428, 
7,
26417, 429, 559,
0,
0,
0,
0,
0,
0,
25873, 430, 560,
0,
0,
0,
0,
0,
0,
10033, 431, 561,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9489, 432, 562,
(Uint16)(419430), (((Uint32)419430)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
10033, 433, 563,
(Uint16)(100663296), (((Uint32)100663296)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9489, 434, 564,
(Uint16)(12582912), (((Uint32)12582912)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
10033, 435, 565,
(Uint16)(100663296), (((Uint32)100663296)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),

20037, 
436, 
1,
26421, 437, 566,
26037, 438, 567,
25909, 439, 568,
25813, 440, 569,
25813, 441, 570,
25909, 442, 571,
28141, 613, 572,
28141, 614, 573,

20024, 
443, 
7,
14477, 444, 574,
(Uint16)(0),
0,
0,
0,
0,
0,
14989, 445, 575,
(Uint16)(192), (((Uint32)192)>>16),
0,
0,
0,
0,
14989, 446, 576,
(Uint16)(1359151123), (((Uint32)1359151123)>>16),
0,
0,
0,
0,
14989, 447, 577,
(Uint16)(1359085569), (((Uint32)1359085569)>>16),
0,
0,
0,
0,
14989, 448, 578,
(Uint16)(1359151122), (((Uint32)1359151122)>>16),
0,
0,
0,
0,
14989, 449, 579,
(Uint16)(1359020033), (((Uint32)1359020033)>>16),
0,
0,
0,
0,
53645, 450, 580,
(Uint16)(0),
0,
0,
0,
0,
0,
53685, 451, 581,
(Uint16)(1),
0,
0,
0,
0,
0,
11837, 452, 582,
(Uint16)(0),
0,
0,
0,
0,
0,
10949, 453, 583,
(Uint16)(6710886), (((Uint32)6710886)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
31373, 644, 584,
0,
0,
0,
0,
0,
0,
15757, 454, 585,
(Uint16)(50461787), (((Uint32)50461787)>>16),

20000, 
455, 
7,
14985, 456, 586,
(Uint16)(1000), (((Uint32)1000)>>16),
0,
0,
0,
0,
11839, 457, 587,
(Uint16)(2),
0,
0,
0,
0,
0,
14473, 458, 588,
(Uint16)(1),
0,
0,
0,
0,
0,
9345, 459, 589,
(Uint16)(9298387), (((Uint32)9298387)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
31369, 460, 590,
0,
0,
0,
0,
0,
0,
30857, 460, 591,
0,
0,
0,
0,
0,
0,
25761, 461, 592,
0,
0,
0,
0,
0,
0,
25729, 462, 593,
0,
0,
0,
0,
0,
0,
25729, 463, 594,
0,
0,
0,
0,
0,
0,
53565, 464, 595,
(Uint16)(0),
0,
0,
0,
0,
0,
9217, 465, 596,
(Uint16)(838860), (((Uint32)838860)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(65535), (((Uint32)65535)>>16),
30857, 466, 597,
0,
0,
0,
0,
0,
0,
30857, 467, 598,
0,
0,
0,
0,
0,
0,
30857, 468, 599,
0,
0,
0,
0,
0,
0,
31369, 469, 600,
0,
0,
0,
0,
0,
0,
30857, 469, 601,
0,
0,
0,
0,
0,
0,
30857, 470, 602,
0,
0,
0,
0,
0,
0,
15241, 471, 603,
(Uint16)(0),
0,
0,
0,
0,
0,
15241, 472, 604,
(Uint16)(7),
0,
0,
0,
0,
0,
15241, 472, 605,
(Uint16)(7335),
0,
0,
0,
0,
0,
15753, 472, 606,
(Uint16)(1973415), (((Uint32)1973415)>>16),
0,
0,
0,
0,
25729, 473, 607,
0,
0,
0,
0,
0,
0,
25729, 474, 608,
0,
0,
0,
0,
0,
0,

20024, 
475, 
7,
25733, 441, 609,
0,
0,
0,
0,
0,
0,
25765, 461, 610,
0,
0,
0,
0,
0,
0,
9381, 476, 611,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
25733, 477, 612,
0,
0,
0,
0,
0,
0,
11837, 478, 613,
(Uint16)(81),
0,
0,
0,
0,
0,
14989, 479, 614,
(Uint16)(500), (((Uint32)500)>>16),
0,
0,
0,
0,
25765, 480, 615,
0,
0,
0,
0,
0,
0,
30861, 481, 616,
0,
0,
0,
0,
0,
0,
4573, 482, 617,
0,
0,
0,
0,
0,
0,
9349, 459, 618,
(Uint16)(4869970), (((Uint32)4869970)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
30861, 483, 619,
0,
0,
0,
0,
0,
0,
30861, 484, 620,
0,
0,
0,
0,
0,
0,
30861, 485, 621,
0,
0,
0,
0,
0,
0,
30861, 486, 622,
0,
0,
0,
0,
0,
0,
53567, 487, 623,
(Uint16)(0),
0,
0,
0,
0,
0,
14477, 488, 624,
(Uint16)(10),
0,
0,
0,
0,
0,
30861, 489, 625,
0,
0,
0,
0,
0,
0,
30861, 490, 626,
0,
0,
0,
0,
0,
0,
30861, 491, 627,
0,
0,
0,
0,
0,
0,
30861, 492, 628,
0,
0,
0,
0,
0,
0,
30861, 493, 629,
0,
0,
0,
0,
0,
0,

20019, 
455, 
7,
14977, 494, 630,
(Uint16)(4096), (((Uint32)4096)>>16),
0,
0,
0,
0,
9369, 459, 631,
(Uint16)(9298387), (((Uint32)9298387)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
30849, 460, 632,
0,
0,
0,
0,
0,
0,
25785, 461, 633,
0,
0,
0,
0,
0,
0,
25753, 462, 634,
0,
0,
0,
0,
0,
0,
25753, 463, 635,
0,
0,
0,
0,
0,
0,
25753, 495, 636,
0,
0,
0,
0,
0,
0,
9241, 496, 637,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
15233, 471, 638,
(Uint16)(0),
0,
0,
0,
0,
0,
15745, 472, 639,
(Uint16)(1973415), (((Uint32)1973415)>>16),
0,
0,
0,
0,
15233, 472, 640,
(Uint16)(7),
0,
0,
0,
0,
0,
15233, 472, 641,
(Uint16)(7335),
0,
0,
0,
0,
0,
25753, 473, 642,
0,
0,
0,
0,
0,
0,
25753, 474, 643,
0,
0,
0,
0,
0,
0,

20025, 
497, 
7,
25669, 498, 644,
0,
0,
0,
0,
0,
0,
32205, 498, 645,
0,
0,
0,
0,
0,
0,
25669, 499, 646,
0,
0,
0,
0,
0,
0,
32205, 499, 647,
0,
0,
0,
0,
0,
0,
30925, 500, 648,
0,
0,
0,
0,
0,
0,
31693, 500, 649,
0,
0,
0,
0,
0,
0,
30925, 501, 650,
0,
0,
0,
0,
0,
0,
31693, 501, 651,
0,
0,
0,
0,
0,
0,
9285, 502, 652,
(Uint16)(16777), (((Uint32)16777)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
15821, 502, 653,
(Uint16)(16777), (((Uint32)16777)>>16),
0,
0,
0,
0,
9285, 503, 654,
(Uint16)(-190048), (((Uint32)-190048)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
15821, 503, 655,
(Uint16)(-190048), (((Uint32)-190048)>>16),
0,
0,
0,
0,
9285, 504, 656,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
15821, 504, 657,
(Uint16)(0), (((Uint32)0)>>16),
0,
0,
0,
0,
9285, 505, 658,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
15821, 505, 659,
(Uint16)(0), (((Uint32)0)>>16),
0,
0,
0,
0,
50253, 506, 660,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50253, 507, 661,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50253, 508, 662,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50253, 509, 663,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),

20161, 
510, 
7,
11821, 511, 664,
(Uint16)(1),
0,
0,
0,
0,
0,
14381, 399, 665,
(Uint16)(1),
0,
0,
0,
0,
0,
50363, 512, 666,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
49327, 427, 667,
(Uint16)(1103101952), (((Uint32)1103101952)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
1211, 513, 668,
0,
0,
0,
0,
0,
0,
1225, 514, 669,
0,
0,
0,
0,
0,
0,
1208, 514, 670,
0,
0,
0,
0,
0,
0,
173, 515, 671,
0,
0,
0,
0,
0,
0,
30765, 516, 672,
0,
0,
0,
0,
0,
0,
172, 517, 673,
0,
0,
0,
0,
0,
0,
172, 518, 674,
0,
0,
0,
0,
0,
0,
172, 519, 675,
0,
0,
0,
0,
0,
0,
172, 421, 676,
0,
0,
0,
0,
0,
0,
172, 520, 677,
0,
0,
0,
0,
0,
0,
172, 584, 678,
0,
0,
0,
0,
0,
0,
175, 521, 679,
0,
0,
0,
0,
0,
0,
175, 522, 680,
0,
0,
0,
0,
0,
0,
175, 523, 681,
0,
0,
0,
0,
0,
0,
244, 524, 682,
0,
0,
0,
0,
0,
0,
30765, 525, 683,
0,
0,
0,
0,
0,
0,

20156, 
510, 
7,
51065, 526, 684,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
51065, 527, 685,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
51065, 528, 686,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50360, 529, 687,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50360, 530, 688,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
57001, 400, 689,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50345, 531, 690,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50363, 532, 691,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
57001, 533, 692,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
49838, 592, 693,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
49910, 591, 694,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
57018, 403, 695,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
31529, 535, 696,
0,
0,
0,
0,
0,
0,
1198, 517, 697,
0,
0,
0,
0,
0,
0,
1198, 518, 698,
0,
0,
0,
0,
0,
0,
1193, 536, 699,
0,
0,
0,
0,
0,
0,
1193, 537, 700,
0,
0,
0,
0,
0,
0,
1193, 538, 701,
0,
0,
0,
0,
0,
0,
1193, 539, 702,
0,
0,
0,
0,
0,
0,

20152, 
510, 
7,
50345, 540, 703,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
51065, 541, 704,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
51065, 542, 705,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50362, 529, 706,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
57001, 400, 707,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50345, 531, 708,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50363, 532, 709,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
57001, 533, 710,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
31529, 535, 711,
0,
0,
0,
0,
0,
0,
31529, 550, 712,
0,
0,
0,
0,
0,
0,
49832, 592, 713,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
49904, 591, 714,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
57012, 403, 715,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
49832, 593, 716,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
175, 553, 717,
0,
0,
0,
0,
0,
0,
1192, 555, 718,
0,
0,
0,
0,
0,
0,
1193, 588, 719,
0,
0,
0,
0,
0,
0,

20159, 
510, 
7,
51069, 543, 720,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
51069, 544, 721,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50364, 545, 722,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50364, 546, 723,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50349, 531, 724,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50355, 532, 725,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
57005, 533, 726,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
49838, 594, 727,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
49838, 593, 728,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
31533, 549, 729,
0,
0,
0,
0,
0,
0,
31533, 550, 730,
0,
0,
0,
0,
0,
0,
49910, 591, 731,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
57010, 403, 732,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50364, 551, 733,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
175, 552, 734,
0,
0,
0,
0,
0,
0,
175, 553, 735,
0,
0,
0,
0,
0,
0,
1198, 554, 736,
0,
0,
0,
0,
0,
0,
1198, 555, 737,
0,
0,
0,
0,
0,
0,

20142, 
556, 
7,
9285, 557, 738,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9637, 558, 739,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
9637, 559, 740,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),

20144, 
510, 
7,
50337, 531, 741,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50355, 532, 742,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
50352, 529, 743,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
56993, 400, 744,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
56993, 533, 745,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
31521, 535, 746,
0,
0,
0,
0,
0,
0,
49838, 592, 747,
(Uint16)(0), (((Uint32)0)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
758, 591, 748,
0,
0,
0,
0,
0,
0,
7858, 403, 749,
0,
0,
0,
0,
0,
0,
1185, 560, 750,
0,
0,
0,
0,
0,
0,
1185, 561, 751,
0,
0,
0,
0,
0,
0,
1185, 562, 752,
0,
0,
0,
0,
0,
0,
1203, 563, 753,
0,
0,
0,
0,
0,
0,
1203, 564, 754,
0,
0,
0,
0,
0,
0,
1203, 565, 755,
0,
0,
0,
0,
0,
0,

19641, 
566, 
3,
15149, 589, 756,
(Uint16)(1),
0,
15149, 567, 757,
(Uint16)(1),
0,
14637, 437, 758,
(Uint16)(9600), (((Uint32)9600)>>16),
14637, 581, 759,
(Uint16)(20000), (((Uint32)20000)>>16),
15149, 583, 760,
(Uint16)(1),
0,
31021, 582, 761,
0,
0,
31533, 568, 762,
0,
0,
31533, 569, 763,
0,
0,
31533, 578, 764,
0,
0,
31533, 467, 765,
0,
0,
31533, 579, 766,
0,
0,
31533, 580, 767,
0,
0,

20149, 
399, 
7,
12453, 571, 768,
(Uint16)(16777216), (((Uint32)16777216)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
12453, 572, 769,
(Uint16)(5033165), (((Uint32)5033165)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
12613, 573, 770,
(Uint16)(5033165), (((Uint32)5033165)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
28997, 574, 771,
0,
0,
0,
0,
0,
0,
28997, 575, 772,
0,
0,
0,
0,
0,
0,
28837, 536, 773,
0,
0,
0,
0,
0,
0,
28837, 537, 774,
0,
0,
0,
0,
0,
0,
30501, 576, 775,
0,
0,
0,
0,
0,
0,

20139, 
626, 
7,
17709, 631, 776,
0,
0,
0,
0,
0,
0,
15149, 627, 777,
(Uint16)(2000),
0,
0,
0,
0,
0,
10661, 628, 778,
(Uint16)(15099494), (((Uint32)15099494)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
10661, 629, 779,
(Uint16)(11744051), (((Uint32)11744051)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
28837, 630, 780,
0,
0,
0,
0,
0,
0,
12453, 632, 781,
(Uint16)(8388608), (((Uint32)8388608)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),

20141, 
635, 
7,
31521, 636, 782,
0,
0,
0,
0,
0,
0,
12793, 637, 783,
(Uint16)(41943040), (((Uint32)41943040)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
12793, 638, 784,
(Uint16)(25165824), (((Uint32)25165824)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
12793, 640, 785,
(Uint16)(67108864), (((Uint32)67108864)>>16),
(Uint16)(0), (((Uint32)0)>>16), (Uint16)(0), (((Uint32)0)>>16),
};

int16 const CO2_OD_TBL1[] = { 
4096, 0,
4097, 3,
4098, 6,
4116, 9,
4118, 12,
4119, 20,
4120, 24,
4608, 39,
4736, 48,
5120, 60,
5121, 73,
5122, 86,
5123, 99,
5124, 112,
5125, 125,
5126, 138,
5127, 151,
5632, 164,
5633, 187,
5634, 210,
5635, 233,
5636, 256,
5637, 279,
5638, 302,
5639, 325,
6144, 348,
6145, 371,
6146, 394,
6147, 417,
6148, 440,
6149, 463,
6150, 486,
6151, 509,
6656, 532,
6657, 555,
6658, 578,
6659, 601,
6660, 624,
6661, 647,
6662, 670,
6663, 693,
8192, 716,
8193, 722,
8194, 726,
8195, 741,
8196, 756,
8197, 761,
8199, 764,
8208, 779,
8209, 794,
8210, 817,
8212, 821,
8213, 825,
8214, 828,
8215, 831,
8320, 834,
8321, 893,
8322, 896,
8323, 909,
8448, 976,
8449, 1003,
8450, 1030,
8451, 1057,
8452, 1084,
8453, 1111,
8454, 1138,
8455, 1165,
8456, 1192,
8457, 1219,
8458, 1246,
8459, 1273,
8460, 1300,
8461, 1327,
8462, 1354,
8463, 1381,
8464, 1408,
8465, 1435,
8466, 1462,
8467, 1489,
8468, 1516,
8469, 1543,
8470, 1570,
8471, 1597,
8472, 1624,
8473, 1651,
8474, 1678,
8475, 1705,
8476, 1732,
8477, 1759,
9474, 1786,
9478, 1813,
9487, 1836,
9522, 1841,
9524, 1853,
9728, 1860,
9984, 1868,
9985, 1872,
12288, 2010,
12544, 2013,
20480, 2041,
20736, 2071,
20737, 2191,
20738, 2320,
20739, 2440,
20740, 2623,
20741, 2743,
20747, 2899,
20750, 2910,
20754, 3012,
20755, 3060,
20756, 3144,
20758, 3183,
20762, 3306,
20792, 3372,
20816, 3399,
20817, 3501,
20818, 3506,
20821, 3716,
20823, 3908,
20852, 4037,
20865, 4220,
20866, 4403,
20867, 4577,
20868, 4733,
20869, 4898,
20870, 4928,
23072, 5066,
23300, 5129,
23301, 5204,
23302, 5261,
};
Uint16 const co2_SPIrange1rw = CO2_SPI_START_ADDR+0;
Uint16 const co2_SPIrange1rwCRC = CO2_SPI_START_ADDR+8;
Uint16 const co2_SPIrange1rwp = CO2_SPI_START_ADDR+10;
Uint16 const co2_SPIrange1rwpCRC = CO2_SPI_START_ADDR+336;
Uint16 const co2_SPIrange1rwps = CO2_SPI_START_ADDR+338;
Uint16 const co2_SPIrange1rwpsCRC = CO2_SPI_START_ADDR+338;
Uint16 const co2_SPIrange2rw = CO2_SPI_START_ADDR+340;
Uint16 const co2_SPIrange2rwCRC = CO2_SPI_START_ADDR+643;
Uint16 const co2_SPIrange2rwp = CO2_SPI_START_ADDR+645;
Uint16 const co2_SPIrange2rwpCRC = CO2_SPI_START_ADDR+670;
Uint16 const co2_SPIrange2rwps = CO2_SPI_START_ADDR+672;
Uint16 const co2_SPIrange2rwpsCRC = CO2_SPI_START_ADDR+672;
Uint16 const co2_SPIrange3rw = CO1_SPI_START_ADDR+674;
Uint16 const co2_SPIrange3rwCRC = CO1_SPI_START_ADDR+688;
Uint16 const co2_SPIrange3rwp = CO1_SPI_START_ADDR+690;
Uint16 const co2_SPIrange3rwpCRC = CO1_SPI_START_ADDR+690;
Uint16 const co2_SPIrange3rwps = CO1_SPI_START_ADDR+692;
Uint16 const co2_SPIrange3rwpsCRC = CO1_SPI_START_ADDR+692;
Uint16 const co2_SPIrange4rw = CO1_SPI_START_ADDR+694;
Uint16 const co2_SPIrange4rwCRC = CO1_SPI_START_ADDR+694;
Uint16 const co2_SPIrange4rwp = CO1_SPI_START_ADDR+696;
Uint16 const co2_SPIrange4rwpCRC = CO1_SPI_START_ADDR+696;
Uint16 const co2_SPIrange4rwps = CO1_SPI_START_ADDR+698;
Uint16 const co2_SPIrange4rwpsCRC = CO1_SPI_START_ADDR+698;
Uint16 const co2_SPIrange5rw = CO1_SPI_START_ADDR+700;
Uint16 const co2_SPIrange5rwCRC = CO1_SPI_START_ADDR+1373;
Uint16 const co2_SPIrange5rwp = CO1_SPI_START_ADDR+1375;
Uint16 const co2_SPIrange5rwpCRC = CO1_SPI_START_ADDR+1379;
Uint16 const co2_SPIrange5rwps = CO1_SPI_START_ADDR+1381;
Uint16 const co2_SPIrange5rwpsCRC = CO1_SPI_START_ADDR+1381;
Uint16 const co2_first1000 = 0;
Uint16 const co2_first2000 = 82;
Uint16 const co2_first3000 = 194;
Uint16 const co2_first4000 = 198;
Uint16 const co2_first5000 = 198;
// Таблица групп
Uint16 const CO2_TYPE_DEF_TABLE[] = {
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
    (Uint16)4,\
    (Uint16)5,\
    (Uint16)0,\
    (Uint16)-1,\
    (Uint16)10,\
    (Uint16)4,\
    (Uint16)0,\
    (Uint16)5,\
    (Uint16)5,\
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
    (Uint16)46,\
    (Uint16)45,\
};
// Таблица CALLBACK-функций
//**************************************************************
long const CO2_OD_CALLBACK_TBL[]={
0, // 1000h.00h
0, // 1001h.00h
0, // 1002h.00h
0, // 1014h.00h
0, // 1016h.00h
0, // 1017h.00h
0, // 1018h.00h
0, // 1200h.00h
0, // 1280h.00h
0, // 1400h.00h
0, // 1401h.00h
0, // 1402h.00h
0, // 1403h.00h
0, // 1404h.00h
0, // 1405h.00h
0, // 1406h.00h
0, // 1407h.00h
0, // 1600h.00h
0, // 1601h.00h
0, // 1602h.00h
0, // 1603h.00h
0, // 1604h.00h
0, // 1605h.00h
0, // 1606h.00h
0, // 1607h.00h
0, // 1800h.00h
0, // 1801h.00h
0, // 1802h.00h
0, // 1803h.00h
0, // 1804h.00h
0, // 1805h.00h
0, // 1806h.00h
0, // 1807h.00h
0, // 1A00h.00h
0, // 1A01h.00h
0, // 1A02h.00h
0, // 1A03h.00h
0, // 1A04h.00h
0, // 1A05h.00h
0, // 1A06h.00h
0, // 1A07h.00h
0, // 2000h.00h
0, // 2001h.00h
0, // 2002h.00h
0, // 2003h.00h
0, // 2004h.00h
0, // 2005h.00h
0, // 2007h.00h
0, // 2010h.00h
0, // 2011h.00h
0, // 2012h.00h
0, // 2014h.00h
0, // 2015h.00h
0, // 2016h.00h
0, // 2017h.00h
0, // 2080h.00h
0, // 2081h.00h
0, // 2082h.00h
0, // 2083h.00h
0, // 2100h.00h
0, // 2101h.00h
0, // 2102h.00h
0, // 2103h.00h
0, // 2104h.00h
0, // 2105h.00h
0, // 2106h.00h
0, // 2107h.00h
0, // 2108h.00h
0, // 2109h.00h
0, // 210Ah.00h
0, // 210Bh.00h
0, // 210Ch.00h
0, // 210Dh.00h
0, // 210Eh.00h
0, // 210Fh.00h
0, // 2110h.00h
0, // 2111h.00h
0, // 2112h.00h
0, // 2113h.00h
0, // 2114h.00h
0, // 2115h.00h
0, // 2116h.00h
0, // 2117h.00h
0, // 2118h.00h
0, // 2119h.00h
0, // 211Ah.00h
0, // 211Bh.00h
0, // 211Ch.00h
0, // 211Dh.00h
0, // 2502h.00h
0, // 2506h.00h
(long)((void(*)(long))callback_RTC), // 250Fh.00h
0, // 2532h.00h
0, // 2534h.00h
0, // 2600h.00h
(long)((void(*)(long))callback_BTcmd), // 2700h.00h
0, // 2701h.00h
0, // 3000h.00h
0, // 3100h.00h
0, // 5000h.00h
0, // 5100h.00h
0, // 5101h.00h
0, // 5102h.00h
0, // 5103h.00h
0, // 5104h.00h
0, // 5105h.00h
0, // 510Bh.00h
0, // 510Eh.00h
0, // 5112h.00h
0, // 5113h.00h
0, // 5114h.00h
0, // 5116h.00h
0, // 511Ah.00h
0, // 5138h.00h
0, // 5150h.00h
(long)((void(*)(long))callback_dlog), // 5151h.00h
0, // 5152h.00h
0, // 5155h.00h
0, // 5157h.00h
0, // 5174h.00h
0, // 5181h.00h
0, // 5182h.00h
0, // 5183h.00h
0, // 5184h.00h
0, // 5185h.00h
0, // 5186h.00h
0, // 5A20h.00h
0, // 5B04h.00h
0, // 5B05h.00h
0, // 5B06h.00h
};

// Массив ссылок для драйвера CANopen двухядерных процессоров
#ifdef COOD_LINKS__5000
long const CO2_LINKS[]={(long)&co2_numOfInd,
                        (long)&CO2_OD_TBL1,
                        (long)&CO2_OD_TBL2,
                        (long)&CO2_OD_TBL3,
                        (long)&CO2_TYPE_DEF_TABLE,
                        (long)&CO2_OD_CALLBACK_TBL,
                        (long)&co2_SPIrange1rw,
                        (long)&co2_SPIrange1rwCRC,
                        (long)&co2_SPIrange1rwp,
                        (long)&co2_SPIrange1rwpCRC,
                        (long)&co2_SPIrange1rwps,
                        (long)&co2_SPIrange1rwpsCRC,
                        (long)&co2_SPIrange2rw,
                        (long)&co2_SPIrange2rwCRC,
                        (long)&co2_SPIrange2rwp,
                        (long)&co2_SPIrange2rwpCRC,
                        (long)&co2_SPIrange2rwps,
                        (long)&co2_SPIrange2rwpsCRC,
                        (long)&co2_SPIrange3rw,
                        (long)&co2_SPIrange3rwCRC,
                        (long)&co2_SPIrange3rwp,
                        (long)&co2_SPIrange3rwpCRC,
                        (long)&co2_SPIrange3rwps,
                        (long)&co2_SPIrange3rwpsCRC,
                        (long)&co2_SPIrange4rw,
                        (long)&co2_SPIrange4rwCRC,
                        (long)&co2_SPIrange4rwp,
                        (long)&co2_SPIrange4rwpCRC,
                        (long)&co2_SPIrange4rwps,
                        (long)&co2_SPIrange4rwpsCRC,
                        (long)&co2_SPIrange5rw,
                        (long)&co2_SPIrange5rwCRC,
                        (long)&co2_SPIrange5rwp,
                        (long)&co2_SPIrange5rwpCRC,
                        (long)&co2_SPIrange5rwps,
                        (long)&co2_SPIrange5rwpsCRC,
                        (long)&co2_first1000,
                        (long)&co2_first2000,
                        (long)&co2_first3000,
                        (long)&co2_first4000,
                        (long)&co2_first5000
};

STATIC_ASSERT(((sizeof(CO2_LINKS)/sizeof(CO2_LINKS[0])) == (COOD_LINKS__5000 + 1)),m);
#endif
