

#include "iec104_model.h"
#include <time.h>

// Создание структуры данных
IEC104_CREATE_DATA_SET(Flags, 2) = {
		{1000},
		{1001}
};


IEC104_CREATE_DATA_SET(iec104values, 2) = {
		{8192},
		{8193},
		// {8194},
		// {8195},
		// {8196},
		// {8197},
		// {8198},
		// {8199},
		// {8200},
		// {8201},
		// {8202},
		// {8203},
        // {8204},
		// {8205},
		// {8206},
		// {8207},
		// {8208},
		// {8209},
		// {8210},
		// {8211},
		// {8212},
		// {8213},
		// {8214},
		// {8215},
};


//----------------------------------------------
iec104_asduBlock iec104Model[2] = {
		{ASDU_ADR, M_SP_TB_1, &Flags},
		{ASDU_ADR, M_ME_TF_1, &iec104values}
};


//----------------------------------------
void iec104_model_init(void)
{
	IEC104_INIT_DATA_SET(Flags);
	IEC104_INIT_DATA_SET(iec104values);
	IEC104_INIT_ASDU(iec104Model);

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