# ESP32 S3 Nano PlatformIO Project

This is an ESP32 S3 Nano project using PlatformIO for E3JK-RR11 photoelectric sensor interfacing and garage door automation.

## Project Structure
- `src/` - Main source code
- `lib/` - Project libraries  
- `include/` - Header files
- `platformio.ini` - PlatformIO configuration
- `.vscode/` - VS Code tasks and settings

## Development Guidelines
- Use Arduino framework for ESP32 S3
- Follow sensor interfacing best practices for photoelectric sensors
- Include proper error handling and debouncing for E3JK-RR11
- Use appropriate GPIO pins for ESP32 S3 Nano
- Implement interrupt-driven approaches for fast response times
- Consider safety applications for garage door automation

## Coding Standards
- Use descriptive variable and function names
- Comment sensor initialization and interrupt handling code
- Include proper debounce timing for reliable detection
- Use interrupt-driven approaches for critical safety sensors
- Implement proper voltage level shifting for 12V sensors

## Available Tasks
- **Build**: Compile the project using PlatformIO
- **Upload**: Flash the compiled code to ESP32 S3 Nano
- **Monitor**: View serial output from the device
- **Clean**: Remove build artifacts

## Sensor Configuration
Primary sensor is the E3JK-RR11 photoelectric proximity sensor. Additional sensors can be enabled/disabled in `include/config.h`:
- E3JK-RR11 (photoelectric proximity sensor) - PRIMARY
- DHT22 (temperature/humidity) - optional
- BMP280 (pressure/altitude) - optional
- Analog sensors - optional
- Digital sensors - optional

## Safety Considerations
- E3JK-RR11 requires 12V power supply
- Use proper voltage level shifting for 3.3V GPIO compatibility
- Implement debouncing for reliable object detection
- Consider fail-safe modes for garage door applications