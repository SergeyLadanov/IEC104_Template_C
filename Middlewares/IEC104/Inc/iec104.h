/*
 * Copyright (C) 2005, Grigoriy Sitkarev                                 
 * sitkarev@komitex.ru                                                
 * Copyright (C) 2007, Vladimir Lettiev                                 
 * lettiev-vv@komi.tgc-9.ru                                                
 *                                                                       
 * This program is free software; you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation; either version 2 of the License, or     
 * (at your option) any later version.                                   
 *                                                                       
 * This program is distributed in the hope that it will be useful,       
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
 * GNU General Public License for more details.                          
 *                                                                       
 * You should have received a copy of the GNU General Public License     
 * along with this program; if not, write to the                         
 * Free Software Foundation, Inc.,                                       
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             
 */

#ifndef __IEC104_H
#define __IEC104_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define IEC104_START_ID 0x68

#define IEC104_TMP_BUF_SIZE 1024
//--------------------------
#define MAX_BUFFER_LENGTH 512
#define RECEIVE_TIME_OUT 50
//--------------------------
#define get_type(a)  (a & 0x03)
//--------------------------
#define I_TYPE 0x00
#define S_TYPE 0x01
#define U_TYPE 0x03

/***********ТИПЫ APCI***************************/
#define STARTDT_ACT (1<<2)
#define STARTDT_CON (1<<3)

#define STOPDT_ACT  (1<<4)
#define STOPDT_CON 	(1<<5)

#define TESTFR_ACT 	(1<<6)
#define TESTFR_CON 	(1<<7)
//--------------------------
//команды
#define C_IC_NA_1 100
#define C_CS_NA_1 103

#define MEK104_M1_IV 0x80   // (invalid/valid): 0 - действительная, 1 - недействительная.
#define MEK104_M1_NT 0x40   // (not topical/topical): 0 - актуальное значение, 1 - неактуальное значение
#define MEK104_M1_SB 0x20   // (substituted/not substituted): 0 - нет замещения, 1 - есть замещение
#define MEK104_M1_BL 0x10   // (blocked/ not blocked): 0 - нет блокировки, 1 - есть блокировка
#define MEK104_M1_IN 0x08   // 1 - инверсия
#define MEK104_M1_GN 0x04   // (general) 1- обобщенная величина
//#define MEK104_M1_00 0x02 // всегда 0
#define MEK104_M1_SPI 0x01  // (single point information) 0 - лог.ноль, 1 - лог.единица
#define MEK104_M1_ALL_OK 0  // все в норме



// M9 - передача в направлении контроля M_ME_NA (код9) и M_ME_NC(код 13)
#define MEK104_M9_IV 0x80   // (invalid/valid): 0 - действительная, 1 - недействительная.
#define MEK104_M9_NT 0x40   // (not topical/topical): 0 - актуальное значение, 1 - неактуальное значение
#define MEK104_M9_SB 0x20   // (substituted/not substituted): 0 - нет замещения, 1 - есть замещение
#define MEK104_M9_BL 0x10   // (blocked/ not blocked): 0 - нет блокировки, 1 - есть блокировка
#define MEK104_M9_AV 0x08   // 0 - мгновенная величина, 1 - средняя величина
//#define MEK104_M9_00 0x04 // всегда 0
//#define MEK104_M9_00 0x02 // всегда 0
#define MEK104_M9_OV 0x01  // (OVERFLOW/NO OVERFLOW) 0 - нет нарушения пределов, 1 - есть нарушение пределов
#define MEK104_M9_ALL_OK 0  // все в норме

// M15 - передача в направлении контроля M_ME_NA (код9) и
#define MEK104_M15_IV 0x80   // (invalid/valid): 0 - действительная, 1 - недействительная.
#define MEK104_M15_CA 0x40   // 1/0 -после последнего считывания счетчик был установлен/не уст-н
#define MEK104_M15_CY 0x20   // 0 - за соответствующий период интегрирования не произошло переполнение счетчика
#define MEK104_M15_SQ 0x1F   // номер передаваемой последовательности
#define MEK104_M15_ALL_OK 0  // все в норме

/**************БЛОК ASDU****************/
//Типы ASDU
#define M_SP_NA_1 1 //Одноэлементная информация
#define M_SP_TA_1 2 //Одноэлементая информация с меткой времени
#define M_DP_NA_1 3 //Двухэлементная информация
#define M_DP_TA_1 4 //Двухэлементная информация с меткой времени
#define M_ST_NA_1 5 //Информация о положении отпаек
#define M_ST_TA_1 6 //Информация о положении отпаек с меткой времени
#define M_BO_NA_1 7
#define M_BO_TA_1 8
#define M_ME_NA_1 9
#define M_ME_TA_1 10
#define M_ME_NB_1 11
#define M_ME_TB_1 12
#define M_ME_NC_1 13
#define M_ME_TC_1 14
#define M_IT_NA_1 15


#define M_SP_TB_1 30
#define M_ME_TF_1 36

//Классификатор типа
#define SQ_FALSE 0x00
#define SQ_TRUE 0x80

//-----------------------------------------------------

