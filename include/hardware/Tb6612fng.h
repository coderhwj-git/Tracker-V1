#ifndef TB6612FNG_H
#define TB6612FNG_H

#include "utils/Logger.h"
#include "config/PinConfig.h"

// TB6612FNG 电机驱动器类
// 用于控制单个电机驱动器
// 包含以下功能：
// 1. 设置电机方向
// 2. 设置电机速度
// 3. 停止电机
// 4. 获取电机运行状态
// 5. 获取电机方向
// 6. 获取当前速度
// 7. 构造函数初始化引脚
// 8. 日志记录功能
class Tb6612fngMOTOR {
    private:
        //电机驱动器对象
        uint8_t in1_pin;
        uint8_t in2_pin;
        uint8_t pwm_pin;
        uint8_t pwm_channel;
        bool is_forward = true;
        int current_speed = 0;
        bool is_running = false;
        WJHelper::Logger logger = WJHelper::Logger(WJHelper::INFO);

    public:
        //电机驱动器初始化
        Tb6612fngMOTOR(uint8_t in1_pin, uint8_t in2_pin, uint8_t pwm_pin,uint8_t pwmChannel)
        : in1_pin(in1_pin), in2_pin(in2_pin), pwm_pin(pwm_pin), pwm_channel(pwmChannel) {
            pinMode(in1_pin, OUTPUT);
            pinMode(in2_pin, OUTPUT);
            pinMode(pwm_pin, OUTPUT);

            // 配置PWM
            ledcSetup(pwmChannel, 20000, 8);  // 20kHz, 8-bit
            ledcAttachPin(pwm_pin, pwmChannel);

            stop(); // 初始化时停止电机
            logger.info("TB6612FNG initialized on IN1:%d IN2:%d PWM:%d", in1_pin, in2_pin, pwm_pin);
        }
        
        //设置电机方向
        void setDirection(bool forward) ;
        
        //停止电机
        void stop() ;
        
        //获取电机运行状态
        bool isRunning() {
            return is_running;
        }
        
        //获取电机方向
        bool isForward() {
            return is_forward;
        }
        
        //获取当前速度
        int getSpeed() {
            return speedGetMapping(current_speed);
        }

        // 速度映射函数
        int speedSetMapping(int speed);

        // 速度映射函数
        int speedGetMapping(int speed);

        //真正设置对应分辨率数值速度
        void writeSpeed(int speed);

        //设置电机速度（0-255）
        void setSpeed(int speed){
            int targetSpeed = this -> speedSetMapping(speed);
            writeSpeed(targetSpeed);
        }


};

// TB6612FNG 创建一个类，用于控制两个电机驱动器，完成以下功能：
// 1. 创建两个电机驱动器对象
// 2. 设置电机驱动器的方向
// 3. 停止电机驱动器
// 4. 获取电机驱动器的运行状态
// 5. 获取电机驱动器的方向
// 6. 获取电机驱动器的当前速度
// 7. 设置电机驱动器的速度（0-255）
class Tb6612fng {
    private:
        Tb6612fngMOTOR * a_motor;
        Tb6612fngMOTOR * b_motor;
        WJHelper::Logger logger = WJHelper::Logger(WJHelper::INFO);
    public:
        Tb6612fng(Tb6612fngMOTOR * a_motor, Tb6612fngMOTOR * b_motor)
        : a_motor(a_motor), b_motor(b_motor) {
            // 初始化STBY引脚
            pinMode(MOTOR_STBY, OUTPUT);
            digitalWrite(MOTOR_STBY, HIGH); // 默认启用电机驱动
        }

        Tb6612fngMOTOR * getAMotor() {
            return a_motor;
        }

        Tb6612fngMOTOR * getBMotor() {
            return b_motor;
        }
        
        // 全局使能/禁用电机驱动
        void enable() {
            digitalWrite(MOTOR_STBY, HIGH);
        }
        
        void disable() {
            digitalWrite(MOTOR_STBY, LOW);
        }

        // 电机测试函数
        void test(int speed,int seconds);
};


#endif // TB6612FNG_H