# ESP32 S3 Nano E3JK-RR11 Photoelectric Sensor Project with Web Dashboard

A PlatformIO-based project for interfacing the Omron E3JK-RR11 photoelectric proximity sensor with the ESP32 S3 Nano development board. Features a real-time web dashboard for monitoring beam status, system logs, and device information. Perfect for garage door automation, object detection, and proximity sensing applications.

## Features

- **E3JK-RR11 photoelectric sensor**: Primary object detection with interrupt-driven input
- **Real-time Web Dashboard**: Monitor beam status, LED state, and system logs remotely
- **WiFi connectivity**: Secure connection with "U64A 2.4" network
- **Live status updates**: Automatic refresh every 2 seconds
- **System logging**: Comprehensive event logging with different severity levels
- **Mobile responsive**: Works on phones, tablets, and desktops
- **Garage door ready**: Optimized for door automation and safety applications  
- **Multi-sensor support**: Optional DHT22, BMP280, and analog sensors
- **Configurable GPIO pins**: Optimized for ESP32 S3 Nano
- **Interrupt handling**: Fast response times for object detection
- **Debounced inputs**: Reliable detection with noise filtering
- **Status LED**: Visual indication of sensor state

## Hardware Requirements

- ESP32 S3 Nano development board
- Omron E3JK-RR11 photoelectric proximity sensor
- Status LED (optional)
- DHT22 temperature/humidity sensor (optional)
- BMP280 pressure sensor (optional)
- Breadboard and jumper wires
- 12V power supply for E3JK-RR11 (if using NPN output model)

## E3JK-RR11 Sensor Specifications

- **Detection range**: 0.1-7m (depending on reflector/object)
- **Response time**: ≤1ms
- **Output**: NPN or PNP transistor output
- **Power**: 12-24V DC
- **Connection**: 3-wire (Brown=+V, Blue=0V, Black=Output)

## Pin Configuration

| Sensor/Component | GPIO Pin | Notes |
|-----------------|----------|-------|
| E3JK-RR11 Output | GPIO 4 | Photoelectric sensor digital output |
| Status LED | GPIO 2 | Indicates object detection state |
| DHT22 Data | GPIO 5 | Temperature/Humidity sensor (optional) |
| BMP280 SDA | GPIO 8 | I2C Data line (optional) |
| BMP280 SCL | GPIO 9 | I2C Clock line (optional) |
| Analog Sensor | A0 | General purpose analog input (optional) |

## E3JK-RR11 Wiring

**For NPN Output Model:**
- Brown wire → 12V+ (external power supply)
- Blue wire → GND (connect to ESP32 GND and power supply GND)
- Black wire → GPIO 4 (through voltage divider if needed)

**Note**: The E3JK-RR11 typically requires 12V power. Use a voltage divider or level shifter to safely connect the 12V output to the ESP32's 3.3V GPIO pin.

## Software Setup

### Prerequisites

1. **VS Code** with PlatformIO extension installed
2. **Git** for version control

### Installation

1. Clone or download this project
2. Open the project folder in VS Code
3. PlatformIO will automatically install dependencies listed in `platformio.ini`

### Configuration

Edit `include/config.h` to customize your setup:

```cpp
// Enable/disable sensors
#define ENABLE_E3JK_RR11           // Primary photoelectric sensor
// #define ENABLE_DHT22            // Uncomment for environmental monitoring
// #define ENABLE_BMP280           // Uncomment for pressure monitoring

// Adjust GPIO pins as needed
#define E3JK_RR11_PIN 4            // E3JK-RR11 digital output
#define LED_INDICATOR_PIN 2        // Status LED
#define DHT22_PIN 5                // Optional DHT22 sensor

// E3JK-RR11 Configuration  
#define E3JK_DEBOUNCE_TIME 50      // Debounce time in milliseconds
#define E3JK_BEAM_BROKEN LOW       // LOW = beam broken (object detected)
#define E3JK_BEAM_CLEAR HIGH       // HIGH = beam clear (no object)

// LED Control
#define LED_ON_BEAM_BROKEN true    // LED turns ON when beam is broken
#define LED_OFF_BEAM_CLEAR true    // LED turns OFF when beam is clear

// Configure sensor reading interval
#define SENSOR_READ_INTERVAL 2000  // milliseconds
```

## 🌐 Web Dashboard

Once your ESP32 connects to WiFi, you can access the web dashboard:

### **Dashboard URL**: `http://[ESP32_IP_ADDRESS]`
- The IP address will be shown in the serial monitor
- Example: `http://192.168.1.100`

### **Dashboard Features**:
- **🔴 Beam Sensor Status**: Real-time indication of beam broken/clear
- **💡 LED Status**: Shows if indicator LED is on/off  
- **📶 WiFi Information**: Signal strength and IP address
- **⏱️ System Information**: Uptime, memory usage, chip temperature
- **📋 Live Logs**: Real-time system events and errors
- **📱 Mobile Responsive**: Works on any device

