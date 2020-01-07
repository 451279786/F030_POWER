#include "dht11.h"


//复位DHT11
void DHT11_Rst(void)	   
{                 
	DHT11_Mode_Out_PP();
    DHT11_Dout_LOW();         //SET OUTPUT  拉低
    HAL_Delay(18);    	//拉低至少18ms
    DHT11_Dout_HIGH();        //DQ=1        拉高
	delay_us(30);     	//主机拉高20~40us
}
//等待DHT11的回应
//返回1:未检测到DHT11的存在
//返回0:存在
uint8_t DHT11_Check(void) 	   
{   
	uint8_t retry=0;
	DHT11_Mode_IPU();//SET INPUT	 
    while (DHT11_Data_IN()&&retry<100)//等待拉低，DHT11会拉低40~80us
	{
		retry++;
		delay_us(1);
	};
	
	if(retry>=100)
	{
		return 1;
	}
	else
	{
		retry=0;
	}
		
	
    while (!DHT11_Data_IN()&&retry<100)//DHT11拉低后会再次拉高40~80us
	{
		retry++;
		delay_us(1);
	};
	if(retry>=100)
	{
		return 1;
	}    
	return 0;
}
//从DHT11读取一个位
//返回值：1/0
uint8_t DHT11_Read_Bit(void) 			 
{

 	uint8_t retry=0;
	while(DHT11_Data_IN()&&retry<100)//等待变为低电平
	{
		retry++;
		delay_us(1);
	}
	retry=0;
	while(!DHT11_Data_IN()&&retry<100)//等待变高电平
	{
		retry++;
		delay_us(1);
	}
	delay_us(40);//等待40us
	if(DHT11_Data_IN())return 1;
	else return 0;	
	   
}
//从DHT11读取一个字节
//返回值：读到的数据
uint8_t DHT11_Read_Byte(void)    
{        
    uint8_t i,dat;
    dat=0;
	for (i=0;i<8;i++) 
	{
   		dat<<=1; 
	    dat|=DHT11_Read_Bit();
    }						    
    return dat;
}
//从DHT11读取一次数据
//temp:温度值(范围:0~50°)
//humi:湿度值(范围:20%~90%)
//返回值：0,正常;1,读取失败
uint8_t DHT11_Read_Data(uint8_t *temp,uint8_t *humi)    
{        
 	uint8_t buf[5];
	uint8_t i;
	DHT11_Rst();
	if(DHT11_Check()==0)
	{
		for(i=0;i<5;i++)//读取40位数据
		{
			buf[i]=DHT11_Read_Byte();
		}
		if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
		{
			*humi=buf[0];
			*temp=buf[2];
		}
	}else
	{
		return 1;
	}
	return 0;	    
}
//初始化DHT11的IO口 DQ 同时检测DHT11的存在
//返回1:不存在
//返回0:存在    	 
uint8_t DHT11_Init(void)
{	 
	//SetPin_OutPut();
	DHT11_Rst();  //复位DHT11
	return DHT11_Check();//等待DHT11的回应
} 

void delay_us(uint32_t time)
{
	uint8_t i;

	while(time)
	{   
		//for (i = 0; i < 10; i++)//48M的时钟延时
		for (i = 0; i < 1; i++)//8M的时钟延时
		{
		  
		}
	time--;
	}
}

//void delay_us(uint32_t us)
//{
//    uint32_t delay = (HAL_RCC_GetHCLKFreq() / 4000000 * us);
//    while (delay--)
//	{
//		;
//	}
//} 


//设置引脚输入
static void DHT11_Mode_IPU(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  /* 串口外设功能GPIO配置 */
  GPIO_InitStruct.Pin   = DHT11_Dout_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  HAL_GPIO_Init(DHT11_Dout_PORT, &GPIO_InitStruct);
        
}

/**
  * 函数功能: 使DHT11-DATA引脚变为推挽输出模式
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
static void DHT11_Mode_Out_PP(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  /* 串口外设功能GPIO配置 */
  GPIO_InitStruct.Pin = DHT11_Dout_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_Dout_PORT, &GPIO_InitStruct);         
}



void Int_To_Str(int x,char *Str)
{
	int t;
	char *Ptr,Buf[5];//定义一个字符串数组和字符串指针，
	int i = 0;  //用于计数
	Ptr = Str;  //内部指针指向外部指针，进行参数传递，是属于源参数传递（通过地址），
	if(x < 10)		                 // 当整数小于10，转换为0x格式
	{
		*Ptr ++ = '0'; //单个数字前面补“0”
		*Ptr ++ = x+0x30;
	}
	else
	{
		while(x > 0)
		{
			t = x % 10;
			x = x / 10;
			Buf[i++] = t+0x30;	// 通过计算把数字编成ASCII码形式
		}
		i -- ;
		for(;i >= 0;i --) 		// 将得到的字符串倒序
		{
			*(Ptr++) = Buf[i];
		}
	}
	*Ptr = '\0';
}
