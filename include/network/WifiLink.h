#ifndef WIFI_LINK_H
#define WIFI_LINK_H

#include "config/BootStrapConfig.h"
#include <WiFi.h>
#include <Preferences.h>
#include "utils/Logger.h"

class WifiLink {
    private:
        WJHelper::Logger logger = WJHelper::Logger(WJHelper::DEBUG);
    public:
        WifiLink() {}
        void init();
        void autoConnect();
};

#endif