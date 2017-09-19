#include "timer.h"
#include "led.h"
#include "delay.h"
#include "includes.h"
u32 count0=0;
u32 count1=0;	
//ͨ�ö�ʱ���жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��2
void Timer2_Init(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//PA0,TIM2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 	  //��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //50Mʱ���ٶ�
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //ʱ��ʹ��
	TIM_DeInit(TIM2);
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������5000Ϊ500ms arr
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  		 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //��0��ʱ�ӷָ�:TDTS = Tck_tim	   psc
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ETRClockMode2Config(TIM2, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0); //����Ϊ�ⲿ����ģʽ,Ԥ��Ƶ�رգ��ߵ�ƽ�������ش���
	TIM_SetCounter(TIM2, 0);					 //����������
	TIM_Cmd(TIM2, ENABLE);  //ʹ��TIMx����
							 
}
/***********************
*getspeed ��ȡ��50ms֮�ڵ�
*
*
************************/
 u16 getspeed()
	{
	u16 deta; 
		
	OS_CPU_SR cpu_sr=0;	
		
	OS_ENTER_CRITICAL();  //�����ٽ���(�ر��ж�)
		
	count0=TIM2->CNT;
	delay_ms(50);					 //deta t=5ms
	count1=TIM2->CNT;
		
	OS_EXIT_CRITICAL();  //�˳��ٽ���(���ж�)

	if(count1<count0)				//������
	{
		deta=0xffff-count0+count1;
	}
	else
		deta=count1-count0;
	return deta;
	}












