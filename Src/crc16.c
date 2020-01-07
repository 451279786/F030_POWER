/************************************************
//ISR_CRC16检验(中断中使用)
***********************************************************************/
#include "crc16.h"
//#include "iwdg.h"

volatile static uint32_t IwdgTime = 0;

uint16_t ISRcal_crc16(uint8_t *p_data,uint8_t leng)  
{
 uint8_t i,DataCrc;
 uint16_t crc16=0xffff;
 while(leng--)
 {
   DataCrc=*p_data;
   p_data++;
     crc16=crc16^DataCrc;
     for( i=0;i<8;i++ )
     {
        if( (crc16&0x0001)==1 )
            crc16=(crc16>>1)^0xa001;
        else
            crc16=crc16>>1;
     }
 }
 return crc16;
}

/*
char* strlwr(char *str)
{
    if(str == NULL)
        return NULL;
         
    char *p = str;
    while (*p != '\0')
    {
        if(*p >= 'A' && *p <= 'Z')
            *p = (*p) + 0x20;
        p++;
    }
    return str;
}
*/
void ReIwdg(void)
{
	if ((HAL_GetTick() - IwdgTime) > 3000)//每8s执行一次喂狗程序
	{
		IwdgTime = HAL_GetTick();
	}
}
