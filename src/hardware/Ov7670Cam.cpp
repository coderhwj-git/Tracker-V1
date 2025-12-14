#include "hardware/Ov7670Cam.h"
#include "utils/Logger.h"
#include "config/PinConfig.h"

// 初始化摄像头
bool Ov7670Cam::initCamera() {
    this -> config.ledc_channel = LEDC_CHANNEL_0;
    this -> config.ledc_timer = LEDC_TIMER_0;
    this -> config.pin_d0 = D0_GPIO_NUM;
    this -> config.pin_d1 = D1_GPIO_NUM;
    this -> config.pin_d2 = D2_GPIO_NUM;
    this -> config.pin_d3 = D3_GPIO_NUM;
    this -> config.pin_d4 = D4_GPIO_NUM;
    this -> config.pin_d5 = D5_GPIO_NUM;
    this -> config.pin_d6 = D6_GPIO_NUM;
    this -> config.pin_d7 = D7_GPIO_NUM;
    this -> config.pin_xclk = XCLK_GPIO_NUM;
    this -> config.pin_pclk = PCLK_GPIO_NUM;
    this -> config.pin_vsync = VSYNC_GPIO_NUM;
    this -> config.pin_href = HREF_GPIO_NUM;
    this -> config.pin_sccb_sda = SIOD_GPIO_NUM;
    this -> config.pin_sccb_scl = SIOC_GPIO_NUM;
    this -> config.pin_pwdn = PWDN_GPIO_NUM;
    this -> config.pin_reset = RESET_GPIO_NUM;
    this -> config.xclk_freq_hz = 10000000; // 提高时钟频率至10MHz以获得更好性能
    // OV7670 不支持 JPEG，使用 RGB565
    this -> config.pixel_format = PIXFORMAT_RGB565;  
    
    // 使用更合适的分辨率 (QVGA 320x240)
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;              // 提高JPEG质量以获得更好的图像效果
    config.fb_count = 2;                   // 使用双缓冲提高性能
    logger.info("设置分辨率为320x240");
    
    // 设置摄像头任务参数
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;

    // 初始化摄像头
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        logger.error("摄像头初始化失败，错误代码：0x%x\n",  err);
        return false;
    }
    logger.info("摄像头初始化成功");

    // 获取摄像头传感器并进行基础设置
    sensor_t * s = esp_camera_sensor_get();
    if (s == NULL) {
        logger.error("无法获取摄像头传感器");
        return false;
    }
    
    // 关闭一些不必要的自动处理功能以提高性能和一致性
    s->set_ae_level(s, 0);        // 自动曝光补偿级别设为0
    s->set_aec2(s, false);        // 关闭自动曝光控制
    s->set_awb_gain(s, false);    // 关闭自动白平衡增益
    s->set_agc_gain(s, 0);        // 关闭自动增益控制
    // s->set_saturation(s, 0);      // 饱和度设为0
    s->set_sharpness(s, 0);       // 锐度设为0
    s->set_denoise(s, 1);         // 关闭降噪功能
    s->set_gain_ctrl(s, 0);       // 关闭自动增益控制
    s->set_exposure_ctrl(s, 0);   // 关闭自动曝光控制
    s->set_hmirror(s, 0);         // 关闭水平镜像
    s->set_vflip(s, 0);           // 关闭垂直翻转
    s->set_colorbar(s, 0);        // 关闭彩条测试模式
    s->set_special_effect(s, 0);  // 关闭特殊效果
    s->set_whitebal(s, 0);        // 关闭自动白平衡
    // s->set_contrast(s, 0);        // 对比度设为0
    s->set_brightness(s, 1);      // 亮度设为0
    
    logger.info("摄像头传感器型号: 0x%x\n", s->id.PID);
    
    
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
        vTaskDelay(100 / portTICK_PERIOD_MS);
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
            if (jpg_buf) {
                free(jpg_buf);
            }
            img.fb = NULL;
            img.buf = NULL;
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
            if (jpg_buf) {
                free(jpg_buf);
            }
            img.fb = NULL;
            img.buf = NULL;
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