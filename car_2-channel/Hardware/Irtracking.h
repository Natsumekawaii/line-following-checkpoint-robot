#ifndef __IRTRACKING_H
#define __IRTRACKING_H

void Irtracking_Init(void);
// 4个传感器的读取函数
uint8_t Left1_Irtracking_Get(void);  // 最左侧传感器
uint8_t Left2_Irtracking_Get(void);  // 左中传感器
uint8_t Right1_Irtracking_Get(void); // 右中传感器
uint8_t Right2_Irtracking_Get(void); // 最右侧传感器
uint8_t Get_Tracking_State(void);    // 获取完整的跟踪状态（4位）

// 向后兼容的函数
uint8_t Left_Irtracking_Get(void);
uint8_t Right_Irtracking_Get(void);

#endif

