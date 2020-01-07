#ifndef _NBIOTDRIVER_H
#define _NBIOTDRIVER_H

#include "stm32f0xx.h"

//为保证下发指令能实时控制继电器，要求5秒上报一次数据，NB模组持续在线
//#define SENDDATATIME 9000

extern volatile uint8_t NBSendAccessFlag;

typedef enum
{
	SUCCESS_REC=0,
	TIME_OUT,
	NO_REC
}teATStatus;

typedef enum
{
	AT_NCONFIG=0,
	AT_NCDP,
	AT_NRB,
	AT_CFUN1,
	AT_CGATT1,
	AT_CGSN,
	AT_CIMI,
	AT_CGDCONT,
	AT_NNMI,
	AT_CGATT,
	AT_NMGS,
	AT_IDIE
}teATCmdNum;

typedef enum
{
	NB_IDIE=0,
	NB_SEND,
	NB_WAIT,
	NB_ACCESS
}teNB_TaskStatus;

typedef struct
{
	char *ATSendStr;
	char *ATRecStr;
	uint16_t TimeOut;
	teATStatus ATStatus;
	uint8_t RtyNum;
}tsATCmds;

void NB_Init(void);
uint8_t NB_Task(void);
uint8_t ReadAccessStatus(void);
void TriggerSendData(void);

void NbPowerOn(void);

void NbPowerOff(void);
#endif
