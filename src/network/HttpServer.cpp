#include "config/BootStrapConfig.h"
#include "network/HttpServer.h"
#include "hardware/Ov7670Cam.h"
#include "hardware/Tb6612fng.h"


// 创建摄像头对象
extern Ov7670Cam * camera;

// 创建Preferences对象用于保存WiFi配置
Preferences preferences;

// 声明外部电机对象
extern Tb6612fng* tb6612fng;


// 自定义视频流响应类
class VideoStreamResponse: public AsyncAbstractResponse {
  private:
    Ov7670Cam* _camera;
  
  public:
    VideoStreamResponse(Ov7670Cam* camera) : _camera(camera) {
      _camera->setStreaming(true);
      _contentType = "multipart/x-mixed-replace; boundary=frame";
      _sendContentLength = false;
      _chunked = true;
    }

    size_t _fillBuffer(uint8_t *buf, size_t maxLen) override {
      if (!_camera->isStreaming()) {
        return 0;
      }

      CamImage image = _camera->videoStream();
      if (!image.buf) {
        // 如果获取图像失败，短暂延迟后重试
        vTaskDelay(5 / portTICK_PERIOD_MS);
        return RESPONSE_TRY_AGAIN;
      }

      // 构造MJPEG帧头
      String header = "\r\n--frame\r\nContent-Type: image/jpeg\r\nContent-Length: ";
      header += String(image.buf_len);
      header += "\r\n\r\n";

      // 检查缓冲区是否有足够空间
      size_t headerLen = header.length();
      if (maxLen < headerLen + image.buf_len) {
        // 释放图像内存
        if (image.fb) {
          esp_camera_fb_return(image.fb);
        } else if (image.buf) {
          free(image.buf);
        }
        return RESPONSE_TRY_AGAIN;
      }

      // 将头部复制到缓冲区
      memcpy(buf, header.c_str(), headerLen);
      
      // 将图像数据复制到缓冲区
      memcpy(buf + headerLen, image.buf, image.buf_len);

      // 释放图像内存
      if (image.fb) {
        esp_camera_fb_return(image.fb);
      } else if (image.buf) {
        free(image.buf);
      }

      // 添加延迟，让其他任务有机会运行，防止看门狗超时
      // 增加延迟时间并主动让出控制权
      vTaskDelay(5 / portTICK_PERIOD_MS);
      //增加电子狗投喂
    
      yield();
  
      
      return headerLen + image.buf_len;
    }

    bool _sourceValid() const override {
      return _camera->isStreaming();
    }

    ~VideoStreamResponse() {
      _camera->setStreaming(false);
    }
};



void HttpServer::init() { 
    //绑定路由处理
    this->bindRuouters();
    // 提供静态文件服务
    server.serveStatic("/", LittleFS, "/");
    
    // 初始化Preferences
    preferences.begin(WIFI_CONFIG_NAME, false);
}

