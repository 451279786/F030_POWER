#include "sensor.h"
#include "nbiotdriver.h"
#include "time.h"
//#include "adc.h"
#include "codec.h"
//#include "iwdg.h"
//#include "tim.h"
#include "gpio.h"

/*��ȼ����ɼ�����
void SensorTask(void)         //��ȼ����������
{
	if(CompareTime(&TimeSensor))//�ɼ������Ƿ񵽴�
	{
		HAL_ADC_Start(&hadc1);//����ADC�ɼ�
		HAL_ADC_PollForConversion(&hadc1, 1);//�ȴ�ADC�ɼ����
		if(HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1),HAL_ADC_STATE_REG_EOC))//�ж�ADC�Ƿ�ת�����
		{
			AdcValue = HAL_ADC_GetValue(&hadc1);//��ȡADC��ֵ
			printf("AdcValue = %d\r\n",AdcValue);
			if(AdcValue < 4000)//���ADCֵС��4000�������ϱ�һ�Σ�NB�����¼�ʱ
			{                  //����´βɼ������������´��ϱ���ʱ����NB��ʱΪ׼
				TriggerSendData();
			}
		}
		SetTime(&TimeSensor,TIMESENSOR);//������ʱ��
	}
}
*/


