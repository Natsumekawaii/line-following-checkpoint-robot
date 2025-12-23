#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "robot.h"
#include "Irtracking.h"
#include "Servo.h"

// 1. 定义转向状态枚举（增强可读性，替代魔法数字）
typedef enum {
    TURN_NONE = 0,  // 无转向/正常前进
    TURN_LEFT  = 1, // 左转
    TURN_RIGHT = 2  // 右转
} Turn_Dir;

static Turn_Dir last_turn_dir = TURN_NONE; // 记录上一次转向方向（静态变量，仅本文件有效）
static uint8_t offline_cnt = 0;            // 脱线计数（消抖+判定脱线时长）
static uint8_t count = 0;            // 脱线计数（消抖+判定脱线时长）

// 2. 单个红外传感器消抖函数（连续3次读取状态一致才判定有效，避免误判）
uint8_t Irtracking_Sensor_Get_Stable(uint8_t sensor_id)
{
    uint8_t cnt = 0;
    uint8_t stable_val = 0;
    // 连续读取3次，间隔5ms（消抖核心）
    for(uint8_t i=0; i<3; i++)
    {
        // 根据sensor_id选择读取不同传感器
        switch(sensor_id)
        {
            case 0: stable_val = Left1_Irtracking_Get();  break;  // 最左传感器
            case 1: stable_val = Left2_Irtracking_Get();  break;  // 左中传感器
            case 2: stable_val = Right1_Irtracking_Get(); break;  // 右中传感器
            case 3: stable_val = Right2_Irtracking_Get(); break;  // 最右传感器
            default: stable_val = 0; break;
        }
        
        // 再次读取并比较
        uint8_t current_val = 0;
        switch(sensor_id)
        {
            case 0: current_val = Left1_Irtracking_Get();  break;
            case 1: current_val = Left2_Irtracking_Get();  break;
            case 2: current_val = Right1_Irtracking_Get(); break;
            case 3: current_val = Right2_Irtracking_Get(); break;
            default: current_val = 0; break;
        }
        
        if(stable_val == current_val)
        {
            cnt++;
        }
        Delay_ms(5); // 短延时避抖
    }
    // 3次中有2次以上一致则返回该值
    return (cnt >= 2) ? stable_val : 0;
}

// 3. 获取4个传感器的稳定状态（返回4位状态码）
uint8_t Irtracking_Get_Stable_State(void)
{
    uint8_t state = 0;
    state |= (Irtracking_Sensor_Get_Stable(0) << 3); // 最左传感器
    state |= (Irtracking_Sensor_Get_Stable(1) << 2); // 左中传感器
    state |= (Irtracking_Sensor_Get_Stable(2) << 1); // 右中传感器
    state |= (Irtracking_Sensor_Get_Stable(3) << 0); // 最右传感器
    return state;
}

// 4. 向后兼容的消抖函数（保持原有接口不变）
uint8_t Irtracking_Get_Stable(uint8_t is_left)
{
    if(is_left) return Irtracking_Sensor_Get_Stable(1);  // 左中传感器（对应原左传感器）
    else return Irtracking_Sensor_Get_Stable(2);        // 右中传感器（对应原右传感器）
}

