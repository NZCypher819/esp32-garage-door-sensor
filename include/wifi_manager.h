#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"

// WiFi function declarations
void initWiFi();
void connectToWiFi();
void checkWiFiConnection();
bool isWiFiConnected();
String getWiFiStatusString();
String getSecurityType();
void printWiFiInfo();

// WiFi status variables
extern bool wifiConnected;
extern unsigned long lastWiFiCheck;
extern int wifiRetryCount;

#endif // WIFI_MANAGER_H