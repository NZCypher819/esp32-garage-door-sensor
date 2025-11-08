#include "sensors.h"
#include "config.h"

#ifdef ENABLE_DHT22
#include <DHT.h>
DHT dht(DHT22_PIN, DHT22);
#endif

#ifdef ENABLE_BMP280
#include <Wire.h>
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp; // I2C interface
#endif

// Global sensor data
SensorData currentSensorData = {false, 0, 0, 0, 0, 0, 0, false};

// E3JK-RR11 variables
volatile bool e3jkStateChanged = false;
volatile bool e3jkBeamBroken = false;
unsigned long lastDebounceTime = 0;

void initializeSensors() {
  Serial.println("Initializing sensors...");
  
  #ifdef ENABLE_E3JK_RR11
  pinMode(E3JK_RR11_PIN, INPUT);
  pinMode(LED_INDICATOR_PIN, OUTPUT);
  digitalWrite(LED_INDICATOR_PIN, LOW); // Start with LED off (beam clear)
  
  // Setup interrupt for E3JK-RR11
  setupE3JKRR11Interrupt();
  Serial.println("E3JK-RR11 photoelectric sensor initialized");
  Serial.println("LED will turn ON when beam is BROKEN");
  #endif
  
  #ifdef ENABLE_DHT22
  dht.begin();
  Serial.println("DHT22 sensor initialized");
  #endif
  
  #ifdef ENABLE_BMP280
  Wire.begin(BMP280_SDA_PIN, BMP280_SCL_PIN);
  if (!bmp.begin(0x76)) { // Try default address first
    if (!bmp.begin(0x77)) { // Try alternative address
      Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    } else {
      Serial.println("BMP280 sensor initialized (address 0x77)");
    }
  } else {
    Serial.println("BMP280 sensor initialized (address 0x76)");
  }
  
  // Configure BMP280 settings
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  #endif
  
  #ifdef ENABLE_ANALOG_SENSOR
  pinMode(ANALOG_SENSOR_PIN, INPUT);
  Serial.println("Analog sensor pin configured");
  #endif
  
  Serial.println("All sensors initialized successfully!");
}

void readAllSensors() {
  if (DEBUG_SENSORS) {
    Serial.println("\n--- Reading Sensors ---");
  }
  
  // Reset data validity
  currentSensorData.dataValid = false;
  
  #ifdef ENABLE_E3JK_RR11
  readE3JKRR11();
  #endif
  
  #ifdef ENABLE_DHT22
  readDHT22();
  #endif
  
  #ifdef ENABLE_BMP280
  readBMP280();
  #endif
  
  #ifdef ENABLE_ANALOG_SENSOR
  readAnalogSensor();
  #endif
  
  currentSensorData.dataValid = true;
  
  if (DEBUG_SENSORS) {
    Serial.println("--- Sensor Reading Complete ---\n");
  }
}

#ifdef ENABLE_DHT22
void readDHT22() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT22 sensor!");
    return;
  }
  
  currentSensorData.temperature = temperature;
  currentSensorData.humidity = humidity;
  
  if (DEBUG_SENSORS) {
    Serial.printf("DHT22 - Temperature: %.2f°C, Humidity: %.2f%%\n", 
                  temperature, humidity);
  }
}
#else
void readDHT22() {
  // DHT22 disabled
}
#endif

#ifdef ENABLE_BMP280
void readBMP280() {
  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0F; // Convert to hPa
  float altitude = bmp.readAltitude(1013.25); // Sea level pressure in hPa
  
  if (isnan(temperature) || isnan(pressure)) {
    Serial.println("Failed to read from BMP280 sensor!");
    return;
  }
  
  currentSensorData.pressure = pressure;
  currentSensorData.altitude = altitude;
  
  // Use BMP280 temperature if DHT22 is not available
  #ifndef ENABLE_DHT22
  currentSensorData.temperature = temperature;
  #endif
  
  if (DEBUG_SENSORS) {
    Serial.printf("BMP280 - Temperature: %.2f°C, Pressure: %.2f hPa, Altitude: %.2fm\n", 
                  temperature, pressure, altitude);
  }
}
#else
void readBMP280() {
  // BMP280 disabled
}
#endif

#ifdef ENABLE_ANALOG_SENSOR
void readAnalogSensor() {
  int analogValue = analogRead(ANALOG_SENSOR_PIN);
  currentSensorData.analogValue = analogValue;
  
  if (DEBUG_SENSORS) {
    float voltage = (analogValue / 4095.0) * 3.3; // Convert to voltage for ESP32
    Serial.printf("Analog Sensor - Raw: %d, Voltage: %.2fV\n", analogValue, voltage);
  }
}
#else
void readAnalogSensor() {
  // Analog sensor disabled
}
#endif

// E3JK-RR11 Photoelectric Sensor Functions
#ifdef ENABLE_E3JK_RR11
void readE3JKRR11() {
  bool currentBeamState = (digitalRead(E3JK_RR11_PIN) == E3JK_BEAM_BROKEN);
  
  // Update sensor data if state changed
  if (currentBeamState != currentSensorData.beamBroken) {
    currentSensorData.beamBroken = currentBeamState;
    currentSensorData.lastStateChangeTime = millis();
    
    // Update LED based on beam status
    updateBeamStatusLED();
    
    if (DEBUG_SENSORS) {
      Serial.printf("E3JK-RR11 - Beam %s at %lu ms\n", 
                    currentBeamState ? "BROKEN (LED ON)" : "CLEAR (LED OFF)", 
                    currentSensorData.lastStateChangeTime);
    }
  }
}

bool isBeamBroken() {
  return currentSensorData.beamBroken;
}

bool isBeamClear() {
  return !currentSensorData.beamBroken;
}

void updateBeamStatusLED() {
  if (LED_ON_BEAM_BROKEN && currentSensorData.beamBroken) {
    digitalWrite(LED_INDICATOR_PIN, HIGH); // Turn LED ON when beam is broken
  } else if (LED_OFF_BEAM_CLEAR && !currentSensorData.beamBroken) {
    digitalWrite(LED_INDICATOR_PIN, LOW);  // Turn LED OFF when beam is clear
  }
}

void setupE3JKRR11Interrupt() {
  attachInterrupt(digitalPinToInterrupt(E3JK_RR11_PIN), e3jkInterruptHandler, CHANGE);
}

void IRAM_ATTR e3jkInterruptHandler() {
  unsigned long currentTime = millis();
  
  // Simple debouncing
  if (currentTime - lastDebounceTime > E3JK_DEBOUNCE_TIME) {
    e3jkStateChanged = true;
    e3jkBeamBroken = (digitalRead(E3JK_RR11_PIN) == E3JK_BEAM_BROKEN);
    lastDebounceTime = currentTime;
  }
}
#else
void readE3JKRR11() {
  // E3JK-RR11 disabled
}

bool isBeamBroken() {
  return false;
}

bool isBeamClear() {
  return true;
}

void updateBeamStatusLED() {
  // E3JK-RR11 disabled
}

void setupE3JKRR11Interrupt() {
  // E3JK-RR11 disabled
}

void IRAM_ATTR e3jkInterruptHandler() {
  // E3JK-RR11 disabled
}
#endif