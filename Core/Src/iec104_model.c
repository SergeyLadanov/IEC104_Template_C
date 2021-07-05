

#include "iec104_model.h"
#include <time.h>

// Создание структуры данных
IEC104_CREATE_DATA_SET(Flags, 2) = {
		{1000},
		{1001}
};


IEC104_CREATE_DATA_SET(iec104values1, 12) = {
		{8192},
		{8193},
		{8194},
		{8195},
		{8196},
		{8197},
		{8198},
		{8199},
		{8200},
		{8201},
		{8202},
		{8203},
};


IEC104_CREATE_DATA_SET(iec104values2, 12) = {
        {8204},
		{8205},
		{8206},
		{8207},
		{8208},
		{8209},
		{8210},
		{8211},
		{8212},
		{8213},
		{8214},
		{8215},
};


//----------------------------------------------
iec104_asduBlock iec104Model[3] = {
		{ASDU_ADR, M_SP_TB_1, &Flags},
		{ASDU_ADR, M_ME_TF_1, &iec104values1},
		{ASDU_ADR, M_ME_TF_1, &iec104values2}
};


//----------------------------------------
void iec104_model_init(iec_104_propTypeDef *hiec)
{
	IEC104_INIT_DATA_SET(Flags);
	IEC104_INIT_DATA_SET(iec104values1);
	IEC104_INIT_DATA_SET(iec104values2);
	IEC104_INIT_ASDU((*hiec), iec104Model);

}

// Функция получения времени
struct tm iec104_GetTime(void)
{
	time_t timer;
	struct tm *u;
	timer = time(NULL);
	u = localtime(&timer);

	return *u;
}


// Обработка приема команды на общий опрос станции
void iec104_PreInrogenRepplyCallback(iec_104_propTypeDef *hiec)
{
	iec104_asduBlock *pasdu; 

	pasdu = iec104_GetAsduByIndex(hiec, 0);
	iec104_SetAsduType(pasdu, M_SP_NA_1);

	pasdu = iec104_GetAsduByIndex(hiec, 1);
	iec104_SetAsduType(pasdu, M_ME_NC_1);

	pasdu = iec104_GetAsduByIndex(hiec, 2);
	iec104_SetAsduType(pasdu, M_ME_NC_1);
}

// Функция получения текущего времени
void iec104_PreSendSporadicCallback(iec_104_propTypeDef *hiec)
{
	iec104_asduBlock *pasdu; 

	pasdu = iec104_GetAsduByIndex(hiec, 0);
	iec104_SetAsduType(pasdu, M_SP_TB_1);

	pasdu = iec104_GetAsduByIndex(hiec, 1);
	iec104_SetAsduType(pasdu, M_ME_TF_1);

	pasdu = iec104_GetAsduByIndex(hiec, 2);
	iec104_SetAsduType(pasdu, M_ME_TF_1);
}

// Функция получения текущего времени
void iec104_PreSendCyclicCallback(iec_104_propTypeDef *hiec)
{
	iec104_asduBlock *pasdu; 

	pasdu = iec104_GetAsduByIndex(hiec, 0);
	iec104_SetAsduType(pasdu, M_SP_NA_1);

	pasdu = iec104_GetAsduByIndex(hiec, 1);
	iec104_SetAsduType(pasdu, M_ME_NC_1);

	pasdu = iec104_GetAsduByIndex(hiec, 2);
	iec104_SetAsduType(pasdu, M_ME_NC_1);
}