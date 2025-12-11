#include "network/WifiLink.h"


// 初始化WiFi链接
void WifiLink::init() {
    // 开启Wifi的AP模式
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    logger.info("AP IP地址: %s",WiFi.softAPIP().toString().c_str());
    
    // 设置为AP+STA模式
    WiFi.mode(WIFI_AP_STA);
}


// 自动连接到之前保存的WiFi网络
void WifiLink::autoConnect() {
    Preferences preferences;
    // 初始化Preferences
    preferences.begin(WIFI_CONFIG_NAME, true);
    
    // 从Preferences读取保存的WiFi配置
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    
    if (ssid.length() > 0) {
        logger.info("尝试自动连接到WiFi: %s", ssid.c_str());
        
        // 断开可能的现有连接
        WiFi.disconnect();
        delay(100);
        
        // 连接到WiFi
        if (password.length() > 0) {
        WiFi.begin(ssid.c_str(), password.c_str());
        } else {
        WiFi.begin(ssid.c_str());
        }
        
        // 等待连接结果（最多10秒），定期让出控制权
        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            delay(100);  // 让出控制权，防止看门狗触发
        }
        
        if (WiFi.status() == WL_CONNECTED) {
        logger.info("自动连接成功");
        logger.info("本地IP地址: %s", WiFi.localIP().toString().c_str());
        } else {
        logger.warn("自动连接失败");
        }
    } else {
        logger.info("没有保存的WiFi配置");
    }
    
    // 关闭Preferences只读访问
    preferences.end();
}