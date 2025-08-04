#include "IRModule.h"
#include "StorageManager.h"

IRModule irModule;

// Static instance pointer for interrupt handler
IRModule* IRModule_instance = nullptr;

IRModule::IRModule() 
    : irInitialized(false), signalReceived(false), isReceivingSignal(false),
      isTransmittingSignal(false), lastReceiveTime(0), rawIndex(0),
      lastEdgeTime(0), historyCount(0), historyIndex(0) {
    IRModule_instance = this;
}

bool IRModule::init() {
    pinMode(IR_RECEIVER_PIN, INPUT);
    pinMode(IR_LED_PIN, OUTPUT);
    digitalWrite(IR_LED_PIN, LOW);
    
    // Initialize raw buffer
    memset(rawBuffer, 0, sizeof(rawBuffer));
    
    irInitialized = true;
    Serial.println("IR module initialized");
    
    return true;
}

void IRModule::update() {
    if (!irInitialized) return;
    
    unsigned long currentTime = millis();
    
    // Check for received signals
    if (isReceivingSignal && (currentTime - lastEdgeTime > 100)) {
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

bool IRModule::receiveSignal() {
    if (!irInitialized) return false;
    
    startReceiving();
    return true;
}

bool IRModule::decodeSignal(IRSignal* signal) {
    if (!signal || rawIndex < 10) return false;
    
    // Try different protocol decoders
    if (decodeNEC(signal)) {
        signal->protocol = IR_NEC;
        return true;
    }
    
    if (decodeSony(signal)) {
        signal->protocol = IR_SONY;
        return true;
    }
    
    if (decodeRC5(signal)) {
        signal->protocol = IR_RC5;
        return true;
    }
    
    if (decodeRC6(signal)) {
        signal->protocol = IR_RC6;
        return true;
    }
    
    if (decodeSamsung(signal)) {
        signal->protocol = IR_SAMSUNG;
        return true;
    }
    
    if (decodeLG(signal)) {
        signal->protocol = IR_LG;
        return true;
    }
    
    // If no protocol matched, store as raw
    signal->protocol = IR_RAW;
    signal->rawLength = rawIndex;
    signal->rawData = new uint16_t[rawIndex];
    memcpy(signal->rawData, rawBuffer, rawIndex * sizeof(uint16_t));
    signal->frequency = 38000; // Default frequency
    signal->name = generateSignalName(IR_RAW, 0);
    signal->timestamp = millis();
    
    return true;
}

String IRModule::getProtocolString(IRProtocol protocol) {
    switch (protocol) {
        case IR_NEC: return "NEC";
        case IR_SONY: return "Sony";
        case IR_RC5: return "RC5";
        case IR_RC6: return "RC6";
        case IR_SAMSUNG: return "Samsung";
        case IR_LG: return "LG";
        case IR_RAW: return "RAW";
        default: return "Unknown";
    }
}

bool IRModule::transmitSignal(const IRSignal* signal) {
    if (!irInitialized || !signal) return false;
    
    isTransmittingSignal = true;
    
    bool success = false;
    
    switch (signal->protocol) {
        case IR_NEC:
            success = transmitNEC(signal->address, signal->command);
            break;
        case IR_SONY:
            success = transmitSony(signal->command, 12); // Assume 12-bit Sony
            break;
        case IR_RAW:
            success = transmitRaw(signal->rawData, signal->rawLength, signal->frequency);
            break;
        default:
            break;
    }
    
    isTransmittingSignal = false;
    return success;
}

bool IRModule::transmitNEC(uint32_t address, uint32_t command) {
    if (!irInitialized) return false;
    
    // NEC protocol implementation
    const int carrierFreq = 38000;
    const int pulseWidth = 1000000 / carrierFreq / 2; // 13µs for 38kHz
    
    // Lead pulse
    for (int i = 0; i < 9000 * carrierFreq / 1000000; i++) {
        digitalWrite(IR_LED_PIN, HIGH);
        delayMicroseconds(pulseWidth);
        digitalWrite(IR_LED_PIN, LOW);
        delayMicroseconds(pulseWidth);
    }
    
    // Lead space
    delayMicroseconds(4500);
    
    // Send address (16 bits)
    for (int i = 0; i < 16; i++) {
        // Pulse
        for (int j = 0; j < 560 * carrierFreq / 1000000; j++) {
            digitalWrite(IR_LED_PIN, HIGH);
            delayMicroseconds(pulseWidth);
            digitalWrite(IR_LED_PIN, LOW);
            delayMicroseconds(pulseWidth);
        }
        
        // Space (560µs for 0, 1690µs for 1)
        if (address & (1 << i)) {
            delayMicroseconds(1690);
        } else {
            delayMicroseconds(560);
        }
    }
    
    // Send command (16 bits)
    for (int i = 0; i < 16; i++) {
        // Pulse
        for (int j = 0; j < 560 * carrierFreq / 1000000; j++) {
            digitalWrite(IR_LED_PIN, HIGH);
            delayMicroseconds(pulseWidth);
            digitalWrite(IR_LED_PIN, LOW);
            delayMicroseconds(pulseWidth);
        }
        
        // Space
        if (command & (1 << i)) {
            delayMicroseconds(1690);
        } else {
            delayMicroseconds(560);
        }
    }
    
    // Final pulse
    for (int i = 0; i < 560 * carrierFreq / 1000000; i++) {
        digitalWrite(IR_LED_PIN, HIGH);
        delayMicroseconds(pulseWidth);
        digitalWrite(IR_LED_PIN, LOW);
        delayMicroseconds(pulseWidth);
    }
    
    return true;
}

bool IRModule::transmitSony(uint32_t data, int nbits) {
    if (!irInitialized) return false;
    
    // Sony protocol implementation (simplified)
    const int carrierFreq = 40000;
    const int pulseWidth = 1000000 / carrierFreq / 2;
    
    // Start pulse
    for (int i = 0; i < 2400 * carrierFreq / 1000000; i++) {
        digitalWrite(IR_LED_PIN, HIGH);
        delayMicroseconds(pulseWidth);
        digitalWrite(IR_LED_PIN, LOW);
        delayMicroseconds(pulseWidth);
    }
    
    delayMicroseconds(600);
    
    // Send data bits
    for (int i = 0; i < nbits; i++) {
        // Pulse
        int pulseTime = (data & (1 << i)) ? 1200 : 600;
        for (int j = 0; j < pulseTime * carrierFreq / 1000000; j++) {
            digitalWrite(IR_LED_PIN, HIGH);
            delayMicroseconds(pulseWidth);
            digitalWrite(IR_LED_PIN, LOW);
            delayMicroseconds(pulseWidth);
        }
        
        delayMicroseconds(600);
    }
    
    return true;
}

bool IRModule::transmitRaw(uint16_t* data, uint16_t length, uint16_t frequency) {
    if (!irInitialized || !data) return false;
    
    const int pulseWidth = 1000000 / frequency / 2;
    
    for (uint16_t i = 0; i < length; i++) {
        if (i % 2 == 0) {
            // Pulse (on)
            for (int j = 0; j < data[i] * frequency / 1000000; j++) {
                digitalWrite(IR_LED_PIN, HIGH);
                delayMicroseconds(pulseWidth);
                digitalWrite(IR_LED_PIN, LOW);
                delayMicroseconds(pulseWidth);
            }
        } else {
            // Space (off)
            delayMicroseconds(data[i]);
        }
    }
    
    return true;
}

bool IRModule::saveSignal(const IRSignal* signal) {
    if (!signal) return false;
    
    String filename = IR_DIR + String("/") + signal->name + IR_EXT;
    
    JsonDocument doc;
    doc["protocol"] = (int)signal->protocol;
    doc["name"] = signal->name;
    doc["command"] = signal->command;
    doc["address"] = signal->address;
    doc["frequency"] = signal->frequency;
    doc["timestamp"] = signal->timestamp;
    
    if (signal->protocol == IR_RAW && signal->rawData) {
        doc["rawLength"] = signal->rawLength;
        JsonArray rawArray = doc.createNestedArray("rawData");
        for (uint16_t i = 0; i < signal->rawLength; i++) {
            rawArray.add(signal->rawData[i]);
        }
    }
    
    return storageManager.writeJsonFile(filename, doc);
}

bool IRModule::loadSignal(const String& filename, IRSignal* signal) {
    if (!signal) return false;
    
    JsonDocument doc;
    if (!storageManager.readJsonFile(filename, doc)) {
        return false;
    }
    
    signal->protocol = (IRProtocol)doc["protocol"].as<int>();
    signal->name = doc["name"].as<String>();
    signal->command = doc["command"].as<uint32_t>();
    signal->address = doc["address"].as<uint32_t>();
    signal->frequency = doc["frequency"].as<uint16_t>();
    signal->timestamp = doc["timestamp"].as<unsigned long>();
    
    if (signal->protocol == IR_RAW && doc.containsKey("rawData")) {
        signal->rawLength = doc["rawLength"].as<uint16_t>();
        signal->rawData = new uint16_t[signal->rawLength];
        JsonArray rawArray = doc["rawData"];
        for (uint16_t i = 0; i < signal->rawLength; i++) {
            signal->rawData[i] = rawArray[i];
        }
    }
    
    return true;
}

void IRModule::deleteSignal(const String& filename) {
    storageManager.deleteFile(filename);
}

int IRModule::getSignalCount() {
    return storageManager.getFileCount(IR_DIR);
}

String IRModule::getSignalFilename(int index) {
    return storageManager.getFileName(IR_DIR, index);
}

void IRModule::addToHistory(const IRSignal* signal) {
    if (!signal) return;
    
    history[historyIndex] = *signal;
    historyIndex = (historyIndex + 1) % MAX_HISTORY;
    if (historyCount < MAX_HISTORY) {
        historyCount++;
    }
}

void IRModule::clearHistory() {
    historyCount = 0;
    historyIndex = 0;
}

int IRModule::getHistoryCount() {
    return historyCount;
}

IRSignal IRModule::getHistoryItem(int index) {
    if (index >= 0 && index < historyCount) {
        int realIndex = (historyIndex - historyCount + index + MAX_HISTORY) % MAX_HISTORY;
        return history[realIndex];
    }
    return IRSignal();
}

bool IRModule::isReceiving() {
    return isReceivingSignal;
}

bool IRModule::isTransmitting() {
    return isTransmittingSignal;
}

void IRModule::startReceiving() {
    if (!irInitialized) return;
    
    rawIndex = 0;
    signalReceived = false;
    isReceivingSignal = true;
    lastEdgeTime = millis();
    
    // Attach interrupt
    attachInterrupt(digitalPinToInterrupt(IR_RECEIVER_PIN), irInterruptHandler, CHANGE);
}

void IRModule::stopReceiving() {
    isReceivingSignal = false;
    detachInterrupt(digitalPinToInterrupt(IR_RECEIVER_PIN));
}

void IRModule::captureRawData() {
    // This would be called from interrupt handler
    if (rawIndex < MAX_RAW_LENGTH - 1) {
        unsigned long currentTime = micros();
        uint16_t duration = currentTime - lastEdgeTime;
        rawBuffer[rawIndex++] = duration;
        lastEdgeTime = currentTime;
    }
}

String IRModule::generateSignalName(IRProtocol protocol, uint32_t command) {
    String protocolName = getProtocolString(protocol);
    if (protocol == IR_RAW) {
        return protocolName + "_" + String(millis() % 10000);
    } else {
        return protocolName + "_0x" + String(command, HEX);
    }
}

// Protocol decoders (simplified implementations)
bool IRModule::decodeNEC(IRSignal* signal) {
    // NEC protocol decoder implementation
    // This is a simplified version
    if (rawIndex < 68) return false; // NEC should have 68 edges minimum
    
    // Check for NEC start sequence (9ms pulse + 4.5ms space)
    if (rawBuffer[0] < 8000 || rawBuffer[0] > 10000) return false;
    if (rawBuffer[1] < 4000 || rawBuffer[1] > 5000) return false;
    
    uint32_t address = 0;
    uint32_t command = 0;
    
    // Decode address (16 bits)
    for (int i = 0; i < 16; i++) {
        int pulseIndex = 2 + i * 2;
        int spaceIndex = pulseIndex + 1;
        
        if (spaceIndex >= rawIndex) return false;
        
        // Check pulse duration (should be ~560µs)
        if (rawBuffer[pulseIndex] < 400 || rawBuffer[pulseIndex] > 700) return false;
        
        // Check space duration (560µs for 0, 1690µs for 1)
        if (rawBuffer[spaceIndex] > 1200) {
            address |= (1 << i);
        }
    }
    
    // Decode command (16 bits)
    for (int i = 0; i < 16; i++) {
        int pulseIndex = 34 + i * 2;
        int spaceIndex = pulseIndex + 1;
        
        if (spaceIndex >= rawIndex) return false;
        
        if (rawBuffer[pulseIndex] < 400 || rawBuffer[pulseIndex] > 700) return false;
        
        if (rawBuffer[spaceIndex] > 1200) {
            command |= (1 << i);
        }
    }
    
    signal->address = address;
    signal->command = command;
    signal->frequency = 38000;
    signal->name = generateSignalName(IR_NEC, command);
    signal->timestamp = millis();
    
    return true;
}

bool IRModule::decodeSony(IRSignal* signal) {
    // Sony protocol decoder (simplified)
    // Implementation would be similar to NEC but with different timing
    return false; // Placeholder
}

bool IRModule::decodeRC5(IRSignal* signal) {
    // RC5 protocol decoder (simplified)
    return false; // Placeholder
}

bool IRModule::decodeRC6(IRSignal* signal) {
    // RC6 protocol decoder (simplified)
    return false; // Placeholder
}

bool IRModule::decodeSamsung(IRSignal* signal) {
    // Samsung protocol decoder (simplified)
    return false; // Placeholder
}

bool IRModule::decodeLG(IRSignal* signal) {
    // LG protocol decoder (simplified)
    return false; // Placeholder
}

void IRAM_ATTR IRModule::irInterruptHandler() {
    if (IRModule_instance && IRModule_instance->isReceivingSignal) {
        IRModule_instance->captureRawData();
    }
}
