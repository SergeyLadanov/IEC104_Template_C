/*
 * iec104.c
 *
 *  Created on: 1 сент. 2020 г.
 *      Author: serge
 */

#include "iec104.h"
#include <string.h>


#define NO_ASK_SENDS 10


#ifndef UNUSED
#define UNUSED(X) (void)X
#endif

#ifndef __weak
#define __weak __attribute__((weak))
#endif


//--------------------------
typedef enum{
	STOPDT  = 0,
	STARTDT = 1
} IEC104_StateTypeDef;
//-------------------------------
typedef enum{
	OK = 0,
	ERR = 1
} IEC104_StatusTypeDef;

typedef enum
{
  SUCCESS_104 = 0U,
  ERROR_104 = !SUCCESS_104
} IEC104_ErrorStatus;

/*********ЗАГОЛОВОК IEC 104******************/
#pragma pack(push,1) // Выравнивание структуры в памяти по 1 байту
typedef struct{
	uint8_t Start; // Идентификатор 0x68
	uint8_t APDU_Len;
	union{
		uint8_t Type;
		uint8_t Data[4];
	};
	uint8_t ASDU[];
} IEC104_Header;

// Блок ASDU
typedef struct {
	uint8_t Type;
	uint8_t Classificator; //Содержит количество величин и SQ
	uint8_t CauseTx;
	uint8_t OA;
	uint16_t ASDU_Addr;
	uint8_t Data[];
}IEC104_ASDU_Header;


//--------------------------
typedef struct{
	uint16_t TxCount;
	uint16_t RxCount;
} IEC104_APCI_I_Format;
//---------------------------
typedef struct{
	uint16_t Unused;
	uint16_t RxCount;
} IEC104_APCI_S_Format;
//---------------------------
typedef struct{
	uint8_t Control; //блок контроля
	uint8_t Unused[3]; //не используется
} IEC104_APCI_U_Format;


//Структуры объектов различных типов
typedef struct {
	uint8_t Addr[3];
	uint8_t SIQ;
	uint8_t Next[];
}M_SP_NA_1_IOtypeDef;

typedef struct {
	uint8_t Addr[3];
	uint8_t SIQ;
	uint8_t CP56Time[7];
	uint8_t Next[];
}M_SP_TB_1_IOtypeDef;

//------------------------------------------
typedef struct {
	uint8_t Addr[3];
	uint16_t Value;
	uint8_t QDS;
	uint8_t Next[];
}M_ME_NA_1_IOtypeDef;
//------------------------------------------

typedef struct {
	uint8_t Addr[3];
	float Value;
	uint8_t QDS;
	uint8_t Next[];
}M_ME_NC_1_IOtypeDef;
//--------------------------------------------

typedef struct {
	uint8_t Addr[3];
	float Value;
	uint8_t QDS;
	uint8_t CP56Time[7];
	uint8_t Next[];
}M_ME_TF_1_IOtypeDef;
#pragma pack(pop) // Возвращение предыдущих настроек выравнивания в памяти


static void IEC104_Read(IEC104_Obj *iec104_prop);

// Функция получения текущего времени
__weak struct tm IEC104_GetTime(void)
{
	struct tm tim = {0};
	UNUSED(tim);
	return tim;
}


// Обработка приема команды на общий опрос станции
__weak void IEC104_PreInrogenRepplyCallback(IEC104_Obj *hiec)
{
	UNUSED(hiec);
}

// Функция получения текущего времени
__weak void IEC104_PreSendSporadicCallback(IEC104_Obj *hiec)
{
	UNUSED(hiec);
}

// Функция получения текущего времени
__weak void IEC104_PreSendCyclicCallback(IEC104_Obj *hiec)
{
	UNUSED(hiec);
}

// Получение ASDU по индексу
IEC104_ASDU_Block *IEC104_GetAsduByIndex(IEC104_Obj *hiec, uint8_t index)
{
	return &hiec->Data[index];
}


// Изменение типа группы
void IEC104_SetAsduType(IEC104_ASDU_Block *asdu, uint8_t idt)
{
	asdu->Idt = idt;
}