//Причины передачи
#define CAUSE_PER_CYC   1  //Периодическая отправка
#define CAUSE_BACK      2  // Фоновое сканирование
#define CAUSE_SPONT     3  //Спорадическая передача
#define CAUSE_INIT      4  //Инициализация
#define CAUSE_INROGEN   20 //Общий опрос
#define CAUSE_ACTCON    7  // Активация
#define CAUSE_END_ACT   10 // Завершение активации

//Флаги
#define CAUSE_PN_FLAG (1<<6)
#define CAUSE_T_FLAG  (1<<7)
//-----------------------------------------------------

#define ASDU_ADDR_0 0
#define ASDU_ADDR_1 1
#define ASDU_ADDR_2 2
#define ASDU_ADDR_3 3
#define ASDU_ADDR_4 4
#define ASDU_ADDR_5 5
#define ASDU_ADDR_6 6
#define ASDU_ADDR_7 7
#define ASDU_ADDR_8 8
#define ASDU_ADDR_9 9
#define ASDU_ADDR_10 10


#define IEC104_CREATE_DATA_SET(Name, Size) IEC104_ASDU_DataSet Name; IEC104_IO_Obj Name ## Raw[Size]
#define IEC104_INIT_DATA_SET(Name) IEC104_InitAsduDataSet(&Name, Name ## Raw, sizeof(Name ## Raw)/sizeof(IEC104_IO_Obj))
#define IEC104_INIT_ASDU(ObjName, AsduName) IEC104_AttachAsduData(&ObjName, AsduName, sizeof(AsduName)/sizeof(IEC104_ASDU_Block))

#define WRITE_IO_ADDRESS(a,b) b[0] = (uint8_t)a; b[1] = (uint8_t)(a >> 8); b[2] = (uint8_t)(a >> 16);

#define IEC104_IV (1<<7)
#define IEC104_NT (1<<6)
#define IEC104_SB (1<<5)
#define IEC104_BL (1<<4)
#define IEC104_OV (1<<0)

//-------------------------------------------
typedef struct {
	uint8_t *Data;
	uint16_t Len;
	uint16_t Capacity;
}IEC104_ByteBufferTypeDef;

//Структура группы параметров
typedef struct{
	uint32_t AdrObj;
	union{
		uint8_t U8;
		uint16_t U16;
		uint32_t U32;
		float FVal;
	} Val;
}IEC104_IO_Obj;

//---------------------------------
typedef struct{
	uint16_t Capacity;
	IEC104_IO_Obj *Data;
}IEC104_ASDU_DataSet;

//---------------------------------
typedef struct{
	uint16_t AdrAsdu;
	uint8_t Idt;
	IEC104_ASDU_DataSet *Objects;
}IEC104_ASDU_Block;

//---------------------------------
typedef struct {
	uint8_t State;  //Состояние соединения
	uint16_t TxCount; //переданное количество сообщений
	uint16_t RxCount; //принятое количество сообщений
	IEC104_ByteBufferTypeDef RxBuf;
	IEC104_ByteBufferTypeDef TxBuf;
	uint16_t NoAskCnt;
	uint32_t Timer;
	uint16_t Capacity;
	IEC104_ASDU_Block *Data;
} IEC104_Obj;



// Обработчики событий
void IEC104_PreInrogenRepplyCallback(IEC104_Obj *hiec);
void IEC104_PreSendSporadicCallback(IEC104_Obj *hiec);
void IEC104_PreSendCyclicCallback(IEC104_Obj *hiec);

//---------------------------
void IEC104_Con_Close(IEC104_Obj *iec104_prop);
void IEC104_SetTxData(IEC104_Obj *hiec, uint8_t *Data, uint16_t Len);
void IEC104_SetRxData(IEC104_Obj *hiec, uint8_t *Data, uint16_t Len);
void IEC104_PacketHandler(IEC104_Obj *iec104_prop);


void IEC104_InitAsduDataSet(IEC104_ASDU_DataSet *DataSet, IEC104_IO_Obj *DataArray, uint16_t Capacity);
void IEC104_AttachAsduData(IEC104_Obj *hiec, IEC104_ASDU_Block *Data, uint16_t Capacity);

uint8_t IEC104_SetFloat(IEC104_Obj *hiec, uint8_t asduAdr, uint32_t ioAdr, float val);
uint8_t IEC104_SetByte(IEC104_Obj *hiec, uint8_t asduAdr, uint32_t ioAdr, uint8_t val);
uint8_t IEC104_SetHalfWord(IEC104_Obj *hiec, uint8_t asduAdr, uint32_t ioAdr, uint16_t val);
void IEC104_CyclicPacket_Prepare(IEC104_Obj *iec104_prop);
void IEC104_SporadicPacket_Prepare(IEC104_Obj *iec104_prop);
struct tm IEC104_GetTime(void);


IEC104_ASDU_Block *IEC104_GetAsduByIndex(IEC104_Obj *hiec, uint8_t index);
void IEC104_SetAsduType(IEC104_ASDU_Block *asdu, uint8_t idt);

#ifdef __cplusplus
}
#endif

#endif	/* __IEC104_H */
