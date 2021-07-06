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



// Функция получения текущего времени
__weak struct tm iec104_GetTime(void)
{
	struct tm tim = {0};
	UNUSED(tim);
	return tim;
}


// Обработка приема команды на общий опрос станции
__weak void iec104_PreInrogenRepplyCallback(iec_104_propTypeDef *hiec)
{
	UNUSED(hiec);
}

// Функция получения текущего времени
__weak void iec104_PreSendSporadicCallback(iec_104_propTypeDef *hiec)
{
	UNUSED(hiec);
}

// Функция получения текущего времени
__weak void iec104_PreSendCyclicCallback(iec_104_propTypeDef *hiec)
{
	UNUSED(hiec);
}

// Получение ASDU по индексу
iec104_asduBlock *iec104_GetAsduByIndex(iec_104_propTypeDef *hiec, uint8_t index)
{
	return &hiec->Data[index];
}


// Изменение типа группы
void iec104_SetAsduType(iec104_asduBlock *asdu, uint8_t idt)
{
	asdu->Idt = idt;
}

// Преобразование временик массиву
static void iec104_SetCP56Time(struct tm *in, uint8_t *out)
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
uint8_t iec104_setVal(iec_104_propTypeDef *hiec, uint8_t asduAdr, uint32_t ioAdr, void *val)
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
uint8_t iec104_setFloat(iec_104_propTypeDef *hiec, uint8_t asduAdr, uint32_t ioAdr, float val)
{
	return iec104_setVal(hiec, asduAdr, ioAdr, &val);
}
//------------------------------------------------
uint8_t iec104_setByte(iec_104_propTypeDef *hiec, uint8_t asduAdr, uint32_t ioAdr, uint8_t val)
{
	return iec104_setVal(hiec, asduAdr, ioAdr, &val);
}
//--------------------------------------------------------------------------
uint8_t iec104_setHalfWord(iec_104_propTypeDef *hiec, uint8_t asduAdr, uint32_t ioAdr, uint16_t val)
{
	return iec104_setVal(hiec, asduAdr, ioAdr, &val);
}
//-----------------------------------------------------------------------
void iec104_initAsduDataSet(iec104_asduDataSet *DataSet, iec104_objTypeDef *DataArray, uint16_t Capacity)
{
	DataSet->Capacity = Capacity;
	DataSet->Data = DataArray;
}
//------------------------------------------------------
void iec104_attachAsduData(iec_104_propTypeDef *hiec, iec104_asduBlock *Data, uint16_t Capacity)
{
	hiec->Data = Data;
	hiec->Capacity = Capacity;
}

// Функция привязки передающего буфера
void iec104_SetTxData(iec_104_propTypeDef *hiec, uint8_t *Data, uint16_t Len)
{
	hiec->TxBuf.Data = Data;
	hiec->TxBuf.Capacity = Len;
	hiec->TxBuf.Len = 0;
}

// Функция привязки приемного буфера
void iec104_SetRxData(iec_104_propTypeDef *hiec, uint8_t *Data, uint16_t Len)
{
	hiec->RxBuf.Data = Data;
	hiec->RxBuf.Capacity = Len;
	hiec->RxBuf.Len = Len;
}

// Запрос на отправку данных
static void iec104_CopyDataToBuffer(ByteBufferTypeDef *Buffer, uint8_t *Data, uint16_t Len)
{
	if (Len > Buffer->Capacity)
	{
		Len = Buffer->Capacity;
	}

	memcpy(&Buffer->Data[Buffer->Len], Data, Len);
	Buffer->Len += Len;
}

// Обработка пакета
void iec104_PacketHandler(iec_104_propTypeDef *iec104_prop)
{
	if (iec104_prop->RxBuf.Data[0] == IEC104_START_ID)
	{
		iec_104_read(iec104_prop);
	}
}
/****************************************************************/


