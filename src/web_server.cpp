#include "web_server.h"
#include "sensors.h"
#include "wifi_manager.h"

// Include missing extern declaration
extern SensorData currentSensorData;

#ifdef ENABLE_WIFI
#include <ArduinoJson.h>
#include <WebServer.h>
#include <vector>

WebServer server(WEB_SERVER_PORT);
bool webServerActive = false;
std::vector<LogEntry> logEntries;

void initWebServer() {
    if (!isWiFiConnected()) {
        Serial.println("❌ Cannot start web server - WiFi not connected");
        return;
    }

    // Serve main dashboard page
    server.on("/", HTTP_GET, []() {
        String html = getMainPageHTML();
        server.send(200, "text/html", html);
    });

    // API endpoints
    server.on("/api/status", HTTP_GET, []() {
        server.send(200, "application/json", getStatusJSON());
    });

    server.on("/api/logs", HTTP_GET, []() {
        server.send(200, "application/json", getLogsJSON());
    });

    server.on("/api/system", HTTP_GET, []() {
        server.send(200, "application/json", getSystemInfoJSON());
    });

    // Clear logs endpoint
    server.on("/api/clear-logs", HTTP_POST, []() {
        logEntries.clear();
        addLogEntry("Logs cleared by user", "INFO");
        server.send(200, "application/json", "{\"status\":\"success\"}");
    });

    // CSS styles
    server.on("/styles.css", HTTP_GET, []() {
        server.send(200, "text/css", getCSS());
    });

    // JavaScript
    server.on("/script.js", HTTP_GET, []() {
        server.send(200, "application/javascript", getJavaScript());
    });

    // Start server
    server.begin();
    webServerActive = true;
    
    String serverURL = "http://" + WiFi.localIP().toString();
    Serial.println("🌐 Web Server Started!");
    Serial.printf("📱 Dashboard URL: %s\n", serverURL.c_str());
    addLogEntry("Web server started at " + serverURL, "INFO");
}

void handleWebServer() {
    if (webServerActive) {
        server.handleClient();
    }
}

void addLogEntry(String message, String level) {
    LogEntry entry;
    entry.timestamp = String(millis());
    entry.level = level;
    entry.message = message;
    
    logEntries.push_back(entry);
    
    // Keep only last MAX_LOG_ENTRIES
    if (logEntries.size() > MAX_LOG_ENTRIES) {
        logEntries.erase(logEntries.begin());
    }
    
    // Also print to serial
    Serial.printf("[%s] %s: %s\n", entry.timestamp.c_str(), level.c_str(), message.c_str());
}

