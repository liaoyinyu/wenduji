#include "timer.h"
#include "led.h"
#include "delay.h"
#include "includes.h"
u32 count0=0;
u32 count1=0;	
//通用定时器中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器2
void Timer2_Init(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//PA0,TIM2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 	  //上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //50M时钟速度
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //时钟使能
	TIM_DeInit(TIM2);
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms arr
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  		 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设0置时钟分割:TDTS = Tck_tim	   psc
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ETRClockMode2Config(TIM2, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0); //设置为外部计数模式,预分频关闭，高电平或上升沿触发
	TIM_SetCounter(TIM2, 0);					 //计数器清零
	TIM_Cmd(TIM2, ENABLE);  //使能TIMx外设
							 
}
/***********************
*getspeed 获取在50ms之内的
*
*
************************/
 u16 getspeed()
	{
	u16 deta; 
		
	OS_CPU_SR cpu_sr=0;	
		
	OS_ENTER_CRITICAL();  //进入临界区(关闭中断)
		
	count0=TIM2->CNT;
	delay_ms(50);					 //deta t=5ms
	count1=TIM2->CNT;
		
	OS_EXIT_CRITICAL();  //退出临界区(开中断)

	if(count1<count0)				//如果溢出
	{
		deta=0xffff-count0+count1;
	}
	else
		deta=count1-count0;
	return deta;
	}












