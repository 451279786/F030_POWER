#include "nbiotdriver.h"
#include "usart.h"
#include "time.h"
#include <string.h>
#include "sensor.h"
#include "codec.h"
#include "crc16.h"
//#include "iwdg.h"

static uint8_t CurrentRty;
static teATCmdNum ATRecCmdNum;

static teATCmdNum ATCurrentCmdNum;
static teATCmdNum ATNextCmdNum;
static teNB_TaskStatus NB_TaskStatus;
static tsTimeType TimeNB;

volatile uint8_t NBSendAccessFlag = 0;//发送数据成功标志，0表示发送失败

tsATCmds ATCmds[]=
{
	//参数分别为 向NB-IOT发送字符串（AT命令）、NB-IOT返回给MCU的字符串、设置超时（毫秒）、接收状态、设置重发次数
	{"AT+NCONFIG=AUTOCONNECT,FALSE\r\n","OK",5000,NO_REC,3},  //关闭射频功能（不进行无线通讯）
	{"AT+NCDP=49.4.85.232,5683\r\n","OK",2000,NO_REC,3},      //设置 IoT 平台 IP 地址（非 COAP 协议可以不配置）
	{"AT+NRB\r\n","OK",10000,NO_REC,3},                       //软重启。重启使用时间比较长，所以这里设置为8秒钟
	{"AT+CFUN=1\r\n","OK",5000,NO_REC,4},                     //开启射频功能
	{"AT+CGATT=1\r\n","OK",5000,NO_REC,3},                    //自动搜网
	{"AT+CGSN=1\r\n","OK",2000,NO_REC,3},                     //查询IMEI号（一般出厂已经设置好）
	{"AT+CIMI\r\n","OK",3000,NO_REC,3},                       //查询SIM卡信息
	{"AT+CGDCONT=1,\"IP\",\"ctnb\"\r\n","OK",2000,NO_REC,3},  //设置APN.注意AT命令中字符串的书写格式：\"IP\",\"ctnb\"（表示电信）
	{"AT+NNMI=1\r\n","OK",5000,NO_REC,3},                     //开启下行数据通知	
	{"AT+CGATT?\r\n","+CGATT:1",5000,NO_REC,20},              //查询网络是否激活?
	{"AT+NMGS=","OK",5000,NO_REC,5},                          //发送数据
};

void ATSend(teATCmdNum ATCmdNum)
{
	//清空接收缓存区
	memset(Usart1type.Usart1RecBuff,0,USART1_REC_SIZE);
	ATCmds[ATCmdNum].ATStatus = NO_REC;//赋值当前AT状态为未接收
	ATRecCmdNum = ATCmdNum; //申请全局变量，赋值解析命令值，用于解析判断
	
	if(ATCmdNum == AT_NMGS)//如果是COAP发送指令，转发到串口2->NB模组
	{
		SendNBData();
	}
	else
	{
		HAL_UART_Transmit(&huart1,(uint8_t *)ATCmds[ATCmdNum].ATSendStr,strlen(ATCmds[ATCmdNum].ATSendStr),100);//发送到串口1->NB模组
	}
	//打开超时定时器
	SetTime(&TimeNB,ATCmds[ATCmdNum].TimeOut);
}
void ATRec(void)
{
	
	if(Usart1type.Usart1RecFlag)
	{
		//printf("%s",Usart1type.Usart1RecBuff);
		if(strstr((const char *)Usart1type.Usart1RecBuff,ATCmds[ATRecCmdNum].ATRecStr) != NULL)
		{
			ATCmds[ATRecCmdNum].ATStatus = SUCCESS_REC;	
			
		}
		if(strstr((const char *)Usart1type.Usart1RecBuff,"+NNMI:13,") != NULL)//如果接收到下发命令，才进行解析
		{
			ParseCommand((char *)Usart1type.Usart1RecBuff,Usart1type.Usart1RecLen);   //数据解析
			
		}
		memset(Usart1type.Usart1RecBuff,0,Usart1type.Usart1RecLen);  //清空接收缓存区，如果不清空，串口1会打印出来
		Usart1type.Usart1RecFlag = 0;
		Usart1type.Usart1RecLen = 0;
	}
	
}

void TriggerSendData(void)//触发发送数据
{
	ATCurrentCmdNum = AT_NMGS;//当前指令为COAP发送
	ATNextCmdNum = AT_IDIE;//下一条指令为空闲
	NB_TaskStatus = NB_SEND;//NB状态为发送
}


void NB_Init(void)//NB初始化
{
	NbPowerOn();
	NB_TaskStatus = NB_SEND;
	ATCurrentCmdNum = AT_NCONFIG;
	ATNextCmdNum = AT_NCDP;
	NBSendAccessFlag = 0;//0表示发送失败
}

uint8_t NB_Task(void)//NB任务
{
	while(1)
		switch(NB_TaskStatus)
		{
			case NB_IDIE:
					//printf("NB入网成功\n");
					TriggerSendData();//需要发送一次数据，避免主任务中重启NB后卡在这里，跳过AT_CGATT
					ATRec();//为保证实时接收到下发给继电器的命令，在空闲时也要实时接收
					HAL_Delay (100);
				return 0;
			case NB_SEND:
					
					if(ATCurrentCmdNum != ATNextCmdNum)//如果为第一次发送
					{
						CurrentRty = ATCmds[ATCurrentCmdNum].RtyNum;//获取指令重发次数
					}
					ATSend(ATCurrentCmdNum);//发送当前指令
					NB_TaskStatus = NB_WAIT;//NB状态为等待
				break;
			case NB_WAIT://等待响应处理
					ATRec();//执行接收指令
					if(ATCmds[ATCurrentCmdNum].ATStatus == SUCCESS_REC)//如果成功接收到返回的字符
					{
						if(ATCurrentCmdNum == AT_CGATT)//如果当前指令为AT_CGATT(成功附着网络)
						{
							NB_TaskStatus = NB_IDIE;//NB状态为入网成功
							break;
						}
						else if(ATCurrentCmdNum == AT_NMGS)//如果当前指令为发送指令
						{
							NBSendAccessFlag = 1;      //1表示发送数据成功
							NB_TaskStatus = NB_ACCESS; //NB状态为空闲
							return 0;
						}
						else
						{
							ATCurrentCmdNum += 1;
							ATNextCmdNum = ATCurrentCmdNum+1;
							NB_TaskStatus = NB_SEND;
							break;
						}
					}
					else if(CompareTime(&TimeNB))
					{
						NBSendAccessFlag = 0;//0表示发送数据失败
						ATCmds[ATCurrentCmdNum].ATStatus = TIME_OUT;
						if(CurrentRty > 0)//如果当前次数没有超过重发次数
						{
							CurrentRty--;
							ATNextCmdNum = ATCurrentCmdNum;
							NB_TaskStatus = NB_SEND;
							break;
						}
						else//如果重发次数达到重发次数，执行NB初始化重启
						{
							if(ATCurrentCmdNum == AT_CGATT)//如果当前指令为AT_CGATT，重发20次后关闭NB电源强制重启
							{
								NbPowerOff();//关闭NB电源
								HAL_Delay (2000);
								//while(1);    //看门狗5秒不刷新将会重启
							}
							NB_Init();
							break;
						}
					}
				break;
			case NB_ACCESS:
				ATRec();//为保证实时接收到下发给继电器的命令，接收20s
				return 0;
			default:
				break;
		}
		return 1;
}
void NbPowerOff(void)
{
	//HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);//关闭NB电源
}
void NbPowerOn(void)
{
	//HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);//打开NB电源
}
