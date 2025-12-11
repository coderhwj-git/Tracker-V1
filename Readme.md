# ESP32 摄像头监控项目

这是一个基于 ESP32 的摄像头监控系统，能够提供 Web 界面访问、实时视频流传输和拍照功能。

## 项目使用的库及作用

### 1. ESP32核心库
- **WiFi.h** - 用于WiFi连接管理，包括创建热点、连接网络等功能
- **Preferences.h** - 用于持久化存储配置信息，如WiFi账号密码
- **FS.h** 和 **LittleFS.h** - 用于文件系统操作，存储网页文件等

### 2. 摄像头相关库
- **esp_camera.h** - ESP32摄像头驱动库，用于控制OV7670摄像头模块，实现拍照和视频流功能

### 3. Web服务器相关库
- **ESPAsyncWebServer** - 异步Web服务器库，用于处理HTTP请求，提供网页服务
- **AsyncTCP** - ESPAsyncWebServer的依赖库，提供异步TCP连接支持

### 4. 数据处理库
- **ArduinoJson** - 用于JSON数据的序列化和反序列化，处理API接口的JSON数据

### 5. 自定义模块
- **Logger.h** - 自定义的日志记录模块，用于输出不同级别的日志信息
- **HttpServer.h** - 自定义的HTTP服务器封装，集成各种路由处理
- **Ov7670Cam.h** - 自定义的摄像头控制模块，封装了摄像头初始化和图像捕获功能
- **VideoStreamResponse.h** - 自定义的视频流响应模块，专门处理视频流传输