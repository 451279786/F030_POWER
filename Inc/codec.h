#ifndef _CODEC_H
#define _CODEC_H
#include "main.h"


extern uint8_t  temperature;  	    
extern uint8_t  humidity; 
extern uint32_t i;

/*****���ݳ�ʼֵ�궨��*****/
#define DEVICE_TYPE       0x0A                //�豸���ͣ�1���ֽ�
#define DEVICE_NUM        0xabcdef98          //�豸��ţ�4���ֽ�,��д��ĸ��Сд��ĸʹ��CRCУ�������һ����
#define DEVICE_STATUS     0x00                //�豸״̬��1���ֽ�,����״̬
#define ALARM_TYPE        0x00                //�������ͣ�1���ֽ�,00������ 01����ѹ
#define DATA_TYPE         0x00                //�������ͣ�1���ֽ�,00������ 01������
#define RSRP              0x00                //�ź�ǿ�ȣ�1���ֽ�
#define SNR               0x00                //����ǿ�ȣ�1���ֽ�,�з�������
#define BATTERY_LEVEL     0x00                //��ص�����1���ֽ�
#define ACQUISITION_TIME(a)  (a)          //�ɼ�ʱ�䣬4���ֽ�
#define VOATAGE           0x000000dc          //�е��ѹ��4���ֽ�
#define CURRENT           temperature          //���ص�����4���ֽ�
#define POWER             humidity          //���ع��ʣ�4���ֽ�
	
//#define CMD_EXE_RESULT                        //����ִ�н����1���ֽڣ�1Ĭ��Ϊʧ�ܣ�0��ʾ�ɹ�
//#define CMD_TYPE
//#define CMD_VALUE
//#define CMD_ID

//���uint32����Ϊһ���ֽ�
#define HI_UINT32(a) (((a) >> 24) & 0xFF)
#define MH_UINT32(a) (((a) >> 16) & 0xFF)
#define ML_UINT32(a) (((a) >> 8) & 0xFF)
#define LO_UINT32(a) ((a) & 0xFF)

//���uint16����Ϊһ���ֽ�
#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)

//�豸��������
typedef struct
{
	uint8_t DeviceType;
	uint8_t DeviceNum[4];
	uint8_t DeviceStatus;
	uint8_t AlarmType;
	uint8_t DataType;
	uint8_t Rsrp;
	uint8_t Snr;
	uint8_t BatteryLevel;
	uint8_t AcquisitionTime[4];
	uint8_t Voltage[4];
	uint8_t Current[4];
	uint8_t Power[4];
	uint8_t CrcCheck[2];
}tsSendChar;

//Ӧ�����ݰ�
typedef struct
{
	uint8_t CmdExeResult;
	uint8_t DeviceNum[4];       
	uint8_t DeviceType;
	uint8_t CmdID[2];
	uint8_t CrcCheck[2];
}tsDevicesRespond;

//��������
typedef struct
{
	uint8_t Cmdnum;
	uint32_t CmdValue;
	uint16_t CmdID;
	uint32_t DeviceNum;
	uint16_t CrcCheck;
}tsRecCommand;

extern tsSendChar SendChar;
	
void SendNBData(void);
void RespondData(uint16_t CmdID);
uint8_t ParseCommand(char *buff,uint8_t len);


#endif