// Преобразование временик массиву
static void IEC104_SetCP56Time(struct tm *in, uint8_t *out)
{
	// Милисекунды
	out[0] = 0;
	out[1] = 0;
	// Минуты
	out[2] = in->tm_min;
	// Флаги минут
	out[2] |= 0;
	// Часы
	out[3] = in->tm_hour;
	// Флаги часов
	out[3] |= 0;
	// Дни недели и дни месяца
	out[4] = (in->tm_wday << 5) | (in->tm_mday);
	// Месяц
	out[5] = (in->tm_mon + 1);
	// Год
	out[6] = (in->tm_year - 100);
}


//Функция установки значения в модель данных
static uint8_t IEC104_SetVal(IEC104_Obj *hiec, uint8_t asduAdr, uint32_t ioAdr, void *val)
{
	uint8_t result = 1;
	for (uint8_t i = 0; i < hiec->Capacity; i++)
	{
		if (hiec->Data[i].AdrAsdu == asduAdr)
		{
			for (uint8_t j = 0; j < hiec->Data[i].Objects->Capacity; j++)
			{
				if (hiec->Data[i].Objects->Data[j].AdrObj == ioAdr)
				{
					switch(hiec->Data[i].Idt)
					{
					case M_SP_NA_1:
					case M_SP_TB_1:
						hiec->Data[i].Objects->Data[j].Val.U8 = *((uint8_t *)val);
						break;
					case M_ME_NA_1:
						hiec->Data[i].Objects->Data[j].Val.U16 = *((uint16_t *)val);
						break;
					case M_ME_NC_1:
					case M_ME_TF_1:
						hiec->Data[i].Objects->Data[j].Val.FVal = *((float *)val);
						break;
					}
					return 0;
				}
			}
		}
	}

	return result;
}

//-----------------------------------------------
uint8_t IEC104_SetFloat(IEC104_Obj *hiec, uint8_t asduAdr, uint32_t ioAdr, float val)
{
	return IEC104_SetVal(hiec, asduAdr, ioAdr, &val);
}
//------------------------------------------------
uint8_t IEC104_SetByte(IEC104_Obj *hiec, uint8_t asduAdr, uint32_t ioAdr, uint8_t val)
{
	return IEC104_SetVal(hiec, asduAdr, ioAdr, &val);
}
//--------------------------------------------------------------------------
uint8_t IEC104_SetHalfWord(IEC104_Obj *hiec, uint8_t asduAdr, uint32_t ioAdr, uint16_t val)
{
	return IEC104_SetVal(hiec, asduAdr, ioAdr, &val);
}
//-----------------------------------------------------------------------
void IEC104_InitAsduDataSet(IEC104_ASDU_DataSet *DataSet, IEC104_IO_Obj *DataArray, uint16_t Capacity)
{
	DataSet->Capacity = Capacity;
	DataSet->Data = DataArray;
}
//------------------------------------------------------
void IEC104_AttachAsduData(IEC104_Obj *hiec, IEC104_ASDU_Block *Data, uint16_t Capacity)
{
	hiec->Data = Data;
	hiec->Capacity = Capacity;
}

// Функция привязки передающего буфера
void IEC104_SetTxData(IEC104_Obj *hiec, uint8_t *Data, uint16_t Len)
{
	hiec->TxBuf.Data = Data;
	hiec->TxBuf.Capacity = Len;
	hiec->TxBuf.Len = 0;
}

// Функция привязки приемного буфера
void IEC104_SetRxData(IEC104_Obj *hiec, uint8_t *Data, uint16_t Len)
{
	hiec->RxBuf.Data = Data;
	hiec->RxBuf.Capacity = Len;
	hiec->RxBuf.Len = Len;
}

// Запрос на отправку данных
static void IEC104_CopyDataToBuffer(IEC104_ByteBufferTypeDef *Buffer, uint8_t *Data, uint16_t Len)
{
	if (Len > Buffer->Capacity)
	{
		Len = Buffer->Capacity;
	}

	memcpy(&Buffer->Data[Buffer->Len], Data, Len);
	Buffer->Len += Len;
}