void HttpServer::bindRuouters() { 
    // 路由配置
  // 1. 首页
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/index.html")) {
      request->send(LittleFS, "/index.html", "text/html");
    } else {
      // 如果文件不存在，则发送默认页面
      String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>ESP32 Web Server</title></head><body><h1>ESP32 Web Server</h1><p>开发板IP: ";
      html += WiFi.softAPIP().toString();
      html += "</p><p>文件系统中缺少index.html文件</p></body></html>";
      request->send(200, "text/html", html);
    }
  });

  // 2. favicon.ico 处理
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
    // 发送空的 ICO 响应以避免错误日志
    request->send(204, "image/x-icon", "");
  });

  // 3. 状态信息
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["ap_ip"] = WiFi.softAPIP().toString();
    if (WiFi.status() == WL_CONNECTED) {
      doc["wifi_state"] = "已连接到 " + WiFi.SSID();
      doc["local_ip"] = WiFi.localIP().toString();
    } else {
      doc["wifi_state"] = "未连接";
    }
    
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });

  // 4. 系统信息
  server.on("/system_info", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    
    // 芯片信息
    doc["chip_model"] = ESP.getChipModel();
    doc["cpu_freq"] = ESP.getCpuFreqMHz();
    
    // 内存信息
    doc["heap_size"] = ESP.getHeapSize() / 1024;
    doc["free_heap"] = ESP.getFreeHeap() / 1024;
    doc["min_free_heap"] = ESP.getMinFreeHeap() / 1024;
    
    // Flash信息
    doc["flash_size"] = ESP.getFlashChipSize() / (1024 * 1024); // MB
    
    // PSRAM信息
#ifdef BOARD_HAS_PSRAM
    doc["psram_enabled"] = true;
    doc["psram_size"] = ESP.getPsramSize() / (1024 * 1024); // MB
    doc["free_psram"] = ESP.getFreePsram() / 1024; // KB
#else
    doc["psram_enabled"] = false;
    doc["psram_size"] = 0;
    doc["free_psram"] = 0;
#endif
    
    // 温度信息
    doc["temperature"] = temperatureRead();
    
    // WiFi信息
    if (WiFi.status() == WL_CONNECTED) {
      doc["wifi_signal"] = WiFi.RSSI();
      doc["local_ip"] = WiFi.localIP().toString();
    } else {
      doc["wifi_signal"] = 0;
      doc["local_ip"] = "未连接";
    }
    
    // MAC地址
    doc["mac_address"] = WiFi.macAddress();
    
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });

  // 5. 连接WiFi
  server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request) {},
  NULL,
  [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      // 解析JSON数据
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data);
      if (error) {
          request->send(400, "application/json", "{\"message\":\"JSON解析错误\"}");
          return;
      }
      
      const char* ssid = doc["ssid"];
      const char* password = doc["password"];
      
      Serial.println("尝试连接到WiFi: " + String(ssid));
      
      // 断开可能的现有连接
      WiFi.disconnect();
      delay(100);
      
      // 连接到WiFi
      if (strlen(password) > 0) {
        WiFi.begin(ssid, password);
      } else {
        WiFi.begin(ssid);
      }
      
      // 保存WiFi配置到Preferences
      preferences.putString("ssid", ssid);
      preferences.putString("password", password);
      
      // 等待连接结果（最多10秒），定期让出控制权
      unsigned long startAttemptTime = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
          delay(100);  // 让出控制权，防止看门狗触发
      }
      
      if (WiFi.status() == WL_CONNECTED) {
          Serial.println("成功连接到WiFi网络");
          Serial.println("本地IP地址: " + WiFi.localIP().toString());
          String response = "{\"message\":\"成功连接到 " + String(ssid) + "\", \"ip\":\"" + WiFi.localIP().toString() + "\"}";
          request->send(200, "application/json", response);
      } else {
          Serial.println("连接WiFi失败");
          request->send(200, "application/json", "{\"message\":\"连接失败，请检查WiFi名称和密码\"}");
      }
  });

  // 6. 清除WiFi配置
  server.on("/clear_wifi_config", HTTP_POST, [](AsyncWebServerRequest *request) {
      // 清除保存的WiFi配置
      preferences.remove("ssid");
      preferences.remove("password");
      
      // 断开当前WiFi连接
      WiFi.disconnect();
      
      request->send(200, "application/json", "{\"message\":\"WiFi配置已清除\"}");
  });


  // 7. 拍照
  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest *request) {
    
    CamImage image = camera->capture();
    if (!image.buf) {
        request->send(500, "text/plain", "拍照失败");
        return;
    }
    
    // 发送图像数据
    AsyncWebServerResponse *response = request->beginResponse_P(200, "image/jpeg", image.buf, image.buf_len);
    response->addHeader("Content-Disposition", "inline; filename=capture.jpg");
    request->send(response);
    
    // 释放帧缓冲区
    if (image.fb) {
        esp_camera_fb_return(image.fb);
    } else if (image.buf) {
        free(image.buf);
    }
  });


  // 8. 视频流
  server.on("/video", HTTP_GET, [this](AsyncWebServerRequest *request) {
    VideoStreamResponse *response = new VideoStreamResponse(camera);
    request->send(response);
  });
  
  // 9. 控制电机
  server.on("/control_motor", HTTP_POST, [](AsyncWebServerRequest *request) {},
  NULL,
  [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      // 解析JSON数据
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, data);
      if (error) {
          request->send(400, "application/json", "{\"message\":\"JSON解析错误\"}");
          return;
      }
      
      // 检查tb6612fng是否已初始化
      if (tb6612fng == nullptr) {
          request->send(500, "application/json", "{\"message\":\"电机控制器未初始化\"}");
          return;
      }
      
      const char* motor = doc["motor"];  // "A" 或 "B"
      bool direction = doc["direction"]; // true=正转, false=反转
      int speed = doc["speed"];          // 0-255
      
      if (strcmp(motor, "A") == 0) {
          Tb6612fngMOTOR* motorA = tb6612fng->getAMotor();
          motorA->setDirection(direction);
          motorA->setSpeed(speed);
      } else if (strcmp(motor, "B") == 0) {
          Tb6612fngMOTOR* motorB = tb6612fng->getBMotor();
          motorB->setDirection(direction);
          motorB->setSpeed(speed);
      } else {
          request->send(400, "application/json", "{\"message\":\"无效的电机标识\"}");
          return;
      }
      
      request->send(200, "application/json", "{\"message\":\"电机控制命令已发送\"}");
  });
  
  // 10. 停止所有电机
  server.on("/stop_motors", HTTP_POST, [](AsyncWebServerRequest *request) {
      // 检查tb6612fng是否已初始化
      if (tb6612fng == nullptr) {
          request->send(500, "application/json", "{\"message\":\"电机控制器未初始化\"}");
          return;
      }
      
      tb6612fng->getAMotor()->stop();
      tb6612fng->getBMotor()->stop();
      request->send(200, "application/json", "{\"message\":\"所有电机已停止\"}");
  });
}