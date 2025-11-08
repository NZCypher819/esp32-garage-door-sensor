#include "wifi_manager.h"

#ifdef ENABLE_WIFI

bool wifiConnected = false;
unsigned long lastWiFiCheck = 0;
int wifiRetryCount = 0;

void initWiFi() {
    Serial.println("\n=== WiFi Initialization ===");
    Serial.printf("SSID: %s\n", WIFI_SSID);
    Serial.println("Connecting to WiFi...");
    
    // Set WiFi mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    // Setup status LED if defined
    #ifdef WIFI_STATUS_LED
    pinMode(WIFI_STATUS_LED, OUTPUT);
    digitalWrite(WIFI_STATUS_LED, WIFI_LED_OFF);
    #endif
    
    connectToWiFi();
}

void connectToWiFi() {
    wifiRetryCount = 0;
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    Serial.print("Connecting");
    unsigned long startTime = millis();
    
    while (WiFi.status() != WL_CONNECTED && 
           millis() - startTime < WIFI_CONNECT_TIMEOUT &&
           wifiRetryCount < WIFI_MAX_RETRIES) {
        
        delay(WIFI_RETRY_DELAY);
        Serial.print(".");
        wifiRetryCount++;
        
        // Blink status LED while connecting
        #ifdef WIFI_STATUS_LED
        digitalWrite(WIFI_STATUS_LED, (millis() / 250) % 2);
        #endif
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("\n✅ WiFi Connected Successfully!");
        Serial.printf("📶 IP Address: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("📡 Signal Strength: %d dBm\n", WiFi.RSSI());
        Serial.printf("🔒 Security: %s\n", getSecurityType().c_str());
        
        #ifdef WIFI_STATUS_LED
        digitalWrite(WIFI_STATUS_LED, WIFI_LED_ON);  // Solid on when connected
        #endif
    } else {
        wifiConnected = false;
        Serial.println("\n❌ WiFi Connection Failed!");
        Serial.printf("Status: %s\n", getWiFiStatusString().c_str());
        Serial.println("Will retry connection...");
        
        #ifdef WIFI_STATUS_LED
        digitalWrite(WIFI_STATUS_LED, WIFI_LED_OFF);
        #endif
    }
}

void checkWiFiConnection() {
    unsigned long currentTime = millis();
    
    // Check WiFi status every 30 seconds
    if (currentTime - lastWiFiCheck >= 30000) {
        lastWiFiCheck = currentTime;
        
        if (WiFi.status() != WL_CONNECTED && wifiConnected) {
            Serial.println("⚠️ WiFi connection lost! Attempting to reconnect...");
            wifiConnected = false;
            connectToWiFi();
        } else if (WiFi.status() == WL_CONNECTED && !wifiConnected) {
            wifiConnected = true;
            Serial.println("✅ WiFi reconnected!");
        }
    }
}

bool isWiFiConnected() {
    return wifiConnected && (WiFi.status() == WL_CONNECTED);
}

String getWiFiStatusString() {
    switch (WiFi.status()) {
        case WL_CONNECTED: return "Connected";
        case WL_NO_SSID_AVAIL: return "SSID not found";
        case WL_CONNECT_FAILED: return "Connection failed";
        case WL_DISCONNECTED: return "Disconnected";
        case WL_CONNECTION_LOST: return "Connection lost";
        case WL_NO_SHIELD: return "No WiFi shield";
        case WL_SCAN_COMPLETED: return "Scan completed";
        default: return "Unknown status";
    }
}

String getSecurityType() {
    switch (WiFi.encryptionType(0)) {
        case WIFI_AUTH_OPEN: return "Open";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA";
        case WIFI_AUTH_WPA2_PSK: return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2 Enterprise";
        default: return "Unknown";
    }
}

void printWiFiInfo() {
    if (isWiFiConnected()) {
        Serial.println("\n=== WiFi Information ===");
        Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
        Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
        Serial.printf("Subnet: %s\n", WiFi.subnetMask().toString().c_str());
        Serial.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str());
        Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
        Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
        Serial.printf("Channel: %d\n", WiFi.channel());
        Serial.println("========================");
    }
}

#else
// WiFi disabled stubs
void initWiFi() { /* WiFi disabled */ }
void connectToWiFi() { /* WiFi disabled */ }
void checkWiFiConnection() { /* WiFi disabled */ }
bool isWiFiConnected() { return false; }
String getWiFiStatusString() { return "WiFi disabled"; }
String getSecurityType() { return "N/A"; }
void printWiFiInfo() { /* WiFi disabled */ }
#endif