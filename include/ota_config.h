#ifndef OTA_CONFIG_H
#define OTA_CONFIG_H

// OTA Configuration
#define OTA_ENABLED true
#define OTA_PASSWORD "GarageDoor2025"  // Change this to something secure
#define OTA_PORT 3232

// Firmware version for OTA updates
#define FIRMWARE_VERSION "1.0.0"
#define FIRMWARE_NAME "ESP32-GarageDoor"

// OTA Update URLs (GitHub releases API)
#define OTA_UPDATE_URL "https://api.github.com/repos/NZCypher819/esp32-garage-door-sensor/releases/latest"
#define OTA_CHECK_INTERVAL 60000  // Check for updates every 60 seconds

// GitHub API Authentication (required for private repositories)
// Generate a Personal Access Token with 'public_repo' scope at:
// https://github.com/settings/personal-access-tokens/tokens
// #define GITHUB_TOKEN "ghp_your_token_here"  // Uncomment and add your token
#define GITHUB_TOKEN ""  // Leave empty for public repositories

// OTA Status
enum OTAUpdateStatus {
    OTA_UPDATE_IDLE,
    OTA_UPDATE_CHECKING,
    OTA_UPDATE_DOWNLOADING,
    OTA_UPDATE_INSTALLING,
    OTA_UPDATE_SUCCESS,
    OTA_UPDATE_ERROR
};

#endif