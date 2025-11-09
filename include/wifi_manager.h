#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"

// WiFi function declarations
void initWiFi();
void connectToWiFi();
bool connectToWiFi(const String& ssid, const String& password);
void checkWiFiConnection();
bool isWiFiConnected();
String getWiFiStatusString();
String getSecurityType();
void printWiFiInfo();

// NVS WiFi credential management
void saveWiFiCredentials(const String& ssid, const String& password);
bool loadWiFiCredentials(String& ssid, String& password);
void clearWiFiCredentials();

// WiFi status variables
extern bool wifiConnected;
extern unsigned long lastWiFiCheck;
extern int wifiRetryCount;

#endif // WIFI_MANAGER_H