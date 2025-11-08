#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration (uncomment to enable)
#define ENABLE_WIFI

// Try to include secrets.h for secure credentials first
#ifdef __has_include
  #if __has_include("secrets.h")
    #include "secrets.h"
  #endif
#endif

// Then include wifi_config.h for defaults
#include "wifi_config.h"

// Testing Configuration
// #define ENABLE_SIMULATION_MODE    // Enable for testing without hardware (DISABLED)
#define SIMULATION_BEAM_INTERVAL 10000  // Simulate beam break every 10 seconds

// Sensor Configuration
#define SENSOR_READ_INTERVAL 2000  // Read sensors every 2 seconds

// GPIO Pin Definitions for ESP32 S3 Nano
#define E3JK_RR11_PIN 4           // E3JK-RR11 photoelectric sensor digital output
#define DHT22_PIN 5               // Optional temperature/humidity sensor
#define BMP280_SDA_PIN 8          // Optional pressure sensor I2C
#define BMP280_SCL_PIN 9          // Optional pressure sensor I2C
#define ANALOG_SENSOR_PIN A0      // Optional analog input
#define LED_INDICATOR_PIN 2       // Status LED for sensor detection

// Sensor Enable/Disable flags
#define ENABLE_E3JK_RR11          // Primary photoelectric sensor
// #define ENABLE_DHT22           // Uncomment for environmental monitoring
// #define ENABLE_BMP280          // Uncomment for pressure monitoring
// #define ENABLE_ANALOG_SENSOR   // Uncomment for additional analog input

// E3JK-RR11 Configuration
#define E3JK_DEBOUNCE_TIME 50     // Debounce time in milliseconds
#define E3JK_BEAM_BROKEN LOW      // LOW = beam broken (object detected), HIGH = beam clear
#define E3JK_BEAM_CLEAR HIGH      // HIGH = beam clear (no object), LOW = beam broken

// LED Control for beam status
#define LED_ON_BEAM_BROKEN true   // Turn LED ON when beam is broken
#define LED_OFF_BEAM_CLEAR true   // Turn LED OFF when beam is clear

// Debugging
#define DEBUG_SENSORS true

#endif // CONFIG_H