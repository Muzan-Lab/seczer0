#include "SettingsManager.h"
#include "StorageManager.h"

SettingsManager settingsManager;

SettingsManager::SettingsManager() 
    : settingsInitialized(false), lastSaveTime(0), settingsFilename(SETTINGS_DIR "/settings.json") {
}

bool SettingsManager::init() {
    // Set default settings first
    setDefaultSettings();
    
    // Try to load settings from storage
    if (!loadSettings()) {
        Serial.println("Could not load settings, using defaults");
        // Save default settings
        saveSettings();
    }
    
    settingsInitialized = true;
    Serial.println("Settings manager initialized");
    
    return true;
}

bool SettingsManager::loadSettings() {
    JsonDocument doc;
    if (!storageManager.readJsonFile(settingsFilename, doc)) {
        return false;
    }
    
    return jsonToSettings(doc);
}

bool SettingsManager::saveSettings() {
    if (!settingsInitialized) return false;
    
    JsonDocument doc = settingsToJson();
    
    bool success = storageManager.writeJsonFile(settingsFilename, doc);
    if (success) {
        lastSaveTime = millis();
        Serial.println("Settings saved successfully");
    } else {
        Serial.println("Failed to save settings");
    }
    
    return success;
}

void SettingsManager::resetToDefaults() {
    setDefaultSettings();
    saveSettings();
    Serial.println("Settings reset to defaults");
}

void SettingsManager::setBrightness(uint8_t brightness) {
    settings.brightness = constrain(brightness, 0, 255);
    if (isInitialized()) saveSettings();
}

void SettingsManager::setVolume(uint8_t volume) {
    settings.volume = constrain(volume, 0, 100);
    if (isInitialized()) saveSettings();
}

void SettingsManager::setSoundEnabled(bool enabled) {
    settings.soundEnabled = enabled;
    if (isInitialized()) saveSettings();
}

void SettingsManager::setAutoSaveEnabled(bool enabled) {
    settings.autoSave = enabled;
    if (isInitialized()) saveSettings();
}

void SettingsManager::setDebugMode(bool enabled) {
    settings.debugMode = enabled;
    if (isInitialized()) saveSettings();
}

void SettingsManager::setDeviceName(const String& name) {
    settings.deviceName = name;
    if (isInitialized()) saveSettings();
}

void SettingsManager::setContrast(uint8_t contrast) {
    settings.contrast = constrain(contrast, 0, 255);
    if (isInitialized()) saveSettings();
}

void SettingsManager::setAutoSleep(bool enabled) {
    settings.autoSleep = enabled;
    if (isInitialized()) saveSettings();
}

void SettingsManager::setSleepTimeout(uint16_t timeout) {
    settings.sleepTimeout = timeout;
    if (isInitialized()) saveSettings();
}

void SettingsManager::setInvertDisplay(bool invert) {
    settings.invertDisplay = invert;
    if (isInitialized()) saveSettings();
}

void SettingsManager::setBeepFrequency(uint16_t frequency) {
    settings.beepFrequency = frequency;
    if (isInitialized()) saveSettings();
}

void SettingsManager::setBeepDuration(uint16_t duration) {
    settings.beepDuration = duration;
    if (isInitialized()) saveSettings();
}

void SettingsManager::setMaxHistoryItems(uint16_t maxItems) {
    settings.maxHistoryItems = maxItems;
    if (isInitialized()) saveSettings();
}

void SettingsManager::setCompressData(bool compress) {
    settings.compressData = compress;
    if (isInitialized()) saveSettings();
}

void SettingsManager::setWiFiEnabled(bool enabled) {
    settings.wifiEnabled = enabled;
    if (isInitialized()) saveSettings();
}

void SettingsManager::setWiFiCredentials(const String& ssid, const String& password) {
    settings.wifiSSID = ssid;
    settings.wifiPassword = password;
    if (isInitialized()) saveSettings();
}

void SettingsManager::setModuleEnabled(const String& module, bool enabled) {
    if (module == "nfc") {
        settings.nfcEnabled = enabled;
    } else if (module == "ir") {
        settings.irEnabled = enabled;
    } else if (module == "ibutton") {
        settings.iButtonEnabled = enabled;
    } else if (module == "rf") {
        settings.rfEnabled = enabled;
    } else if (module == "gpio") {
        settings.gpioEnabled = enabled;
    }
    
    if (isInitialized()) saveSettings();
}

bool SettingsManager::isModuleEnabled(const String& module) {
    if (module == "nfc") {
        return settings.nfcEnabled;
    } else if (module == "ir") {
        return settings.irEnabled;
    } else if (module == "ibutton") {
        return settings.iButtonEnabled;
    } else if (module == "rf") {
        return settings.rfEnabled;
    } else if (module == "gpio") {
        return settings.gpioEnabled;
    }
    
    return false;
}

void SettingsManager::setDefaultSettings() {
    // Display settings
    settings.brightness = 128;
    settings.contrast = 128;
    settings.autoSleep = true;
    settings.sleepTimeout = 30000; // 30 seconds
    settings.invertDisplay = false;
    
    // Sound settings
    settings.soundEnabled = true;
    settings.volume = 50;
    settings.beepFrequency = 1000;
    settings.beepDuration = 100;
    
    // Storage settings
    settings.autoSave = true;
    settings.maxHistoryItems = 50;
    settings.compressData = false;
    
    // System settings
    settings.deviceName = "FlipperS3";
    settings.debugMode = false;
    settings.baudRate = 115200;
    settings.wifiEnabled = false;
    settings.wifiSSID = "";
    settings.wifiPassword = "";
    
    // Module settings
    settings.nfcEnabled = true;
    settings.irEnabled = true;
    settings.iButtonEnabled = true;
    settings.rfEnabled = true;
    settings.gpioEnabled = true;
}