// Обработка пакета
void IEC104_PacketHandler(IEC104_Obj *iec104_prop)
{
	if (iec104_prop->RxBuf.Data[0] == IEC104_START_ID)
	{
		IEC104_Read(iec104_prop);
	}
}
/****************************************************************/
void IEC104_Con_Close(IEC104_Obj *iec104_prop)
{
    iec104_prop->State = STOPDT;
    iec104_prop->TxCount = 0;
    iec104_prop->RxCount = 0;
}
//-------------------------------------------------------------------
static void IEC104_APCI_Prepare(IEC104_Obj *iec104_prop, IEC104_Header *iec_104_pkt)
{
	uint8_t type = get_type(iec_104_pkt->Type);

	if ((type & 0x01) == 0)
		type = 0;


	if (type == I_TYPE)
	{
		IEC104_APCI_I_Format *apci_I_pkt = (void *)iec_104_pkt->Data;
		apci_I_pkt->TxCount = iec104_prop->TxCount;
		apci_I_pkt->RxCount = iec104_prop->RxCount;
		iec104_prop->TxCount += 2;

	}
	else if (type == S_TYPE)
	{
		IEC104_APCI_I_Format *apci_S_pkt = (void *)iec_104_pkt->Data;
		apci_S_pkt->RxCount = iec104_prop->RxCount;

	}
	else if (type == U_TYPE)
	{
		//IEC104_APCI_I_Format *apci_U_pkt = (void *)iec_104_pkt->data;

	}

}
//---------------------------------------------------------------
static void IEC104_PacketPrepare(IEC104_Header *iec_104_pkt, uint8_t type, uint8_t len)
{
	//Заполнение типа протокола и длины APDU
	iec_104_pkt->Start = 0x68;
	iec_104_pkt->APDU_Len = len;
	iec_104_pkt->Type = type;
}
//---------------------------------------------------------------
static uint8_t IEC104_ASDU_Prepare(IEC104_ASDU_Header *asdu_pkt, uint8_t type, uint16_t asduAddr, uint8_t sq, uint8_t causeTx, uint8_t OA, uint8_t numIx)
{
	asdu_pkt->Type = type;
	asdu_pkt->Classificator = sq | numIx;
	asdu_pkt->CauseTx = causeTx;
	asdu_pkt->OA = OA;
	asdu_pkt->ASDU_Addr = asduAddr;

	return sizeof(IEC104_ASDU_Header);
}
//---------------------------------------------------------------
//функция подготовки блока со значением float
static uint8_t M_ME_NC_1_prepare(M_ME_NC_1_IOtypeDef *io_pkt, uint32_t addr, float value, uint8_t QDS)
{
	WRITE_IO_ADDRESS(addr,io_pkt->Addr);
	io_pkt->Value = value;
	io_pkt->QDS = QDS;

	return sizeof(M_ME_NC_1_IOtypeDef);
}
//--------------------------------------------------------------
//функция подготовки блока со значением float и с меткой времени
static uint8_t M_ME_TF_1_prepare(M_ME_TF_1_IOtypeDef *io_pkt, uint32_t addr, float value, uint8_t QDS, struct tm *u)
{
	WRITE_IO_ADDRESS(addr,io_pkt->Addr);
	io_pkt->Value = value;
	io_pkt->QDS = QDS;
	IEC104_SetCP56Time(u, io_pkt->CP56Time);

	return sizeof(M_ME_TF_1_IOtypeDef);
}
//--------------------------------------------------------------
//функция подготовки блока двухбайтным значением
static uint8_t M_ME_NA_1_prepare(M_ME_NA_1_IOtypeDef *io_pkt, uint32_t addr, uint16_t value, uint8_t QDS)
{
	WRITE_IO_ADDRESS(addr,io_pkt->Addr);
	io_pkt->Value = value;
	io_pkt->QDS = QDS;

	return sizeof(M_ME_NA_1_IOtypeDef);
}
//--------------------------------------------------------------
//функция подготовки блока с однобайтным значением
static uint8_t M_SP_NA_1_prepare(M_SP_NA_1_IOtypeDef *io_pkt, uint32_t addr, uint8_t SIQ)
{
	WRITE_IO_ADDRESS(addr,io_pkt->Addr);
	io_pkt->SIQ = SIQ;
	return sizeof(M_SP_NA_1_IOtypeDef);
}
//---------------------------------------------------------------
//функция подготовки блока с однобайтным значением с меткой времени
static uint8_t M_SP_TB_1_prepare(M_SP_TB_1_IOtypeDef *io_pkt, uint32_t addr, uint8_t SIQ, struct tm *u)
{
	WRITE_IO_ADDRESS(addr,io_pkt->Addr);
	io_pkt->SIQ = SIQ;
	IEC104_SetCP56Time(u, io_pkt->CP56Time);
	return sizeof(M_SP_TB_1_IOtypeDef);
}

