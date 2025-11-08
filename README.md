# ESP32 Garage Door Sensor with OTA Updates

A comprehensive IoT solution for monitoring garage doors using the E3JK-RR11 photoelectric sensor with the ESP32 S3 Nano microcontroller.

## 🚀 Features

### Hardware Support
- **ESP32 S3 Nano** - High-performance microcontroller with WiFi
- **E3JK-RR11** - Photoelectric proximity sensor for beam detection
- **GPIO Indicators** - LED status indicators
- **Sensor Expansion** - Support for DHT22, BMP280 (optional)

### Software Capabilities  
- **Real-time Web Dashboard** - Mobile-responsive monitoring interface
- **WiFi Connectivity** - Secure credential management
- **Over-the-Air (OTA) Updates** - Network and web-based firmware updates
- **Simulation Mode** - Test without physical sensors
- **System Monitoring** - Memory usage, uptime, WiFi status
- **Event Logging** - Real-time activity logs with timestamps

### Security Features
- **Encrypted WiFi** - WPA2/WPA3 support
- **OTA Authentication** - Password-protected updates  
- **Credential Protection** - Git-ignored secrets management
- **GitHub Integration** - Automated secure deployments

## 📦 Quick Start

### 1. Hardware Setup
```
ESP32 S3 Nano Connections:
├── GPIO 4  → E3JK-RR11 Signal (with voltage level shifter)
├── GPIO 2  → Status LED
├── 3.3V    → Sensor Logic (via level shifter)
└── GND     → Common Ground

E3JK-RR11 Power: 12V DC supply (separate)
```

### 2. Software Installation

