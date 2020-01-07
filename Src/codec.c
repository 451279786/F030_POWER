#include "codec.h"
#include "usart.h"
#include <string.h>
#include "crc16.h"
#include "sensor.h"


#define SENDNBDATASIZE     80
#define NBRESPONDDATASIZE  35

#define PARSECOMBUFFSIZE  13

uint8_t  temperature = 0;  	    
uint8_t  humidity = 0; 
uint32_t i = 0;
/******************�������ݹ�29���ֽڣ�Ӧ�����ݹ�10���ֽڣ�ƽ̨�·����9���ֽ�*******************/
tsSendChar SendChar = {0};
tsDevicesRespond DevRespond = {0};
tsRecCommand RecComchar = {0};

/************************�������ݺ���**********************/


void SendNBData(void)
{
	//�鷢���ֽ�
	SendChar.DeviceType = DEVICE_TYPE;             //�����������ݽ�������ӻ�������
	SendChar.DeviceNum[0] = HI_UINT32(DEVICE_NUM);
	SendChar.DeviceNum[1] = MH_UINT32(DEVICE_NUM);
	SendChar.DeviceNum[2] = ML_UINT32(DEVICE_NUM);
	SendChar.DeviceNum[3] = LO_UINT32(DEVICE_NUM);
	//SendChar.DeviceStatus = DEVICE_STATUS;       //�豸����״̬
	SendChar.AlarmType = ALARM_TYPE;
	SendChar.DataType = DATA_TYPE;
	SendChar.Rsrp = RSRP;
	SendChar.Snr = SNR;
	SendChar.BatteryLevel = BATTERY_LEVEL;
	SendChar.AcquisitionTime[0] = HI_UINT32(ACQUISITION_TIME(i));
	SendChar.AcquisitionTime[1] = MH_UINT32(ACQUISITION_TIME(i));
	SendChar.AcquisitionTime[2] = ML_UINT32(ACQUISITION_TIME(i));
	SendChar.AcquisitionTime[3] = LO_UINT32(ACQUISITION_TIME(i));
	
	SendChar.Voltage[0] = HI_UINT32(VOATAGE);
	SendChar.Voltage[1] = MH_UINT32(VOATAGE);
	SendChar.Voltage[2] = ML_UINT32(VOATAGE);
	SendChar.Voltage[3] = LO_UINT32(VOATAGE);
	
	SendChar.Current[0] = HI_UINT32(CURRENT);
	SendChar.Current[1] = MH_UINT32(CURRENT);
	SendChar.Current[2] = ML_UINT32(CURRENT);
	SendChar.Current[3] = LO_UINT32(CURRENT);
	
	SendChar.Power[0] = HI_UINT32(POWER);
	SendChar.Power[1] = MH_UINT32(POWER);
	SendChar.Power[2] = ML_UINT32(POWER);
	SendChar.Power[3] = LO_UINT32(POWER);
	
	//����У�����ͱ�׼CRC16-MODBUSһ��
	SendChar.CrcCheck[0] = HI_UINT16(ISRcal_crc16((uint8_t *)&SendChar,27));//CRCУ���룬��1���ֽ�
	SendChar.CrcCheck[1] = LO_UINT16(ISRcal_crc16((uint8_t *)&SendChar,27));//CRCУ���룬��1���ֽ�

	char NbSendData[SENDNBDATASIZE] = {0};
	memset(NbSendData,0,SENDNBDATASIZE);
	//�鷢�Ͱ�
	sprintf(NbSendData,"AT+NMGS=29,%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",
			SendChar.DeviceType,SendChar.DeviceNum[0],SendChar.DeviceNum[1],SendChar.DeviceNum[2],SendChar.DeviceNum[3],SendChar.DeviceStatus,SendChar.AlarmType,
			SendChar.DataType,SendChar.Rsrp,SendChar.Snr,SendChar.BatteryLevel,SendChar.AcquisitionTime[0],SendChar.AcquisitionTime[1],SendChar.AcquisitionTime[2],
			SendChar.AcquisitionTime[3],SendChar.Voltage[0],SendChar.Voltage[1],SendChar.Voltage[2],SendChar.Voltage[3],SendChar.Current[0],SendChar.Current[1],
			SendChar.Current[2],SendChar.Current[3],SendChar.Power[0],SendChar.Power[1],SendChar.Power[2],SendChar.Power[3],SendChar.CrcCheck[0],SendChar.CrcCheck[1]);//ƴ���ַ���

	HAL_UART_Transmit(&huart1,(uint8_t *)&NbSendData,strlen(NbSendData),100);//���͵�����2->NBģ��

}
/************************�������ݺ���**********************/


