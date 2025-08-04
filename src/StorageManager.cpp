#include "StorageManager.h"

StorageManager storageManager;

StorageManager::StorageManager() 
    : storageInitialized(false), sdMounted(false), lastError(""), lastUpdate(0) {
}

bool StorageManager::init() {
    // Initialize SPI for SD card
    SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    
    // Try to mount SD card
    if (!mountSDCard()) {
        Serial.println("SD Card mount failed, continuing without storage");
        setError("SD Card not available");
        // Don't return false - allow system to continue without SD
    }
    
    // Create default directories if SD is available
    if (sdMounted) {
        createDefaultDirectories();
    }
    
    storageInitialized = true;
    Serial.println("Storage manager initialized");
    
    return true;
}

void StorageManager::update() {
    unsigned long currentTime = millis();
    
    // Check SD card status periodically (every 5 seconds)
    if (currentTime - lastUpdate > 5000) {
        bool wasMonted = sdMounted;
        
        // Check if SD card is still present
        if (sdMounted && !SD.exists("/")) {
            sdMounted = false;
            setError("SD Card removed");
            Serial.println("SD Card removed");
        } else if (!sdMounted && SD.begin(SD_CS_PIN)) {
            sdMounted = true;
            lastError = "";
            createDefaultDirectories();
            Serial.println("SD Card inserted");
        }
        
        lastUpdate = currentTime;
    }
}

bool StorageManager::isSDCardPresent() {
    return SD.exists("/");
}

bool StorageManager::mountSDCard() {
    if (SD.begin(SD_CS_PIN)) {
        sdMounted = true;
        lastError = "";
        Serial.println("SD Card mounted successfully");
        
        // Print card info
        uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        Serial.printf("SD Card Size: %lluMB\n", cardSize);
        
        return true;
    } else {
        sdMounted = false;
        setError("SD Card mount failed");
        Serial.println("SD Card mount failed");
        return false;
    }
}

void StorageManager::unmountSDCard() {
    if (sdMounted) {
        SD.end();
        sdMounted = false;
        Serial.println("SD Card unmounted");
    }
}

uint64_t StorageManager::getTotalSpace() {
    if (!sdMounted) return 0;
    return SD.cardSize();
}

uint64_t StorageManager::getUsedSpace() {
    if (!sdMounted) return 0;
    return SD.usedBytes();
}

uint64_t StorageManager::getFreeSpace() {
    if (!sdMounted) return 0;
    return SD.totalBytes() - SD.usedBytes();
}

bool StorageManager::createDirectory(const String& path) {
    if (!sdMounted) {
        setError("SD Card not available");
        return false;
    }
    
    if (SD.mkdir(path)) {
        Serial.println("Directory created: " + path);
        return true;
    } else {
        setError("Failed to create directory: " + path);
        return false;
    }
}

bool StorageManager::deleteDirectory(const String& path) {
    if (!sdMounted) {
        setError("SD Card not available");
        return false;
    }
    
    if (SD.rmdir(path)) {
        Serial.println("Directory deleted: " + path);
        return true;
    } else {
        setError("Failed to delete directory: " + path);
        return false;
    }
}

bool StorageManager::directoryExists(const String& path) {
    if (!sdMounted) return false;
    
    File dir = SD.open(path);
    if (dir && dir.isDirectory()) {
        dir.close();
        return true;
    }
    return false;
}

int StorageManager::getFileCount(const String& directory) {
    if (!sdMounted) return 0;
    
    File dir = SD.open(directory);
    if (!dir || !dir.isDirectory()) {
        return 0;
    }
    
    int count = 0;
    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            count++;
        }
        file.close();
        file = dir.openNextFile();
    }
    
    dir.close();
    return count;
}

String StorageManager::getFileName(const String& directory, int index) {
    if (!sdMounted) return "";
    
    File dir = SD.open(directory);
    if (!dir || !dir.isDirectory()) {
        return "";
    }
    
    int count = 0;
    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            if (count == index) {
                String filename = file.name();
                file.close();
                dir.close();
                return filename;
            }
            count++;
        }
        file.close();
        file = dir.openNextFile();
    }
    
    dir.close();
    return "";
}

bool StorageManager::writeFile(const String& path, const String& data) {
    if (!sdMounted) {
        setError("SD Card not available");
        return false;
    }
    
    // Ensure directory exists
    String parentDir = getParentDirectory(path);
    if (!directoryExists(parentDir)) {
        if (!createDirectory(parentDir)) {
            return false;
        }
    }
    
    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        setError("Failed to open file for writing: " + path);
        return false;
    }
    
    size_t bytesWritten = file.print(data);
    file.close();
    
    if (bytesWritten == data.length()) {
        return true;
    } else {
        setError("Failed to write complete data to file: " + path);
        return false;
    }
}