**Prerequisites:**
- [PlatformIO](https://platformio.org/) or Arduino IDE
- [Git](https://git-scm.com/)

**Clone and Setup:**
```bash
git clone https://github.com/NZCypher819/esp32-garage-door-sensor.git
cd esp32-garage-door-sensor

# Configure WiFi credentials
cp include/secrets.h.template include/secrets.h
# Edit secrets.h with your WiFi details
```

**Build and Upload:**
```bash
# Using PlatformIO
pio run -t upload

# Using PlatformIO with specific port
pio run -t upload --upload-port COM4
```

### 3. Configuration

Edit `include/secrets.h`:
```cpp
#define WIFI_SSID "Your-WiFi-Network"
#define WIFI_PASSWORD "Your-WiFi-Password"
```

Optional settings in `include/config.h`:
```cpp
#define ENABLE_SIMULATION_MODE    // Test without hardware
#define ENABLE_DHT22             // Temperature sensor
#define ENABLE_BMP280            // Pressure sensor
```

## 🌐 Web Dashboard

Once connected, access the dashboard at: `http://[ESP32-IP-ADDRESS]`

### Dashboard Features
- **🚨 Real-time Beam Status** - CLEAR/BLOCKED with timestamps
- **📊 System Information** - Memory, uptime, WiFi signal
- **📋 Event Logs** - Sensor state changes and system events
- **🔄 OTA Controls** - Check for updates and manage firmware
- **📱 Mobile Responsive** - Works on phones, tablets, desktop

### API Endpoints
```
GET  /api/status      # Sensor status and beam state
GET  /api/logs        # Recent system logs  
GET  /api/system      # System information
GET  /api/ota/status  # OTA update status
GET  /api/ota/info    # Version information
POST /api/clear-logs  # Clear log history
POST /api/ota/check   # Manual update check
```

## 🔄 OTA Updates

### Automatic Updates
The device automatically checks GitHub releases every 60 seconds and can update itself when new firmware is available.

### Manual Update Methods

**1. Web Interface:**
- Visit `http://[device-ip]/update`  
- Login: `admin` | Password: `[your-ota-password]`
- Upload `.bin` file directly

**2. Network OTA (Arduino IDE/PlatformIO):**
```bash
# PlatformIO over network
pio run -t upload --upload-port [device-ip]

# Arduino IDE: Tools → Port → Network Port
```

**3. GitHub Releases:**
- Device automatically detects new releases
- Downloads and installs firmware
- Reboots with new version

## 🛠 Development

### Project Structure
```
├── src/                    # Main source code
│   ├── main.cpp           # Application entry point
│   ├── sensors.cpp        # E3JK-RR11 and sensor management
│   ├── wifi_manager.cpp   # WiFi connection handling
│   ├── web_server.cpp     # HTTP server and dashboard
│   └── ota_manager.cpp    # OTA update functionality
├── include/               # Header files
│   ├── config.h          # Feature configuration
│   ├── secrets.h.template # WiFi credentials template
│   ├── ota_config.h       # OTA settings
│   └── *.h               # Module headers
├── .github/workflows/     # CI/CD automation
├── platformio.ini         # PlatformIO configuration
└── README.md             # This file
```

### Building Locally
```bash
# Install dependencies
pio lib install

# Build firmware
pio run

# Upload to device
pio run -t upload

# Monitor serial output
pio device monitor
```

### Testing
```bash
# Enable simulation mode in config.h
#define ENABLE_SIMULATION_MODE

# Build and upload - beam will toggle every 10 seconds
pio run -t upload
```

## 🔧 Configuration Reference

### WiFi Settings (`secrets.h`)
```cpp
#define WIFI_SSID "Your-Network-Name"
#define WIFI_PASSWORD "Your-Network-Password"
```

### OTA Settings (`ota_config.h`)
```cpp
#define OTA_PASSWORD "your-secure-password"  // Change this!
#define OTA_PORT 3232                        // Network OTA port
#define FIRMWARE_VERSION "1.0.0"             // Current version
#define OTA_CHECK_INTERVAL 60000             // Update check interval (ms)
```

### Sensor Configuration (`config.h`)
```cpp
#define BEAM_SENSOR_PIN 4               // E3JK-RR11 input pin
#define LED_INDICATOR_PIN 2             // Status LED pin  
#define BEAM_DEBOUNCE_TIME 50           // Debounce delay (ms)
#define SIMULATION_BEAM_INTERVAL 10000  // Simulation toggle (ms)
```

## 🔒 Security

### Best Practices
1. **Change default OTA password** in `ota_config.h`
2. **Use strong WiFi credentials** 
3. **Keep firmware updated** via automatic updates
4. **Secure physical access** to the device
5. **Monitor access logs** via web dashboard

### Credential Management
- WiFi credentials in `secrets.h` (git-ignored)
- OTA password configurable in `ota_config.h`  
- No hardcoded sensitive data in source code
- GitHub Actions uses dummy credentials for CI builds

## 🚨 Troubleshooting

### Common Issues

**Device not connecting to WiFi:**
```cpp
// Check credentials in secrets.h
// Verify 2.4GHz network (ESP32 doesn't support 5GHz)
// Check WiFi signal strength
```

**Can't access web dashboard:**
```bash
# Find device IP address
pio device monitor
# Look for: "WiFi connected! IP: 192.168.x.x"

# Or scan network
nmap -sn 192.168.1.0/24 | grep ESP32
```

**OTA update fails:**
```cpp
// Check OTA password
// Ensure sufficient memory (>50% free)
// Verify GitHub release has .bin file
// Check internet connectivity
```

**E3JK-RR11 not responding:**
```cpp
// Verify 12V power supply to sensor
// Check voltage level shifter (12V → 3.3V logic)
// Test with multimeter: sensor output 0V/12V
// Enable ENABLE_SIMULATION_MODE for testing
```

### Debug Mode
Enable verbose logging:
```cpp
// In platformio.ini, add:
build_flags = -DCORE_DEBUG_LEVEL=5

// Monitor serial output:
pio device monitor --baud 115200
```

## 📊 Performance

### Memory Usage
- **Flash**: ~1MB (30% of 8MB)
- **RAM**: ~51KB (15% of 320KB)  
- **Free heap**: ~270KB available for operation

### Network Performance
- **WiFi connection**: ~2-5 seconds
- **Web dashboard load**: <200ms
- **API response time**: <50ms
- **OTA download**: ~30-60 seconds (depends on connection)

## 🤝 Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Setup
```bash
git clone https://github.com/NZCypher819/esp32-garage-door-sensor.git
cd esp32-garage-door-sensor
cp include/secrets.h.template include/secrets.h
# Edit secrets.h with your WiFi credentials
pio run
```

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🎯 Roadmap

- [ ] **Mobile App** - Native iOS/Android application
- [ ] **MQTT Integration** - Home Assistant compatibility  
- [ ] **Multi-sensor Support** - Multiple garage doors
- [ ] **Cloud Dashboard** - Remote monitoring capabilities
- [ ] **Backup/Restore** - Configuration management
- [ ] **Webhook Notifications** - External alerting integration

## 🆘 Support

- **Documentation**: Check this README and inline code comments
- **Issues**: [GitHub Issues](https://github.com/NZCypher819/esp32-garage-door-sensor/issues)
- **Discussions**: [GitHub Discussions](https://github.com/NZCypher819/esp32-garage-door-sensor/discussions)

---

**Built with ❤️ for garage door automation**