int main(void)
{
    uint8_t tracking_state; // 存储4个传感器的状态码
    
    Servo_Init();
    robot_Init();        // 机器人初始化
    Irtracking_Init();   // 红外循迹传感器初始化
	
    while (1)
    {
        // 读取消抖后的4个传感器状态码（0=未压线，1=压线）
        tracking_state = Irtracking_Get_Stable_State();

        // 场景1：所有传感器均压线（终点/触发点）
        if(tracking_state == 0xFF) // 1111
        {
			count++;
			if(count == 2)
			{
            makerobo_brake(500);		
            Servo_SetAngle(90);  // 占空比7.5%，90度
            Delay_ms(1000);
            Servo_SetAngle(180); // 占空比12.5%，180度（修正原注释错误）
            Delay_ms(1000);
            makerobo_brake(2000);	
			  makerobo_run(70,0);   // 前进
			  Delay_ms(2000);
            
          count = 0;
			}
			last_turn_dir = TURN_NONE; // 触发刷卡后重置转向记录
			offline_cnt = 0;           // 重置脱线计数
			
        }
        // 场景2：所有传感器均未压线（脱线）
        else if(tracking_state == 0x00) // 0000
        {
            offline_cnt++; // 脱线计数+1（避免瞬间脱线误触发）
            // 脱线超过20ms（4次循环，每次5ms消抖）才判定为真脱线
            if(offline_cnt >= 4)
            {
                // 脱线后沿上一次转向方向继续寻线
                if(last_turn_dir == TURN_LEFT)
                {
                    makerobo_Left(60,0); // 左转寻线（速度略降，避免过冲）
                }
                else if(last_turn_dir == TURN_RIGHT)
                {
                    makerobo_Right(60,0); // 右转寻线（速度略降）
                }
                // 若之前无转向记录（初始状态/正常前进脱线），默认小幅左转寻线
                else
                {
                    makerobo_Left(50,0); // 低速左转，降低失控风险
                }
            }
            // 瞬间脱线（未达阈值），暂保持前进
            else
            {
                makerobo_run(70,0); // 正常前进
            }
        }
        // 场景3：中间两个传感器压线（正常前进）
        else if(tracking_state == 0x0C) // 1100 (bit3=1, bit2=1) 对应最左和左中传感器，表明小车在中心线
        {
            makerobo_run(70,0); // 正常前进
            last_turn_dir = TURN_NONE; // 重置转向记录
            offline_cnt = 0;           // 重置脱线计数
        }
        // 场景4：只有左中传感器压线（小幅左转修正）
        else if(tracking_state == 0x04) // 0100 (bit2=1) 对应左中传感器，小车略微偏右
        {
            makerobo_Left(65,0);    // 小幅向左转
            last_turn_dir = TURN_LEFT; // 记录上一次转向为左转
            offline_cnt = 0;           // 重新检测到线，重置脱线计数
        }
        // 场景5：只有右中传感器压线（小幅右转修正）
        else if(tracking_state == 0x02) // 0010 (bit1=1) 对应右中传感器，小车略微偏左
        {
            makerobo_Right(65,0); // 小幅向右转
            last_turn_dir = TURN_RIGHT; // 记录上一次转向为右转
            offline_cnt = 0;            // 重新检测到线，重置脱线计数
        }
        // 场景6：只有左中+右中传感器压线（扩展场景，保持前进）
        else if(tracking_state == 0x06) // 0110 (bit2=1, bit1=1) 对应左中和右中传感器
        {
            makerobo_run(70,0); // 正常前进
            last_turn_dir = TURN_NONE; // 重置转向记录
            offline_cnt = 0;           // 重置脱线计数
        }
        // 场景7：右中+最右传感器压线（大幅右转修正）
        else if(tracking_state == 0x03) // 0011 (bit1=1, bit0=1) 对应右中和最右传感器
        {
            makerobo_Right(75,0); // 大幅向右转
            last_turn_dir = TURN_RIGHT; // 记录上一次转向为右转
            offline_cnt = 0;            // 重新检测到线，重置脱线计数
        }
        // 场景8：只有最左传感器压线（急左转）
        else if(tracking_state == 0x08) // 1000 (bit3=1) 对应最左传感器
        {
            makerobo_Left(80,0);    // 急向左转
            last_turn_dir = TURN_LEFT; // 记录上一次转向为左转
            offline_cnt = 0;           // 重新检测到线，重置脱线计数
        }
        // 场景9：只有最右传感器压线（急右转）
        else if(tracking_state == 0x01) // 0001 (bit0=1) 对应最右传感器
        {
            makerobo_Right(80,0); // 急向右转
            last_turn_dir = TURN_RIGHT; // 记录上一次转向为右转
            offline_cnt = 0;            // 重新检测到线，重置脱线计数
        }
    }
}
