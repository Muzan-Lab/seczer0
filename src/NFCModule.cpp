#include "NFCModule.h"
#include "StorageManager.h"

NFCModule nfcModule;

NFCModule::NFCModule() 
    : mfrc522(NFC_SS_PIN, NFC_RST_PIN), nfcInitialized(false), 
      cardPresent(false), lastScanTime(0), historyCount(0), historyIndex(0) {
}

bool NFCModule::init() {
    SPI.begin();
    mfrc522.PCD_Init();
    
    // Check if the MFRC522 is connected
    byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
    if (version == 0x00 || version == 0xFF) {
        Serial.println("MFRC522 not found");
        return false;
    }
    
    nfcInitialized = true;
    Serial.print("MFRC522 initialized, version: 0x");
    Serial.println(version, HEX);
    
    return true;
}

void NFCModule::update() {
    if (!nfcInitialized) return;
    
    unsigned long currentTime = millis();
    
    // Check for cards every 100ms
    if (currentTime - lastScanTime > 100) {
        cardPresent = mfrc522.PICC_IsNewCardPresent();
        lastScanTime = currentTime;
    }
}

bool NFCModule::scanForCard() {
    if (!nfcInitialized) return false;
    
    return mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial();
}

bool NFCModule::readCard(NFCCard* card) {
    if (!nfcInitialized || !card) return false;
    
    if (!scanForCard()) return false;
    
    // Read UID
    card->uid = formatUID(mfrc522.uid.uidByte, mfrc522.uid.size);
    
    // Identify card type
    card->type = identifyCardType(mfrc522.PICC_GetType(mfrc522.uid.sak));
    
    // Generate card name
    card->name = generateCardName(card->uid, card->type);
    
    // Read card data based on type
    card->dataSize = 0;
    
    switch (card->type) {
        case NFC_MIFARE_CLASSIC:
            // Read Mifare Classic data
            for (byte sector = 0; sector < 16; sector++) {
                MFRC522::MIFARE_Key key;
                // Try default keys
                for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
                
                if (authenticateSector(sector, &key)) {
                    // Read blocks in this sector
                    for (byte block = 0; block < 4; block++) {
                        byte blockAddr = sector * 4 + block;
                        if (blockAddr % 4 == 3) continue; // Skip trailer blocks
                        
                        byte buffer[18];
                        byte size = sizeof(buffer);
                        
                        if (mfrc522.MIFARE_Read(blockAddr, buffer, &size) == MFRC522::STATUS_OK) {
                            memcpy(&card->data[card->dataSize], buffer, 16);
                            card->dataSize += 16;
                        }
                    }
                }
            }
            break;
            
        case NFC_MIFARE_ULTRALIGHT:
        case NFC_NTAG213:
        case NFC_NTAG215:
        case NFC_NTAG216:
            // Read NTAG/Ultralight data
            for (byte page = 0; page < 64; page++) {
                byte buffer[18];
                byte size = sizeof(buffer);
                
                if (mfrc522.MIFARE_Read(page, buffer, &size) == MFRC522::STATUS_OK) {
                    memcpy(&card->data[card->dataSize], buffer, 4);
                    card->dataSize += 4;
                } else {
                    break; // End of readable area
                }
            }
            break;
            
        default:
            // Unknown card type, just store UID
            memcpy(card->data, mfrc522.uid.uidByte, mfrc522.uid.size);
            card->dataSize = mfrc522.uid.size;
            break;
    }
    
    card->timestamp = millis();
    
    // Add to history
    addToHistory(card);
    
    // Stop communication with card
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    
    return true;
}

String NFCModule::getCardTypeString(NFCCardType type) {
    switch (type) {
        case NFC_MIFARE_CLASSIC: return "MIFARE Classic";
        case NFC_MIFARE_ULTRALIGHT: return "MIFARE Ultralight";
        case NFC_NTAG213: return "NTAG213";
        case NFC_NTAG215: return "NTAG215";
        case NFC_NTAG216: return "NTAG216";
        default: return "Unknown";
    }
}

bool NFCModule::emulateCard(const NFCCard* card) {
    // Card emulation is complex and hardware-dependent
    // This is a placeholder implementation
    if (!nfcInitialized || !card) return false;
    
    Serial.println("Card emulation not fully implemented");
    return false;
}

void NFCModule::stopEmulation() {
    // Stop any ongoing emulation
}