//---------------------------------------------------------------
static uint8_t* IEC104_WriteValue(IEC104_IO_Obj *obj, uint8_t Idt, uint8_t *buf, IEC104_ErrorStatus stat_request, uint8_t* length, uint8_t number)
{

	if ( Idt == M_SP_NA_1 ) {  // 1байт
		M_SP_NA_1_IOtypeDef *io_pkt = (void *)buf;
		*length += M_SP_NA_1_prepare(io_pkt,obj->AdrObj,MEK104_M1_ALL_OK | obj->Val.U8);
		return 	(void *)io_pkt->Next; //возвращение указателя на блок со следующим значением
	}



	if ( Idt == M_ME_NA_1 ) {   // 2 байта + байт качества
		 M_ME_NA_1_IOtypeDef *io_pkt = (void *)buf;
		*length += M_ME_NA_1_prepare(io_pkt, obj->AdrObj,obj->Val.U16,MEK104_M9_ALL_OK);

		return 	(void *)io_pkt->Next; //возвращение указателя на блок со следующим значением
	}

	if ( Idt == M_ME_NC_1 )    // 4 байта float + байт качества
	{
		M_ME_NC_1_IOtypeDef *io_pkt = (void *)buf;
		*length += M_ME_NC_1_prepare(io_pkt, obj->AdrObj, obj->Val.FVal, MEK104_M9_ALL_OK);
		return 	(void *)io_pkt->Next;	//возвращение указателя на блок со следующим значением
	}

// Объекты с метками времени
	if ( Idt == M_SP_TB_1 )    // 1 байт + байт качества + метка времени
	{
		struct tm tim = IEC104_GetTime();
		M_SP_TB_1_IOtypeDef *io_pkt = (void *)buf;
		*length += M_SP_TB_1_prepare(io_pkt,obj->AdrObj,MEK104_M1_ALL_OK | obj->Val.U8, &tim);
	
		return 	(void *)io_pkt->Next;	//возвращение указателя на блок со следующим значением
	}


	if ( Idt == M_ME_TF_1 )    // 4 байта float + байт качества + метка времени
	{
		struct tm tim = IEC104_GetTime();
		M_ME_TF_1_IOtypeDef *io_pkt = (void *)buf;
		*length += M_ME_TF_1_prepare(io_pkt, obj->AdrObj, obj->Val.FVal, MEK104_M9_ALL_OK, &tim);
		return 	(void *)io_pkt->Next;	//возвращение указателя на блок со следующим значением
	}

	return NULL;
}


static uint8_t IEC104_GetBaseData(IEC104_IO_Obj *obj, uint8_t Idt, uint8_t *buf, uint8_t num)
{
	IEC104_ErrorStatus stat_request = SUCCESS_104;
	uint8_t   length = 0;
	uint32_t adr_tmp = obj->AdrObj; //сохраняется адрес текущей группы

	// расположим данные в buf.data.send в соответствии с объектами информации кадра МЭК104
	for(uint8_t i = 0; i < num; i++)
	{
			buf = IEC104_WriteValue(&obj[i], Idt, buf, stat_request, &length, i);
	}

	obj->AdrObj = adr_tmp;

	return length;
}

