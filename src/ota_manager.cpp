#include "ota_manager.h"
#include "web_server.h"

OTAManager otaManager;

void OTAManager::init() {
    currentStatus = OTA_UPDATE_IDLE;
    statusMessage = "OTA initialized";
    
    // Configure ArduinoOTA
    ArduinoOTA.setPort(OTA_PORT);
    ArduinoOTA.setHostname(FIRMWARE_NAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else {  // U_SPIFFS
            type = "filesystem";
        }
        Serial.println("Start updating " + type);
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    
    ArduinoOTA.begin();
    
    Serial.println("OTA Manager initialized");
    Serial.printf("Current firmware version: %s\n", currentVersion.c_str());
    Serial.printf("OTA enabled on port %d\n", OTA_PORT);
    Serial.printf("OTA hostname: %s\n", FIRMWARE_NAME);
}

void OTAManager::loop() {
    // Handle ArduinoOTA
    ArduinoOTA.handle();
    
    // Check for updates periodically
    if (WiFi.status() == WL_CONNECTED && 
        millis() - lastUpdateCheck > OTA_CHECK_INTERVAL) {
        
        // Log heap memory before OTA check (for memory monitoring)
        Serial.printf("[MEM] Free heap before OTA check: %d bytes\n", ESP.getFreeHeap());
        
        checkForUpdate();
        lastUpdateCheck = millis();
        
        // Log heap memory after OTA check
        Serial.printf("[MEM] Free heap after OTA check: %d bytes\n", ESP.getFreeHeap());
        
        // Force garbage collection if available heap is low (less than 50KB)
        if (ESP.getFreeHeap() < 50000) {
            Serial.println("[MEM] Low memory detected, triggering cleanup");
            // Force any pending operations to complete
            delay(100);
        }
    }
}

void OTAManager::enableWebOTA() {
    // ArduinoOTA provides network OTA functionality
    // Web interface can be added separately if needed
    Serial.println("Network OTA enabled");
    Serial.printf("Use Arduino IDE or PlatformIO to upload over network\n");
    Serial.printf("Hostname: %s\n", FIRMWARE_NAME);
    Serial.printf("Port: %d\n", OTA_PORT);
}

bool OTAManager::checkForUpdate() {
    if (currentStatus != OTA_UPDATE_IDLE) {
        Serial.println("OTA check skipped - already in progress");
        return false;  // Already updating
    }
    
    currentStatus = OTA_UPDATE_CHECKING;
    statusMessage = "Checking for updates...";
    
    // Force flush any pending logs before network operation
    Serial.flush();
    
    Serial.println("=== OTA UPDATE CHECK STARTED ===");
    Serial.printf("Current version: %s\n", currentVersion.c_str());
    Serial.printf("Connecting to: %s\n", OTA_UPDATE_URL);
    
    HTTPClient http;
    http.begin(OTA_UPDATE_URL);
    http.addHeader("User-Agent", "ESP32-GarageDoor-OTA");
    
    // Add GitHub authentication if token is provided
    if (strlen(GITHUB_TOKEN) > 0) {
        String authHeader = "Bearer " + String(GITHUB_TOKEN);
        http.addHeader("Authorization", authHeader.c_str());
        Serial.println("Using GitHub authentication token");
    } else {
        Serial.println("Public repository access (no token)");
    }
    
    Serial.println("Sending HTTP GET request...");
    int httpCode = http.GET();
    
    Serial.printf("=== GitHub API Response: %d ===\n", httpCode);
    Serial.flush();  // Ensure logs are output immediately
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.printf("Received %d bytes of JSON data\n", payload.length());
        Serial.println("=== Parsing JSON Response ===");
        
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (error) {
            Serial.printf("JSON parse error: %s\n", error.c_str());
            statusMessage = "Failed to parse update response";
            currentStatus = OTA_UPDATE_IDLE;
            http.end();
            return false;
        }
        
        if (!doc["tag_name"].isNull()) {
            latestVersion = doc["tag_name"].as<String>();
            Serial.printf("=== Latest version from API: %s ===\n", latestVersion.c_str());
            
            // Remove 'v' prefix if present for comparison
            String compareVersion = latestVersion;
            if (compareVersion.startsWith("v")) {
                compareVersion = compareVersion.substring(1);
            }
            Serial.printf("=== Version Comparison: %s vs %s ===\n", currentVersion.c_str(), compareVersion.c_str());
            
            if (compareVersion != currentVersion) {
                statusMessage = "Update available: " + latestVersion;
                Serial.printf("*** UPDATE AVAILABLE: %s -> %s ***\n", currentVersion.c_str(), latestVersion.c_str());
                Serial.flush();  // Force immediate output
                
                // Look for firmware asset
                JsonArray assets = doc["assets"];
                Serial.printf("Checking %d assets for firmware binary\n", assets.size());
                
                for (JsonVariant asset : assets) {
                    String name = asset["name"];
                    Serial.printf("Found asset: %s\n", name.c_str());
                    
                    if (name.endsWith(".bin") && name.indexOf("firmware") >= 0) {
                        String downloadUrl = asset["browser_download_url"];
                        Serial.printf("*** FIRMWARE FOUND: %s ***\n", name.c_str());
                        Serial.printf("Download URL: %s\n", downloadUrl.c_str());
                        
                        // Auto-update can be enabled here
                        // performUpdate(downloadUrl);
                        
                        currentStatus = OTA_UPDATE_IDLE;
                        http.end();
                        Serial.println("=== OTA CHECK COMPLETE - UPDATE AVAILABLE ===");
                        return true;
                    }
                }
                Serial.println("No .bin firmware file found in release assets");
                statusMessage = "No firmware binary found in release";
            } else {
                statusMessage = "Firmware up to date";
                Serial.println("=== Firmware is up to date ===");
            }
        } else {
            statusMessage = "Invalid response from update server";
            Serial.println("ERROR: No tag_name in API response");
            Serial.printf("Response keys: %s\n", payload.substring(0, 200).c_str());
        }
    } else {
        statusMessage = "Failed to check for updates: " + String(httpCode);
        Serial.printf("*** API request failed with code: %d ***\n", httpCode);
        if (httpCode > 0) {
            String response = http.getString();
            Serial.printf("Error response: %s\n", response.substring(0, 200).c_str());
        }
    }
    
    currentStatus = OTA_UPDATE_IDLE;
    http.end();
    Serial.println("=== OTA CHECK COMPLETE ===");
    Serial.flush();  // Ensure all logs are flushed before continuing
    
    return false;
}

