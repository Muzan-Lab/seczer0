#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>

// Settings structure
struct SystemSettings {
    // Display settings
    uint8_t brightness;
    uint8_t contrast;
    bool autoSleep;
    uint16_t sleepTimeout;
    bool invertDisplay;
    
    // Sound settings
    bool soundEnabled;
    uint8_t volume;
    uint16_t beepFrequency;
    uint16_t beepDuration;
    
    // Storage settings
    bool autoSave;
    uint16_t maxHistoryItems;
    bool compressData;
    
    // System settings
    String deviceName;
    bool debugMode;
    uint32_t baudRate;
    bool wifiEnabled;
    String wifiSSID;
    String wifiPassword;
    
    // Module settings
    bool nfcEnabled;
    bool irEnabled;
    bool iButtonEnabled;
    bool rfEnabled;
    bool gpioEnabled;
};

class SettingsManager {
public:
    SettingsManager();
    bool init();
    
    // Settings management
    bool loadSettings();
    bool saveSettings();
    void resetToDefaults();
    
    // Getters
    SystemSettings* getSettings() { return &settings; }
    uint8_t getBrightness() { return settings.brightness; }
    uint8_t getVolume() { return settings.volume; }
    bool isSoundEnabled() { return settings.soundEnabled; }
    bool isAutoSaveEnabled() { return settings.autoSave; }
    bool isDebugModeEnabled() { return settings.debugMode; }
    
    // Setters
    void setBrightness(uint8_t brightness);
    void setVolume(uint8_t volume);
    void setSoundEnabled(bool enabled);
    void setAutoSaveEnabled(bool enabled);
    void setDebugMode(bool enabled);
    void setDeviceName(const String& name);
    
    // Display settings
    void setContrast(uint8_t contrast);
    void setAutoSleep(bool enabled);
    void setSleepTimeout(uint16_t timeout);
    void setInvertDisplay(bool invert);
    
    // Sound settings
    void setBeepFrequency(uint16_t frequency);
    void setBeepDuration(uint16_t duration);
    
    // Storage settings
    void setMaxHistoryItems(uint16_t maxItems);
    void setCompressData(bool compress);
    
    // WiFi settings
    void setWiFiEnabled(bool enabled);
    void setWiFiCredentials(const String& ssid, const String& password);
    
    // Module settings
    void setModuleEnabled(const String& module, bool enabled);
    bool isModuleEnabled(const String& module);
    
    // Status
    bool isInitialized() { return settingsInitialized; }
    unsigned long getLastSaveTime() { return lastSaveTime; }

private:
    SystemSettings settings;
    bool settingsInitialized;
    unsigned long lastSaveTime;
    String settingsFilename;
    
    void setDefaultSettings();
    bool validateSettings();
    JsonDocument settingsToJson();
    bool jsonToSettings(const JsonDocument& doc);
};

extern SettingsManager settingsManager;

#endif