//**********************************//
//
//�������ƣ�   ParseCommand
//
//����������   �����յ������ݣ�ƽ̨�·�������� 9 ���ֽ�
//
//����������   uint8_t *buff,uint8_t len
//
//����ֵ��     uint8_t
//
//�����ߣ� 
//*******************************//
//�յĵ�buff����16�����������ַ�����Ϊ���ֽ�Ϊ��λ�����ͣ�����ˣ������ٸ�
int hex2byte(uint32_t *dst,char * src)
{
	char *p;                          //����pָ�򡰣�����ֱ�ӷ��ء������ĵ�ַ
	if((p = strchr(src,0x2c)) == NULL)//���pΪ�գ�ֱ���˳�
	{	
		return 0;
	}else
	{
		p++;
	}
	
	while(*p)
	{
		sscanf(p,"%02x",dst);
		p+=2;
		dst++;
	}
	return 1;
}

uint8_t ParseCommand(char *src,uint8_t len)//�յĵ�buff����16������,�����ַ���
{
	uint32_t buffsrc[PARSECOMBUFFSIZE] = {0};
	uint8_t buff[PARSECOMBUFFSIZE] = {0}; //����ת����Ĵ�
	if(hex2byte(buffsrc,src) == 0)   //���Ϊ0��˵��û���ҵ�������
	{
		return 0;
	}
	
	for (int i = 0; i < PARSECOMBUFFSIZE; i++)  //�ȸ�Ϊ9���ֽ�
	{
		buff[i] = buffsrc[i];                      //��32λ4�ֽڵĸ�Ϊ1�ֽڵ����ݣ����ܽ���У��
	}
	
	/*���CRC16У��ʧ�ܣ�ֱ���˳�*/
	RecComchar.CrcCheck = buff[PARSECOMBUFFSIZE-2] <<8 | buff[PARSECOMBUFFSIZE-1];
	uint16_t Crc16Data = ISRcal_crc16(buff, PARSECOMBUFFSIZE - 2);//ȥ��У����Ϊ7���ֽڣ���У��͹������ɵ�һ�£�������㷨

#ifdef DEBUG
	for (int i = 0; i < len; i++)      
	{
		printf("0x%x ",src[i]);			           //��ӡԴ��
	}
	printf("\n������Դ��\n");
	
	for (int i = 0; i < PARSECOMBUFFSIZE; i++)     
	{
		printf("0x%x ",buff[i]);                   //��ӡ�ֽ�����
	}
	printf("\n������ת��Ϊ�ֽڵ�����\n");
	
	uint8_t ch2[60] = {0};
	sscanf(src,"%s",ch2);                          //��ӡ��ֵ��ַ���
	printf("ch2 =%s\n�����ǲ�ֵ��ַ���\n",ch2);
	
	printf("RecComchar.CrcCheck = %x\n",RecComchar.CrcCheck);
	printf("У������֤Crc16Data = %x\n",Crc16Data);
#endif
	
	RecComchar.Cmdnum = buff[0];
	RecComchar.CmdValue = buff[1] <<24 | buff[2] <<16 | buff[3] <<8 | buff[4];
	RecComchar.CmdID = buff[5] <<8 | buff[6];
	
	if (Crc16Data != RecComchar.CrcCheck)
	{
#ifdef DEBUG	
		printf("У��ʧ��\n");
#endif		
		RespondData(RecComchar.CmdID);    //CRCУ��ʧ��ҲҪ����Ӧ��ʧ�ܵ�����,��ID����ȥ 
		memset(buff, 0, PARSECOMBUFFSIZE);//���BUFF
		return 0;
	}
	
	/*���CRC16У��ɹ������Ž���*/
	if(RecComchar.Cmdnum == 0)            //4,������0��ʾƽ̨�·���Ӧ�����·����
	{
		/*������ϵͳʱ��*/
		//Ŀǰ������
	}else if(RecComchar.Cmdnum == 1)      //������Ϊ1����ʾ���·�����
	{
		
#ifdef DEBUG	
		printf("У��ɹ�,��������\n");
#endif	
		if(RecComchar.CmdValue == 1)//�����·���������Ϊ1��˳ʱ���
		{
			//RelayOn();//˳ʱ��
			HAL_Delay(200);
//			if(ReadPinB1())//�������220V����λ����ִ�гɹ�
//			{
//				SendChar.DeviceStatus = 0x1;     //�豸�ϱ�״̬
//				DevRespond.CmdExeResult = 0x00;  //0��ʾ����ִ�гɹ�
//				printf("Relay On OK!\n");
//				RespondData(RecComchar.CmdID);
//			}
//			else
//			{
//				DevRespond.CmdExeResult = 0x01;//1��ʾ����ִ��ʧ��
//				RespondData(RecComchar.CmdID);
//				printf("Relay On fail!\n");
//			}
			
			
		}else if(RecComchar.CmdValue == 0)//�����·���������Ϊ0����ʱ��ر�
		{
			//RelayOff();//��ʱ��
			HAL_Delay(200);
//			if(!ReadPinB1())
//			{
//				SendChar.DeviceStatus = 0x0;     //�豸�ϱ�״̬
//				DevRespond.CmdExeResult = 0x00;  //0��ʾ����ִ�гɹ���1��ʾ����ִ��ʧ��
//				printf("Relay Off OK!\n");
//				RespondData(RecComchar.CmdID);
//			}
//			else
//			{
//				DevRespond.CmdExeResult = 0x01;//1��ʾ����ִ��ʧ��
//				RespondData(RecComchar.CmdID);
//				printf("Relay Off fail!\n");
//			}
		}
	}
	
	return 0;
}
/************************�����յ�������**********************/


