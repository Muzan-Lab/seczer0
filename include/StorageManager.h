#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <Arduino.h>
#include <SD.h>
#include <FS.h>
#include <ArduinoJson.h>

// Storage pin definitions
#define SD_CS_PIN    15
#define SD_MOSI_PIN  16
#define SD_MISO_PIN  17
#define SD_SCK_PIN   18

// Directory structure
#define ROOT_DIR        "/"
#define SETTINGS_DIR    "/settings"
#define NFC_DIR         "/nfc"
#define IR_DIR          "/ir"
#define IBUTTON_DIR     "/ibutton"
#define RF_DIR          "/rf"
#define GPIO_DIR        "/gpio"
#define LOGS_DIR        "/logs"
#define BACKUP_DIR      "/backup"

// File extensions
#define SETTINGS_EXT    ".json"
#define NFC_EXT         ".nfc"
#define IR_EXT          ".ir"
#define IBUTTON_EXT     ".ibtn"
#define RF_EXT          ".rf"
#define GPIO_EXT        ".gpio"
#define LOG_EXT         ".log"

class StorageManager {
public:
    StorageManager();
    bool init();
    void update();
    
    // SD Card management
    bool isSDCardPresent();
    bool mountSDCard();
    void unmountSDCard();
    uint64_t getTotalSpace();
    uint64_t getUsedSpace();
    uint64_t getFreeSpace();
    
    // Directory management
    bool createDirectory(const String& path);
    bool deleteDirectory(const String& path);
    bool directoryExists(const String& path);
    int getFileCount(const String& directory);
    String getFileName(const String& directory, int index);
    
    // File operations
    bool writeFile(const String& path, const String& data);
    bool readFile(const String& path, String& data);
    bool appendFile(const String& path, const String& data);
    bool deleteFile(const String& path);
    bool fileExists(const String& path);
    size_t getFileSize(const String& path);
    
    // JSON file operations
    bool writeJsonFile(const String& path, const JsonDocument& doc);
    bool readJsonFile(const String& path, JsonDocument& doc);
    
    // Binary file operations
    bool writeBinaryFile(const String& path, const uint8_t* data, size_t size);
    bool readBinaryFile(const String& path, uint8_t* data, size_t& size);
    
    // Backup and restore
    bool backupSettings();
    bool restoreSettings();
    bool backupModuleData(const String& module);
    bool restoreModuleData(const String& module);
    
    // Logging
    void logMessage(const String& message, const String& level = "INFO");
    void logError(const String& error);
    void logDebug(const String& debug);
    bool clearLogs();
    String getLogContents(int maxLines = 100);
    
    // Cleanup
    void cleanupOldFiles(const String& directory, int maxFiles);
    void cleanupOldLogs(int maxDays);
    
    // Status
    bool isInitialized() { return storageInitialized; }
    bool isSDMounted() { return sdMounted; }
    String getLastError() { return lastError; }

private:
    bool storageInitialized;
    bool sdMounted;
    String lastError;
    unsigned long lastUpdate;
    
    // File system helpers
    void setError(const String& error);
    bool ensureDirectoryExists(const String& path);
    String getParentDirectory(const String& path);
    String getFileExtension(const String& filename);
    bool isValidFilename(const String& filename);
    
    // Directory creation helpers
    void createDefaultDirectories();
    
    // Logging helpers
    String formatLogEntry(const String& message, const String& level);
    String getCurrentTimestamp();
};

extern StorageManager storageManager;

#endif
