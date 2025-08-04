#ifndef RFMODULE_H
#define RFMODULE_H

#include <Arduino.h>
#include <ArduinoJson.h>

// RF pin definitions
#define RF_RECEIVER_PIN 12
#define RF_TRANSMITTER_PIN 13

// RF protocols
enum RFProtocol {
    RF_UNKNOWN = 0,
    RF_ASK_OOK,
    RF_FSK,
    RF_MANCHESTER,
    RF_PWM,
    RF_RAW
};

// RF signal structure
struct RFSignal {
    RFProtocol protocol;
    String name;
    uint32_t frequency;
    uint32_t bitrate;
    uint32_t data;
    uint8_t* rawData;
    size_t rawLength;
    uint8_t modulation;
    unsigned long timestamp;
};

class RFModule {
public:
    RFModule();
    bool init();
    void update();
    
    // Receiving functions
    bool receiveSignal();
    bool decodeSignal(RFSignal* signal);
    String getProtocolString(RFProtocol protocol);
    
    // Transmitting functions
    bool transmitSignal(const RFSignal* signal);
    bool transmitASK(uint32_t data, uint8_t bits, uint32_t frequency);
    bool transmitFSK(uint32_t data, uint8_t bits, uint32_t frequency);
    bool transmitRaw(uint8_t* data, size_t length, uint32_t frequency);
    
    // Frequency scanning
    void startFrequencyScan(uint32_t startFreq, uint32_t endFreq);
    void stopFrequencyScan();
    bool isScanning();
    uint32_t getCurrentScanFrequency();
    
    // Data management
    bool saveSignal(const RFSignal* signal);
    bool loadSignal(const String& filename, RFSignal* signal);
    void deleteSignal(const String& filename);
    int getSignalCount();
    String getSignalFilename(int index);
    
    // History management
    void addToHistory(const RFSignal* signal);
    void clearHistory();
    int getHistoryCount();
    RFSignal getHistoryItem(int index);
    
    // Status
    bool isReceiving();
    bool isTransmitting();
    bool isInitialized() { return rfInitialized; }

private:
    bool rfInitialized;
    RFSignal currentSignal;
    bool signalReceived;
    bool isReceivingSignal;
    bool isTransmittingSignal;
    bool frequencyScanning;
    uint32_t currentFrequency;
    uint32_t scanStartFreq;
    uint32_t scanEndFreq;
    unsigned long lastReceiveTime;
    
    // Raw data buffer
    static const int MAX_RAW_LENGTH = 1000;
    uint8_t rawBuffer[MAX_RAW_LENGTH];
    volatile size_t rawIndex;
    volatile unsigned long lastEdgeTime;
    
    // History storage
    static const int MAX_HISTORY = 50;
    RFSignal history[MAX_HISTORY];
    int historyCount;
    int historyIndex;
    
    // Protocol decoders
    bool decodeASK(RFSignal* signal);
    bool decodeFSK(RFSignal* signal);
    bool decodeManchester(RFSignal* signal);
    bool decodePWM(RFSignal* signal);
    
    // Helper functions
    void startReceiving();
    void stopReceiving();
    void captureRawData();
    String generateSignalName(RFProtocol protocol, uint32_t frequency);
    void setFrequency(uint32_t frequency);
    static void IRAM_ATTR rfInterruptHandler();
};

extern RFModule rfModule;

#endif
