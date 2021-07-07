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
IEC104_ASDU_Block iec104Model[3] = {
		{ASDU_ADR, M_SP_TB_1, &Flags},
		{ASDU_ADR, M_ME_TF_1, &iec104values1},
		{ASDU_ADR, M_ME_TF_1, &iec104values2}
};


//----------------------------------------
void IEC104_Model_Init(IEC104_Obj *hiec)
{
	memset(hiec, 0, sizeof(IEC104_Obj));
	IEC104_INIT_DATA_SET(Flags);
	IEC104_INIT_DATA_SET(iec104values1);
	IEC104_INIT_DATA_SET(iec104values2);
	IEC104_INIT_ASDU((*hiec), iec104Model);

}

// Функция получения времени
struct tm IEC104_GetTime(void)
{
	time_t timer;
	struct tm *u;
	timer = time(NULL);
	u = localtime(&timer);

	return *u;
}


// Обработка приема команды на общий опрос станции
void IEC104_PreInrogenRepplyCallback(IEC104_Obj *hiec)
{
	IEC104_ASDU_Block *pasdu; 

	pasdu = IEC104_GetAsduByIndex(hiec, 0);
	IEC104_SetAsduType(pasdu, M_SP_NA_1);

	pasdu = IEC104_GetAsduByIndex(hiec, 1);
	IEC104_SetAsduType(pasdu, M_ME_NC_1);

	pasdu = IEC104_GetAsduByIndex(hiec, 2);
	IEC104_SetAsduType(pasdu, M_ME_NC_1);
}

// Функция получения текущего времени
void IEC104_PreSendSporadicCallback(IEC104_Obj *hiec)
{
	IEC104_ASDU_Block *pasdu; 

	pasdu = IEC104_GetAsduByIndex(hiec, 0);
	IEC104_SetAsduType(pasdu, M_SP_TB_1);

	pasdu = IEC104_GetAsduByIndex(hiec, 1);
	IEC104_SetAsduType(pasdu, M_ME_TF_1);

	pasdu = IEC104_GetAsduByIndex(hiec, 2);
	IEC104_SetAsduType(pasdu, M_ME_TF_1);
}

// Функция получения текущего времени
void IEC104_PreSendCyclicCallback(IEC104_Obj *hiec)
{
	IEC104_ASDU_Block *pasdu; 

	pasdu = IEC104_GetAsduByIndex(hiec, 0);
	IEC104_SetAsduType(pasdu, M_SP_NA_1);

	pasdu = IEC104_GetAsduByIndex(hiec, 1);
	IEC104_SetAsduType(pasdu, M_ME_NC_1);

	pasdu = IEC104_GetAsduByIndex(hiec, 2);
	IEC104_SetAsduType(pasdu, M_ME_NC_1);
}