//-----------------------------------------------------------
void iec_104_conn_close(iec_104_propTypeDef *iec104_prop)
{
    iec104_prop->state = STOPDT;
    iec104_prop->tx_count = 0;
    iec104_prop->rx_count = 0;
    iec104_prop->cyclyc_tx = 0;
}
//-------------------------------------------------------------------
static void apci_prepare(iec_104_propTypeDef *iec104_prop, iec_104_hdr *iec_104_pkt)
{
	uint8_t type = get_type(iec_104_pkt->type);

	if ((type & 0x01) == 0)
		type = 0;


	if (type == I_TYPE)
	{
		apci_I_format *apci_I_pkt = (void *)iec_104_pkt->data;
		apci_I_pkt->tx_count = iec104_prop->tx_count;
		apci_I_pkt->rx_count = iec104_prop->rx_count;
		iec104_prop->tx_count += 2;

	}
	else if (type == S_TYPE)
	{
		apci_I_format *apci_S_pkt = (void *)iec_104_pkt->data;
		apci_S_pkt->rx_count = iec104_prop->rx_count;

	}
	else if (type == U_TYPE)
	{
		//apci_I_format *apci_U_pkt = (void *)iec_104_pkt->data;

	}

}
//---------------------------------------------------------------
static void iec_104_packet_prepare(iec_104_hdr *iec_104_pkt, uint8_t type, uint8_t len)
{
	//Заполнение типа протокола и длины APDU
	iec_104_pkt->start = 0x68;
	iec_104_pkt->apdu_len = len;
	iec_104_pkt->type = type;
}
//---------------------------------------------------------------
static uint8_t asdu_prepare(asdu_hdr_typeDef *asdu_pkt, uint8_t type, uint16_t asduAddr, uint8_t sq, uint8_t causeTx, uint8_t OA, uint8_t numIx)
{
	asdu_pkt->type = type;
	asdu_pkt->classificator = sq | numIx;
	asdu_pkt->causeTx = causeTx;
	asdu_pkt->OA = OA;
	asdu_pkt->asduAddr = asduAddr;

	return sizeof(asdu_hdr_typeDef);
}
//---------------------------------------------------------------
//функция подготовки блока со значением float
static uint8_t M_ME_NC_1_prepare(M_ME_NC_1_IOtypeDef *io_pkt, uint32_t addr, float value, uint8_t QDS)
{
	WRITE_IO_ADDRESS(addr,io_pkt->addr);
	io_pkt->value = value;
	io_pkt->QDS = QDS;

	return sizeof(M_ME_NC_1_IOtypeDef);
}
//--------------------------------------------------------------
//функция подготовки блока со значением float и с меткой времени
static uint8_t M_ME_TF_1_prepare(M_ME_TF_1_IOtypeDef *io_pkt, uint32_t addr, float value, uint8_t QDS, struct tm *u)
{
	WRITE_IO_ADDRESS(addr,io_pkt->addr);
	io_pkt->value = value;
	io_pkt->QDS = QDS;
	iec104_SetCP56Time(u, io_pkt->CP56Time);

	return sizeof(M_ME_TF_1_IOtypeDef);
}
//--------------------------------------------------------------
//функция подготовки блока двухбайтным значением
static uint8_t M_ME_NA_1_prepare(M_ME_NA_1_IOtypeDef *io_pkt, uint32_t addr, uint16_t value, uint8_t QDS)
{
	WRITE_IO_ADDRESS(addr,io_pkt->addr);
	io_pkt->value = value;
	io_pkt->QDS = QDS;

	return sizeof(M_ME_NA_1_IOtypeDef);
}
//--------------------------------------------------------------
//функция подготовки блока с однобайтным значением
static uint8_t M_SP_NA_1_prepare(M_SP_NA_1_IOtypeDef *io_pkt, uint32_t addr, uint8_t SIQ)
{
	WRITE_IO_ADDRESS(addr,io_pkt->addr);
	io_pkt->SIQ = SIQ;
	return sizeof(M_SP_NA_1_IOtypeDef);
}
//---------------------------------------------------------------
//функция подготовки блока с однобайтным значением с меткой времени
static uint8_t M_SP_TB_1_prepare(M_SP_TB_1_IOtypeDef *io_pkt, uint32_t addr, uint8_t SIQ, struct tm *u)
{
	WRITE_IO_ADDRESS(addr,io_pkt->addr);
	io_pkt->SIQ = SIQ;
	iec104_SetCP56Time(u, io_pkt->CP56Time);
	return sizeof(M_SP_TB_1_IOtypeDef);
}