String getStatusJSON() {
    JsonDocument doc;
    
    doc["timestamp"] = millis();
    doc["wifi_connected"] = isWiFiConnected();
    doc["wifi_ip"] = WiFi.localIP().toString();
    doc["wifi_rssi"] = WiFi.RSSI();
    
    #ifdef ENABLE_E3JK_RR11
    doc["beam_broken"] = currentSensorData.beamBroken;
    doc["led_status"] = digitalRead(LED_INDICATOR_PIN);
    doc["last_state_change"] = currentSensorData.lastStateChangeTime;
    #else
    doc["beam_broken"] = false;
    doc["led_status"] = false;
    doc["last_state_change"] = 0;
    #endif
    
    doc["uptime"] = millis();
    doc["free_heap"] = ESP.getFreeHeap();
    doc["chip_temp"] = temperatureRead();
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

String getLogsJSON() {
    JsonDocument doc;
    JsonArray logs = doc["logs"].to<JsonArray>();
    
    // Add recent log entries (last 20)
    int start = max(0, (int)logEntries.size() - 20);
    for (int i = start; i < logEntries.size(); i++) {
        JsonObject logObj = logs.add<JsonObject>();
        logObj["timestamp"] = logEntries[i].timestamp;
        logObj["level"] = logEntries[i].level;
        logObj["message"] = logEntries[i].message;
    }
    
    doc["total_logs"] = logEntries.size();
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

String getSystemInfoJSON() {
    JsonDocument doc;
    
    doc["chip_model"] = ESP.getChipModel();
    doc["chip_cores"] = ESP.getChipCores();
    doc["cpu_freq"] = ESP.getCpuFreqMHz();
    doc["flash_size"] = ESP.getFlashChipSize();
    doc["free_heap"] = ESP.getFreeHeap();
    doc["uptime"] = millis();
    doc["version"] = "1.0.0";
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

String getMainPageHTML() {
    return R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Garage Door Monitor</title>
    <link rel="stylesheet" href="/styles.css">
</head>
<body>
    <header>
        <h1>🏠 ESP32 Garage Door Monitor</h1>
        <div id="connection-status" class="status-indicator">Connecting...</div>
    </header>

    <main>
        <section class="status-grid">
            <div class="status-card">
                <h3>🔴 Beam Sensor</h3>
                <div id="beam-status" class="status-value">Unknown</div>
                <div id="beam-indicator" class="indicator"></div>
            </div>
            
            <div class="status-card">
                <h3>💡 LED Status</h3>
                <div id="led-status" class="status-value">Unknown</div>
                <div id="led-indicator" class="indicator"></div>
            </div>
            
            <div class="status-card">
                <h3>📶 WiFi Signal</h3>
                <div id="wifi-signal" class="status-value">-- dBm</div>
                <div class="wifi-info">
                    <div id="wifi-ip">IP: --</div>
                </div>
            </div>
            
            <div class="status-card">
                <h3>⏱️ System Info</h3>
                <div id="uptime" class="status-value">--</div>
                <div class="system-info">
                    <div id="free-memory">Memory: --</div>
                    <div id="chip-temp">Temp: --°C</div>
                </div>
            </div>
        </section>

        <section class="logs-section">
            <h3>📋 System Logs</h3>
            <div class="logs-controls">
                <button id="clear-logs">Clear Logs</button>
                <button id="refresh-logs">Refresh</button>
            </div>
            <div id="logs-container">
                <div class="log-loading">Loading logs...</div>
            </div>
        </section>
    </main>

    <script src="/script.js"></script>
</body>
</html>
    )";
}

String getCSS() {
    return R"(
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Arial, sans-serif;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    min-height: 100vh;
    color: #333;
}

header {
    background: rgba(255, 255, 255, 0.95);
    padding: 1rem 2rem;
    display: flex;
    justify-content: space-between;
    align-items: center;
    box-shadow: 0 2px 20px rgba(0,0,0,0.1);
}

h1 {
    color: #4a5568;
    font-size: 1.5rem;
}

.status-indicator {
    padding: 0.5rem 1rem;
    border-radius: 20px;
    font-weight: bold;
    font-size: 0.9rem;
}

.status-indicator.connected {
    background: #48bb78;
    color: white;
}

.status-indicator.disconnected {
    background: #f56565;
    color: white;
}

main {
    padding: 2rem;
    max-width: 1200px;
    margin: 0 auto;
}

.status-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
    gap: 1.5rem;
    margin-bottom: 2rem;
}

.status-card {
    background: rgba(255, 255, 255, 0.95);
    padding: 1.5rem;
    border-radius: 15px;
    box-shadow: 0 8px 32px rgba(0,0,0,0.1);
    text-align: center;
}

.status-card h3 {
    margin-bottom: 1rem;
    color: #4a5568;
}

.status-value {
    font-size: 1.5rem;
    font-weight: bold;
    margin-bottom: 1rem;
}

.indicator {
    width: 20px;
    height: 20px;
    border-radius: 50%;
    margin: 0 auto;
    transition: all 0.3s ease;
}

.indicator.on {
    background: #48bb78;
    box-shadow: 0 0 20px #48bb78;
}

.indicator.off {
    background: #e2e8f0;
}

.indicator.broken {
    background: #f56565;
    box-shadow: 0 0 20px #f56565;
}

.wifi-info, .system-info {
    font-size: 0.9rem;
    color: #718096;
}

.logs-section {
    background: rgba(255, 255, 255, 0.95);
    padding: 1.5rem;
    border-radius: 15px;
    box-shadow: 0 8px 32px rgba(0,0,0,0.1);
}

.logs-controls {
    margin-bottom: 1rem;
    display: flex;
    gap: 1rem;
}

button {
    background: #4299e1;
    color: white;
    border: none;
    padding: 0.5rem 1rem;
    border-radius: 8px;
    cursor: pointer;
    font-size: 0.9rem;
    transition: all 0.2s ease;
}

button:hover {
    background: #3182ce;
    transform: translateY(-1px);
}

#logs-container {
    max-height: 300px;
    overflow-y: auto;
    border: 1px solid #e2e8f0;
    border-radius: 8px;
    padding: 1rem;
    background: #f7fafc;
}