bool NFCModule::saveCard(const NFCCard* card) {
    if (!card) return false;
    
    String filename = NFC_DIR + String("/") + card->uid + NFC_EXT;
    
    // Create JSON document
    JsonDocument doc;
    doc["uid"] = card->uid;
    doc["type"] = (int)card->type;
    doc["name"] = card->name;
    doc["timestamp"] = card->timestamp;
    doc["dataSize"] = card->dataSize;
    
    // Convert binary data to hex string
    String hexData = "";
    for (size_t i = 0; i < card->dataSize; i++) {
        if (card->data[i] < 16) hexData += "0";
        hexData += String(card->data[i], HEX);
    }
    doc["data"] = hexData;
    
    return storageManager.writeJsonFile(filename, doc);
}

bool NFCModule::loadCard(const String& filename, NFCCard* card) {
    if (!card) return false;
    
    JsonDocument doc;
    if (!storageManager.readJsonFile(filename, doc)) {
        return false;
    }
    
    card->uid = doc["uid"].as<String>();
    card->type = (NFCCardType)doc["type"].as<int>();
    card->name = doc["name"].as<String>();
    card->timestamp = doc["timestamp"].as<unsigned long>();
    card->dataSize = doc["dataSize"].as<size_t>();
    
    // Convert hex string back to binary data
    String hexData = doc["data"].as<String>();
    for (size_t i = 0; i < card->dataSize && i < sizeof(card->data); i++) {
        String hexByte = hexData.substring(i * 2, i * 2 + 2);
        card->data[i] = strtol(hexByte.c_str(), NULL, 16);
    }
    
    return true;
}

void NFCModule::deleteCard(const String& filename) {
    storageManager.deleteFile(filename);
}

int NFCModule::getCardCount() {
    return storageManager.getFileCount(NFC_DIR);
}

String NFCModule::getCardFilename(int index) {
    return storageManager.getFileName(NFC_DIR, index);
}

void NFCModule::addToHistory(const NFCCard* card) {
    if (!card) return;
    
    // Copy card to history
    history[historyIndex] = *card;
    
    // Update counters
    historyIndex = (historyIndex + 1) % MAX_HISTORY;
    if (historyCount < MAX_HISTORY) {
        historyCount++;
    }
}

void NFCModule::clearHistory() {
    historyCount = 0;
    historyIndex = 0;
}

int NFCModule::getHistoryCount() {
    return historyCount;
}

NFCCard NFCModule::getHistoryItem(int index) {
    if (index >= 0 && index < historyCount) {
        int realIndex = (historyIndex - historyCount + index + MAX_HISTORY) % MAX_HISTORY;
        return history[realIndex];
    }
    return NFCCard(); // Return empty card if index is invalid
}

bool NFCModule::isCardPresent() {
    return cardPresent;
}

NFCCardType NFCModule::identifyCardType(MFRC522::PICC_Type piccType) {
    switch (piccType) {
        case MFRC522::PICC_TYPE_MIFARE_MINI:
        case MFRC522::PICC_TYPE_MIFARE_1K:
        case MFRC522::PICC_TYPE_MIFARE_4K:
            return NFC_MIFARE_CLASSIC;
        case MFRC522::PICC_TYPE_MIFARE_UL:
            return NFC_MIFARE_ULTRALIGHT;
        case MFRC522::PICC_TYPE_TNP3XXX:
            return NFC_NTAG213; // Assume NTAG213 for now
        default:
            return NFC_UNKNOWN;
    }
}

bool NFCModule::authenticateSector(byte sector, MFRC522::MIFARE_Key* key) {
    byte trailerBlock = sector * 4 + 3;
    MFRC522::StatusCode status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, key, &(mfrc522.uid));
    return (status == MFRC522::STATUS_OK);
}

String NFCModule::generateCardName(const String& uid, NFCCardType type) {
    String typeName = getCardTypeString(type);
    return typeName + "_" + uid.substring(0, 8);
}

String NFCModule::formatUID(byte* uid, byte uidSize) {
    String uidString = "";
    for (byte i = 0; i < uidSize; i++) {
        if (uid[i] < 16) uidString += "0";
        uidString += String(uid[i], HEX);
        if (i < uidSize - 1) uidString += ":";
    }
    uidString.toUpperCase();
    return uidString;
}
