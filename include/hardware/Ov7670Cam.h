#ifndef HARDWARE_OV7670CAM_H
#define HARDWARE_OV7670CAM_H
#include "utils/Logger.h"
#include "esp_camera.h"


class CamImage {
    public:
        uint8_t * buf = NULL;
        size_t buf_len = 0;
        camera_fb_t * fb;

        CamImage(){
        }
        CamImage(uint8_t * buf,size_t buf_len,camera_fb_t * fb){
            this ->buf = buf;
            this ->buf_len = buf_len;
            this ->fb = fb;
        }
};

class Ov7670Cam {
    private:
        //摄像头对象
        camera_config_t config;
        bool is_streaming = false;
        WJHelper::Logger logger = WJHelper::Logger(WJHelper::INFO);

        //摄像头初始化
        void init();
    public:
        Ov7670Cam() {}
        //初始化摄像头
        bool initCamera();
        //拍照
        CamImage capture();
        //视频流
        CamImage videoStream();
        //设置流媒体状态
        void setStreaming(bool streaming);
        //获取流媒体状态
        bool isStreaming();
};

#endif  