#ifndef __TIME__H
#define __TIME__H

#include "stm32f0xx.h"

typedef struct
{
	uint32_t TimeStart;//��ʼʱ��
	uint32_t TimeInter;//���ʱ��
}tsTimeType;

void SetTime(tsTimeType *TimeType,uint32_t TimeInter);
uint8_t CompareTime(tsTimeType *TimeType);

#endif

