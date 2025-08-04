#ifndef IRMODULE_H
#define IRMODULE_H

#include <Arduino.h>
#include <ArduinoJson.h>

// IR pin definitions
#define IR_RECEIVER_PIN 7
#define IR_LED_PIN      8

// IR protocols
enum IRProtocol {
    IR_UNKNOWN = 0,
    IR_NEC,
    IR_SONY,
    IR_RC5,
    IR_RC6,
    IR_SAMSUNG,
    IR_LG,
    IR_RAW
};

// IR signal structure
struct IRSignal {
    IRProtocol protocol;
    String name;
    uint32_t command;
    uint32_t address;
    uint16_t* rawData;
    uint16_t rawLength;
    uint16_t frequency;
    unsigned long timestamp;
};

class IRModule {
public:
    IRModule();
    bool init();
    void update();
    
    // Receiving functions
    bool receiveSignal();
    bool decodeSignal(IRSignal* signal);
    String getProtocolString(IRProtocol protocol);
    
    // Transmitting functions
    bool transmitSignal(const IRSignal* signal);
    bool transmitNEC(uint32_t address, uint32_t command);
    bool transmitSony(uint32_t data, int nbits);
    bool transmitRaw(uint16_t* data, uint16_t length, uint16_t frequency);
    
    // Data management
    bool saveSignal(const IRSignal* signal);
    bool loadSignal(const String& filename, IRSignal* signal);
    void deleteSignal(const String& filename);
    int getSignalCount();
    String getSignalFilename(int index);
    
    // History management
    void addToHistory(const IRSignal* signal);
    void clearHistory();
    int getHistoryCount();
    IRSignal getHistoryItem(int index);
    
    // Status
    bool isReceiving();
    bool isTransmitting();
    bool isInitialized() { return irInitialized; }

private:
    bool irInitialized;
    IRSignal currentSignal;
    bool signalReceived;
    bool isReceivingSignal;
    bool isTransmittingSignal;
    unsigned long lastReceiveTime;
    
    // Raw data buffer
    static const int MAX_RAW_LENGTH = 300;
    uint16_t rawBuffer[MAX_RAW_LENGTH];
    volatile uint16_t rawIndex;
    volatile unsigned long lastEdgeTime;
    
    // History storage
    static const int MAX_HISTORY = 50;
    IRSignal history[MAX_HISTORY];
    int historyCount;
    int historyIndex;
    
    // Protocol decoders
    bool decodeNEC(IRSignal* signal);
    bool decodeSony(IRSignal* signal);
    bool decodeRC5(IRSignal* signal);
    bool decodeRC6(IRSignal* signal);
    bool decodeSamsung(IRSignal* signal);
    bool decodeLG(IRSignal* signal);
    
    // Helper functions
    void startReceiving();
    void stopReceiving();
    void captureRawData();
    String generateSignalName(IRProtocol protocol, uint32_t command);
    static void IRAM_ATTR irInterruptHandler();
};

extern IRModule irModule;

#endif
