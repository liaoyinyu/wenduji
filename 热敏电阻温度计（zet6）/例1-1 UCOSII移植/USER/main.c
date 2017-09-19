#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "OLED.h"
#include "bmp.h"
#include "includes.h"
/************************************************
 ALIENTEK 精英版STM32开发板UCOS实验 
 UCOSII移植
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

//START 任务
//设置任务优先级
#define START_TASK_PRIO			10  ///开始任务的优先级为最低
//设置任务堆栈大小
#define START_STK_SIZE			128
//任务任务堆栈
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);

//LED0任务
//设置任务优先级
#define LED0_TASK_PRIO			7
//设置任务堆栈大小
#define LED0_STK_SIZE			64
//任务堆栈
OS_STK LED0_TASK_STK[LED0_STK_SIZE];
//任务函数
void led0_task(void *pdata);

//LED1任务
//设置任务优先级
#define OLED_TASK_PRIO			6
//设置任务堆栈大小
#define OLED_STK_SIZE			64
//任务堆栈
OS_STK LED1_TASK_STK[OLED_STK_SIZE];
//任务函数
void OLED_task(void *pdata);

//浮点测试任务
#define GETT_TASK_PRIO			5
//设置任务堆栈大小
#define GETT_STK_SIZE			128
//任务堆栈
//如果任务中使用printf来打印浮点数据的话一点要8字节对齐
__align(8) OS_STK GETT_TASK_STK[GETT_STK_SIZE]; 
//任务函数
void GETT_task(void *pdata);

//设置全局变量speed
u16 speed;
u16 counter3[10]={0,0,0,0,0,0,0,0,0,0};
u32 counter_m=0;
u8 bmph=0;//用于决定显示温度计的温度高度
float temperature=0;
float flequency=0;
/**************************************
*频率转换为温度
*暂定返回为频率值
**************************************/
float fl_to_T(float flequency)
{
		float temperature=0;
		float p1[6]={-0.3121,-0.0466 ,-0.01395,0.001321,-0.0009559,-0.0009559};
		float p2[6]={ 7.894 ,3.41    ,2.226   ,1.52    ,1.667     ,1.667     };
		float p3[6]={ -17.79,1.185   ,11.95   ,21.72   ,13.69     ,13.69     };
		u8 i=0;
		OS_CPU_SR cpu_sr=0;
		
		i=(u8)flequency/10;
		if(i>5) i=5;
		
		OS_ENTER_CRITICAL();  //进入临界区(关闭中断)
//		temperature=0.004228*flequency*flequency+1.173*flequency+25.29;
			temperature=p1[i]*flequency*flequency+p2[i]*flequency+p3[i];		
		OS_EXIT_CRITICAL();  //退出临界区(开中断)
		
		return temperature;
}

int main(void)
{	
	

	//系统初始化
	SystemInit();
	delay_init();       //延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //中断分组配置
	OLED_Init();			//初始化OLED  
	Timer2_Init(0xffff,0);	//计数器2初始化
	uart_init(115200);    //串口波特率设置
	LED_Init();  	//LED初始化
	OLED_Clear(); 
	OLED_DrawBMP(0,0,128,8,BMP1); 
	LED0=0;
	
	OSInit();  		//UCOS初始化
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO); //创建开始任务
	OSStart(); 	//开始任务
}

//开始任务
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;
	pdata=pdata;
	OSStatInit();  //开启统计任务
	
	OS_ENTER_CRITICAL();  //进入临界区(关闭中断)
	OSTaskCreate(led0_task,(void*)0,(OS_STK*)&LED0_TASK_STK[LED0_STK_SIZE-1],LED0_TASK_PRIO);//创建LED0任务
	OSTaskCreate(OLED_task,(void*)0,(OS_STK*)&LED1_TASK_STK[OLED_STK_SIZE-1],OLED_TASK_PRIO);//创建OLED任务
	OSTaskCreate(GETT_task,(void*)0,(OS_STK*)&GETT_TASK_STK[GETT_STK_SIZE-1],GETT_TASK_PRIO);//创建计算温度任务
	OSTaskSuspend(START_TASK_PRIO);//挂起开始任务
	OS_EXIT_CRITICAL();  //退出临界区(开中断)
}
 

//LED0任务
void led0_task(void *pdata)
{
	while(1)
	{
		LED0=0; 
		delay_ms(1000);
		LED0=1;
		delay_ms(1000);
	}
}

//OLED任务
void OLED_task(void *pdata)
{
	while(1)
	{
				OLED_showbh(0,BMP2[bmph]);
		//	OLED_ShowNum(64,4,t,3,16);//显示ASCII字符的码值 							
				show_R(64,4,temperature);
	//			show_R(64,6,flequency);
				delay_ms(1200);
	}
}

//获取计数值，计算频率，计算温度任务
void GETT_task(void *pdata)
{

	u16 i=0; //用于统计循环次数
//	OS_CPU_SR cpu_sr=0;
	while(1)
	{
//	OS_ENTER_CRITICAL();	//进入临界区(关闭中断)
	speed=getspeed();//获取计数器在50ms之内的计数值
//	OS_EXIT_CRITICAL();		//退出临界区(开中断)
	for (i=0;i<9;i++)
			counter3[i] = counter3[i+1];
	counter3[9]=speed;
	counter_m=0;
	for(i=0;i<10;i++)
			counter_m=counter_m+counter3[i];		//计算10次计数值之和		
	//			flequency=counter_m*20/1000/10;		//计算输入信号频率，单位khz
	flequency=(float)counter_m/500;		
	temperature=fl_to_T(flequency);
	//	temperature=fl_to_T(flequency);
	bmph=temperature/10;		
	if(bmph>12) bmph=12;
	delay_ms(100);
	}
}