//---------------------------------------------------------------
static uint8_t* iec104_write_value(iec104_objTypeDef *obj, uint8_t Idt, uint8_t *buf, IEC104_ErrorStatus stat_request, uint8_t* length, uint8_t number)
{

	if ( Idt == M_SP_NA_1 ) {  // 1байт
		M_SP_NA_1_IOtypeDef *io_pkt = (void *)buf;
		*length += M_SP_NA_1_prepare(io_pkt,obj->AdrObj,MEK104_M1_ALL_OK | obj->Val.U8);
		return 	(void *)io_pkt->next; //возвращение указателя на блок со следующим значением
	}



	if ( Idt == M_ME_NA_1 ) {   // 2 байта + байт качества
		 M_ME_NA_1_IOtypeDef *io_pkt = (void *)buf;
		*length += M_ME_NA_1_prepare(io_pkt, obj->AdrObj,obj->Val.U16,MEK104_M9_ALL_OK);

		return 	(void *)io_pkt->next; //возвращение указателя на блок со следующим значением
	}

	if ( Idt == M_ME_NC_1 )    // 4 байта float + байт качества
	{
		M_ME_NC_1_IOtypeDef *io_pkt = (void *)buf;
		*length += M_ME_NC_1_prepare(io_pkt, obj->AdrObj, obj->Val.FVal, MEK104_M9_ALL_OK);
		return 	(void *)io_pkt->next;	//возвращение указателя на блок со следующим значением
	}

// Объекты с метками времени
	if ( Idt == M_SP_TB_1 )    // 1 байт + байт качества + метка времени
	{
		struct tm tim = iec104_GetTime();
		M_SP_TB_1_IOtypeDef *io_pkt = (void *)buf;
		*length += M_SP_TB_1_prepare(io_pkt,obj->AdrObj,MEK104_M1_ALL_OK | obj->Val.U8, &tim);
	
		return 	(void *)io_pkt->next;	//возвращение указателя на блок со следующим значением
	}


	if ( Idt == M_ME_TF_1 )    // 4 байта float + байт качества + метка времени
	{
		struct tm tim = iec104_GetTime();
		M_ME_TF_1_IOtypeDef *io_pkt = (void *)buf;
		*length += M_ME_TF_1_prepare(io_pkt, obj->AdrObj, obj->Val.FVal, MEK104_M9_ALL_OK, &tim);
		return 	(void *)io_pkt->next;	//возвращение указателя на блок со следующим значением
	}

	return NULL;
}


static uint8_t get_base_data(iec104_objTypeDef *obj, uint8_t Idt, uint8_t *buf, uint8_t num)
{
	IEC104_ErrorStatus stat_request = SUCCESS_104;
	uint8_t   length = 0;
	uint32_t adr_tmp = obj->AdrObj; //сохраняется адрес текущей группы

	// расположим данные в buf.data.send в соответствии с объектами информации кадра МЭК104
	for(uint8_t i = 0; i < num; i++)
	{
			buf = iec104_write_value(&obj[i], Idt, buf, stat_request, &length, i);
	}

	obj->AdrObj = adr_tmp;

	return length;
}

