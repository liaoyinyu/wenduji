#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "OLED.h"
#include "bmp.h"
#include "includes.h"
/************************************************
 ALIENTEK ��Ӣ��STM32������UCOSʵ�� 
 UCOSII��ֲ
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

//START ����
//�����������ȼ�
#define START_TASK_PRIO			10  ///��ʼ��������ȼ�Ϊ���
//���������ջ��С
#define START_STK_SIZE			128
//���������ջ
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);

//LED0����
//�����������ȼ�
#define LED0_TASK_PRIO			7
//���������ջ��С
#define LED0_STK_SIZE			64
//�����ջ
OS_STK LED0_TASK_STK[LED0_STK_SIZE];
//������
void led0_task(void *pdata);

//LED1����
//�����������ȼ�
#define OLED_TASK_PRIO			6
//���������ջ��С
#define OLED_STK_SIZE			64
//�����ջ
OS_STK LED1_TASK_STK[OLED_STK_SIZE];
//������
void OLED_task(void *pdata);

//�����������
#define GETT_TASK_PRIO			5
//���������ջ��С
#define GETT_STK_SIZE			128
//�����ջ
//���������ʹ��printf����ӡ�������ݵĻ�һ��Ҫ8�ֽڶ���
__align(8) OS_STK GETT_TASK_STK[GETT_STK_SIZE]; 
//������
void GETT_task(void *pdata);

//����ȫ�ֱ���speed
u16 speed;
u16 counter3[10]={0,0,0,0,0,0,0,0,0,0};
u32 counter_m=0;
u8 bmph=0;//���ھ�����ʾ�¶ȼƵ��¶ȸ߶�
float temperature=0;
float flequency=0;
/**************************************
*Ƶ��ת��Ϊ�¶�
*�ݶ�����ΪƵ��ֵ
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
		
		OS_ENTER_CRITICAL();  //�����ٽ���(�ر��ж�)
//		temperature=0.004228*flequency*flequency+1.173*flequency+25.29;
			temperature=p1[i]*flequency*flequency+p2[i]*flequency+p3[i];		
		OS_EXIT_CRITICAL();  //�˳��ٽ���(���ж�)
		
		return temperature;
}

int main(void)
{	
	

	//ϵͳ��ʼ��
	SystemInit();
	delay_init();       //��ʱ��ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�жϷ�������
	OLED_Init();			//��ʼ��OLED  
	Timer2_Init(0xffff,0);	//������2��ʼ��
	uart_init(115200);    //���ڲ���������
	LED_Init();  	//LED��ʼ��
	OLED_Clear(); 
	OLED_DrawBMP(0,0,128,8,BMP1); 
	LED0=0;
	
	OSInit();  		//UCOS��ʼ��
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO); //������ʼ����
	OSStart(); 	//��ʼ����
}

//��ʼ����
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;
	pdata=pdata;
	OSStatInit();  //����ͳ������
	
	OS_ENTER_CRITICAL();  //�����ٽ���(�ر��ж�)
	OSTaskCreate(led0_task,(void*)0,(OS_STK*)&LED0_TASK_STK[LED0_STK_SIZE-1],LED0_TASK_PRIO);//����LED0����
	OSTaskCreate(OLED_task,(void*)0,(OS_STK*)&LED1_TASK_STK[OLED_STK_SIZE-1],OLED_TASK_PRIO);//����OLED����
	OSTaskCreate(GETT_task,(void*)0,(OS_STK*)&GETT_TASK_STK[GETT_STK_SIZE-1],GETT_TASK_PRIO);//���������¶�����
	OSTaskSuspend(START_TASK_PRIO);//����ʼ����
	OS_EXIT_CRITICAL();  //�˳��ٽ���(���ж�)
}
 

//LED0����
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

//OLED����
void OLED_task(void *pdata)
{
	while(1)
	{
				OLED_showbh(0,BMP2[bmph]);
		//	OLED_ShowNum(64,4,t,3,16);//��ʾASCII�ַ�����ֵ 							
				show_R(64,4,temperature);
	//			show_R(64,6,flequency);
				delay_ms(1200);
	}
}

//��ȡ����ֵ������Ƶ�ʣ������¶�����
void GETT_task(void *pdata)
{

	u16 i=0; //����ͳ��ѭ������
//	OS_CPU_SR cpu_sr=0;
	while(1)
	{
//	OS_ENTER_CRITICAL();	//�����ٽ���(�ر��ж�)
	speed=getspeed();//��ȡ��������50ms֮�ڵļ���ֵ
//	OS_EXIT_CRITICAL();		//�˳��ٽ���(���ж�)
	for (i=0;i<9;i++)
			counter3[i] = counter3[i+1];
	counter3[9]=speed;
	counter_m=0;
	for(i=0;i<10;i++)
			counter_m=counter_m+counter3[i];		//����10�μ���ֵ֮��		
	//			flequency=counter_m*20/1000/10;		//���������ź�Ƶ�ʣ���λkhz
	flequency=(float)counter_m/500;		
	temperature=fl_to_T(flequency);
	//	temperature=fl_to_T(flequency);
	bmph=temperature/10;		
	if(bmph>12) bmph=12;
	delay_ms(100);
	}
}

