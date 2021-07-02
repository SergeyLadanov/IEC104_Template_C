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

#include "main.h"

//-----------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define IEC104_START_ID 0x68
//--------------------------
#define MAX_BUFFER_LENGTH 512
#define RECEIVE_TIME_OUT 50
//--------------------------
#define get_type(a)  (a & 0x03)
//--------------------------
#define I_TYPE 0x00
#define S_TYPE 0x01
#define U_TYPE 0x03
//--------------------------
typedef enum{
	STOPDT  = 0,
	STARTDT = 1
} iec104_stateTypeDef;
//-------------------------------
typedef enum{
	OK = 0,
	ERR = 1
} iec104_statusTypeDef;

/*********ЗАГОЛОВОК IEC 104******************/
typedef struct{
	uint8_t start; // Идентификатор 0x68
	uint8_t apdu_len;
	union{
		uint8_t type;
		uint8_t data[4];
	};
	uint8_t asdu[];
} iec_104_hdr;
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
//--------------------------
typedef struct{
	uint16_t tx_count;
	uint16_t rx_count;
} apci_I_format;
//---------------------------
typedef struct{
	uint16_t unused;
	uint16_t rx_count;
} apci_S_format;
//---------------------------
typedef struct{
	uint8_t control; //блок контроля
	uint8_t unused[3]; //не используется
} apci_U_format;



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

//Классификатор типа
#define SQ_FALSE 0x00
#define SQ_TRUE 0x80

//-----------------------------------------------------

//Причины передачи
#define CAUSE_PER_CYC 1  //Периодическая отправка
#define CAUSE_BACK    2  // Фоновое сканирование
#define CAUSE_SPONT   3  //Спорадическая передача
#define CAUSE_INIT    4  //Инициализация

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





#define IEC104_CREATE_DATA_SET(Name, Size) iec104_asduDataSet Name; iec104_objTypeDef Name ## Raw[Size]
#define IEC104_INIT_DATA_SET(Name) iec104_initAsduDataSet(&Name, Name ## Raw, sizeof(Name ## Raw)/sizeof(iec104_objTypeDef))
#define IEC104_INIT_ASDU(Name) iec104_attachAsduData(Name, sizeof(Name)/sizeof(iec104_asduBlock))

typedef enum
{
  SUCCESS_104 = 0U,
  ERROR_104 = !SUCCESS_104
} ErrorStatus;

typedef struct {
	uint8_t type;
	uint8_t classificator; //Содержит количество величин и SQ
	uint8_t causeTx;
	uint8_t OA;
	uint16_t asduAddr;
	uint8_t data[];
}asdu_hdr_typeDef;
//---------------------------
#define WRITE_IO_ADDRESS(a,b) b[0] = (uint8_t)a; b[1] = (uint8_t)(a >> 8); b[2] = (uint8_t)(a >> 16);

//Структуры объектов различных типов
typedef struct {
	uint8_t addr[3];
	uint8_t SIQ;
	uint8_t next[];
}M_SP_NA_1_IOtypeDef;

//typedef struct {

//}M_ME_NB_1_IOtypeDef;
//------------------------------------------
#define IEC104_IV (1<<7)
#define IEC104_NT (1<<6)
#define IEC104_SB (1<<5)
#define IEC104_BL (1<<4)
#define IEC104_OV (1<<0)
//------------------------------------------
#pragma pack(push,1) // Выравнивание структуры в памяти по 1 байту
typedef struct {
	uint8_t addr[3];
	uint16_t value;
	uint8_t QDS;
	uint8_t next[];
}M_ME_NA_1_IOtypeDef;
#pragma pack(pop) // Возвращение предыдущих настроек выравнивания в памяти
//------------------------------------------
#pragma pack(push,1) // Выравнивание структуры в памяти по 1 байту
typedef struct {
	uint8_t addr[3];
	float value;
	uint8_t QDS;
	uint8_t next[];
}M_ME_NC_1_IOtypeDef;
#pragma pack(pop) // Возвращение предыдущих настроек выравнивания в памяти


//------------------------------------------
typedef struct {
	uint32_t addr;
	uint32_t length;
}groupInfoTypeDef;
//-------------------------------------------
//typedef struct {
//	RG_groop_mek104_t group_mek104;
//	uint32_t a_obj;
//	uint32_t time_counter;
//}groupParametersTypeDef;
/*****************************************/

typedef struct {
	uint8_t *Data;
	uint16_t Len;
	uint16_t Capacity;
}ByteBufferTypeDef;
/*****************************************/

typedef struct {
	uint8_t state;  //Состояние соединения
	uint16_t tx_count; //переданное количество сообщений
	uint16_t rx_count; //принятое количество сообщений
	uint8_t cyclyc_tx; //количество переданных циклических сообщений
	ByteBufferTypeDef RxBuf;
	ByteBufferTypeDef TxBuf;
	uint16_t no_ask_counter;
	uint32_t timer;
} iec_104_propTypeDef;

//Структура группы параметров
typedef struct{
	uint32_t AdrObj;
	union{
		uint8_t U8;
		uint16_t U16;
		uint32_t U32;
		float FVal;
	} Val;
}iec104_objTypeDef;

//---------------------------------

typedef struct{
	uint16_t Capacity;
	iec104_objTypeDef *Data;
}iec104_asduDataSet;
//---------------------------------
typedef struct{
	uint16_t AdrAsdu;
	uint8_t Idt;
	iec104_asduDataSet *Objects;
}iec104_asduBlock;
//---------------------------------
typedef struct{
	uint16_t Capacity;
	iec104_asduBlock *Data;
}iec104_dataSettings;

//---------------------------
void iec104_ini(void);
void TCP_sys_timer(void);
void iec104_process(void);
void timeOutHandler(void);
void iec_104_conn_close(iec_104_propTypeDef *iec104_prop);




void iec104_AttachBuffer(ByteBufferTypeDef *Buffer, uint8_t *Data, uint16_t Len);
void iec104_SetDataBuffer(ByteBufferTypeDef *Buffer, uint8_t *Data, uint16_t Len);
void iec104_CopyDataToBuffer(ByteBufferTypeDef *Buffer, uint8_t *Data, uint16_t Len);
void iec_104_read(iec_104_propTypeDef *iec104_prop);
void iec104_PacketHandler(iec_104_propTypeDef *iec104_prop);


void iec104_initAsduDataSet(iec104_asduDataSet *DataSet, iec104_objTypeDef *DataArray, uint16_t Capacity);
void iec104_attachAsduData(iec104_asduBlock *Data, uint16_t Capacity);

uint8_t iec104_setFloat(uint8_t asduAdr, uint32_t ioAdr, float val);
uint8_t iec104_setByte(uint8_t asduAdr, uint32_t ioAdr, uint8_t val);
uint8_t iec104_setHalfWord(uint8_t asduAdr, uint32_t ioAdr, uint16_t val);

#endif	/* __IEC104_H */
