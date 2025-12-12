#ifndef PinConfig_h
#define PinConfig_h
//开发板ESP32-S3 N16R8，已启用PSRAM

//=============================================OV7670摄像头引脚配置=============================================
//重要：摄像头设置占用了timeer0、channel0 通道

// OV7670 引脚定义 (18脚模块)
#define PWDN_GPIO_NUM    1
#define RESET_GPIO_NUM   2

//时钟信号的引脚定义
#define SIOD_GPIO_NUM    9
#define HREF_GPIO_NUM    10
#define XCLK_GPIO_NUM    11
#define SIOC_GPIO_NUM    12
#define VSYNC_GPIO_NUM   13
#define PCLK_GPIO_NUM    14


//数据传输引脚
#define D6_GPIO_NUM      42  
#define D4_GPIO_NUM      41  
#define D2_GPIO_NUM      40  
#define D0_GPIO_NUM      39  
#define D7_GPIO_NUM      47  
#define D5_GPIO_NUM      21  
#define D3_GPIO_NUM      20  
#define D1_GPIO_NUM      19  



//=============================================TB6612FNG电机驱动器引脚配置=============================================
// TB6612FNG 引脚定义
// Motor A
#define MOTOR_A_IN1   6
#define MOTOR_A_IN2   5
#define MOTOR_A_PWM   4
#define MOTOR_A_PWM_CHANEL   1

// Motor B
#define MOTOR_B_IN1   15
#define MOTOR_B_IN2   16
#define MOTOR_B_PWM   17
#define MOTOR_B_PWM_CHANEL   2

// Standby pin
#define MOTOR_STBY    7

#endif