.log-entry {
    margin-bottom: 0.5rem;
    padding: 0.5rem;
    border-radius: 4px;
    font-family: 'Courier New', monospace;
    font-size: 0.85rem;
}

.log-entry.INFO {
    background: #e6f7ff;
    border-left: 4px solid #1890ff;
}

.log-entry.WARN {
    background: #fffbe6;
    border-left: 4px solid #faad14;
}

.log-entry.ERROR {
    background: #fff2f0;
    border-left: 4px solid #f5222d;
}

.log-timestamp {
    color: #8c8c8c;
    font-size: 0.8rem;
}

@media (max-width: 768px) {
    header {
        flex-direction: column;
        gap: 1rem;
    }
    
    main {
        padding: 1rem;
    }
    
    .status-grid {
        grid-template-columns: 1fr;
    }
}
    )";
}

String getJavaScript() {
    return R"(
class GarageDoorMonitor {
    constructor() {
        this.isConnected = false;
        this.init();
    }

    init() {
        this.setupEventListeners();
        this.startPeriodicUpdates();
        this.loadInitialData();
    }

    setupEventListeners() {
        document.getElementById('clear-logs').addEventListener('click', () => {
            this.clearLogs();
        });

        document.getElementById('refresh-logs').addEventListener('click', () => {
            this.loadLogs();
        });
    }

    async loadInitialData() {
        await Promise.all([
            this.loadStatus(),
            this.loadLogs(),
            this.loadSystemInfo()
        ]);
    }

    startPeriodicUpdates() {
        // Update status every 2 seconds
        setInterval(() => {
            this.loadStatus();
        }, 2000);

        // Update logs every 5 seconds
        setInterval(() => {
            this.loadLogs();
        }, 5000);

        // Update system info every 10 seconds
        setInterval(() => {
            this.loadSystemInfo();
        }, 10000);
    }

    async loadStatus() {
        try {
            const response = await fetch('/api/status');
            const data = await response.json();
            this.updateStatus(data);
            this.setConnectionStatus(true);
        } catch (error) {
            console.error('Error loading status:', error);
            this.setConnectionStatus(false);
        }
    }

    async loadLogs() {
        try {
            const response = await fetch('/api/logs');
            const data = await response.json();
            this.updateLogs(data.logs);
        } catch (error) {
            console.error('Error loading logs:', error);
        }
    }

    async loadSystemInfo() {
        try {
            const response = await fetch('/api/system');
            const data = await response.json();
            this.updateSystemInfo(data);
        } catch (error) {
            console.error('Error loading system info:', error);
        }
    }

    updateStatus(data) {
        // Beam status
        const beamStatus = document.getElementById('beam-status');
        const beamIndicator = document.getElementById('beam-indicator');
        if (data.beam_broken) {
            beamStatus.textContent = 'BROKEN';
            beamStatus.style.color = '#f56565';
            beamIndicator.className = 'indicator broken';
        } else {
            beamStatus.textContent = 'CLEAR';
            beamStatus.style.color = '#48bb78';
            beamIndicator.className = 'indicator off';
        }

        // LED status
        const ledStatus = document.getElementById('led-status');
        const ledIndicator = document.getElementById('led-indicator');
        if (data.led_status) {
            ledStatus.textContent = 'ON';
            ledStatus.style.color = '#f56565';
            ledIndicator.className = 'indicator on';
        } else {
            ledStatus.textContent = 'OFF';
            ledStatus.style.color = '#718096';
            ledIndicator.className = 'indicator off';
        }

        // WiFi info
        document.getElementById('wifi-signal').textContent = `${data.wifi_rssi} dBm`;
        document.getElementById('wifi-ip').textContent = `IP: ${data.wifi_ip}`;

        // Uptime and memory
        document.getElementById('uptime').textContent = this.formatUptime(data.uptime);
        document.getElementById('free-memory').textContent = `Memory: ${this.formatBytes(data.free_heap)}`;
        document.getElementById('chip-temp').textContent = `Temp: ${data.chip_temp.toFixed(1)}°C`;
    }

    updateLogs(logs) {
        const container = document.getElementById('logs-container');
        container.innerHTML = '';

        if (logs.length === 0) {
            container.innerHTML = '<div class="log-loading">No logs available</div>';
            return;
        }

        logs.reverse().forEach(log => {
            const logElement = document.createElement('div');
            logElement.className = `log-entry ${log.level}`;
            logElement.innerHTML = `
                <span class="log-timestamp">[${this.formatTimestamp(log.timestamp)}]</span>
                <strong>${log.level}:</strong> ${log.message}
            `;
            container.appendChild(logElement);
        });

        // Scroll to top to show newest logs
        container.scrollTop = 0;
    }

    updateSystemInfo(data) {
        // Could add more system info display here if needed
        console.log('System Info:', data);
    }

    setConnectionStatus(connected) {
        const statusElement = document.getElementById('connection-status');
        if (connected !== this.isConnected) {
            this.isConnected = connected;
            if (connected) {
                statusElement.textContent = '✅ Connected';
                statusElement.className = 'status-indicator connected';
            } else {
                statusElement.textContent = '❌ Disconnected';
                statusElement.className = 'status-indicator disconnected';
            }
        }
    }

    async clearLogs() {
        try {
            await fetch('/api/clear-logs', { method: 'POST' });
            this.loadLogs(); // Reload logs after clearing
        } catch (error) {
            console.error('Error clearing logs:', error);
        }
    }

    formatUptime(milliseconds) {
        const seconds = Math.floor(milliseconds / 1000);
        const minutes = Math.floor(seconds / 60);
        const hours = Math.floor(minutes / 60);
        const days = Math.floor(hours / 24);

        if (days > 0) return `${days}d ${hours % 24}h`;
        if (hours > 0) return `${hours}h ${minutes % 60}m`;
        if (minutes > 0) return `${minutes}m ${seconds % 60}s`;
        return `${seconds}s`;
    }

    formatBytes(bytes) {
        if (bytes < 1024) return bytes + ' B';
        if (bytes < 1048576) return (bytes / 1024).toFixed(1) + ' KB';
        return (bytes / 1048576).toFixed(1) + ' MB';
    }

    formatTimestamp(timestamp) {
        const ms = parseInt(timestamp);
        const seconds = Math.floor(ms / 1000);
        return new Date(Date.now() - (Date.now() % 1000) + (ms % 1000)).toLocaleTimeString();
    }
}

// Initialize the monitor when page loads
document.addEventListener('DOMContentLoaded', () => {
    new GarageDoorMonitor();
});
    )";
}

#else
// WiFi disabled stubs
void initWebServer() { /* WiFi disabled */ }
void handleWebServer() { /* WiFi disabled */ }
void addLogEntry(String message, String level) { 
    Serial.printf("[%s] %s\n", level.c_str(), message.c_str());
}
String getStatusJSON() { return "{}"; }
String getLogsJSON() { return "{}"; }
String getSystemInfoJSON() { return "{}"; }
#endif