### **Dashboard Screenshots**:
```
┌─────────────────────────────────────┐
│ 🏠 ESP32 Garage Door Monitor       │
├─────────────────────────────────────┤
│ Beam Sensor: CLEAR      🟢         │
│ LED Status:  OFF        ⚫         │
│ WiFi Signal: -45 dBm    📶         │
│ Uptime:     2h 15m      ⏱️         │
├─────────────────────────────────────┤
│ System Logs:                        │
│ [INFO] Beam clear - path restored   │
│ [WARN] Beam broken - object detect  │
│ [INFO] WiFi connected to U64A 2.4   │
└─────────────────────────────────────┘
```

## E3JK-RR11 Behavior

- **Beam Clear** (no object): LED OFF, Green indicator on dashboard
- **Beam Broken** (object detected): LED ON, Red indicator on dashboard  
- **Response time**: <1ms with interrupt handling
- **Debounced**: 50ms debounce prevents false triggers
- **Web monitoring**: Real-time status updates every 2 seconds

## Usage

1. **Power on** your ESP32 S3 Nano
2. **Connect to WiFi** - Check serial monitor for connection status
3. **Open web dashboard** - Navigate to the IP address shown
4. **Monitor beam status** - Watch real-time updates as objects break the beam
5. **View system logs** - Check the logs panel for detailed event history

### **Typical Log Output**:
```
[INFO] System started successfully
[INFO] WiFi connected to U64A 2.4
[INFO] Web server started at http://192.168.1.100
[WARN] Beam broken - object detected!
[INFO] Beam clear - path restored
```

## Building and Uploading

### Using VS Code (Recommended)
1. Connect your ESP32 S3 Nano to your computer via USB
2. Use **Ctrl+Shift+P** → **Tasks: Run Task** and select:
   - **PlatformIO Build** - Compile the project
   - **PlatformIO Upload** - Flash to device
   - **PlatformIO Monitor** - View serial output

### Using Terminal
If PlatformIO CLI is not in your PATH, use the full path:
```powershell
# Build
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run

# Upload
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run --target upload

# Monitor
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" device monitor
```

### Build Output
- **RAM Usage**: ~14% (46,012 bytes) 
- **Flash Usage**: ~23.5% (786,841 bytes)
- **Build Time**: ~7 seconds
- **Libraries**: ArduinoJson, WebServer, WiFi, sensors

## Usage

1. Upload the code to your ESP32 S3 Nano
2. Open the serial monitor (115200 baud)
3. View real-time sensor readings
4. Modify `src/main.cpp` to add your application logic

## Project Structure

```
├── platformio.ini          # PlatformIO configuration
├── include/
│   ├── config.h            # Project configuration
│   └── sensors.h           # Sensor function declarations
├── src/
│   ├── main.cpp            # Main application code
│   └── sensors.cpp         # Sensor implementation
└── lib/                    # Custom libraries (if any)
```

## Adding New Sensors

1. Add sensor-specific libraries to `platformio.ini`
2. Define GPIO pins in `include/config.h`
3. Add function declarations to `include/sensors.h`
4. Implement sensor code in `src/sensors.cpp`
5. Call sensor functions from `src/main.cpp`

## Troubleshooting

### Common Issues

- **Sensor not detected**: Check wiring and I2C addresses
- **Upload failed**: Ensure correct board and port selection
- **Serial output garbled**: Verify baud rate (115200)
- **Compilation errors**: Check library dependencies in `platformio.ini`

### Debugging

Enable debug output in `include/config.h`:
```cpp
#define DEBUG_SENSORS true
```

## WiFi Configuration (Secure)

### Quick Setup
1. **Edit credentials**: Open `include/secrets.h`
2. **Replace password**: Change `YOUR_ACTUAL_PASSWORD_HERE` to your actual WiFi password
3. **SSID is pre-configured**: "U64A 2.4" is already set

### Security Methods

**Method 1: secrets.h file (Recommended)**
```cpp
// In include/secrets.h
#define WIFI_SSID "U64A 2.4"
#define WIFI_PASSWORD "your_real_password"
```

**Method 2: Environment Variables (Advanced)**
```bash
# Set environment variables before building
export WIFI_SSID_ENV="U64A 2.4"
export WIFI_PASSWORD_ENV="your_password"
```

**Method 3: Runtime Configuration (Most Secure)**
- Use WiFi Manager library for captive portal setup
- No hardcoded credentials in code

### ⚠️ Security Important
- `include/secrets.h` is in `.gitignore` - won't be committed to Git
- Never commit WiFi passwords to version control
- Use strong, unique passwords for your WiFi network

### WiFi Features
- **Auto-reconnection**: Automatically reconnects if connection is lost
- **Connection monitoring**: Checks status every 30 seconds  
- **Status LED**: Built-in LED indicates WiFi status
- **Detailed logging**: Shows IP, signal strength, security type
- **Timeout protection**: Won't hang if WiFi is unavailable

## License

This project is open source and available under the MIT License.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request