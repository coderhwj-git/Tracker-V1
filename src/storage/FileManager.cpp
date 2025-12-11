#include "storage/FileManager.h"


void FileManager::init() {
    if (!LittleFS.begin(true)) {
        logger.error("初始化文件系统失败");
    } else {
        logger.info("文件系统初始化成功");
    }
}

void FileManager::printFiles() {
    logger.info("文件系统内容:");
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while(file){
        logger.info("FILE: %s",file.name());
        file = root.openNextFile();
    }
}