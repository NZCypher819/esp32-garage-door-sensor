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
        Serial.println("");
        Serial.print("[MEMORY] Free heap before OTA check: ");
        Serial.print(ESP.getFreeHeap());
        Serial.println(" bytes");
        
        checkForUpdate();
        lastUpdateCheck = millis();
        
        // Log heap memory after OTA check
        Serial.print("[MEMORY] Free heap after OTA check: ");
        Serial.print(ESP.getFreeHeap());
        Serial.println(" bytes");
        Serial.flush();
        
        // Force garbage collection if available heap is low (less than 50KB)
        if (ESP.getFreeHeap() < 50000) {
            Serial.println("[MEMORY] Low memory detected, triggering cleanup");
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
    
    Serial.println("");
    Serial.println("========================================");
    Serial.println("      OTA UPDATE CHECK STARTED");
    Serial.println("========================================");
    Serial.print("Current version: ");
    Serial.println(currentVersion);
    Serial.print("Connecting to: ");
    Serial.println(OTA_UPDATE_URL);
    Serial.flush();
    delay(100);  // Allow serial output to complete
    
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
    Serial.flush();
    int httpCode = http.GET();
    
    Serial.println("========================================");
    Serial.print("GitHub API Response Code: ");
    Serial.println(httpCode);
    Serial.println("========================================");
    Serial.flush();
    delay(50);
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.print("Received JSON payload: ");
        Serial.print(payload.length());
        Serial.println(" bytes");
        Serial.println("----------------------------------------");
        Serial.println("Parsing JSON Response...");
        Serial.flush();
        delay(50);
        
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (error) {
            Serial.print("!!! JSON PARSE ERROR: ");
            Serial.println(error.c_str());
            statusMessage = "Failed to parse update response";
            currentStatus = OTA_UPDATE_IDLE;
            http.end();
            return false;
        }
        
        Serial.println("JSON parsed successfully!");
        
        if (!doc["tag_name"].isNull()) {
            latestVersion = doc["tag_name"].as<String>();
            Serial.println("========================================");
            Serial.print(">>> LATEST VERSION FROM API: ");
            Serial.println(latestVersion);
            Serial.println("========================================");
            Serial.flush();
            delay(100);
            
            // Remove 'v' prefix if present for comparison
            String compareVersion = latestVersion;
            if (compareVersion.startsWith("v")) {
                compareVersion = compareVersion.substring(1);
                Serial.print("Stripped 'v' prefix: ");
                Serial.println(compareVersion);
            }
            
            Serial.println("----------------------------------------");
            Serial.print("COMPARING: '");
            Serial.print(currentVersion);
            Serial.print("' vs '");
            Serial.print(compareVersion);
            Serial.println("'");
            Serial.println("----------------------------------------");
            Serial.flush();
            delay(100);
            
            if (compareVersion != currentVersion) {
                statusMessage = "Update available: " + latestVersion;
                Serial.println("");
                Serial.println("****************************************");
                Serial.println("***      UPDATE AVAILABLE!!!        ***");
                Serial.print("***  ");
                Serial.print(currentVersion);
                Serial.print(" -> ");
                Serial.print(latestVersion);
                Serial.println("  ***");
                Serial.println("****************************************");
                Serial.flush();
                delay(200);
                
                // Look for firmware asset
                JsonArray assets = doc["assets"];
                Serial.print("Checking ");
                Serial.print(assets.size());
                Serial.println(" assets for firmware binary");
                
                for (JsonVariant asset : assets) {
                    String name = asset["name"];
                    Serial.print("Asset found: ");
                    Serial.println(name);
                    
                    if (name.endsWith(".bin") && name.indexOf("firmware") >= 0) {
                        String downloadUrl = asset["browser_download_url"];
                        Serial.println("****************************************");
                        Serial.print("*** FIRMWARE BINARY FOUND: ");
                        Serial.println(name);
                        Serial.print("*** Download URL: ");
                        Serial.println(downloadUrl);
                        Serial.println("****************************************");
                        
                        // Auto-update can be enabled here
                        // performUpdate(downloadUrl);
                        
                        currentStatus = OTA_UPDATE_IDLE;
                        http.end();
                        Serial.println("=== OTA CHECK COMPLETE - UPDATE AVAILABLE ===");
                        return true;
                    }
                }
                Serial.println("!!! No .bin firmware file found in release assets !!!");
                statusMessage = "No firmware binary found in release";
            } else {
                statusMessage = "Firmware up to date";
                Serial.println("========================================");
                Serial.println("    Firmware is UP TO DATE");
                Serial.println("========================================");
            }
        } else {
            statusMessage = "Invalid response from update server";
            Serial.println("!!! ERROR: No tag_name in API response !!!");
            // Show first 200 chars of response for debugging
            Serial.print("Response preview: ");
            Serial.println(payload.substring(0, 200));
        }
    } else {
        statusMessage = "Failed to check for updates: " + String(httpCode);
        Serial.println("****************************************");
        Serial.print("*** API REQUEST FAILED: Code ");
        Serial.println(httpCode);
        Serial.println("****************************************");
        if (httpCode > 0) {
            String response = http.getString();
            Serial.print("Error response (first 200 chars): ");
            Serial.println(response.substring(0, 200));
        }
        Serial.flush();
    }
    
    currentStatus = OTA_UPDATE_IDLE;
    http.end();
    Serial.println("========================================");
    Serial.println("      OTA CHECK COMPLETE");
    Serial.println("========================================");
    Serial.flush();
    delay(100);  // Ensure all logs are flushed
    
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