//**********************************//
//
//�������ƣ�   RespondData
//
//����������   Ӧ��ƽ̨�·�������,��10���ֽ�
//
//����������   
////////////��ʱ�Ȳ��ôӻ�Ӧ��,��Ҫ���Ĵӻ�Ӧ��ָ��
//����ֵ��     
//////////�ӻ�ִ�������Ҫ��ƽ̨Ӧ��Ҫ�Ѷ�Ӧ������ID����ƽ̨
//�����ߣ� 
//*******************************//
void RespondData(uint16_t CmdID)
{
	/*Ҫ�Ѵ��豸�ı�ŷ������ſ�����Ӧ��Ŀǰ���豸������������豸���*/
//	DevRespond.CmdExeResult = 0x00;//����ִ�н���ӽ��������ȡ��00��ʾ�ɹ���01��ʾʧ��
	DevRespond.DeviceNum[0] = HI_UINT32(DEVICE_NUM);//�豸��ţ�4���ֽ�
	DevRespond.DeviceNum[1] = MH_UINT32(DEVICE_NUM);
	DevRespond.DeviceNum[2] = ML_UINT32(DEVICE_NUM);
	DevRespond.DeviceNum[3] = LO_UINT32(DEVICE_NUM);
	DevRespond.DeviceType = DEVICE_TYPE;            //�豸����1���ֽ�
	DevRespond.CmdID[0] = HI_UINT16(CmdID);         //����ID��������������ȡ
	DevRespond.CmdID[1] = LO_UINT16(CmdID);
	
	DevRespond.CrcCheck[0] = HI_UINT16(ISRcal_crc16((uint8_t *)&DevRespond,8));//CRCУ���룬��1���ֽ�
	DevRespond.CrcCheck[1] = LO_UINT16(ISRcal_crc16((uint8_t *)&DevRespond,8));//CRCУ���룬��2���ֽ�
	
	char NbRespondData[NBRESPONDDATASIZE] = {0};
	memset(NbRespondData,0,NBRESPONDDATASIZE);
	
	sprintf(NbRespondData,"AT+NMGS=10,%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",DevRespond.CmdExeResult,DevRespond.DeviceNum[0],DevRespond.DeviceNum[1],DevRespond.DeviceNum[2],
			DevRespond.DeviceNum[3],DevRespond.DeviceType,DevRespond.CmdID[0],DevRespond.CmdID[1],DevRespond.CrcCheck[0],DevRespond.CrcCheck[1]);//ƴ���ַ������ڷ���
	
	HAL_UART_Transmit(&huart1,(uint8_t *)NbRespondData,strlen(NbRespondData),100);//���͵�����1->NBģ��

#ifdef DEBUG

#endif
	DevRespond.CmdExeResult = 0x01; //����Ӧ��֮�����������Ϊ01������ʼ��Ϊִ��ʧ��
}
/************************Ӧ��ƽ̨�·�������**********************/




