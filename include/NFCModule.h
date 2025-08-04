#ifndef NFCMODULE_H
#define NFCMODULE_H

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>

// NFC pin definitions
#define NFC_SS_PIN    10
#define NFC_RST_PIN   9

// NFC card types
enum NFCCardType {
    NFC_UNKNOWN = 0,
    NFC_MIFARE_CLASSIC,
    NFC_MIFARE_ULTRALIGHT,
    NFC_NTAG213,
    NFC_NTAG215,
    NFC_NTAG216
};

// NFC data structure
struct NFCCard {
    String uid;
    NFCCardType type;
    String name;
    byte data[1024];
    size_t dataSize;
    unsigned long timestamp;
};

class NFCModule {
public:
    NFCModule();
    bool init();
    void update();
    
    // Scanning functions
    bool scanForCard();
    bool readCard(NFCCard* card);
    String getCardTypeString(NFCCardType type);
    
    // Emulation functions
    bool emulateCard(const NFCCard* card);
    void stopEmulation();
    
    // Data management
    bool saveCard(const NFCCard* card);
    bool loadCard(const String& filename, NFCCard* card);
    void deleteCard(const String& filename);
    int getCardCount();
    String getCardFilename(int index);
    
    // History management
    void addToHistory(const NFCCard* card);
    void clearHistory();
    int getHistoryCount();
    NFCCard getHistoryItem(int index);
    
    // Status
    bool isCardPresent();
    bool isInitialized() { return nfcInitialized; }

private:
    MFRC522 mfrc522;
    bool nfcInitialized;
    NFCCard currentCard;
    bool cardPresent;
    unsigned long lastScanTime;
    
    // History storage
    static const int MAX_HISTORY = 50;
    NFCCard history[MAX_HISTORY];
    int historyCount;
    int historyIndex;
    
    // Helper functions
    NFCCardType identifyCardType(MFRC522::PICC_Type piccType);
    bool authenticateSector(byte sector, MFRC522::MIFARE_Key* key);
    String generateCardName(const String& uid, NFCCardType type);
    String formatUID(byte* uid, byte uidSize);
};

extern NFCModule nfcModule;

#endif
