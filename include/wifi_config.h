#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// WiFi Configuration - Use environment variables for security
#ifdef WIFI_SSID_ENV
  #define WIFI_SSID WIFI_SSID_ENV
#endif

#ifdef WIFI_PASSWORD_ENV
  #define WIFI_PASSWORD WIFI_PASSWORD_ENV
#endif

// Fallback values if not defined in secrets.h or environment
#ifndef WIFI_SSID
  #define WIFI_SSID "YOUR_WIFI_SSID"  // Set in secrets.h
#endif

#ifndef WIFI_PASSWORD
  #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"  // Set in secrets.h
#endif

// WiFi Settings
#define WIFI_CONNECT_TIMEOUT 10000    // 10 seconds timeout
#define WIFI_RETRY_DELAY 1000         // 1 second between retries
#define WIFI_MAX_RETRIES 5            // Maximum connection attempts

// WiFi Status LED (optional)
#define WIFI_STATUS_LED 48            // Built-in LED for ESP32-S3
#define WIFI_LED_ON HIGH
#define WIFI_LED_OFF LOW

#endif // WIFI_CONFIG_H