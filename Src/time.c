#include "time.h"

void SetTime(tsTimeType *TimeType,uint32_t TimeInter)
{
	TimeType->TimeStart = HAL_GetTick();//��ȡ�ṹ��ָ��
	TimeType->TimeInter = TimeInter;
}
uint8_t CompareTime(tsTimeType *TimeType)//�ȽϺ���
{
	return((HAL_GetTick()-TimeType->TimeStart) >= TimeType->TimeInter);
}


