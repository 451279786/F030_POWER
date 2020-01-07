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
/******************发送数据共29个字节，应答数据共10个字节，平台下发命令共9个字节*******************/
tsSendChar SendChar = {0};
tsDevicesRespond DevRespond = {0};
tsRecCommand RecComchar = {0};

/************************发送数据函数**********************/


void SendNBData(void)
{
	//组发送字节
	SendChar.DeviceType = DEVICE_TYPE;             //接收主机数据解析里面从机的数据
	SendChar.DeviceNum[0] = HI_UINT32(DEVICE_NUM);
	SendChar.DeviceNum[1] = MH_UINT32(DEVICE_NUM);
	SendChar.DeviceNum[2] = ML_UINT32(DEVICE_NUM);
	SendChar.DeviceNum[3] = LO_UINT32(DEVICE_NUM);
	//SendChar.DeviceStatus = DEVICE_STATUS;       //设备开关状态
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
	
	//此种校验结果和标准CRC16-MODBUS一致
	SendChar.CrcCheck[0] = HI_UINT16(ISRcal_crc16((uint8_t *)&SendChar,27));//CRC校验码，第1个字节
	SendChar.CrcCheck[1] = LO_UINT16(ISRcal_crc16((uint8_t *)&SendChar,27));//CRC校验码，第1个字节

	char NbSendData[SENDNBDATASIZE] = {0};
	memset(NbSendData,0,SENDNBDATASIZE);
	//组发送包
	sprintf(NbSendData,"AT+NMGS=29,%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",
			SendChar.DeviceType,SendChar.DeviceNum[0],SendChar.DeviceNum[1],SendChar.DeviceNum[2],SendChar.DeviceNum[3],SendChar.DeviceStatus,SendChar.AlarmType,
			SendChar.DataType,SendChar.Rsrp,SendChar.Snr,SendChar.BatteryLevel,SendChar.AcquisitionTime[0],SendChar.AcquisitionTime[1],SendChar.AcquisitionTime[2],
			SendChar.AcquisitionTime[3],SendChar.Voltage[0],SendChar.Voltage[1],SendChar.Voltage[2],SendChar.Voltage[3],SendChar.Current[0],SendChar.Current[1],
			SendChar.Current[2],SendChar.Current[3],SendChar.Power[0],SendChar.Power[1],SendChar.Power[2],SendChar.Power[3],SendChar.CrcCheck[0],SendChar.CrcCheck[1]);//拼接字符串

	HAL_UART_Transmit(&huart1,(uint8_t *)&NbSendData,strlen(NbSendData),100);//发送到串口2->NB模组

}
/************************发送数据函数**********************/


