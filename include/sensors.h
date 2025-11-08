#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// Function declarations
void initializeSensors();
void readAllSensors();
void readE3JKRR11();
void readDHT22();
void readBMP280();
void readAnalogSensor();

// E3JK-RR11 specific functions
bool isBeamBroken();
bool isBeamClear();
void updateBeamStatusLED();
void setupE3JKRR11Interrupt();
void IRAM_ATTR e3jkInterruptHandler();

// Sensor data structures
struct SensorData {
  bool beamBroken;              // E3JK-RR11 beam status (true = broken, false = clear)
  unsigned long lastStateChangeTime; // Last state change timestamp
  float temperature;
  float humidity;
  float pressure;
  float altitude;
  int analogValue;
  bool dataValid;
};

extern SensorData currentSensorData;

#endif // SENSORS_H