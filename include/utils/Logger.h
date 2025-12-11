#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
// #include <stdarg.h>

namespace WJHelper {
    enum LogLevel {
        DEBUG = 0,
        INFO = 1,
        WARN = 2,
        ERROR = 3
    };

    class Logger
    {
    private:
        int level;
        
        void log(LogLevel msgLevel, const char* format, va_list args) {
            if (msgLevel >= this->level) {
                // 根据日志级别添加前缀
                switch (msgLevel) {
                    case DEBUG:
                        Serial.print("[DEBUG] ");
                        break;
                    case INFO:
                        Serial.print("[INFO] ");
                        break;
                    case WARN:
                        Serial.print("[WARN] ");
                        break;
                    case ERROR:
                        Serial.print("[ERROR] ");
                        break;
                }
                
                // 打印实际的日志内容
                char buffer[256];
                vsnprintf(buffer, sizeof(buffer), format, args);
                Serial.println(buffer);
            }
        }

    public:
        Logger(int level = INFO) {
            if(!Serial){
                Serial.begin(115200);
                Serial.setDebugOutput(true);
            }
            this->level = level;
        }
        
        void setLevel(int level) {
            this->level = level;
        }
        
        void debug(const char* format, ...) {
            va_list args;
            va_start(args, format);
            log(DEBUG, format, args);
            va_end(args);
        }
        
        void info(const char* format, ...) {
            va_list args;
            va_start(args, format);
            log(INFO, format, args);
            va_end(args);
        }
        
        void warn(const char* format, ...) {
            va_list args;
            va_start(args, format);
            log(WARN, format, args);
            va_end(args);
        }
        
        void error(const char* format, ...) {
            va_list args;
            va_start(args, format);
            log(ERROR, format, args);
            va_end(args);
        }
        
        ~Logger() {}

    };
}

#endif