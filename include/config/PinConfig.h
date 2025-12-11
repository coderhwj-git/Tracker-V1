#ifndef PinConfig_h
#define PinConfig_h
//开发板ESP32-S3 N16R8，已启用PSRAM

// OV7670 引脚定义 (18脚模块)
#define PWDN_GPIO_NUM    1
#define RESET_GPIO_NUM   2

//控制引脚
// #define XCLK_GPIO_NUM    9
// #define PCLK_GPIO_NUM    10
// #define HREF_GPIO_NUM    11
// #define VSYNC_GPIO_NUM   12
// #define SIOD_GPIO_NUM    13
// #define SIOC_GPIO_NUM    14

#define SIOD_GPIO_NUM    9
#define HREF_GPIO_NUM    10
#define XCLK_GPIO_NUM    11
#define SIOC_GPIO_NUM    12
#define VSYNC_GPIO_NUM   13
#define PCLK_GPIO_NUM    14




//数据传输引脚
// #define Y9_GPIO_NUM      42  // D7
// #define Y8_GPIO_NUM      41  // D6
// #define Y7_GPIO_NUM      40  // D5
// #define Y6_GPIO_NUM      39  // D4
// #define Y5_GPIO_NUM      47  // D3
// #define Y4_GPIO_NUM      21  // D2
// #define Y3_GPIO_NUM      20  // D1
// #define Y2_GPIO_NUM      19  // D0

#define Y8_GPIO_NUM      42  // D6
#define Y6_GPIO_NUM      41  // D4
#define Y4_GPIO_NUM      40  // D2
#define Y2_GPIO_NUM      39  // D0
#define Y9_GPIO_NUM      47  // D7
#define Y7_GPIO_NUM      21  // D5
#define Y5_GPIO_NUM      20  // D3
#define Y3_GPIO_NUM      19  // D1


// TB6612FNG 引脚定义
// Motor A
#define MOTOR_A_IN1   6
#define MOTOR_A_IN2   5
#define MOTOR_A_PWM   4

// Motor B
#define MOTOR_B_IN1   15
#define MOTOR_B_IN2   16
#define MOTOR_B_PWM   17

// Standby pin
#define MOTOR_STBY    7

#endif