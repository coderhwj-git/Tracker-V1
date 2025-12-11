#ifndef FileManager_h
#define FileManager_h
#include <LittleFS.h>
#include <Arduino.h>
#include "utils/Logger.h"

class FileManager {
    private:
        WJHelper::Logger logger = WJHelper::Logger(WJHelper::DEBUG);
    public:
        FileManager() {}
        void init();
        void printFiles();
};


#endif