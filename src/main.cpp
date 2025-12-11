#include <Arduino.h>
#include "utils/Logger.h"
#include "config/BootStrapConfig.h"
#include "config/PinConfig.h"
#include "network/WifiLink.h"
#include "network/HttpServer.h"
#include "hardware/Tb6612fng.h"

// 初始化串口监视器
WJHelper::Logger logger = WJHelper::Logger(WJHelper::DEBUG);

// 创建WiFi连接对象
WifiLink wifiLink;

// 创建异步Web服务器对象，监听80端口
HttpServer server;
//创建电机
Tb6612fngMOTOR * a_motor = new Tb6612fngMOTOR(MOTOR_A_IN1, MOTOR_A_IN2, MOTOR_A_PWM, 0);
Tb6612fngMOTOR * b_motor = new Tb6612fngMOTOR(MOTOR_B_IN1, MOTOR_B_IN2, MOTOR_B_PWM, 1);
Tb6612fng tb6612fng = Tb6612fng(a_motor, b_motor);



void setup() {
  logger.info("系统初始化中...");

  //wifi初始化及自动连接
  wifiLink.init();
  wifiLink.autoConnect();
  logger.info("Wifi 网络初始化完成...");

  //启动HTTP服务器
  server.start();
  logger.info("HTTP服务器启动完成...");
  
  //设置电机测试 - 运行更长时间以便观察效果
  logger.info("开始电机测试...");
  tb6612fng.test(120, 5);
  
}

void loop() {
  // 主循环可以留空
}