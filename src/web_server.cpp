#include "web_server.h"
#include "sensors.h" 
#include "wifi_manager.h"
#include "ota_manager.h"

extern SensorData currentSensorData;

#ifdef ENABLE_WIFI
#include <ArduinoJson.h>
#include <WebServer.h>

WebServer server(WEB_SERVER_PORT);
bool webServerActive = false;

void initWebServer() {
    if (!isWiFiConnected()) {
        Serial.println("Cannot start web server - WiFi not connected");
        return;
    }

        // Main page
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", getMainPageHTML());
    });

    // Refresh endpoint - redirects to main page for live data
    server.on("/refresh", HTTP_GET, []() {
        server.sendHeader("Location", "/");
        server.send(302, "text/plain", "Redirecting...");
    });

    // API endpoint for status JSON
    server.on("/api/status", HTTP_GET, []() {
        server.send(200, "application/json", getStatusJSON());
    });

    // OTA endpoints
    server.on("/api/ota/status", HTTP_GET, []() {
        server.send(200, "application/json", getOTAStatusJSON());
    });

    server.on("/api/ota/check", HTTP_POST, []() {
        Serial.println("=== MANUAL OTA CHECK REQUEST ===");
        otaManager.triggerUpdateCheck();
        delay(100); // Give it a moment to start checking
        server.send(200, "application/json", "{\"status\":\"checking\",\"message\":\"Update check triggered\"}");
    });

    server.on("/api/ota/install", HTTP_POST, []() {
        Serial.println("=== OTA INSTALL REQUEST RECEIVED ===");
        
        // Send immediate response
        server.send(200, "application/json", "{\"status\":\"installing\",\"message\":\"Update started, device will restart\"}");
        
        // Start OTA update after response is sent
        delay(100);  // Brief delay to ensure response is sent
        Serial.println("=== STARTING OTA UPDATE ===");
        otaManager.installLatestRelease();
    });

    server.begin();
    webServerActive = true;
    Serial.println("Web server started successfully");
    Serial.print("Access dashboard at: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
}

void handleWebServer() {
    if (webServerActive && isWiFiConnected()) {
        server.handleClient();
    }
}

void stopWebServer() {
    if (webServerActive) {
        server.stop();
        webServerActive = false;
        Serial.println("Web server stopped");
    }
}

bool isWebServerActive() {
    return webServerActive;
}

// Simple log function stub (lightweight version)
void addLogEntry(String message, String level) {
    // In lightweight version, just print to serial
    Serial.print("[");
    Serial.print(level);
    Serial.print("] ");
    Serial.println(message);
}