bool OTAManager::performUpdate(String firmwareUrl) {
    if (currentStatus != OTA_UPDATE_IDLE) {
        return false;
    }
    
    currentStatus = OTA_UPDATE_DOWNLOADING;
    statusMessage = "Downloading firmware...";
    
    HTTPClient http;
    http.begin(firmwareUrl);
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        statusMessage = "Download failed: " + String(httpCode);
        currentStatus = OTA_UPDATE_ERROR;
        http.end();
        return false;
    }
    
    int contentLength = http.getSize();
    if (contentLength <= 0) {
        statusMessage = "Invalid firmware size";
        currentStatus = OTA_UPDATE_ERROR;
        http.end();
        return false;
    }
    
    bool canBegin = Update.begin(contentLength);
    if (!canBegin) {
        statusMessage = "Not enough space for update";
        currentStatus = OTA_UPDATE_ERROR;
        http.end();
        return false;
    }
    
    currentStatus = OTA_UPDATE_INSTALLING;
    statusMessage = "Installing firmware...";
    
    WiFiClient* client = http.getStreamPtr();
    size_t written = Update.writeStream(*client);
    
    if (written == contentLength) {
        if (Update.end()) {
            statusMessage = "Update successful! Rebooting...";
            currentStatus = OTA_UPDATE_SUCCESS;
            http.end();
            
            delay(2000);
            ESP.restart();
            return true;
        } else {
            statusMessage = "Update failed: " + String(Update.getError());
        }
    } else {
        statusMessage = "Partial update: " + String(written) + "/" + String(contentLength);
    }
    
    currentStatus = OTA_UPDATE_ERROR;
    http.end();
    return false;
}