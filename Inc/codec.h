#ifndef _CODEC_H
#define _CODEC_H
#include "main.h"


extern uint8_t  temperature;  	    
extern uint8_t  humidity; 
extern uint32_t i;

/*****数据初始值宏定义*****/
#define DEVICE_TYPE       0x0A                //设备类型，1个字节
#define DEVICE_NUM        0xabcdef98          //设备编号，4个字节,大写字母和小写字母使用CRC校验出来是一样的
#define DEVICE_STATUS     0x00                //设备状态，1个字节,开关状态
#define ALARM_TYPE        0x00                //报警类型，1个字节,00：正常 01：过压
#define DATA_TYPE         0x00                //数据类型，1个字节,00：正常 01：报警
#define RSRP              0x00                //信号强度，1个字节
#define SNR               0x00                //噪声强度，1个字节,有符号整型
#define BATTERY_LEVEL     0x00                //电池电量，1个字节
#define ACQUISITION_TIME(a)  (a)          //采集时间，4个字节
#define VOATAGE           0x000000dc          //市电电压，4个字节
#define CURRENT           temperature          //负载电流，4个字节
#define POWER             humidity          //负载功率，4个字节
	
//#define CMD_EXE_RESULT                        //命令执行结果，1个字节，1默认为失败，0表示成功
//#define CMD_TYPE
//#define CMD_VALUE
//#define CMD_ID

//拆分uint32类型为一个字节
#define HI_UINT32(a) (((a) >> 24) & 0xFF)
#define MH_UINT32(a) (((a) >> 16) & 0xFF)
#define ML_UINT32(a) (((a) >> 8) & 0xFF)
#define LO_UINT32(a) ((a) & 0xFF)

//拆分uint16类型为一个字节
#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)

//设备发送数据
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

//应答数据包
typedef struct
{
	uint8_t CmdExeResult;
	uint8_t DeviceNum[4];       
	uint8_t DeviceType;
	uint8_t CmdID[2];
	uint8_t CrcCheck[2];
}tsDevicesRespond;

//接收命令
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
