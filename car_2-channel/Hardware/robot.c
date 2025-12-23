#include "stm32f10x.h"                  // Device header
#include "PWM.h"
#include "Delay.h"

// �����˳�ʼ��
void robot_Init(void)
{
	PWM_Init(); 
}

//��·PWM�����ٶȵ���
void robot_speed(uint8_t left1_speed,uint8_t left2_speed,uint8_t right1_speed,uint8_t right2_speed)
{	
	    TIM_SetCompare1(TIM4,left1_speed);
      TIM_SetCompare2(TIM4,left2_speed);
      TIM_SetCompare3(TIM4,right1_speed);
      TIM_SetCompare4(TIM4,right2_speed);
}

// �������˶�����
// ������ǰ��
void makerobo_run(int8_t speed,uint16_t time)  //ǰ������
{
      if(speed > 100)
			{
				speed = 100;
			}
			if(speed < 0)
			{
				speed = 0;
			}
	    robot_speed(0,speed,0,speed);
			Delay_ms(time);                 // ʱ��Ϊ����
			//robot_speed(0,0,0,0);           // ������ֹͣ
 
}

void makerobo_brake(uint16_t time) //ɲ������
{
		robot_speed(0,0,0,0);     // ���ֹͣ 
		Delay_ms(time);          // ʱ��Ϊ����   
}

void makerobo_Left(int8_t speed,uint16_t time) //��ת����
{
	    if(speed > 100)
			{
				speed = 100;
			}
			if(speed < 0)
			{
				speed = 0;
			}
		robot_speed(0,0,0,speed);
		Delay_ms(time);                 //ʱ��Ϊ����  
	  //robot_speed(0,0,0,0);           // ������ֹͣ

}

void makerobo_Spin_Left(int8_t speed,uint16_t time) //����ת����
{
		  if(speed > 100)
			{
				speed = 100;
			}
			if(speed < 0)
			{
				speed = 0;
			}  
		robot_speed(0,speed,speed,0);
		Delay_ms(time);                    //ʱ��Ϊ���� 
    //robot_speed(0,0,0,0);           // ������ֹͣ			
}

void makerobo_Right(int8_t speed,uint16_t time) //��ת����
{
	    if(speed > 100)
			{
				speed = 100;
			}
			if(speed < 0)
			{
				speed = 0;
			}
		robot_speed(0,speed,0,0);
		Delay_ms(time);                 //ʱ��Ϊ����  
	  //robot_speed(0,0,0,0);           // ������ֹͣ

}

void makerobo_Spin_Right(int8_t speed,uint16_t time) //����ת����
{
		  if(speed > 100)
			{
				speed = 100;
			}
			if(speed < 0)
			{
				speed = 0;
			}  
		robot_speed(speed,0,0,speed);
		Delay_ms(time);                    //ʱ��Ϊ���� 
    //robot_speed(0,0,0,0);           // ������ֹͣ			
}

void makerobo_back(int8_t speed,uint16_t time)  //���˺���
{
      if(speed > 100)
			{
				speed = 100;
			}
			if(speed < 0)
			{
				speed = 0;
			}
	    robot_speed(0,speed,0,speed);
			Delay_ms(time);                 // ʱ��Ϊ����
			//robot_speed(0,0,0,0);           // ������ֹͣ
 
}
