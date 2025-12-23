#include "stm32f10x.h"                  // Device header
#include "Irtracking.h"

void Irtracking_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

// 最左侧传感器读取
uint8_t Left1_Irtracking_Get(void)
{
	return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13);
}

// 左中传感器读取
uint8_t Left2_Irtracking_Get(void)
{
	return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12);
}

// 右中传感器读取
uint8_t Right1_Irtracking_Get(void)
{
	return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11);
}

// 最右侧传感器读取
uint8_t Right2_Irtracking_Get(void)
{
	return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10);
}

// 获取完整的跟踪状态（4位）
uint8_t Get_Tracking_State(void)
{
	uint8_t state = 0;
	
	// 最左侧传感器状态(bit3)
	state |= (Left1_Irtracking_Get() << 3);
	// 左中传感器状态(bit2)
	state |= (Left2_Irtracking_Get() << 2);
	// 右中传感器状态(bit1)
	state |= (Right1_Irtracking_Get() << 1);
	// 最右侧传感器状态(bit0)
	state |= Right2_Irtracking_Get();
	
	return state;
}

// 向后兼容的左侧传感器读取
uint8_t Left_Irtracking_Get(void)
{
	return Left2_Irtracking_Get();
}

// 向后兼容的右侧传感器读取
uint8_t Right_Irtracking_Get(void)
{
	return Right1_Irtracking_Get();
}