//-----------------------------------------------------------
static uint8_t IEC104_GroupDataPrepare(IEC104_Obj *iec104_prop, uint8_t * buf, IEC104_ASDU_Block *groupParam, uint8_t caseTx)
{
	uint8_t total_len = 0;
	IEC104_Header *iec_104_pkt = (void *)buf; //Указатель на структуру пакета 104 протокола приравнивается адресу буфера передачи
//--------------------------------------------------------
	IEC104_ASDU_Header *asdu_pkt = (void*)iec_104_pkt->ASDU;
  //20 - причина передачи CAUSE_INROGEN
	// Количество блоко устанавливается здесь
	total_len += IEC104_ASDU_Prepare(asdu_pkt, groupParam->Idt, groupParam->AdrAsdu, SQ_FALSE, caseTx, 0, groupParam->Objects->Capacity); //подготовка блока ASDU

	total_len += IEC104_GetBaseData(groupParam->Objects->Data, groupParam->Idt, (void *)asdu_pkt->Data, groupParam->Objects->Capacity); //Запись значений из группы в пакет

	total_len += sizeof(IEC104_Header); //добавляется к общей длине пакета размер заголовка пакета
	IEC104_PacketPrepare(iec_104_pkt, I_TYPE, total_len - 2); //Ставится тип пакета, и длина
	IEC104_APCI_Prepare(iec104_prop, iec_104_pkt); // подготовка поля apci


	return total_len;

}

