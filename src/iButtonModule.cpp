#include "iButtonModule.h"
#include "StorageManager.h"

iButtonModule iButtonModule;

iButtonModule::iButtonModule() 
    : oneWire(IBUTTON_PIN), iButtonInitialized(false), keyPresent(false),
      lastScanTime(0), historyCount(0), historyIndex(0) {
}

bool iButtonModule::init() {
    // OneWire bus should be initialized automatically
    iButtonInitialized = true;
    Serial.println("iButton module initialized");
    return true;
}

void iButtonModule::update() {
    if (!iButtonInitialized) return;
    
    unsigned long currentTime = millis();
    
    // Check for keys every 500ms
    if (currentTime - lastScanTime > 500) {
        keyPresent = oneWire.search(currentKey.address);
        lastScanTime = currentTime;
    }
}

bool iButtonModule::scanForKey() {
    if (!iButtonInitialized) return false;
    
    oneWire.reset_search();
    return oneWire.search(currentKey.address);
}

bool iButtonModule::readKey(iButtonKey* key) {
    if (!iButtonInitialized || !key) return false;
    
    if (!scanForKey()) return false;
    
    // Copy address
    memcpy(key->address, currentKey.address, 8);
    
    // Verify CRC
    if (OneWire::crc8(key->address, 7) != key->address[7]) {
        Serial.println("CRC error in iButton address");
        return false;
    }
    
    // Identify family
    key->family = identifyFamily(key->address[0]);
    
    // Generate key name
    key->name = generateKeyName(key->address, key->family);
    
    // Read key data based on family
    key->hasData = readKeyData(key->address, key);
    
    key->timestamp = millis();
    
    // Add to history
    addToHistory(key);
    
    return true;
}

String iButtonModule::getFamilyString(iButtonFamily family) {
    switch (family) {
        case IBUTTON_DS1990A: return "DS1990A (ID Only)";
        case IBUTTON_DS1991: return "DS1991 (MultiKey)";
        case IBUTTON_DS1994: return "DS1994 (4K + Clock)";
        case IBUTTON_DS1992: return "DS1992 (1K NVRAM)";
        case IBUTTON_DS1993: return "DS1993 (4K NVRAM)";
        case IBUTTON_DS1996: return "DS1996 (64K NVRAM)";
        case IBUTTON_DS1982: return "DS1982 (1K EPROM)";
        case IBUTTON_DS1985: return "DS1985 (16K NVRAM)";
        case IBUTTON_DS1986: return "DS1986 (64K NVRAM)";
        default: return "Unknown";
    }
}

bool iButtonModule::emulateKey(const iButtonKey* key) {
    // iButton emulation is complex and requires precise timing
    // This is a placeholder implementation
    if (!iButtonInitialized || !key) return false;
    
    Serial.println("iButton emulation not fully implemented");
    return false;
}

void iButtonModule::stopEmulation() {
    // Stop any ongoing emulation
}

bool iButtonModule::saveKey(const iButtonKey* key) {
    if (!key) return false;
    
    String filename = IBUTTON_DIR + String("/") + key->name + IBUTTON_EXT;
    
    JsonDocument doc;
    doc["name"] = key->name;
    doc["family"] = (int)key->family;
    doc["hasData"] = key->hasData;
    doc["dataSize"] = key->dataSize;
    doc["timestamp"] = key->timestamp;
    
    // Convert address to hex string
    String addressHex = "";
    for (int i = 0; i < 8; i++) {
        if (key->address[i] < 16) addressHex += "0";
        addressHex += String(key->address[i], HEX);
    }
    doc["address"] = addressHex;
    
    // Convert data to hex string if present
    if (key->hasData && key->dataSize > 0) {
        String dataHex = "";
        for (size_t i = 0; i < key->dataSize; i++) {
            if (key->data[i] < 16) dataHex += "0";
            dataHex += String(key->data[i], HEX);
        }
        doc["data"] = dataHex;
    }
    
    return storageManager.writeJsonFile(filename, doc);
}

bool iButtonModule::loadKey(const String& filename, iButtonKey* key) {
    if (!key) return false;
    
    JsonDocument doc;
    if (!storageManager.readJsonFile(filename, doc)) {
        return false;
    }
    
    key->name = doc["name"].as<String>();
    key->family = (iButtonFamily)doc["family"].as<int>();
    key->hasData = doc["hasData"].as<bool>();
    key->dataSize = doc["dataSize"].as<size_t>();
    key->timestamp = doc["timestamp"].as<unsigned long>();
    
    // Convert hex string back to address
    String addressHex = doc["address"].as<String>();
    for (int i = 0; i < 8; i++) {
        String hexByte = addressHex.substring(i * 2, i * 2 + 2);
        key->address[i] = strtol(hexByte.c_str(), NULL, 16);
    }
    
    // Convert hex string back to data if present
    if (key->hasData && doc.containsKey("data")) {
        String dataHex = doc["data"].as<String>();
        for (size_t i = 0; i < key->dataSize && i < sizeof(key->data); i++) {
            String hexByte = dataHex.substring(i * 2, i * 2 + 2);
            key->data[i] = strtol(hexByte.c_str(), NULL, 16);
        }
    }
    
    return true;
}

