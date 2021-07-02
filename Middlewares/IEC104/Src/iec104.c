/*
 * iec104.c
 *
 *  Created on: 1 сент. 2020 г.
 *      Author: serge
 */

#include "iec104.h"
#include <string.h>


#define NO_ASK_SENDS 10




static iec104_dataSettings iec104Data;


//Функция установки значения в модель данных
uint8_t iec104_setVal(uint8_t asduAdr, uint32_t ioAdr, void *val)
{
	uint8_t result = 1;
	for (uint8_t i = 0; i < iec104Data.Capacity; i++)
	{
		if (iec104Data.Data[i].AdrAsdu == asduAdr)
		{
			for (uint8_t j = 0; j < iec104Data.Data[i].Objects->Capacity; j++)
			{
				if (iec104Data.Data[i].Objects->Data[j].AdrObj == ioAdr)
				{
					switch(iec104Data.Data[i].Idt)
					{
					case M_SP_NA_1:
						iec104Data.Data[i].Objects->Data[j].Val.U8 = *((uint8_t *)val);
						break;
					case M_ME_NA_1:
						iec104Data.Data[i].Objects->Data[j].Val.U16 = *((uint16_t *)val);
						break;
					case M_ME_NC_1:
						iec104Data.Data[i].Objects->Data[j].Val.FVal = *((float *)val);
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
uint8_t iec104_setFloat(uint8_t asduAdr, uint32_t ioAdr, float val)
{
	return iec104_setVal(asduAdr, ioAdr, &val);
}
//------------------------------------------------
uint8_t iec104_setByte(uint8_t asduAdr, uint32_t ioAdr, uint8_t val)
{
	return iec104_setVal(asduAdr, ioAdr, &val);
}
//--------------------------------------------------------------------------
uint8_t iec104_setHalfWord(uint8_t asduAdr, uint32_t ioAdr, uint16_t val)
{
	return iec104_setVal(asduAdr, ioAdr, &val);
}
//-----------------------------------------------------------------------
void iec104_initAsduDataSet(iec104_asduDataSet *DataSet, iec104_objTypeDef *DataArray, uint16_t Capacity)
{
	DataSet->Capacity = Capacity;
	DataSet->Data = DataArray;
}
//------------------------------------------------------
void iec104_attachAsduData(iec104_asduBlock *Data, uint16_t Capacity)
{
	iec104Data.Data = Data;
	iec104Data.Capacity = Capacity;
}

// Функция привязки буфера
void iec104_AttachBuffer(ByteBufferTypeDef *Buffer, uint8_t *Data, uint16_t Len)
{
	Buffer->Data = Data;
	Buffer->Capacity = Len;
	Buffer->Len = 0;
}
//------------------------------------
void iec104_SetDataBuffer(ByteBufferTypeDef *Buffer, uint8_t *Data, uint16_t Len)
{
	Buffer->Data = Data;
	Buffer->Capacity = Len;
	Buffer->Len = Len;
}
// Запрос на отправку данных
void iec104_CopyDataToBuffer(ByteBufferTypeDef *Buffer, uint8_t *Data, uint16_t Len)
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
	//char msg[] = "Hello it is work!!!\r\n";
	//iec104_CopyDataToBuffer(&iec104_prop->TxBuf, (uint8_t *)msg, strlen(msg));
//	iec104_CopyDataToBuffer(&iec104_prop->TxBuf, iec104_prop->RxBuf.Data, iec104_prop->RxBuf.Len);

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
void apci_prepare(iec_104_propTypeDef *iec104_prop, iec_104_hdr *iec_104_pkt)
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
void iec_104_packet_prepare(iec_104_hdr *iec_104_pkt, uint8_t type, uint8_t len)
{
	//Заполнение типа протокола и длины APDU
	iec_104_pkt->start = 0x68;
	iec_104_pkt->apdu_len = len;
	iec_104_pkt->type = type;
}
//---------------------------------------------------------------
uint8_t asdu_prepare(asdu_hdr_typeDef *asdu_pkt, uint8_t type, uint16_t asduAddr, uint8_t sq, uint8_t causeTx, uint8_t OA, uint8_t numIx)
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
uint8_t M_ME_NC_1_prepare(M_ME_NC_1_IOtypeDef *io_pkt, uint32_t addr, float value, uint8_t QDS)
{
	WRITE_IO_ADDRESS(addr,io_pkt->addr);
	io_pkt->value = value;
	io_pkt->QDS = QDS;

	return sizeof(M_ME_NC_1_IOtypeDef);


}
//--------------------------------------------------------------
//функция подготовки блока двухбайтным значением
uint8_t M_ME_NA_1_prepare(M_ME_NA_1_IOtypeDef *io_pkt, uint32_t addr, uint16_t value, uint8_t QDS)
{
	WRITE_IO_ADDRESS(addr,io_pkt->addr);
	io_pkt->value = value;
	io_pkt->QDS = QDS;

	return sizeof(M_ME_NA_1_IOtypeDef);
}
//--------------------------------------------------------------
//функция подготовки блока с однобайтным значением
uint8_t M_SP_NA_1_prepare(M_SP_NA_1_IOtypeDef *io_pkt, uint32_t addr, uint8_t SIQ)
{
	WRITE_IO_ADDRESS(addr,io_pkt->addr);
	io_pkt->SIQ = SIQ;
	return sizeof(M_SP_NA_1_IOtypeDef);
}
//---------------------------------------------------------------
uint8_t* iec104_write_value(iec104_objTypeDef *obj, uint8_t Idt, uint8_t *buf, ErrorStatus stat_request, uint8_t* length, uint8_t number)
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

	return NULL;
}


uint8_t get_base_data(iec104_objTypeDef *obj, uint8_t Idt, uint8_t *buf, uint8_t num)
{
	ErrorStatus stat_request = SUCCESS_104;
//	uint32_t addr_desc;
	uint8_t   length = 0;

	uint32_t adr_tmp = obj->AdrObj; //сохраняется адрес текущей группы

	// расположим данные в buf.data.send в соответствии с объектами информации кадра МЭК104
	for(uint8_t i = 0; i < num; i++)
	{
			buf = iec104_write_value(&obj[i], Idt, buf, stat_request, &length, i);

			//groupParam->a_obj = groupParam->a_obj + sizeof(RG_obj_mek104_t); //адрес следующего объекта в описателе
																							 //"группа параметров" в eeprom = кс
	}

	obj->AdrObj = adr_tmp;

	return length;
}

//-----------------------------------------------------------
uint8_t group_data_prepare(iec_104_propTypeDef *iec104_prop, uint8_t * buf, iec104_asduBlock *groupParam, uint8_t caseTx)
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
	uint8_t dataBuf[512];
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
			asdu_prepare(asdu_pkt, C_IC_NA_1, ASDU_ADDR_1, SQ_FALSE, 7, 0, 1);
			M_SP_NA_1_IOtypeDef *io_pkt = (void *)asdu_pkt->data;
			M_SP_NA_1_prepare(io_pkt, 0, 20);
			apci_prepare(iec104_prop, iec_104_pkt);

			iec104_CopyDataToBuffer(&iec104_prop->TxBuf, iec104_prop->RxBuf.Data, iec_104_pkt->apdu_len + 2);

			uint8_t len = 0;
			for (uint8_t i = 0; i < iec104Data.Capacity; i++)
			{

				len = group_data_prepare(iec104_prop, dataBuf, &iec104Data.Data[i],20);
				iec104_CopyDataToBuffer(&iec104_prop->TxBuf, dataBuf, len);
			}


			//завершение активации
			len = 0;
			iec_104_pkt = (void *)dataBuf;
			//--------------------------------------------------------
			asdu_pkt = (void*)iec_104_pkt->asdu;
			io_pkt = (void *)asdu_pkt->data;
			//20 - причина передачи CAUSE_INROGEN
			len += asdu_prepare(asdu_pkt, C_IC_NA_1, ASDU_ADDR_1, SQ_FALSE, 10, 0, 1); //подготовка блока ASDU
			len += M_SP_NA_1_prepare(io_pkt, 0, 20);

			len += sizeof(iec_104_hdr); //добавляется к общей длине пакета размер заголовка пакета
			iec_104_packet_prepare(iec_104_pkt, I_TYPE, len - 2); //Ставится тип пакета, и длина
			apci_prepare(iec104_prop, iec_104_pkt); // подготовка поля apci


			iec104_CopyDataToBuffer(&iec104_prop->TxBuf, dataBuf, len);

		}
		else if (asdu_pkt->type == C_CS_NA_1)
		{
			asdu_prepare(asdu_pkt, C_CS_NA_1, ASDU_ADDR_1, SQ_FALSE, 7, 0, 1);
			//M_SP_NA_1_IOtypeDef *io_pkt = (void *)asdu_pkt->data;
			//M_SP_NA_1_prepare(io_pkt, 0, 20);
			apci_prepare(iec104_prop, iec_104_pkt);

			iec104_CopyDataToBuffer(&iec104_prop->TxBuf, iec104_prop->RxBuf.Data, iec_104_pkt->apdu_len + 2);
		}

	}
	else if (type == S_TYPE) //Если принят пакет типа S
	{
		//apci_S_format *apci_S_pkt = (void *)iec_104_pkt->data;
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

			//get_group_parameters(); // считывание настроек баз данных



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