bool SettingsManager::validateSettings() {
    // Validate and correct any out-of-range values
    settings.brightness = constrain(settings.brightness, 0, 255);
    settings.contrast = constrain(settings.contrast, 0, 255);
    settings.volume = constrain(settings.volume, 0, 100);
    
    if (settings.sleepTimeout < 5000) settings.sleepTimeout = 5000; // Minimum 5 seconds
    if (settings.sleepTimeout > 300000) settings.sleepTimeout = 300000; // Maximum 5 minutes
    
    if (settings.beepFrequency < 100) settings.beepFrequency = 100;
    if (settings.beepFrequency > 5000) settings.beepFrequency = 5000;
    
    if (settings.beepDuration < 50) settings.beepDuration = 50;
    if (settings.beepDuration > 2000) settings.beepDuration = 2000;
    
    if (settings.maxHistoryItems < 10) settings.maxHistoryItems = 10;
    if (settings.maxHistoryItems > 1000) settings.maxHistoryItems = 1000;
    
    if (settings.deviceName.length() == 0) settings.deviceName = "FlipperS3";
    if (settings.deviceName.length() > 32) settings.deviceName = settings.deviceName.substring(0, 32);
    
    return true;
}

JsonDocument SettingsManager::settingsToJson() {
    JsonDocument doc;
    
    // Display settings
    JsonObject display = doc.createNestedObject("display");
    display["brightness"] = settings.brightness;
    display["contrast"] = settings.contrast;
    display["autoSleep"] = settings.autoSleep;
    display["sleepTimeout"] = settings.sleepTimeout;
    display["invertDisplay"] = settings.invertDisplay;
    
    // Sound settings
    JsonObject sound = doc.createNestedObject("sound");
    sound["enabled"] = settings.soundEnabled;
    sound["volume"] = settings.volume;
    sound["beepFrequency"] = settings.beepFrequency;
    sound["beepDuration"] = settings.beepDuration;
    
    // Storage settings
    JsonObject storage = doc.createNestedObject("storage");
    storage["autoSave"] = settings.autoSave;
    storage["maxHistoryItems"] = settings.maxHistoryItems;
    storage["compressData"] = settings.compressData;
    
    // System settings
    JsonObject system = doc.createNestedObject("system");
    system["deviceName"] = settings.deviceName;
    system["debugMode"] = settings.debugMode;
    system["baudRate"] = settings.baudRate;
    
    // WiFi settings
    JsonObject wifi = doc.createNestedObject("wifi");
    wifi["enabled"] = settings.wifiEnabled;
    wifi["ssid"] = settings.wifiSSID;
    wifi["password"] = settings.wifiPassword;
    
    // Module settings
    JsonObject modules = doc.createNestedObject("modules");
    modules["nfc"] = settings.nfcEnabled;
    modules["ir"] = settings.irEnabled;
    modules["ibutton"] = settings.iButtonEnabled;
    modules["rf"] = settings.rfEnabled;
    modules["gpio"] = settings.gpioEnabled;
    
    return doc;
}

bool SettingsManager::jsonToSettings(const JsonDocument& doc) {
    try {
        // Display settings
        if (doc.containsKey("display")) {
            JsonObject display = doc["display"];
            settings.brightness = display["brightness"] | settings.brightness;
            settings.contrast = display["contrast"] | settings.contrast;
            settings.autoSleep = display["autoSleep"] | settings.autoSleep;
            settings.sleepTimeout = display["sleepTimeout"] | settings.sleepTimeout;
            settings.invertDisplay = display["invertDisplay"] | settings.invertDisplay;
        }
        
        // Sound settings
        if (doc.containsKey("sound")) {
            JsonObject sound = doc["sound"];
            settings.soundEnabled = sound["enabled"] | settings.soundEnabled;
            settings.volume = sound["volume"] | settings.volume;
            settings.beepFrequency = sound["beepFrequency"] | settings.beepFrequency;
            settings.beepDuration = sound["beepDuration"] | settings.beepDuration;
        }
        
        // Storage settings
        if (doc.containsKey("storage")) {
            JsonObject storage = doc["storage"];
            settings.autoSave = storage["autoSave"] | settings.autoSave;
            settings.maxHistoryItems = storage["maxHistoryItems"] | settings.maxHistoryItems;
            settings.compressData = storage["compressData"] | settings.compressData;
        }
        
        // System settings
        if (doc.containsKey("system")) {
            JsonObject system = doc["system"];
            settings.deviceName = system["deviceName"] | settings.deviceName;
            settings.debugMode = system["debugMode"] | settings.debugMode;
            settings.baudRate = system["baudRate"] | settings.baudRate;
        }
        
        // WiFi settings
        if (doc.containsKey("wifi")) {
            JsonObject wifi = doc["wifi"];
            settings.wifiEnabled = wifi["enabled"] | settings.wifiEnabled;
            settings.wifiSSID = wifi["ssid"] | settings.wifiSSID;
            settings.wifiPassword = wifi["password"] | settings.wifiPassword;
        }
        
        // Module settings
        if (doc.containsKey("modules")) {
            JsonObject modules = doc["modules"];
            settings.nfcEnabled = modules["nfc"] | settings.nfcEnabled;
            settings.irEnabled = modules["ir"] | settings.irEnabled;
            settings.iButtonEnabled = modules["ibutton"] | settings.iButtonEnabled;
            settings.rfEnabled = modules["rf"] | settings.rfEnabled;
            settings.gpioEnabled = modules["gpio"] | settings.gpioEnabled;
        }
        
        // Validate loaded settings
        validateSettings();
        
        return true;
    } catch (...) {
        Serial.println("Error parsing settings JSON");
        return false;
    }
}
