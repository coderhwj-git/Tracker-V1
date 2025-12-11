#include "hardware/Tb6612fng.h"

// 设置电机方向
void Tb6612fngMOTOR::setDirection(bool forward) {
    is_forward = forward;
    if (is_running) {
        // 如果电机正在运行，更新方向
        if (forward) {
            digitalWrite(in1_pin, HIGH);
            digitalWrite(in2_pin, LOW);
        } else {
            digitalWrite(in1_pin, LOW);
            digitalWrite(in2_pin, HIGH);
        }
    }
}

// 停止电机
void Tb6612fngMOTOR::stop() {
    digitalWrite(in1_pin, LOW);
    digitalWrite(in2_pin, LOW);
    ledcWrite(pwm_channel, 0);
    is_running = false;
    current_speed = 0;
}

int Tb6612fngMOTOR::speedSetMapping(int speed){
    return speed;
}

int Tb6612fngMOTOR::speedGetMapping(int speed){
    return speed;
}

// 设置电机速度（0-255）
void Tb6612fngMOTOR::writeSpeed(int speed) {
    if (speed < 0) speed = 0;
    if (speed > 255) speed = 255;
    
    current_speed = speed;
    
    if (speed > 0) {
        // 设置方向
        if (is_forward) {
            digitalWrite(in1_pin, HIGH);
            digitalWrite(in2_pin, LOW);
        } else {
            digitalWrite(in1_pin, LOW);
            digitalWrite(in2_pin, HIGH);
        }
        // 设置PWM速度
        ledcWrite(pwm_channel, speed);
        is_running = true;
    } else {
        stop();
    }

}

void Tb6612fng::test(int speed,int seconds){
    // 测试电机A
    logger.info("启动电机A，正转，速度%d，持续%d秒",speed,seconds);
    Tb6612fngMOTOR* motorA = getAMotor();
    motorA->setDirection(true);
    motorA->setSpeed(speed);
    delay(5000);
    motorA->stop();
    logger.info("电机A测试结束");
    
    // 测试电机Bs
    logger.info("启动电机B，正转，速度%d，持续%d秒",speed,seconds);
    Tb6612fngMOTOR* motorB = getBMotor();
    motorB->setDirection(true); 
    motorB->setSpeed(speed);      
    delay(5000);                
    motorB->stop();
    logger.info("电机B测试结束");
    
    // 反向测试电机A
    logger.info("启动电机A，反转，速度%d，持续%d秒",speed,seconds);
    motorA->setDirection(false);
    motorA->setSpeed(speed);
    delay(5000);
    motorA->stop();
    logger.info("电机A反向测试结束");
    
    // 反向测试电机B
    logger.info("启动电机B，反转，速度%d，持续%d秒",speed,seconds);
    motorB->setDirection(false);
    motorB->setSpeed(speed);
    delay(5000);
    motorB->stop();
    logger.info("电机B反向测试结束");

    logger.info("系统初始化完成");
}