//------------------------------------------------------------
static void IEC104_Read(IEC104_Obj *iec104_prop)
{
	uint8_t dataBuf[IEC104_TMP_BUF_SIZE];
	IEC104_Header *iec_104_pkt = (IEC104_Header *) iec104_prop->RxBuf.Data;

	iec_104_pkt = (void*)iec104_prop->RxBuf.Data;

	uint8_t type = get_type(iec_104_pkt->Type);

	//Если нулевой бит равен 0, то это тип I
	 if ((type & 0x01) == 0)
		type = 0;

	if (type == I_TYPE) //Если принят пакет типа I
	{
		iec104_prop->RxCount += 2;
		IEC104_ASDU_Header *asdu_pkt = (void*)iec_104_pkt->ASDU;

		if (asdu_pkt->Type == C_IC_NA_1) // ответ на команду общего опроса
		{
			//-----------------------------------------------------------------
			IEC104_ASDU_Prepare(asdu_pkt, C_IC_NA_1, ASDU_ADDR_1, SQ_FALSE, CAUSE_ACTCON, 0, 1);
			M_SP_NA_1_IOtypeDef *io_pkt = (void *)asdu_pkt->Data;
			M_SP_NA_1_prepare(io_pkt, 0, 20);
			IEC104_APCI_Prepare(iec104_prop, iec_104_pkt);

			IEC104_CopyDataToBuffer(&iec104_prop->TxBuf, iec104_prop->RxBuf.Data, iec_104_pkt->APDU_Len + 2);

			IEC104_PreInrogenRepplyCallback(iec104_prop);

			uint8_t len = 0;
			for (uint8_t i = 0; i < iec104_prop->Capacity; i++)
			{

				len = IEC104_GroupDataPrepare(iec104_prop, dataBuf, &iec104_prop->Data[i], CAUSE_INROGEN);
				IEC104_CopyDataToBuffer(&iec104_prop->TxBuf, dataBuf, len);
			}


			//завершение активации
			len = 0;
			iec_104_pkt = (void *)dataBuf;
			//--------------------------------------------------------
			asdu_pkt = (void*)iec_104_pkt->ASDU;
			io_pkt = (void *)asdu_pkt->Data;
			len += IEC104_ASDU_Prepare(asdu_pkt, C_IC_NA_1, ASDU_ADDR_1, SQ_FALSE, CAUSE_END_ACT, 0, 1); //подготовка блока ASDU
			len += M_SP_NA_1_prepare(io_pkt, 0, 20);

			len += sizeof(IEC104_Header); //добавляется к общей длине пакета размер заголовка пакета
			IEC104_PacketPrepare(iec_104_pkt, I_TYPE, len - 2); //Ставится тип пакета, и длина
			IEC104_APCI_Prepare(iec104_prop, iec_104_pkt); // подготовка поля apci


			IEC104_CopyDataToBuffer(&iec104_prop->TxBuf, dataBuf, len);

		}
		else if (asdu_pkt->Type == C_CS_NA_1)
		{
			IEC104_ASDU_Prepare(asdu_pkt, C_CS_NA_1, ASDU_ADDR_1, SQ_FALSE, 7, 0, 1);
			IEC104_APCI_Prepare(iec104_prop, iec_104_pkt);
			IEC104_CopyDataToBuffer(&iec104_prop->TxBuf, iec104_prop->RxBuf.Data, iec_104_pkt->APDU_Len + 2);
		}

	}
	else if (type == S_TYPE) //Если принят пакет типа S
	{
		iec104_prop->NoAskCnt = NO_ASK_SENDS;
		IEC104_CopyDataToBuffer(&iec104_prop->TxBuf, (uint8_t *)iec_104_pkt, iec_104_pkt->APDU_Len + 2);

	}
	else if (type == U_TYPE) //Если принят пакет типа U
	{
		IEC104_APCI_U_Format *apci_U_pkt = (void *)iec_104_pkt->Data;
	//-----------------------------------------------------------------------
		if (apci_U_pkt->Control & STARTDT_ACT)
		{
			apci_U_pkt->Control &= ~STARTDT_ACT;
			apci_U_pkt->Control |= STARTDT_CON; //Потверждение соединения

			iec104_prop->State = STARTDT;

			iec104_prop->Timer = 0;
			iec104_prop->NoAskCnt = NO_ASK_SENDS;

		}
	//-------------------------------------------------------------------------------
		if (apci_U_pkt->Control & STOPDT_ACT)
		{
			apci_U_pkt->Control &= ~STOPDT_ACT;
			apci_U_pkt->Control |= STOPDT_CON;
			iec104_prop->State = STOPDT;
		}
	//-----------------------------------------------------------------------------------
		if (apci_U_pkt->Control & TESTFR_ACT)
		{
			apci_U_pkt->Control &= ~TESTFR_ACT;
			apci_U_pkt->Control |= TESTFR_CON;
		}
	//---------------------------------------------------------------------------------------
		IEC104_CopyDataToBuffer(&iec104_prop->TxBuf, (uint8_t *)iec_104_pkt, iec_104_pkt->APDU_Len + 2);

	}

}

// Данные циклической передачи
void IEC104_CyclicPacket_Prepare(IEC104_Obj *iec104_prop)
{
	uint8_t dataBuf[IEC104_TMP_BUF_SIZE];
	uint8_t len = 0;

	IEC104_PreSendCyclicCallback(iec104_prop);

	for (uint8_t i = 0; i < iec104_prop->Capacity; i++)
	{

		len = IEC104_GroupDataPrepare(iec104_prop, dataBuf, &iec104_prop->Data[i], CAUSE_PER_CYC);
		IEC104_CopyDataToBuffer(&iec104_prop->TxBuf, dataBuf, len);
	}
}


// Данные спорадической передачи
void IEC104_SporadicPacket_Prepare(IEC104_Obj *iec104_prop)
{
	uint8_t dataBuf[IEC104_TMP_BUF_SIZE];
	uint8_t len = 0;

	IEC104_PreSendSporadicCallback(iec104_prop);

	for (uint8_t i = 0; i < iec104_prop->Capacity; i++)
	{

		len = IEC104_GroupDataPrepare(iec104_prop, dataBuf, &iec104_prop->Data[i], CAUSE_SPONT);
		IEC104_CopyDataToBuffer(&iec104_prop->TxBuf, dataBuf, len);
	}
}