//-----------------------------------------------------------
static uint8_t group_data_prepare(iec_104_propTypeDef *iec104_prop, uint8_t * buf, iec104_asduBlock *groupParam, uint8_t caseTx)
{
	uint8_t total_len = 0;
	iec_104_hdr *iec_104_pkt = (void *)buf; //Указатель на структуру пакета 104 протокола приравнивается адресу буфера передачи
//--------------------------------------------------------
	asdu_hdr_typeDef *asdu_pkt = (void*)iec_104_pkt->asdu;
  //20 - причина передачи CAUSE_INROGEN
	// Количество блоко устанавливается здесь
	total_len += asdu_prepare(asdu_pkt, groupParam->Idt, groupParam->AdrAsdu, SQ_FALSE, caseTx, 0, groupParam->Objects->Capacity); //подготовка блока ASDU

	total_len += get_base_data(groupParam->Objects->Data, groupParam->Idt, (void *)asdu_pkt->data, groupParam->Objects->Capacity); //Запись значений из группы в пакет

	total_len += sizeof(iec_104_hdr); //добавляется к общей длине пакета размер заголовка пакета
	iec_104_packet_prepare(iec_104_pkt, I_TYPE, total_len - 2); //Ставится тип пакета, и длина
	apci_prepare(iec104_prop, iec_104_pkt); // подготовка поля apci


	return total_len;

}
//------------------------------------------------------------
void iec_104_read(iec_104_propTypeDef *iec104_prop)
{
	uint8_t dataBuf[IEC104_TMP_BUF_SIZE];
	iec_104_hdr *iec_104_pkt = (iec_104_hdr *) iec104_prop->RxBuf.Data;

//	memcpy(iec104_prop->TxBuf.Data, iec104_prop->RxBuf.Data, iec_104_pkt->apdu_len + 2);
	iec_104_pkt = (void*)iec104_prop->RxBuf.Data;

	uint8_t type = get_type(iec_104_pkt->type);

	//Если нулевой бит равен 0, то это тип I
	 if ((type & 0x01) == 0)
		type = 0;

	if (type == I_TYPE) //Если принят пакет типа I
	{
		iec104_prop->rx_count += 2;
		//apci_I_format *apci_I_pkt = (void *)iec_104_pkt->data;
		asdu_hdr_typeDef *asdu_pkt = (void*)iec_104_pkt->asdu;

		if (asdu_pkt->type == C_IC_NA_1) // ответ на команду общего опроса
		{
			//-----------------------------------------------------------------
			//7 - причина передачи CAUSE_ACTCON
			asdu_prepare(asdu_pkt, C_IC_NA_1, ASDU_ADDR_1, SQ_FALSE, CAUSE_ACTCON, 0, 1);
			M_SP_NA_1_IOtypeDef *io_pkt = (void *)asdu_pkt->data;
			M_SP_NA_1_prepare(io_pkt, 0, 20);
			apci_prepare(iec104_prop, iec_104_pkt);

			iec104_CopyDataToBuffer(&iec104_prop->TxBuf, iec104_prop->RxBuf.Data, iec_104_pkt->apdu_len + 2);

			iec104_PreInrogenRepplyCallback(iec104_prop);

			uint8_t len = 0;
			for (uint8_t i = 0; i < iec104_prop->Capacity; i++)
			{

				len = group_data_prepare(iec104_prop, dataBuf, &iec104_prop->Data[i], CAUSE_INROGEN);
				iec104_CopyDataToBuffer(&iec104_prop->TxBuf, dataBuf, len);
			}


			//завершение активации
			len = 0;
			iec_104_pkt = (void *)dataBuf;
			//--------------------------------------------------------
			asdu_pkt = (void*)iec_104_pkt->asdu;
			io_pkt = (void *)asdu_pkt->data;
			len += asdu_prepare(asdu_pkt, C_IC_NA_1, ASDU_ADDR_1, SQ_FALSE, CAUSE_END_ACT, 0, 1); //подготовка блока ASDU
			len += M_SP_NA_1_prepare(io_pkt, 0, 20);

			len += sizeof(iec_104_hdr); //добавляется к общей длине пакета размер заголовка пакета
			iec_104_packet_prepare(iec_104_pkt, I_TYPE, len - 2); //Ставится тип пакета, и длина
			apci_prepare(iec104_prop, iec_104_pkt); // подготовка поля apci


			iec104_CopyDataToBuffer(&iec104_prop->TxBuf, dataBuf, len);

		}
		else if (asdu_pkt->type == C_CS_NA_1)
		{
			asdu_prepare(asdu_pkt, C_CS_NA_1, ASDU_ADDR_1, SQ_FALSE, 7, 0, 1);
			apci_prepare(iec104_prop, iec_104_pkt);
			iec104_CopyDataToBuffer(&iec104_prop->TxBuf, iec104_prop->RxBuf.Data, iec_104_pkt->apdu_len + 2);
		}

	}
	else if (type == S_TYPE) //Если принят пакет типа S
	{
		iec104_prop->no_ask_counter = NO_ASK_SENDS;
		iec104_CopyDataToBuffer(&iec104_prop->TxBuf, (uint8_t *)iec_104_pkt, iec_104_pkt->apdu_len + 2);

	}
	else if (type == U_TYPE) //Если принят пакет типа U
	{
		apci_U_format *apci_U_pkt = (void *)iec_104_pkt->data;
	//-----------------------------------------------------------------------
		if (apci_U_pkt->control & STARTDT_ACT)
		{
			apci_U_pkt->control &= ~STARTDT_ACT;
			apci_U_pkt->control |= STARTDT_CON; //Потверждение соединения

			iec104_prop->state = STARTDT;

			iec104_prop->timer = 0;
			iec104_prop->no_ask_counter = NO_ASK_SENDS;

		}
	//-------------------------------------------------------------------------------
		if (apci_U_pkt->control & STOPDT_ACT)
		{
			apci_U_pkt->control &= ~STOPDT_ACT;
			apci_U_pkt->control |= STOPDT_CON;
			iec104_prop->state = STOPDT;
		}
	//-----------------------------------------------------------------------------------
		if (apci_U_pkt->control & TESTFR_ACT)
		{
			apci_U_pkt->control &= ~TESTFR_ACT;
			apci_U_pkt->control |= TESTFR_CON;
		}
	//---------------------------------------------------------------------------------------
		iec104_CopyDataToBuffer(&iec104_prop->TxBuf, (uint8_t *)iec_104_pkt, iec_104_pkt->apdu_len + 2);

	}

}

// Данные циклической передачи
void iec104_cyclic_prepare(iec_104_propTypeDef *iec104_prop)
{
	uint8_t dataBuf[IEC104_TMP_BUF_SIZE];
	uint8_t len = 0;

	iec104_PreSendCyclicCallback(iec104_prop);

	for (uint8_t i = 0; i < iec104_prop->Capacity; i++)
	{

		len = group_data_prepare(iec104_prop, dataBuf, &iec104_prop->Data[i], CAUSE_PER_CYC);
		iec104_CopyDataToBuffer(&iec104_prop->TxBuf, dataBuf, len);
	}
}


// Данные спорадической передачи
void iec104_sporadic_prepare(iec_104_propTypeDef *iec104_prop)
{
	uint8_t dataBuf[IEC104_TMP_BUF_SIZE];
	uint8_t len = 0;

	iec104_PreSendSporadicCallback(iec104_prop);

	for (uint8_t i = 0; i < iec104_prop->Capacity; i++)
	{

		len = group_data_prepare(iec104_prop, dataBuf, &iec104_prop->Data[i], CAUSE_SPONT);
		iec104_CopyDataToBuffer(&iec104_prop->TxBuf, dataBuf, len);
	}
}