String getStatusJSON() {
    JsonDocument doc;
    
    // Basic device info
    doc["device"]["name"] = "ESP32 Garage Door Sensor";
    doc["device"]["version"] = FIRMWARE_VERSION;
    doc["device"]["uptime"] = millis();
    doc["device"]["free_heap"] = ESP.getFreeHeap();
    
    // WiFi status
    doc["wifi"]["connected"] = WiFi.isConnected();
    doc["wifi"]["ip"] = WiFi.localIP().toString();
    doc["wifi"]["rssi"] = WiFi.RSSI();
    
    // Sensor status - simple and clear
    doc["sensors"]["beam"]["status"] = currentSensorData.beamBroken ? "BLOCKED" : "CLEAR";
    doc["sensors"]["beam"]["pin"] = E3JK_RR11_PIN;
    doc["sensors"]["led"]["status"] = digitalRead(LED_INDICATOR_PIN) ? "ON" : "OFF";
    doc["sensors"]["led"]["pin"] = LED_INDICATOR_PIN;
    
    // DHT22 environmental data
    #ifdef ENABLE_DHT22
    if (currentSensorData.dataValid) {
        doc["sensors"]["temperature"]["value"] = currentSensorData.temperature;
        doc["sensors"]["temperature"]["unit"] = "°C";
        doc["sensors"]["humidity"]["value"] = currentSensorData.humidity;
        doc["sensors"]["humidity"]["unit"] = "%";
    } else {
        doc["sensors"]["temperature"]["value"] = "N/A";
        doc["sensors"]["temperature"]["unit"] = "°C";
        doc["sensors"]["humidity"]["value"] = "N/A";
        doc["sensors"]["humidity"]["unit"] = "%";
    }
    #endif
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

String getOTAStatusJSON() {
    JsonDocument doc;
    
    doc["current_version"] = FIRMWARE_VERSION;
    doc["latest_version"] = otaManager.getLatestVersion();
    doc["update_available"] = otaManager.isUpdateAvailable();
    doc["last_check"] = "now";
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

String getMainPageHTML() {
    // Get current sensor data to embed in HTML
    String beamStatus = currentSensorData.beamBroken ? "BLOCKED" : "CLEAR";
    String beamClass = currentSensorData.beamBroken ? "blocked" : "clear";
    String ledStatus = digitalRead(LED_INDICATOR_PIN) ? "ON" : "OFF";
    String ledClass = digitalRead(LED_INDICATOR_PIN) ? "on" : "";
    String uptime = String(millis() / 1000) + "s";
    String memory = String(ESP.getFreeHeap() / 1024) + " KB";
    // Get latest version info for display
    #ifdef ENABLE_WIFI
    String latestVersion = otaManager.getLatestVersion();
    bool updateAvailable = otaManager.isUpdateAvailable();
    String updateStatus = updateAvailable ? "Update available!" : "Up to date";
    String updateClass = updateAvailable ? "update-available" : "update-current";
    #else
    String latestVersion = "Unknown";
    bool updateAvailable = false;
    String updateStatus = "WiFi disabled";
    String updateClass = "";
    #endif
    
    // DHT22 sensor data
    #ifdef ENABLE_DHT22
    String temperature = currentSensorData.dataValid ? String(currentSensorData.temperature, 1) + "°C" : "N/A";
    String humidity = currentSensorData.dataValid ? String(currentSensorData.humidity, 1) + "%" : "N/A";
    String tempClass = currentSensorData.dataValid ? "temp-normal" : "";
    String humidityClass = currentSensorData.dataValid ? "humidity-normal" : "";
    #else
    String temperature = "Disabled";
    String humidity = "Disabled";
    String tempClass = "";
    String humidityClass = "";
    #endif
    
    return R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Garage Door Sensor</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 0 auto; padding: 20px; background: #f5f5f5; }
        .container { background: white; border-radius: 8px; padding: 20px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); margin-bottom: 20px; }
        h1 { color: #333; text-align: center; margin-bottom: 30px; }
        .status-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; margin-bottom: 30px; }
        .status-card { background: #f8f9fa; border-radius: 6px; padding: 15px; text-align: center; border-left: 4px solid #007bff; }
        .status-card.blocked { border-left-color: #dc3545; background: #f8d7da; }
        .status-card.clear { border-left-color: #28a745; background: #d4edda; }
        .status-card.on { border-left-color: #ffc107; background: #fff3cd; }
        .status-card.temp-normal { border-left-color: #17a2b8; background: #d1ecf1; }
        .status-card.humidity-normal { border-left-color: #6f42c1; background: #e2d9f3; }
        .update-available { color: #856404; background: #fff3cd; padding: 5px 10px; border-radius: 4px; font-weight: bold; }
        .update-current { color: #155724; background: #d4edda; padding: 5px 10px; border-radius: 4px; font-weight: bold; }
        .status-value { font-size: 24px; font-weight: bold; margin-bottom: 5px; }
        .status-label { color: #666; font-size: 14px; }
        .button { background: #007bff; color: white; border: none; padding: 12px 24px; border-radius: 4px; cursor: pointer; font-size: 16px; margin: 5px; text-decoration: none; display: inline-block; }
        .button:hover { background: #0056b3; }
        .button.success { background: #28a745; }
        .update-section { text-align: center; }
        .refresh-note { color: #666; font-size: 12px; margin-top: 10px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 Garage Door Sensor</h1>
        <div class="status-grid">
            <div class="status-card )" + beamClass + R"(">
                <div class="status-value">)" + beamStatus + R"(</div>
                <div class="status-label">Beam Sensor</div>
            </div>
            <div class="status-card )" + ledClass + R"(">
                <div class="status-value">)" + ledStatus + R"(</div>
                <div class="status-label">Status LED</div>
            </div>
            <div class="status-card )" + tempClass + R"(">
                <div class="status-value">)" + temperature + R"(</div>
                <div class="status-label">Temperature</div>
            </div>
            <div class="status-card )" + humidityClass + R"(">
                <div class="status-value">)" + humidity + R"(</div>
                <div class="status-label">Humidity</div>
            </div>
            <div class="status-card">
                <div class="status-value">)" + uptime + R"(</div>
                <div class="status-label">Uptime</div>
            </div>
            <div class="status-card">
                <div class="status-value">)" + memory + R"(</div>
                <div class="status-label">Free Memory</div>
            </div>
        </div>
        <div class="update-section">
            <h3>Firmware Updates</h3>
            <p>Current Version: )" + String(FIRMWARE_VERSION) + R"(</p>
            <p>Latest Available: )" + latestVersion + R"( <span class=")" + updateClass + R"(">)" + updateStatus + R"(</span></p>
            <div style="margin: 20px 0; padding: 20px; background: #e8f5e8; border-radius: 8px; border: 2px solid #28a745;">
                <h4 style="margin: 0 0 10px 0; color: #155724;">🚀 Install Latest Update</h4>
                <p style="margin: 0 0 15px 0; color: #155724;">Click the button below to install the latest firmware version.</p>
                <form method="POST" action="/api/ota/install" style="margin: 0;">
                    <button type="submit" class="button success" style="font-size: 18px; padding: 15px 30px; background: #28a745; border: none; color: white; border-radius: 6px; cursor: pointer; font-weight: bold;">
                        ⬇️ Install Update Now
                    </button>
                </form>
                <p style="margin-top: 10px; font-size: 12px; color: #666;">Device will restart automatically after update</p>
            </div>
            <p><a href="/refresh" class="button">🔄 Refresh Status</a></p>
            <p>
                <form method="POST" action="/api/ota/check" style="display: inline; margin: 5px;">
                    <button type="submit" class="button" style="background: #17a2b8;">🔍 Check for Updates</button>
                </form>
            </p>
            <p><a href="/api/status" class="button">📊 View Status JSON</a></p>
            <p><a href="/api/ota/status" class="button">🔍 Check Updates JSON</a></p>
            <div class="refresh-note">Click "Refresh Status" to update sensor readings</div>
        </div>
    </div>
</body>
</html>)";
}

#endif // ENABLE_WIFI