bool StorageManager::readFile(const String& path, String& data) {
    if (!sdMounted) {
        setError("SD Card not available");
        return false;
    }
    
    if (!fileExists(path)) {
        setError("File does not exist: " + path);
        return false;
    }
    
    File file = SD.open(path, FILE_READ);
    if (!file) {
        setError("Failed to open file for reading: " + path);
        return false;
    }
    
    data = file.readString();
    file.close();
    
    return true;
}

bool StorageManager::appendFile(const String& path, const String& data) {
    if (!sdMounted) {
        setError("SD Card not available");
        return false;
    }
    
    // Ensure directory exists
    String parentDir = getParentDirectory(path);
    if (!directoryExists(parentDir)) {
        if (!createDirectory(parentDir)) {
            return false;
        }
    }
    
    File file = SD.open(path, FILE_APPEND);
    if (!file) {
        setError("Failed to open file for appending: " + path);
        return false;
    }
    
    size_t bytesWritten = file.print(data);
    file.close();
    
    return (bytesWritten == data.length());
}

bool StorageManager::deleteFile(const String& path) {
    if (!sdMounted) {
        setError("SD Card not available");
        return false;
    }
    
    if (SD.remove(path)) {
        Serial.println("File deleted: " + path);
        return true;
    } else {
        setError("Failed to delete file: " + path);
        return false;
    }
}

bool StorageManager::fileExists(const String& path) {
    if (!sdMounted) return false;
    return SD.exists(path);
}

size_t StorageManager::getFileSize(const String& path) {
    if (!sdMounted || !fileExists(path)) return 0;
    
    File file = SD.open(path, FILE_READ);
    if (!file) return 0;
    
    size_t size = file.size();
    file.close();
    return size;
}

bool StorageManager::writeJsonFile(const String& path, const JsonDocument& doc) {
    String jsonString;
    serializeJson(doc, jsonString);
    return writeFile(path, jsonString);
}

bool StorageManager::readJsonFile(const String& path, JsonDocument& doc) {
    String jsonString;
    if (!readFile(path, jsonString)) {
        return false;
    }
    
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error) {
        setError("JSON parse error: " + String(error.c_str()));
        return false;
    }
    
    return true;
}

bool StorageManager::writeBinaryFile(const String& path, const uint8_t* data, size_t size) {
    if (!sdMounted || !data) {
        setError("SD Card not available or invalid data");
        return false;
    }
    
    // Ensure directory exists
    String parentDir = getParentDirectory(path);
    if (!directoryExists(parentDir)) {
        if (!createDirectory(parentDir)) {
            return false;
        }
    }
    
    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        setError("Failed to open file for binary writing: " + path);
        return false;
    }
    
    size_t bytesWritten = file.write(data, size);
    file.close();
    
    return (bytesWritten == size);
}

bool StorageManager::readBinaryFile(const String& path, uint8_t* data, size_t& size) {
    if (!sdMounted || !data) {
        setError("SD Card not available or invalid buffer");
        return false;
    }
    
    if (!fileExists(path)) {
        setError("File does not exist: " + path);
        return false;
    }
    
    File file = SD.open(path, FILE_READ);
    if (!file) {
        setError("Failed to open file for binary reading: " + path);
        return false;
    }
    
    size_t fileSize = file.size();
    if (fileSize > size) {
        file.close();
        setError("Buffer too small for file: " + path);
        return false;
    }
    
    size_t bytesRead = file.read(data, fileSize);
    size = bytesRead;
    file.close();
    
    return (bytesRead == fileSize);
}

bool StorageManager::backupSettings() {
    if (!sdMounted) return false;
    
    String backupPath = BACKUP_DIR "/settings_backup_" + String(millis()) + ".json";
    
    // Copy current settings file
    String settingsData;
    if (readFile(SETTINGS_DIR "/settings.json", settingsData)) {
        return writeFile(backupPath, settingsData);
    }
    
    return false;
}

bool StorageManager::restoreSettings() {
    // Find most recent settings backup
    if (!sdMounted) return false;
    
    // This is a simplified implementation
    // In a full implementation, you would scan the backup directory
    // and find the most recent backup file
    
    setError("Settings restore not fully implemented");
    return false;
}

