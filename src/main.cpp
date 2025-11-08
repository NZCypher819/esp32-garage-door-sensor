#include <Arduino.h>
#include "sensors.h"
#include "config.h"
#ifdef ENABLE_WIFI
#include "wifi_manager.h"
#include "web_server.h"
#endif

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // Wait for serial port to connect
  }
  
  Serial.println("ESP32 S3 Nano Sensor Interface Starting...");
  
  #ifdef ENABLE_SIMULATION_MODE
  Serial.println("🧪 SIMULATION MODE ENABLED - No hardware required!");
  Serial.println("🔄 Beam will automatically break/clear every 10 seconds");
  #endif
  
  // Initialize sensors
  initializeSensors();
  
  // Initialize WiFi if enabled
  #ifdef ENABLE_WIFI
  initWiFi();
  delay(2000);  // Give WiFi time to settle
  if (isWiFiConnected()) {
    printWiFiInfo();
    // Start web server
    initWebServer();
    addLogEntry("System started successfully", "INFO");
  }
  #endif
  
  Serial.println("System initialized successfully!");
}

void loop() {
  // Check WiFi connection status
  #ifdef ENABLE_WIFI
  checkWiFiConnection();
  handleWebServer();
  #endif
  
  // Read sensors periodically
  readAllSensors();
  
  // Check for beam break with E3JK-RR11
  #ifdef ENABLE_E3JK_RR11
  
  #ifdef ENABLE_SIMULATION_MODE
  // Simulate beam break every 10 seconds for testing
  static unsigned long lastSimulation = 0;
  static bool simulatedBeamBroken = false;
  
  if (millis() - lastSimulation >= SIMULATION_BEAM_INTERVAL) {
    lastSimulation = millis();
    simulatedBeamBroken = !simulatedBeamBroken;
    currentSensorData.beamBroken = simulatedBeamBroken;
    currentSensorData.lastStateChangeTime = millis();
    
    // Update LED for simulation
    digitalWrite(LED_INDICATOR_PIN, simulatedBeamBroken ? HIGH : LOW);
    
    if (simulatedBeamBroken) {
      Serial.println("🧪 SIMULATION: Beam BROKEN - LED ON");
      addLogEntry("SIMULATION: Beam broken - object detected!", "WARN");
    } else {
      Serial.println("🧪 SIMULATION: Beam CLEAR - LED OFF");
      addLogEntry("SIMULATION: Beam clear - path restored", "INFO");
    }
  }
  #else
  // Real hardware beam detection
  static bool lastBeamBroken = false;
  bool currentBeamBroken = isBeamBroken();
  
  if (currentBeamBroken && !lastBeamBroken) {
    Serial.println(">>> BEAM BROKEN - LED ON <<<");
    addLogEntry("Beam broken - object detected!", "WARN");
  } else if (!currentBeamBroken && lastBeamBroken) {
    Serial.println(">>> BEAM CLEAR - LED OFF <<<");
    addLogEntry("Beam clear - path restored", "INFO");
  }
  
  lastBeamBroken = currentBeamBroken;
  #endif
  
  #endif
  
  // Wait before next reading cycle
  delay(SENSOR_READ_INTERVAL);
}