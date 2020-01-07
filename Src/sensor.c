#include "sensor.h"
#include "nbiotdriver.h"
#include "time.h"
//#include "adc.h"
#include "codec.h"
//#include "iwdg.h"
//#include "tim.h"
#include "gpio.h"

/*可燃气体采集任务
void SensorTask(void)         //可燃气体检测任务
{
	if(CompareTime(&TimeSensor))//采集周期是否到达
	{
		HAL_ADC_Start(&hadc1);//开启ADC采集
		HAL_ADC_PollForConversion(&hadc1, 1);//等待ADC采集完成
		if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1),HAL_ADC_STATE_REG_EOC))//判断ADC是否转换完成
		{
			AdcValue = HAL_ADC_GetValue(&hadc1);//获取ADC的值
			printf("AdcValue = %d\r\n",AdcValue);
			if(AdcValue < 4000)//如果ADC值小于4000则立即上报一次，NB会重新计时
			{                  //如果下次采集数据正常，下次上报的时间以NB计时为准
				TriggerSendData();
			}
		}
		SetTime(&TimeSensor,TIMESENSOR);//重启定时器
	}
}
*/


