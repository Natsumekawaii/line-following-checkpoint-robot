#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "robot.h"
#include "Irtracking.h"
#include "Servo.h"
#include <stdint.h>                      // 确保uint8_t等类型可用

// 1. 定义转向状态枚举（增加幅度区分，适配4路循迹）
typedef enum {
    TURN_NONE    = 0,  // 无转向/直行
    TURN_LEFT_S  = 1,  // 小幅左转（内侧压线）
    TURN_LEFT_B  = 2,  // 大幅左转（外侧压线）
    TURN_RIGHT_S = 3,  // 小幅右转（内侧压线）
    TURN_RIGHT_B = 4   // 大幅右转（外侧压线）
} Turn_Dir;

// 2. 全局状态变量（静态隔离，仅本文件可见）
static Turn_Dir last_turn_dir = TURN_NONE; // 记录上一次转向方向
static uint8_t offline_cnt   = 0;          // 脱线计数（消抖+判定脱线时长）
static uint8_t trigger_cnt   = 0;          // 触发点计数（替代原模糊的count）

// 3. 传感器类型枚举（替代魔法数字，适配4路）
typedef enum {
    IR_LEFT1  = 0,  // 左内侧
    IR_LEFT2  = 1,  // 左外侧
    IR_RIGHT1 = 2,  // 右内侧
    IR_RIGHT2 = 3   // 右外侧
} IR_Channel;

// 4. 通用消抖函数（修复原逻辑错误，支持4路传感器）
// 功能：连续读取3次传感器值，间隔5ms，3次中有≥2次一致则返回稳定值
uint8_t Irtracking_Get_Stable(IR_Channel ch)
{
    uint8_t read_val;    // 单次读取值
    uint8_t stable_val = 0; // 稳定值缓存
    uint8_t stable_cnt = 0; // 稳定次数计数

    for(uint8_t i = 0; i < 3; i++)
    {
        // 按通道读取对应传感器值（核心：每次循环只读一次，避免原逻辑错误）
        switch(ch)
        {
            case IR_LEFT1:  read_val = Left1_Irtracking_Get();  break;
            case IR_LEFT2:  read_val = Left2_Irtracking_Get();  break;
            case IR_RIGHT1: read_val = Right1_Irtracking_Get(); break;
            case IR_RIGHT2: read_val = Right2_Irtracking_Get(); break;
            default:        read_val = 0; break;
        }

        // 第一次读取时初始化稳定值
        if(i == 0)
        {
            stable_val = read_val;
        }
        // 后续读取与稳定值对比，一致则计数+1
        else if(read_val == stable_val)
        {
            stable_cnt++;
        }

        Delay_ms(5); // 5ms消抖延时，过滤高频噪声
    }

    // 3次中有≥2次一致则返回稳定值，否则返回0（表示状态不稳定）
    return (stable_cnt >= 2) ? stable_val : 0;
}

int main(void)
{
    // 4路传感器稳定值（0=未压线，1=压线）
    uint8_t left1_val, left2_val, right1_val, right2_val;

    // 外设初始化（顺序：舵机→电机→传感器）
    Servo_Init();
    robot_Init();
    Irtracking_Init();

    while (1)
    {
		/****************************************************************
		* **电机硬件排查测试**
		* 
		* 1. 下面的代码会屏蔽所有复杂的循迹逻辑，只让小车以80的速度前进。
		* 2. 请将此代码烧录到您的主控板中。
		* 3. 如果烧录后轮子转动了，说明您的硬件（电机、驱动板、接线）是好的。
		* 4. 如果烧录后轮子【仍然不转】，则几乎可以肯定是硬件问题。请重点检查：
		*    - **电机驱动板的独立供电**是否接好？（最重要！）
		*    - 主控板和电机驱动板是否**共地**？
		*    - 电机驱动板上的**使能跳线帽**是否插着？
		*
		* 解决硬件问题后，您可以撤销此处的修改，恢复原来的循迹代码。
		****************************************************************/
		makerobo_run(80, 0); // 以80的速度前进
		Delay_ms(100);       // 短暂延时
    }
}
