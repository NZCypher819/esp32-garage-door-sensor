#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include "ota_config.h"

class OTAManager {
private:
    unsigned long lastUpdateCheck = 0;
    OTAUpdateStatus currentStatus = OTA_UPDATE_IDLE;
    String statusMessage = "";
    String currentVersion = FIRMWARE_VERSION;
    String latestVersion = "";
    
public:
    void init();
    void loop();
    bool checkForUpdate();
    bool performUpdate(String firmwareUrl);
    void enableWebOTA();
    
    // Status getters
    OTAUpdateStatus getStatus() { return currentStatus; }
    String getStatusMessage() { return statusMessage; }
    String getCurrentVersion() { return currentVersion; }
    String getLatestVersion() { return latestVersion; }
    
    // Manual update trigger
    void triggerUpdateCheck() { lastUpdateCheck = 0; }
};

extern OTAManager otaManager;

#endif