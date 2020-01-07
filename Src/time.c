#include "time.h"

void SetTime(tsTimeType *TimeType,uint32_t TimeInter)
{
	TimeType->TimeStart = HAL_GetTick();//获取结构体指针
	TimeType->TimeInter = TimeInter;
}
uint8_t CompareTime(tsTimeType *TimeType)//比较函数
{
	return((HAL_GetTick()-TimeType->TimeStart) >= TimeType->TimeInter);
}


