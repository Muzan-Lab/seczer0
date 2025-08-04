#ifndef IBUTTONMODULE_H
#define IBUTTONMODULE_H

#include <Arduino.h>
#include <OneWire.h>
#include <ArduinoJson.h>

// iButton pin definition
#define IBUTTON_PIN 11

// iButton family codes
enum iButtonFamily {
    IBUTTON_UNKNOWN = 0x00,
    IBUTTON_DS1990A = 0x01,  // Serial number only
    IBUTTON_DS1991 = 0x02,   // MultiKey
    IBUTTON_DS1994 = 0x04,   // 4K NVRAM + Clock
    IBUTTON_DS1992 = 0x08,   // 1K NVRAM
    IBUTTON_DS1993 = 0x0C,   // 4K NVRAM
    IBUTTON_DS1996 = 0x0C,   // 64K NVRAM
    IBUTTON_DS1982 = 0x09,   // 1K EPROM
    IBUTTON_DS1985 = 0x0B,   // 16K NVRAM
    IBUTTON_DS1986 = 0x0F    // 64K NVRAM
};

// iButton data structure
struct iButtonKey {
    String name;
    byte address[8];
    iButtonFamily family;
    byte data[8192];  // Maximum data size
    size_t dataSize;
    bool hasData;
    unsigned long timestamp;
};

class iButtonModule {
public:
    iButtonModule();
    bool init();
    void update();
    
    // Scanning functions
    bool scanForKey();
    bool readKey(iButtonKey* key);
    String getFamilyString(iButtonFamily family);
    
    // Emulation functions
    bool emulateKey(const iButtonKey* key);
    void stopEmulation();
    
    // Data management
    bool saveKey(const iButtonKey* key);
    bool loadKey(const String& filename, iButtonKey* key);
    void deleteKey(const String& filename);
    int getKeyCount();
    String getKeyFilename(int index);
    
    // History management
    void addToHistory(const iButtonKey* key);
    void clearHistory();
    int getHistoryCount();
    iButtonKey getHistoryItem(int index);
    
    // Status
    bool isKeyPresent();
    bool isInitialized() { return iButtonInitialized; }

private:
    OneWire oneWire;
    bool iButtonInitialized;
    iButtonKey currentKey;
    bool keyPresent;
    unsigned long lastScanTime;
    
    // History storage
    static const int MAX_HISTORY = 50;
    iButtonKey history[MAX_HISTORY];
    int historyCount;
    int historyIndex;
    
    // Helper functions
    iButtonFamily identifyFamily(byte familyCode);
    bool readKeyData(byte* address, iButtonKey* key);
    bool writeKeyData(byte* address, const iButtonKey* key);
    String generateKeyName(byte* address, iButtonFamily family);
    String formatAddress(byte* address);
    uint8_t calculateCRC8(byte* data, int length);
    bool verifyKey(byte* address);
};

extern iButtonModule iButtonModule;

#endif
