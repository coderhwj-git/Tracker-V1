#include "hardware/Ov7670Cam.h"
#include "utils/Logger.h"
#include "config/PinConfig.h"

// 初始化摄像头
bool Ov7670Cam::initCamera() {
    this -> config.ledc_channel = LEDC_CHANNEL_0;
    this -> config.ledc_timer = LEDC_TIMER_0;
    this -> config.pin_d0 = Y2_GPIO_NUM;
    this -> config.pin_d1 = Y3_GPIO_NUM;
    this -> config.pin_d2 = Y4_GPIO_NUM;
    this -> config.pin_d3 = Y5_GPIO_NUM;
    this -> config.pin_d4 = Y6_GPIO_NUM;
    this -> config.pin_d5 = Y7_GPIO_NUM;
    this -> config.pin_d6 = Y8_GPIO_NUM;
    this -> config.pin_d7 = Y9_GPIO_NUM;
    this -> config.pin_xclk = XCLK_GPIO_NUM;
    this -> config.pin_pclk = PCLK_GPIO_NUM;
    this -> config.pin_vsync = VSYNC_GPIO_NUM;
    this -> config.pin_href = HREF_GPIO_NUM;
    this -> config.pin_sccb_sda = SIOD_GPIO_NUM;
    this -> config.pin_sccb_scl = SIOC_GPIO_NUM;
    this -> config.pin_pwdn = PWDN_GPIO_NUM;
    this -> config.pin_reset = RESET_GPIO_NUM;
    this -> config.xclk_freq_hz = 10000000; // 降低时钟频率以减少功耗和发热
    // OV7670 不支持 JPEG，使用 RGB565
    this -> config.pixel_format = PIXFORMAT_RGB565;  
    
    // 检查PSRAM是否可用，如果可用则使用更高分辨率和更多缓冲区
    if(psramFound()){
        config.frame_size = FRAMESIZE_QVGA;   // 使用QVGA分辨率 (320x240)
        config.jpeg_quality = 16;             // JPEG质量适中
        config.fb_count = 2;                  // 使用双缓冲
        logger.info("使用PSRAM，设置分辨率为QVGA");
    } else {
        config.frame_size = FRAMESIZE_QVGA;   // 使用QVGA分辨率 (320x240)
        config.jpeg_quality = 12;             // JPEG质量适中
        config.fb_count = 1;                  // 使用单缓冲
        logger.info("未使用PSRAM，设置分辨率为QVGA");
    }
    
    // 设置摄像头任务参数以避免堆栈溢出
    config.grab_mode = CAMERA_GRAB_LATEST; // 改为获取最新帧，而不是等待空帧
    config.fb_location = CAMERA_FB_IN_PSRAM;

    // 初始化摄像头
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        logger.error("摄像头初始化失败，错误代码：0x%x\n",  err);
        return false;
    }
    
    // 获取摄像头传感器并进行基础设置
    sensor_t * s = esp_camera_sensor_get();
    if (s == NULL) {
        logger.error("无法获取摄像头传感器");
        return false;
    }
    
    logger.info("摄像头传感器型号: 0x%x\n", s->id.PID);
    s->set_framesize(s, config.frame_size);
    
    // 优化摄像头参数以减少内存使用
    s->set_brightness(s, 0);     // 亮度
    s->set_contrast(s, 0);       // 对比度
    s->set_saturation(s, 0);     // 饱和度
    s->set_special_effect(s, 0); // 关闭特效
    s->set_whitebal(s, 1);       // 白平衡
    s->set_awb_gain(s, 1);       // 自动白平衡增益
    s->set_wb_mode(s, 0);        // 白平衡模式
    s->set_exposure_ctrl(s, 1);  // 自动曝光
    s->set_aec2(s, 0);           // 关闭自动曝光值
    s->set_ae_level(s, 0);       // AE等级
    s->set_aec_value(s, 300);    // AE值
    s->set_gain_ctrl(s, 1);      // 自动增益
    s->set_agc_gain(s, 0);       // AGC增益
    s->set_gainceiling(s, (gainceiling_t)0); // 增益上限
    s->set_bpc(s, 1);            // 黑点校正
    s->set_wpc(s, 1);            // 白点校正
    s->set_raw_gma(s, 1);        // Gamma校正
    s->set_lenc(s, 1);           // 镜头校正
    s->set_hmirror(s, 0);        // 水平镜像
    s->set_vflip(s, 0);          // 垂直翻转
    s->set_dcw(s, 1);            // 降采样
    s->set_colorbar(s, 0);       // 彩条
    
    logger.info("摄像头初始化成功");
    return true;
}

