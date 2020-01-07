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

volatile uint8_t NBSendAccessFlag = 0;//�������ݳɹ���־��0��ʾ����ʧ��

tsATCmds ATCmds[]=
{
	//�����ֱ�Ϊ ��NB-IOT�����ַ�����AT�����NB-IOT���ظ�MCU���ַ��������ó�ʱ�����룩������״̬�������ط�����
	{"AT+NCONFIG=AUTOCONNECT,FALSE\r\n","OK",5000,NO_REC,3},  //�ر���Ƶ���ܣ�����������ͨѶ��
	{"AT+NCDP=49.4.85.232,5683\r\n","OK",2000,NO_REC,3},      //���� IoT ƽ̨ IP ��ַ���� COAP Э����Բ����ã�
	{"AT+NRB\r\n","OK",10000,NO_REC,3},                       //������������ʹ��ʱ��Ƚϳ���������������Ϊ8����
	{"AT+CFUN=1\r\n","OK",5000,NO_REC,4},                     //������Ƶ����
	{"AT+CGATT=1\r\n","OK",5000,NO_REC,3},                    //�Զ�����
	{"AT+CGSN=1\r\n","OK",2000,NO_REC,3},                     //��ѯIMEI�ţ�һ������Ѿ����úã�
	{"AT+CIMI\r\n","OK",3000,NO_REC,3},                       //��ѯSIM����Ϣ
	{"AT+CGDCONT=1,\"IP\",\"ctnb\"\r\n","OK",2000,NO_REC,3},  //����APN.ע��AT�������ַ�������д��ʽ��\"IP\",\"ctnb\"����ʾ���ţ�
	{"AT+NNMI=1\r\n","OK",5000,NO_REC,3},                     //������������֪ͨ	
	{"AT+CGATT?\r\n","+CGATT:1",5000,NO_REC,20},              //��ѯ�����Ƿ񼤻�?
	{"AT+NMGS=","OK",5000,NO_REC,5},                          //��������
};

void ATSend(teATCmdNum ATCmdNum)
{
	//��ս��ջ�����
	memset(Usart1type.Usart1RecBuff,0,USART1_REC_SIZE);
	ATCmds[ATCmdNum].ATStatus = NO_REC;//��ֵ��ǰAT״̬Ϊδ����
	ATRecCmdNum = ATCmdNum; //����ȫ�ֱ�������ֵ��������ֵ�����ڽ����ж�
	
	if(ATCmdNum == AT_NMGS)//�����COAP����ָ�ת��������2->NBģ��
	{
		SendNBData();
	}
	else
	{
		HAL_UART_Transmit(&huart1,(uint8_t *)ATCmds[ATCmdNum].ATSendStr,strlen(ATCmds[ATCmdNum].ATSendStr),100);//���͵�����1->NBģ��
	}
	//�򿪳�ʱ��ʱ��
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
		if(strstr((const char *)Usart1type.Usart1RecBuff,"+NNMI:13,") != NULL)//������յ��·�����Ž��н���
		{
			ParseCommand((char *)Usart1type.Usart1RecBuff,Usart1type.Usart1RecLen);   //���ݽ���
			
		}
		memset(Usart1type.Usart1RecBuff,0,Usart1type.Usart1RecLen);  //��ս��ջ��������������գ�����1���ӡ����
		Usart1type.Usart1RecFlag = 0;
		Usart1type.Usart1RecLen = 0;
	}
	
}

void TriggerSendData(void)//������������
{
	ATCurrentCmdNum = AT_NMGS;//��ǰָ��ΪCOAP����
	ATNextCmdNum = AT_IDIE;//��һ��ָ��Ϊ����
	NB_TaskStatus = NB_SEND;//NB״̬Ϊ����
}


void NB_Init(void)//NB��ʼ��
{
	NbPowerOn();
	NB_TaskStatus = NB_SEND;
	ATCurrentCmdNum = AT_NCONFIG;
	ATNextCmdNum = AT_NCDP;
	NBSendAccessFlag = 0;//0��ʾ����ʧ��
}

uint8_t NB_Task(void)//NB����
{
	while(1)
		switch(NB_TaskStatus)
		{
			case NB_IDIE:
					//printf("NB�����ɹ�\n");
					TriggerSendData();//��Ҫ����һ�����ݣ�����������������NB�����������AT_CGATT
					ATRec();//Ϊ��֤ʵʱ���յ��·����̵���������ڿ���ʱҲҪʵʱ����
					HAL_Delay (100);
				return 0;
			case NB_SEND:
					
					if(ATCurrentCmdNum != ATNextCmdNum)//���Ϊ��һ�η���
					{
						CurrentRty = ATCmds[ATCurrentCmdNum].RtyNum;//��ȡָ���ط�����
					}
					ATSend(ATCurrentCmdNum);//���͵�ǰָ��
					NB_TaskStatus = NB_WAIT;//NB״̬Ϊ�ȴ�
				break;
			case NB_WAIT://�ȴ���Ӧ����
					ATRec();//ִ�н���ָ��
					if(ATCmds[ATCurrentCmdNum].ATStatus == SUCCESS_REC)//����ɹ����յ����ص��ַ�
					{
						if(ATCurrentCmdNum == AT_CGATT)//�����ǰָ��ΪAT_CGATT(�ɹ���������)
						{
							NB_TaskStatus = NB_IDIE;//NB״̬Ϊ�����ɹ�
							break;
						}
						else if(ATCurrentCmdNum == AT_NMGS)//�����ǰָ��Ϊ����ָ��
						{
							NBSendAccessFlag = 1;      //1��ʾ�������ݳɹ�
							NB_TaskStatus = NB_ACCESS; //NB״̬Ϊ����
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
						NBSendAccessFlag = 0;//0��ʾ��������ʧ��
						ATCmds[ATCurrentCmdNum].ATStatus = TIME_OUT;
						if(CurrentRty > 0)//�����ǰ����û�г����ط�����
						{
							CurrentRty--;
							ATNextCmdNum = ATCurrentCmdNum;
							NB_TaskStatus = NB_SEND;
							break;
						}
						else//����ط������ﵽ�ط�������ִ��NB��ʼ������
						{
							if(ATCurrentCmdNum == AT_CGATT)//�����ǰָ��ΪAT_CGATT���ط�20�κ�ر�NB��Դǿ������
							{
								NbPowerOff();//�ر�NB��Դ
								HAL_Delay (2000);
								//while(1);    //���Ź�5�벻ˢ�½�������
							}
							NB_Init();
							break;
						}
					}
				break;
			case NB_ACCESS:
				ATRec();//Ϊ��֤ʵʱ���յ��·����̵������������20s
				return 0;
			default:
				break;
		}
		return 1;
}
void NbPowerOff(void)
{
	//HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);//�ر�NB��Դ
}
void NbPowerOn(void)
{
	//HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);//��NB��Դ
}
