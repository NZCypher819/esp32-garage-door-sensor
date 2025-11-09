#include "wifi_manager.h"
#include <Preferences.h>

#ifdef ENABLE_WIFI

Preferences wifiPrefs;
bool wifiConnected = false;
unsigned long lastWiFiCheck = 0;
int wifiRetryCount = 0;

void initWiFi() {
    Serial.println("\n=== WiFi Initialization with NVS Support ===");
    
    // Initialize NVS preferences
    wifiPrefs.begin("wifi", false);
    
    // Try to load saved credentials first
    String savedSSID = wifiPrefs.getString("ssid", "");
    String savedPassword = wifiPrefs.getString("password", "");
    
    if (savedSSID.length() > 0) {
        Serial.printf("Found saved WiFi credentials for: %s\n", savedSSID.c_str());
        if (connectToWiFi(savedSSID, savedPassword)) {
            return; // Successfully connected with saved credentials
        }
        Serial.println("Saved credentials failed, trying hardcoded...");
    }
    
    // Try hardcoded credentials from secrets.h
    Serial.printf("Trying hardcoded SSID: %s\n", WIFI_SSID);
    if (connectToWiFi(WIFI_SSID, WIFI_PASSWORD)) {
        // Save working credentials to NVS for future OTA updates
        wifiPrefs.putString("ssid", WIFI_SSID);
        wifiPrefs.putString("password", WIFI_PASSWORD);
        Serial.println("✅ WiFi credentials saved to NVS for future OTA updates");
        return;
    }
    
    Serial.println("❌ No WiFi connection possible with saved or hardcoded credentials");
}

bool connectToWiFi(const String& ssid, const String& password) {
    // Set WiFi mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    // Setup status LED if defined
    #ifdef WIFI_STATUS_LED
    pinMode(WIFI_STATUS_LED, OUTPUT);
    digitalWrite(WIFI_STATUS_LED, WIFI_LED_OFF);
    #endif
    
    wifiRetryCount = 0;
    WiFi.begin(ssid.c_str(), password.c_str());
    
    Serial.printf("Connecting to %s", ssid.c_str());
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
        Serial.println(" Connected!");
        Serial.printf("📶 IP address: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("📡 Signal strength: %d dBm\n", WiFi.RSSI());
        
        #ifdef WIFI_STATUS_LED
        digitalWrite(WIFI_STATUS_LED, WIFI_LED_ON);
        #endif
        
        return true;
    } else {
        wifiConnected = false;
        Serial.println(" Failed!");
        Serial.printf("Connection status: %s\n", getWiFiStatusString().c_str());
        
        #ifdef WIFI_STATUS_LED
        digitalWrite(WIFI_STATUS_LED, WIFI_LED_OFF);
        #endif
        
        return false;
    }
}

void connectToWiFi() {
    String ssid = wifiPrefs.getString("ssid", WIFI_SSID);
    String password = wifiPrefs.getString("password", WIFI_PASSWORD);
    connectToWiFi(ssid, password);
}

void checkWiFiConnection() {
    if (millis() - lastWiFiCheck > 30000) { // Check every 30 seconds
        lastWiFiCheck = millis();
        
        if (WiFi.status() != WL_CONNECTED) {
            wifiConnected = false;
            Serial.println("WiFi disconnected, attempting to reconnect...");
            connectToWiFi();
        } else if (!wifiConnected) {
            wifiConnected = true;
            Serial.println("WiFi reconnected successfully");
        }
    }
}

bool isWiFiConnected() {
    return wifiConnected && (WiFi.status() == WL_CONNECTED);
}

String getWiFiStatusString() {
    switch (WiFi.status()) {
        case WL_IDLE_STATUS: return "Idle";
        case WL_NO_SSID_AVAIL: return "SSID not available";
        case WL_SCAN_COMPLETED: return "Scan completed";
        case WL_CONNECTED: return "Connected";
        case WL_CONNECT_FAILED: return "Connection failed";
        case WL_CONNECTION_LOST: return "Connection lost";
        case WL_DISCONNECTED: return "Disconnected";
        default: return "Unknown status";
    }
}

String getSecurityType() {
    if (WiFi.status() != WL_CONNECTED) return "Not connected";
    
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
    } else {
        Serial.println("WiFi not connected - cannot display info");
    }
}

// NVS management functions for WiFi credentials
void saveWiFiCredentials(const String& ssid, const String& password) {
    wifiPrefs.putString("ssid", ssid);
    wifiPrefs.putString("password", password);
    Serial.printf("📱 WiFi credentials saved to NVS: %s\n", ssid.c_str());
}

bool loadWiFiCredentials(String& ssid, String& password) {
    ssid = wifiPrefs.getString("ssid", "");
    password = wifiPrefs.getString("password", "");
    return (ssid.length() > 0);
}

void clearWiFiCredentials() {
    wifiPrefs.clear();
    Serial.println("🗑️ WiFi credentials cleared from NVS");
}

#else
// WiFi disabled stubs
void initWiFi() { /* WiFi disabled */ }
void connectToWiFi() { /* WiFi disabled */ }
bool connectToWiFi(const String& ssid, const String& password) { return false; }
void checkWiFiConnection() { /* WiFi disabled */ }
bool isWiFiConnected() { return false; }
String getWiFiStatusString() { return "WiFi disabled"; }
String getSecurityType() { return "N/A"; }
void printWiFiInfo() { /* WiFi disabled */ }
void saveWiFiCredentials(const String& ssid, const String& password) { /* WiFi disabled */ }
bool loadWiFiCredentials(String& ssid, String& password) { return false; }
void clearWiFiCredentials() { /* WiFi disabled */ }

#endif