//**********************************//
//
//函数名称：   ParseCommand
//
//函数描述：   解析收到的数据，平台下发的命令共有 9 个字节
//
//函数参数：   uint8_t *buff,uint8_t len
//
//返回值：     uint8_t
//
//创建者： 
//*******************************//
//收的的buff不是16进制流，将字符串改为以字节为单位的整型，先如此，后面再改
int hex2byte(uint32_t *dst,char * src)
{
	char *p;                          //定义p指向“，”，直接返回“，”的地址
	if((p = strchr(src,0x2c)) == NULL)//如果p为空，直接退出
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

uint8_t ParseCommand(char *src,uint8_t len)//收的的buff不是16进制流,而是字符串
{
	uint32_t buffsrc[PARSECOMBUFFSIZE] = {0};
	uint8_t buff[PARSECOMBUFFSIZE] = {0}; //接收转换后的串
	if(hex2byte(buffsrc,src) == 0)   //如果为0，说明没有找到“，”
	{
		return 0;
	}
	
	for (int i = 0; i < PARSECOMBUFFSIZE; i++)  //先改为9个字节
	{
		buff[i] = buffsrc[i];                      //将32位4字节的改为1字节的数据，才能进行校验
	}
	
	/*如果CRC16校验失败，直接退出*/
	RecComchar.CrcCheck = buff[PARSECOMBUFFSIZE-2] <<8 | buff[PARSECOMBUFFSIZE-1];
	uint16_t Crc16Data = ISRcal_crc16(buff, PARSECOMBUFFSIZE - 2);//去掉校验码为7个字节，此校验和工具生成的一致，上润的算法

#ifdef DEBUG
	for (int i = 0; i < len; i++)      
	{
		printf("0x%x ",src[i]);			           //打印源串
	}
	printf("\n上面是源串\n");
	
	for (int i = 0; i < PARSECOMBUFFSIZE; i++)     
	{
		printf("0x%x ",buff[i]);                   //打印字节数组
	}
	printf("\n上面是转换为字节的数组\n");
	
	uint8_t ch2[60] = {0};
	sscanf(src,"%s",ch2);                          //打印拆分的字符串
	printf("ch2 =%s\n上面是拆分的字符串\n",ch2);
	
	printf("RecComchar.CrcCheck = %x\n",RecComchar.CrcCheck);
	printf("校验码验证Crc16Data = %x\n",Crc16Data);
#endif
	
	RecComchar.Cmdnum = buff[0];
	RecComchar.CmdValue = buff[1] <<24 | buff[2] <<16 | buff[3] <<8 | buff[4];
	RecComchar.CmdID = buff[5] <<8 | buff[6];
	
	if (Crc16Data != RecComchar.CrcCheck)
	{
#ifdef DEBUG	
		printf("校验失败\n");
#endif		
		RespondData(RecComchar.CmdID);    //CRC校验失败也要发送应答失败的命令,把ID传过去 
		memset(buff, 0, PARSECOMBUFFSIZE);//清空BUFF
		return 0;
	}
	
	/*如果CRC16校验成功，接着解析*/
	if(RecComchar.Cmdnum == 0)            //4,命令编号0表示平台下发的应答，无下发命令，
	{
		/*待解析系统时间*/
		//目前无需求
	}else if(RecComchar.Cmdnum == 1)      //命令编号为1，表示有下发命令
	{
		
#ifdef DEBUG	
		printf("校验成功,继续解析\n");
#endif	
		if(RecComchar.CmdValue == 1)//解析下发的命令，如果为1，顺时针打开
		{
			//RelayOn();//顺时针
			HAL_Delay(200);
//			if(ReadPinB1())//如果读到220V，置位命令执行成功
//			{
//				SendChar.DeviceStatus = 0x1;     //设备上报状态
//				DevRespond.CmdExeResult = 0x00;  //0表示命令执行成功
//				printf("Relay On OK!\n");
//				RespondData(RecComchar.CmdID);
//			}
//			else
//			{
//				DevRespond.CmdExeResult = 0x01;//1表示命令执行失败
//				RespondData(RecComchar.CmdID);
//				printf("Relay On fail!\n");
//			}
			
			
		}else if(RecComchar.CmdValue == 0)//解析下发的命令，如果为0，逆时针关闭
		{
			//RelayOff();//逆时针
			HAL_Delay(200);
//			if(!ReadPinB1())
//			{
//				SendChar.DeviceStatus = 0x0;     //设备上报状态
//				DevRespond.CmdExeResult = 0x00;  //0表示命令执行成功，1表示命令执行失败
//				printf("Relay Off OK!\n");
//				RespondData(RecComchar.CmdID);
//			}
//			else
//			{
//				DevRespond.CmdExeResult = 0x01;//1表示命令执行失败
//				RespondData(RecComchar.CmdID);
//				printf("Relay Off fail!\n");
//			}
		}
	}
	
	return 0;
}
/************************解析收到的数据**********************/


//**********************************//
//
//函数名称：   RespondData
//
//函数描述：   应答平台下发的命令,共10个字节
//
//函数参数：   
////////////暂时先不让从机应答,需要更改从机应答指令
//返回值：     
//////////从机执行命令后要给平台应答，要把对应的命令ID发给平台
//创建者： 
//*******************************//
void RespondData(uint16_t CmdID)
{
	/*要把从设备的编号发过来才可以做应答，目前的设备编号是主机的设备编号*/
//	DevRespond.CmdExeResult = 0x00;//命令执行结果从解析里面读取，00表示成功，01表示失败
	DevRespond.DeviceNum[0] = HI_UINT32(DEVICE_NUM);//设备编号，4个字节
	DevRespond.DeviceNum[1] = MH_UINT32(DEVICE_NUM);
	DevRespond.DeviceNum[2] = ML_UINT32(DEVICE_NUM);
	DevRespond.DeviceNum[3] = LO_UINT32(DEVICE_NUM);
	DevRespond.DeviceType = DEVICE_TYPE;            //设备类型1个字节
	DevRespond.CmdID[0] = HI_UINT16(CmdID);         //命令ID从命令解析里面获取
	DevRespond.CmdID[1] = LO_UINT16(CmdID);
	
	DevRespond.CrcCheck[0] = HI_UINT16(ISRcal_crc16((uint8_t *)&DevRespond,8));//CRC校验码，第1个字节
	DevRespond.CrcCheck[1] = LO_UINT16(ISRcal_crc16((uint8_t *)&DevRespond,8));//CRC校验码，第2个字节
	
	char NbRespondData[NBRESPONDDATASIZE] = {0};
	memset(NbRespondData,0,NBRESPONDDATASIZE);
	
	sprintf(NbRespondData,"AT+NMGS=10,%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",DevRespond.CmdExeResult,DevRespond.DeviceNum[0],DevRespond.DeviceNum[1],DevRespond.DeviceNum[2],
			DevRespond.DeviceNum[3],DevRespond.DeviceType,DevRespond.CmdID[0],DevRespond.CmdID[1],DevRespond.CrcCheck[0],DevRespond.CrcCheck[1]);//拼接字符串用于发送
	
	HAL_UART_Transmit(&huart1,(uint8_t *)NbRespondData,strlen(NbRespondData),100);//发送到串口1->NB模组

#ifdef DEBUG

#endif
	DevRespond.CmdExeResult = 0x01; //发送应答之后把命令结果置为01，即初始化为执行失败
}
/************************应答平台下发的命令**********************/




