#include "RFModule.h"
#include "StorageManager.h"

RFModule rfModule;

// Static instance pointer for interrupt handler
RFModule* RFModule_instance = nullptr;

RFModule::RFModule() 
    : rfInitialized(false), signalReceived(false), isReceivingSignal(false),
      isTransmittingSignal(false), frequencyScanning(false), currentFrequency(433920000),
      scanStartFreq(0), scanEndFreq(0), lastReceiveTime(0), rawIndex(0),
      lastEdgeTime(0), historyCount(0), historyIndex(0) {
    RFModule_instance = this;
}

bool RFModule::init() {
    pinMode(RF_RECEIVER_PIN, INPUT);
    pinMode(RF_TRANSMITTER_PIN, OUTPUT);
    digitalWrite(RF_TRANSMITTER_PIN, LOW);
    
    // Initialize raw buffer
    memset(raw#include "RFModule.h"
#include "StorageManager.h"

RFModule rfModule;

// Static instance pointer for interrupt handler
RFModule* RFModule_instance = nullptr;

RFModule::RFModule() 
    : rfInitialized(false), signalReceived(false), isReceivingSignal(false),
      isTransmittingSignal(false), frequencyScanning(false), currentFrequency(433920000),
      scanStartFreq(300000000), scanEndFreq(928000000), lastReceiveTime(0),
      rawIndex(0), lastEdgeTime(0), historyCount(0), historyIndex(0) {
    RFModule_instance = this;
}

bool RFModule::init() {
    pinMode(RF_RECEIVER_PIN, INPUT);
    pinMode(RF_TRANSMITTER_PIN, OUTPUT);
    digitalWrite(RF_TRANSMITTER_PIN, LOW);
    
    // Initialize raw buffer
    memset(rawBuffer, 0, sizeof(rawBuffer));
    
    // Set default frequency (433.92 MHz)
    setFrequency(433920000);
    
    rfInitialized = true;
    Serial.println("RF module initialized");
    
    return true;
}

void RFModule::update() {
    if (!rfInitialized) return;
    
    unsigned long currentTime = millis();
    
    // Handle frequency scanning
    if (frequencyScanning) {
        // Step through frequencies
        currentFrequency += 100000; // 100kHz steps
        if (currentFrequency > scanEndFreq) {
            currentFrequency = scanStartFreq;
        }
        setFrequency(currentFrequency);
    }
    
    // Check for received signals
    if (isReceivingSignal && (currentTime - lastEdgeTime > 200)) {
        // Signal reception timeout, process received data
        if (rawIndex > 10) { // Minimum signal length
            signalReceived = true;
            if (decodeSignal(&currentSignal)) {
                addToHistory(&currentSignal);
            }
        }
        stopReceiving();
    }
}

bool RFModule::receiveSignal() {
    if (!rfInitialized) return false;
    
    startReceiving();
    return true;
}

bool RFModule::decodeSignal(RFSignal* signal) {
    if (!signal || rawIndex < 10) return false;
    
    // Try different protocol decoders
    if (decodeASK(signal)) {
        signal->protocol = RF_ASK_OOK;
        return true;
    }
    
    if (decodeFSK(signal)) {
        signal->protocol = RF_FSK;
        return true;
    }
    
    if (decodeManchester(signal)) {
        signal->protocol = RF_MANCHESTER;
        return true;
    }
    
    if (decodePWM(signal)) {
        signal->protocol = RF_PWM;
        return true;
    }
    
    // If no protocol matched, store as raw
    signal->protocol = RF_RAW;
    signal->rawLength = rawIndex;
    signal->rawData = new uint8_t[rawIndex];
    memcpy(signal->rawData, rawBuffer, rawIndex);
    signal->frequency = currentFrequency;
    signal->bitrate = 4800; // Default bitrate
    signal->name = generateSignalName(RF_RAW, currentFrequency);
    signal->timestamp = millis();
    
    return true;
}

String RFModule::getProtocolString(RFProtocol protocol) {
    switch (protocol) {
        case RF_ASK_OOK: return "ASK/OOK";
        case RF_FSK: return "FSK";
        case RF_MANCHESTER: return "Manchester";
        case RF_PWM: return "PWM";
        case RF_RAW: return "RAW";
        default: return "Unknown";
    }
}

bool RFModule::transmitSignal(const RFSignal* signal) {
    if (!rfInitialized || !signal) return false;
    
    isTransmittingSignal = true;
    
    // Set frequency for transmission
    setFrequency(signal->frequency);
    
    bool success = false;
    
    switch (signal->protocol) {
        case RF_ASK_OOK:
            success = transmitASK(signal->data, 32, signal->frequency); // Assume 32 bits
            break;
        case RF_FSK:
            success = transmitFSK(signal->data, 32, signal->frequency);
            break;
        case RF_RAW:
            success = transmitRaw(signal->rawData, signal->rawLength, signal->frequency);
            break;
        default:
            break;
    }
    
    isTransmittingSignal = false;
    return success;
}

bool RFModule::transmitASK(uint32_t data, uint8_t bits, uint32_t frequency) {
    if (!rfInitialized) return false;
    
    // Simple ASK/OOK modulation
    const int bitDuration = 1000000 / 4800; // 4800 bps = ~208µs per bit
    
    // Preamble
    for (int i = 0; i < 32; i++) {
        digitalWrite(RF_TRANSMITTER_PIN, HIGH);
        delayMicroseconds(bitDuration / 2);
        digitalWrite(RF_TRANSMITTER_PIN, LOW);
        delayMicroseconds(bitDuration / 2);
    }
    
    // Send data bits
    for (int i = bits - 1; i >= 0; i--) {
        if (data & (1UL << i)) {
            // Send '1' - longer pulse
            digitalWrite(RF_TRANSMITTER_PIN, HIGH);
            delayMicroseconds(bitDuration * 3 / 4);
            digitalWrite(RF_TRANSMITTER_PIN, LOW);
            delayMicroseconds(bitDuration / 4);
        } else {
            // Send '0' - shorter pulse
            digitalWrite(RF_TRANSMITTER_PIN, HIGH);
            delayMicroseconds(bitDuration / 4);
            digitalWrite(RF_TRANSMITTER_PIN, LOW);
            delayMicroseconds(bitDuration * 3 / 4);
        }
    }
    
    // End transmission
    digitalWrite(RF_TRANSMITTER_PIN, LOW);
    
    return true;
}

bool RFModule::transmitFSK(uint32_t data, uint8_t bits, uint32_t frequency) {
    if (!rfInitialized) return false;
    
    // FSK modulation (simplified)
    const int bitDuration = 1000000 / 4800; // 4800 bps
    const int highFreqPeriod = 1000000 / (frequency + 10000); // +10kHz deviation
    const int lowFreqPeriod = 1000000 / (frequency - 10000);  // -10kHz deviation
    
    // Send data bits
    for (int i = bits - 1; i >= 0; i--) {
        int period = (data & (1UL << i)) ? highFreqPeriod : lowFreqPeriod;
        int cycles = bitDuration / period;
        
        for (int c = 0; c < cycles; c++) {
            digitalWrite(RF_TRANSMITTER_PIN, HIGH);
            delayMicroseconds(period / 2);
            digitalWrite(RF_TRANSMITTER_PIN, LOW);
            delayMicroseconds(period / 2);
        }
    }
    
    return true;
}

bool RFModule::transmitRaw(uint8_t* data, size_t length, uint32_t frequency) {
    if (!rfInitialized || !data) return false;
    
    // Raw data transmission
    for (size_t i = 0; i < length; i++) {
        uint8_t byte = data[i];
        for (int bit = 7; bit >= 0; bit--) {
            if (byte & (1 << bit)) {
                digitalWrite(RF_TRANSMITTER_PIN, HIGH);
            } else {
                digitalWrite(RF_TRANSMITTER_PIN, LOW);
            }
            delayMicroseconds(208); // ~4800 bps
        }
    }
    
    digitalWrite(RF_TRANSMITTER_PIN, LOW);
    return true;
}

void RFModule::startFrequencyScan(uint32_t startFreq, uint32_t endFreq) {
    if (!rfInitialized) return;
    
    scanStartFreq = startFreq;
    scanEndFreq = endFreq;
    currentFrequency = startFreq;
    frequencyScanning = true;
    
    startReceiving();
}

void RFModule::stopFrequencyScan() {
    frequencyScanning = false;
    stopReceiving();
}

bool RFModule::isScanning() {
    return frequencyScanning;
}

uint32_t RFModule::getCurrentScanFrequency() {
    return currentFrequency;
}

bool RFModule::saveSignal(const RFSignal* signal) {
    if (!signal) return false;
    
    String filename = RF_DIR + String("/") + signal->name + RF_EXT;
    
    JsonDocument doc;
    doc["protocol"] = (int)signal->protocol;
    doc["name"] = signal->name;
    doc["frequency"] = signal->frequency;
    doc["bitrate"] = signal->bitrate;
    doc["data"] = signal->data;
    doc["modulation"] = signal->modulation;
    doc["timestamp"] = signal->timestamp;
    
    if (signal->protocol == RF_RAW && signal->rawData) {
        doc["rawLength"] = signal->rawLength;
        JsonArray rawArray = doc.createNestedArray("rawData");
        for (size_t i = 0; i < signal->rawLength; i++) {
            rawArray.add(signal->rawData[i]);
        }
    }
    
    return storageManager.writeJsonFile(filename, doc);
}

bool RFModule::loadSignal(const String& filename, RFSignal* signal) {
    if (!signal) return false;
    
    JsonDocument doc;
    if (!storageManager.readJsonFile(filename, doc)) {
        return false;
    }
    
    signal->protocol = (RFProtocol)doc["protocol"].as<int>();
    signal->name = doc["name"].as<String>();
    signal->frequency = doc["frequency"].as<uint32_t>();
    signal->bitrate = doc["bitrate"].as<uint32_t>();
    signal->data = doc["data"].as<uint32_t>();
    signal->modulation = doc["modulation"].as<uint8_t>();
    signal->timestamp = doc["timestamp"].as<unsigned long>();
    
    if (signal->protocol == RF_RAW && doc.containsKey("rawData")) {
        signal->rawLength = doc["rawLength"].as<size_t>();
        signal->rawData = new uint8_t[signal->rawLength];
        JsonArray rawArray = doc["rawData"];
        for (size_t i = 0; i < signal->rawLength; i++) {
            signal->rawData[i] = rawArray[i];
        }
    }
    
    return true;
}

void RFModule::deleteSignal(const String& filename) {
    storageManager.deleteFile(filename);
}

int RFModule::getSignalCount() {
    return storageManager.getFileCount(RF_DIR);
}

String RFModule::getSignalFilename(int index) {
    return storageManager.getFileName(RF_DIR, index);
}

void RFModule::addToHistory(const RFSignal* signal) {
    if (!signal) return;
    
    history[historyIndex] = *signal;
    historyIndex = (historyIndex + 1) % MAX_HISTORY;
    if (historyCount < MAX_HISTORY) {
        historyCount++;
    }
}

void RFModule::clearHistory() {
    historyCount = 0;
    historyIndex = 0;
}

int RFModule::getHistoryCount() {
    return historyCount;
}

RFSignal RFModule::getHistoryItem(int index) {
    if (index >= 0 && index < historyCount) {
        int realIndex = (historyIndex - historyCount + index + MAX_HISTORY) % MAX_HISTORY;
        return history[realIndex];
    }
    return RFSignal();
}

bool RFModule::isReceiving() {
    return isReceivingSignal;
}

bool RFModule::isTransmitting() {
    return isTransmittingSignal;
}

void RFModule::startReceiving() {
    if (!rfInitialized) return;
    
    rawIndex = 0;
    signalReceived = false;
    isReceivingSignal = true;
    lastEdgeTime = millis();
    
    // Attach interrupt
    attachInterrupt(digitalPinToInterrupt(RF_RECEIVER_PIN), rfInterruptHandler, CHANGE);
}

void RFModule::stopReceiving() {
    isReceivingSignal = false;
    detachInterrupt(digitalPinToInterrupt(RF_RECEIVER_PIN));
}

void RFModule::captureRawData() {
    if (rawIndex < MAX_RAW_LENGTH - 1) {
        unsigned long currentTime = micros();
        uint8_t duration = min(255, (currentTime - lastEdgeTime) / 10); // Scale to fit uint8_t
        rawBuffer[rawIndex++] = duration;
        lastEdgeTime = currentTime;
    }
}

String RFModule::generateSignalName(RFProtocol protocol, uint32_t frequency) {
    String protocolName = getProtocolString(protocol);
    String freqStr = String(frequency / 1000000.0, 2) + "MHz";
    return protocolName + "_" + freqStr + "_" + String(millis() % 10000);
}

void RFModule::setFrequency(uint32_t frequency) {
    currentFrequency = frequency;
    // In a real implementation, this would configure the RF transceiver
    // For this simulation, we just store the frequency
}

// Protocol decoders (simplified implementations)
bool RFModule::decodeASK(RFSignal* signal) {
    // ASK/OOK decoder implementation
    if (rawIndex < 32) return false; // Need minimum signal length
    
    uint32_t data = 0;
    int validBits = 0;
    
    // Simple threshold-based decoding
    uint8_t threshold = 50; // Adjust based on signal characteristics
    
    for (size_t i = 0; i < rawIndex && validBits < 32; i++) {
        if (rawBuffer[i] > threshold) {
            data = (data << 1) | 1;
            validBits++;
        } else if (rawBuffer[i] > 10) { // Ignore very short pulses (noise)
            data = (data << 1) | 0;
            validBits++;
        }
    }
    
    if (validBits >= 8) { // At least one byte decoded
        signal->data = data;
        signal->frequency = currentFrequency;
        signal->bitrate = 4800;
        signal->modulation = 0; // ASK
        signal->name = generateSignalName(RF_ASK_OOK, currentFrequency);
        signal->timestamp = millis();
        return true;
    }
    
    return false;
}

bool RFModule::decodeFSK(RFSignal* signal) {
    // FSK decoder implementation (simplified)
    // Would analyze frequency deviations in the received signal
    return false; // Placeholder
}

bool RFModule::decodeManchester(RFSignal* signal) {
    // Manchester encoding decoder
    if (rawIndex < 16) return false; // Need minimum signal length
    
    uint32_t data = 0;
    int validBits = 0;
    
    // Manchester decoding: transition in middle of bit period
    // Rising edge = 0, falling edge = 1 (or vice versa)
    for (size_t i = 0; i < rawIndex - 1 && validBits < 32; i += 2) {
        uint8_t firstHalf = rawBuffer[i];
        uint8_t secondHalf = rawBuffer[i + 1];
        
        // Look for transitions
        if (firstHalf < secondHalf) {
            // Rising transition = 0
            data = (data << 1) | 0;
            validBits++;
        } else if (firstHalf > secondHalf) {
            // Falling transition = 1
            data = (data << 1) | 1;
            validBits++;
        }
    }
    
    if (validBits >= 8) {
        signal->data = data;
        signal->frequency = currentFrequency;
        signal->bitrate = 2400; // Typical for Manchester
        signal->modulation = 1; // Manchester
        signal->name = generateSignalName(RF_MANCHESTER, currentFrequency);
        signal->timestamp = millis();
        return true;
    }
    
    return false;
}

bool RFModule::decodePWM(RFSignal* signal) {
    // PWM decoder implementation (simplified)
    // Would analyze pulse width variations
    return false; // Placeholder
}

void IRAM_ATTR RFModule::rfInterruptHandler() {
    if (RFModule_instance && RFModule_instance->isReceivingSignal) {
        RFModule_instance->captureRawData();
    }
}
