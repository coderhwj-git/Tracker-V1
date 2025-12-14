#include <Arduino.h>
#include "utils/Logger.h"
#include "config/BootStrapConfig.h"
#include "config/PinConfig.h"
#include "storage/FileManager.h"
#include "network/WifiLink.h"
#include "network/HttpServer.h"
#include "hardware/Tb6612fng.h"
#include "hardware/Ov7670Cam.h"

// 初始化串口监视器
WJHelper::Logger logger = WJHelper::Logger(WJHelper::DEBUG);

// 创建文件管理对象
FileManager fileManager;

// 声明WiFi连接对象指针
WifiLink * wifiLink = nullptr;

// 声明电机对象指针
Tb6612fngMOTOR * a_motor = nullptr;
Tb6612fngMOTOR * b_motor = nullptr;
Tb6612fng * tb6612fng = nullptr;

// 创建摄像头对象
Ov7670Cam * camera;

// 声明HttpServer指针
HttpServer * server = nullptr;

void setup() {
  logger.info("系统初始化中...");
  fileManager.init();

  //wifi初始化及自动连接
  wifiLink = new WifiLink();
  wifiLink->init();
  wifiLink->autoConnect();
  logger.info("Wifi 网络初始化完成...");

  // 初始化电机驱动器
  a_motor = new Tb6612fngMOTOR(MOTOR_A_IN1, MOTOR_A_IN2, MOTOR_A_PWM, MOTOR_A_PWM_CHANEL);
  b_motor = new Tb6612fngMOTOR(MOTOR_B_IN1, MOTOR_B_IN2, MOTOR_B_PWM, MOTOR_B_PWM_CHANEL);
  tb6612fng = new Tb6612fng(a_motor, b_motor);

  //设置电机测试 - 运行更长时间以便观察效果
  logger.info("开始电机测试...");
  // tb6612fng->test(120, 5);
  delay(1000 * 5);

  camera = new Ov7670Cam();
  if (!camera->initCamera()) {  
    logger.error("摄像头初始化失败");
  } else {
    logger.info("摄像头初始化成功");
  }
  
  //启动HTTP服务器（在电机对象创建后再初始化）
  server = new HttpServer();
  server->start();
  logger.info("HTTP服务器启动完成...");
}

void loop() {
  // 主循环可以留空
}