void iButtonModule::deleteKey(const String& filename) {
    storageManager.deleteFile(filename);
}

int iButtonModule::getKeyCount() {
    return storageManager.getFileCount(IBUTTON_DIR);
}

String iButtonModule::getKeyFilename(int index) {
    return storageManager.getFileName(IBUTTON_DIR, index);
}

void iButtonModule::addToHistory(const iButtonKey* key) {
    if (!key) return;
    
    history[historyIndex] = *key;
    historyIndex = (historyIndex + 1) % MAX_HISTORY;
    if (historyCount < MAX_HISTORY) {
        historyCount++;
    }
}

void iButtonModule::clearHistory() {
    historyCount = 0;
    historyIndex = 0;
}

int iButtonModule::getHistoryCount() {
    return historyCount;
}

iButtonKey iButtonModule::getHistoryItem(int index) {
    if (index >= 0 && index < historyCount) {
        int realIndex = (historyIndex - historyCount + index + MAX_HISTORY) % MAX_HISTORY;
        return history[realIndex];
    }
    return iButtonKey();
}

bool iButtonModule::isKeyPresent() {
    return keyPresent;
}

iButtonFamily iButtonModule::identifyFamily(byte familyCode) {
    switch (familyCode) {
        case 0x01: return IBUTTON_DS1990A;
        case 0x02: return IBUTTON_DS1991;
        case 0x04: return IBUTTON_DS1994;
        case 0x08: return IBUTTON_DS1992;
        case 0x0C: return IBUTTON_DS1993;
        case 0x0C: return IBUTTON_DS1996; // Same family code as DS1993
        case 0x09: return IBUTTON_DS1982;
        case 0x0B: return IBUTTON_DS1985;
        case 0x0F: return IBUTTON_DS1986;
        default: return IBUTTON_UNKNOWN;
    }
}

bool iButtonModule::readKeyData(byte* address, iButtonKey* key) {
    if (!address || !key) return false;
    
    iButtonFamily family = identifyFamily(address[0]);
    
    switch (family) {
        case IBUTTON_DS1990A:
            // ID-only device, no additional data
            key->dataSize = 0;
            return true;
            
        case IBUTTON_DS1992:
        case IBUTTON_DS1993:
        case IBUTTON_DS1994:
        case IBUTTON_DS1996:
            // NVRAM devices - read memory
            // This is a simplified implementation
            oneWire.reset();
            oneWire.select(address);
            oneWire.write(0xF0); // Read Memory command
            oneWire.write(0x00); // Address low
            oneWire.write(0x00); // Address high
            
            // Read first 256 bytes (or until device stops responding)
            key->dataSize = 0;
            for (int i = 0; i < 256 && i < sizeof(key->data); i++) {
                key->data[i] = oneWire.read();
                key->dataSize++;
            }
            return true;
            
        default:
            // Unknown or unsupported family
            key->dataSize = 0;
            return false;
    }
}

bool iButtonModule::writeKeyData(byte* address, const iButtonKey* key) {
    // Writing to iButton devices requires specific commands and is family-dependent
    // This is a placeholder implementation
    if (!address || !key) return false;
    
    Serial.println("iButton write not fully implemented");
    return false;
}

String iButtonModule::generateKeyName(byte* address, iButtonFamily family) {
    String familyName = "";
    switch (family) {
        case IBUTTON_DS1990A: familyName = "DS1990A"; break;
        case IBUTTON_DS1991: familyName = "DS1991"; break;
        case IBUTTON_DS1994: familyName = "DS1994"; break;
        case IBUTTON_DS1992: familyName = "DS1992"; break;
        case IBUTTON_DS1993: familyName = "DS1993"; break;
        default: familyName = "Unknown"; break;
    }
    
    String addressStr = formatAddress(address);
    return familyName + "_" + addressStr.substring(0, 8);
}

String iButtonModule::formatAddress(byte* address) {
    String addressString = "";
    for (int i = 0; i < 8; i++) {
        if (address[i] < 16) addressString += "0";
        addressString += String(address[i], HEX);
        if (i < 7) addressString += ":";
    }
    addressString.toUpperCase();
    return addressString;
}

uint8_t iButtonModule::calculateCRC8(byte* data, int length) {
    // Use OneWire library's CRC calculation
    return OneWire::crc8(data, length);
}

bool iButtonModule::verifyKey(byte* address) {
    if (!address) return false;
    
    // Verify CRC
    return (OneWire::crc8(address, 7) == address[7]);
}