void Ov7670Cam::setStreaming(bool streaming) {
    this->is_streaming = streaming;
}

bool Ov7670Cam::isStreaming() {
    return this->is_streaming;
}


CamImage Ov7670Cam::capture() {
    
    logger.debug("开始抓取图片...");

    camera_fb_t * fb = NULL;
    CamImage img;

    // 如果正在进行视频流，则等待一小段时间让其释放摄像头资源
    if(this->is_streaming) {
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    
    // 丢弃几帧旧图像，确保获取最新图像
    for(int i = 0; i < 2; i++) {  // 增加丢弃帧数以提高图像质量
        camera_fb_t* fb_old = esp_camera_fb_get();
        if (fb_old) {
            esp_camera_fb_return(fb_old);
        }
        vTaskDelay(5 / portTICK_PERIOD_MS); // 减少延迟时间
    }

    // 获取一帧图像用于拍照
    fb = esp_camera_fb_get();
    img.fb = fb;

    if (!fb) {
        logger.error("获取图像失败");
        return img;
    }

    if(fb->format != PIXFORMAT_JPEG){
        // 对于非JPEG格式，需要进行转换
        logger.debug("图片格式不是JPEG，将转换为JPEG");

       
        uint8_t * jpg_buf = NULL;
        size_t jpg_buf_len = 0;
        
        // 将RGB565转换为JPEG，使用更高的质量以减少失真
        bool jpeg_converted = fmt2jpg(fb->buf, fb->len, fb->width, fb->height, fb->format, config.jpeg_quality, &jpg_buf, &jpg_buf_len);
        
        if (!jpeg_converted) {
            logger.error("JPEG压缩失败");
            esp_camera_fb_return(fb);
            free(jpg_buf);
            img.fb = NULL;
            return img;
        }
        
        // 成功转换后才释放原始帧缓冲区
        esp_camera_fb_return(fb);
        img.fb = NULL;
        
        img.buf_len = jpg_buf_len;
        img.buf = jpg_buf;
    } else {
        // 已经是JPEG格式
        img.buf_len = fb->len;
        img.buf = fb->buf;
    }

    // 返回捕获到的帧
    return img;
}

CamImage Ov7670Cam::videoStream() {
    logger.debug("开始视频流");
    
    CamImage img;
    camera_fb_t * fb = NULL;
    
    // 获取一帧图像用于视频流
    fb = esp_camera_fb_get();
    img.fb = fb;

    if (!fb) {
        logger.error("获取图像失败");
        return img;
    }

    if(fb->format != PIXFORMAT_JPEG){
        // 对于非JPEG格式，需要进行转换
        logger.debug("图片格式不是JPEG，将转换为JPEG");

        uint8_t * jpg_buf = NULL;
        size_t jpg_buf_len = 0;
        
        // 将RGB565转换为JPEG，使用适当的质量平衡大小和质量
        bool jpeg_converted = fmt2jpg(fb->buf, fb->len, fb->width, fb->height, fb->format, config.jpeg_quality, &jpg_buf, &jpg_buf_len);
        
        if (!jpeg_converted) {
            logger.error("JPEG压缩失败");
            esp_camera_fb_return(fb);
            free(jpg_buf);
            img.fb = NULL;
            return img;
        }
        
        // 成功转换后才释放原始帧缓冲区
        esp_camera_fb_return(fb);
        img.fb = NULL;
        
        img.buf_len = jpg_buf_len;
        img.buf = jpg_buf;
    } else {
        // 已经是JPEG格式
        img.buf_len = fb->len;
        img.buf = fb->buf;
    }

    // 返回捕获到的帧
    return img;
}