bool StorageManager::backupModuleData(const String& module) {
    if (!sdMounted) return false;
    
    String backupDir = BACKUP_DIR "/" + module + "_" + String(millis());
    
    // This would copy all files from the module directory to backup
    setError("Module backup not fully implemented");
    return false;
}

bool StorageManager::restoreModuleData(const String& module) {
    if (!sdMounted) return false;
    
    setError("Module restore not fully implemented");
    return false;
}

void StorageManager::logMessage(const String& message, const String& level) {
    if (!sdMounted) return;
    
    String logEntry = formatLogEntry(message, level);
    String logPath = LOGS_DIR "/system.log";
    
    appendFile(logPath, logEntry);
}

void StorageManager::logError(const String& error) {
    logMessage(error, "ERROR");
    Serial.println("ERROR: " + error);
}

void StorageManager::logDebug(const String& debug) {
    logMessage(debug, "DEBUG");
    if (Serial) {
        Serial.println("DEBUG: " + debug);
    }
}

bool StorageManager::clearLogs() {
    if (!sdMounted) return false;
    
    String logPath = LOGS_DIR "/system.log";
    return deleteFile(logPath);
}

String StorageManager::getLogContents(int maxLines) {
    if (!sdMounted) return "SD Card not available";
    
    String logPath = LOGS_DIR "/system.log";
    String logData;
    
    if (!readFile(logPath, logData)) {
        return "No log file found";
    }
    
    // Simple implementation - return last part of log
    // In a full implementation, you would parse lines and return last N lines
    if (logData.length() > 2000) {
        return logData.substring(logData.length() - 2000);
    }
    
    return logData;
}

void StorageManager::cleanupOldFiles(const String& directory, int maxFiles) {
    if (!sdMounted) return;
    
    // This would scan directory and delete oldest files if count exceeds maxFiles
    // Implementation would involve sorting files by timestamp
    Serial.println("File cleanup not fully implemented");
}

void StorageManager::cleanupOldLogs(int maxDays) {
    if (!sdMounted) return;
    
    // This would delete log files older than maxDays
    Serial.println("Log cleanup not fully implemented");
}

void StorageManager::setError(const String& error) {
    lastError = error;
    if (error.length() > 0) {
        Serial.println("Storage Error: " + error);
    }
}

bool StorageManager::ensureDirectoryExists(const String& path) {
    if (directoryExists(path)) {
        return true;
    }
    return createDirectory(path);
}

String StorageManager::getParentDirectory(const String& path) {
    int lastSlash = path.lastIndexOf('/');
    if (lastSlash > 0) {
        return path.substring(0, lastSlash);
    }
    return "/";
}

String StorageManager::getFileExtension(const String& filename) {
    int lastDot = filename.lastIndexOf('.');
    if (lastDot > 0) {
        return filename.substring(lastDot);
    }
    return "";
}

bool StorageManager::isValidFilename(const String& filename) {
    // Check for invalid characters
    if (filename.length() == 0 || filename.length() > 255) {
        return false;
    }
    
    // Check for invalid characters (simplified check)
    if (filename.indexOf('<') >= 0 || filename.indexOf('>') >= 0 ||
        filename.indexOf(':') >= 0 || filename.indexOf('"') >= 0 ||
        filename.indexOf('|') >= 0 || filename.indexOf('?') >= 0 ||
        filename.indexOf('*') >= 0) {
        return false;
    }
    
    return true;
}

void StorageManager::createDefaultDirectories() {
    if (!sdMounted) return;
    
    // Create all default directories
    ensureDirectoryExists(SETTINGS_DIR);
    ensureDirectoryExists(NFC_DIR);
    ensureDirectoryExists(IR_DIR);
    ensureDirectoryExists(IBUTTON_DIR);
    ensureDirectoryExists(RF_DIR);
    ensureDirectoryExists(GPIO_DIR);
    ensureDirectoryExists(LOGS_DIR);
    ensureDirectoryExists(BACKUP_DIR);
    
    Serial.println("Default directories created");
}

String StorageManager::formatLogEntry(const String& message, const String& level) {
    String timestamp = getCurrentTimestamp();
    return "[" + timestamp + "] " + level + ": " + message + "\n";
}

String StorageManager::getCurrentTimestamp() {
    // Simple timestamp based on millis()
    // In a full implementation, you might use RTC
    unsigned long ms = millis();
    unsigned long seconds = ms / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    
    return String(hours % 24) + ":" + 
           String((minutes % 60) < 10 ? "0" : "") + String(minutes % 60) + ":" +
           String((seconds % 60) < 10 ? "0" : "") + String(seconds % 60);
}
