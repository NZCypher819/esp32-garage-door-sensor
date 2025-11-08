#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include "config.h"

#ifdef ENABLE_WIFI
#include <WebServer.h>
#include <vector>

// Web server functions
void initWebServer();
void handleWebServer();
void addLogEntry(String message, String level = "INFO");
String getStatusJSON();
String getLogsJSON();
String getSystemInfoJSON();
String getOTAStatusJSON();
String getOTAInfoJSON();
String getMainPageHTML();
String getCSS();
String getJavaScript();

// Log management
struct LogEntry {
    String timestamp;
    String level;     // INFO, WARN, ERROR
    String message;
};

// Web server variables
extern WebServer server;
extern bool webServerActive;
extern std::vector<LogEntry> logEntries;

// Constants
#define MAX_LOG_ENTRIES 100
#define WEB_SERVER_PORT 80

#else
// WiFi disabled stubs
void initWebServer();
void handleWebServer();
void addLogEntry(String message, String level = "INFO");
#endif

#endif // WEB_SERVER_H