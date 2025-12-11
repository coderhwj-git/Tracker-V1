#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <FS.h>
#include <LittleFS.h>
#include "utils/Logger.h"
#include "hardware/Ov7670Cam.h"

// 创建异步Web服务器对象，监听80端口    
#define SERVER_PORT 80

// 自定义视频流响应类声明
class VideoStreamResponse;

class HttpServer {
    private:
        AsyncWebServer server{SERVER_PORT};
        WJHelper::Logger logger = WJHelper::Logger(WJHelper::DEBUG);
    public:
        HttpServer() {}
        //指定根目录
        void init();
        //绑定路由处理
        void bindRuouters();
        void start() {
            this->init();
            server.begin();
            logger.info("服务器启动成功!!!");
        }
};

// 声明全局Preferences对象
extern Preferences preferences;

#endif // HTTP_SERVER_H