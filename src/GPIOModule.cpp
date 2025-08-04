#include "GPIOModule.h"
#include "StorageManager.h"

GPIOModule gpioModule;

GPIOModule::GPIOModule() 
    : gpioInitialized(false), configuredPinCount(0), analyzing(false),
      scenarioRunning(false), scenarioStartTime(0), changeCount(0), changeIndex(0) {
}

bool GPIOModule::init() {
    // Initialize pin structures
    for (int i = 0; i < MAX_GPIO_PINS; i++) {
        pins[i].pin = GPIO_START_PIN + i;
        pins[i].mode = GPIO_MODE_INPUT;
        pins[i].name = "";
        pins[i].value = 0;
        pins[i].pwmChannel = -1;
        pins[i].pwmFrequency = 1000;
        pins[i].pwmResolution = 8;
        pins[i].enabled = false;
    }
    
    // Initialize change tracking
    memset(changes, 0, sizeof(changes));
    
    gpioInitialized = true;
    Serial.println("GPIO module initialized");
    
    return true;
}

void GPIOModule::update() {
    if (!gpioInitialized) return;
    
    // Update analog readings for configured analog pins
    for (int i = 0; i < configuredPinCount; i++) {
        if (pins[i].enabled && pins[i].mode == GPIO_MODE_ANALOG) {
            updateAnalogReading(pins[i].pin);
        }
    }
    
    // Check for pin changes if analyzing
    if (analyzing) {
        checkPinChanges();
    }
    
    // Update running scenario
    if (scenarioRunning) {
        unsigned long elapsed = millis() - scenarioStartTime;
        if (elapsed >= currentScenario.duration) {
            if (currentScenario.repeat) {
                // Restart scenario
                scenarioStartTime = millis();
            } else {
                stopScenario();
            }
        }
    }
}

bool GPIOModule::configurePin(uint8_t pin, GPIOMode mode, const String& name) {
    if (!isValidPin(pin)) return false;
    
    int index = pinToIndex(pin);
    if (index == -1) return false;
    
    // Configure the physical pin
    initializePin(pin, mode);
    
    // Update pin structure
    pins[index].pin = pin;
    pins[index].mode = mode;
    pins[index].name = name.length() > 0 ? name : generatePinName(pin, mode);
    pins[index].enabled = true;
    
    // Add to configured pins if not already present
    bool alreadyConfigured = false;
    for (int i = 0; i < configuredPinCount; i++) {
        if (pins[i].pin == pin) {
            alreadyConfigured = true;
            break;
        }
    }
    
    if (!alreadyConfigured && configuredPinCount < MAX_GPIO_PINS) {
        configuredPinCount++;
    }
    
    Serial.print("Configured GPIO ");
    Serial.print(pin);
    Serial.print(" as ");
    Serial.println(getModeString(mode));
    
    return true;
}

bool GPIOModule::setPinValue(uint8_t pin, int value) {
    if (!isValidPin(pin)) return false;
    
    GPIOPin* pinObj = getPin(pin);
    if (!pinObj || !pinObj->enabled) return false;
    
    int oldValue = pinObj->value;
    
    switch (pinObj->mode) {
        case GPIO_MODE_OUTPUT:
            digitalWrite(pin, value ? HIGH : LOW);
            pinObj->value = value ? 1 : 0;
            break;
            
        case GPIO_MODE_PWM:
            if (pinObj->pwmChannel >= 0) {
                ledcWrite(pinObj->pwmChannel, value);
                pinObj->value = value;
            }
            break;
            
        default:
            return false; // Can't set value on input pins
    }
    
    // Log pin change if analyzing
    if (analyzing && oldValue != pinObj->value) {
        logPinChange(pin, oldValue, pinObj->value);
    }
    
    return true;
}

int GPIOModule::getPinValue(uint8_t pin) {
    if (!isValidPin(pin)) return -1;
    
    GPIOPin* pinObj = getPin(pin);
    if (!pinObj || !pinObj->enabled) return -1;
    
    switch (pinObj->mode) {
        case GPIO_MODE_INPUT:
        case GPIO_MODE_INPUT_PULLUP:
        case GPIO_MODE_INPUT_PULLDOWN:
            pinObj->value = digitalRead(pin);
            break;
            
        case GPIO_MODE_ANALOG:
            pinObj->value = analogRead(pin);
            break;
            
        case GPIO_MODE_OUTPUT:
        case GPIO_MODE_PWM:
            // Value is already stored
            break;
    }
    
    return pinObj->value;
}

bool GPIOModule::setPWM(uint8_t pin, int frequency, int dutyCycle, int resolution) {
    if (!isValidPin(pin)) return false;
    
    GPIOPin* pinObj = getPin(pin);
    if (!pinObj) return false;
    
    // Find available PWM channel
    int channel = -1;
    for (int i = 0; i < 16; i++) { // ESP32 has 16 PWM channels
        bool channelUsed = false;
        for (int j = 0; j < configuredPinCount; j++) {
            if (pins[j].pwmChannel == i) {
                channelUsed = true;
                break;
            }
        }
        if (!channelUsed) {
            channel = i;
            break;
        }
    }
    
    if (channel == -1) return false; // No available channels
    
    // Configure PWM
    ledcSetup(channel, frequency, resolution);
    ledcAttachPin(pin, channel);
    ledcWrite(channel, dutyCycle);
    
    // Update pin structure
    pinObj->mode = GPIO_MODE_PWM;
    pinObj->pwmChannel = channel;
    pinObj->pwmFrequency = frequency;
    pinObj->pwmResolution = resolution;
    pinObj->value = dutyCycle;
    pinObj->enabled = true;
    
    return true;
}

void GPIOModule::startAnalysis() {
    if (!gpioInitialized) return;
    
    analyzing = true;
    changeCount = 0;
    changeIndex = 0;
    
    Serial.println("GPIO analysis started");
}

void GPIOModule::stopAnalysis() {
    analyzing = false;
    Serial.println("GPIO analysis stopped");
}

bool GPIOModule::isAnalyzing() {
    return analyzing;
}

void GPIOModule::logPinChange(uint8_t pin, int oldValue, int newValue) {
    if (changeCount < MAX_CHANGES) {
        changes[changeIndex].pin = pin;
        changes[changeIndex].oldValue = oldValue;
        changes[changeIndex].newValue = newValue;
        changes[changeIndex].timestamp = millis();
        
        changeIndex = (changeIndex + 1) % MAX_CHANGES;
        if (changeCount < MAX_CHANGES) {
            changeCount++;
        }
        
        Serial.print("GPIO ");
        Serial.print(pin);
        Serial.print(" changed from ");
        Serial.print(oldValue);
        Serial.print(" to ");
        Serial.println(newValue);
    }
}

bool GPIOModule::saveScenario(const GPIOScenario* scenario) {
    if (!scenario) return false;
    
    String filename = GPIO_DIR + String("/") + scenario->name + GPIO_EXT;
    
    JsonDocument doc;
    doc["name"] = scenario->name;
    doc["duration"] = scenario->duration;
    doc["repeat"] = scenario->repeat;
    doc["timestamp"] = scenario->timestamp;
    doc["pinCount"] = scenario->pinCount;
    
    JsonArray pinsArray = doc.createNestedArray("pins");
    for (int i = 0; i < scenario->pinCount; i++) {
        JsonObject pinObj = pinsArray.createNestedObject();
        pinObj["pin"] = scenario->pins[i].pin;
        pinObj["mode"] = (int)scenario->pins[i].mode;
        pinObj["name"] = scenario->pins[i].name;
        pinObj["value"] = scenario->pins[i].value;
        pinObj["pwmFrequency"] = scenario->pins[i].pwmFrequency;
        pinObj["pwmResolution"] = scenario->pins[i].pwmResolution;
    }
    
    return storageManager.writeJsonFile(filename, doc);
}

bool GPIOModule::loadScenario(const String& filename, GPIOScenario* scenario) {
    if (!scenario) return false;
    
    JsonDocument doc;
    if (!storageManager.readJsonFile(filename, doc)) {
        return false;
    }
    
    scenario->name = doc["name"].as<String>();
    scenario->duration = doc["duration"].as<unsigned long>();
    scenario->repeat = doc["repeat"].as<bool>();
    scenario->timestamp = doc["timestamp"].as<unsigned long>();
    scenario->pinCount = doc["pinCount"].as<int>();
    
    JsonArray pinsArray = doc["pins"];
    for (int i = 0; i < scenario->pinCount && i < MAX_GPIO_PINS; i++) {
        JsonObject pinObj = pinsArray[i];
        scenario->pins[i].pin = pinObj["pin"];
        scenario->pins[i].mode = (GPIOMode)pinObj["mode"].as<int>();
        scenario->pins[i].name = pinObj["name"].as<String>();
        scenario->pins[i].value = pinObj["value"];
        scenario->pins[i].pwmFrequency = pinObj["pwmFrequency"];
        scenario->pins[i].pwmResolution = pinObj["pwmResolution"];
    }
    
    return true;
}

bool GPIOModule::executeScenario(const GPIOScenario* scenario) {
    if (!scenario || scenarioRunning) return false;
    
    // Copy scenario
    currentScenario = *scenario;
    
    // Configure and set all pins
    for (int i = 0; i < scenario->pinCount; i++) {
        const GPIOPin& pin = scenario->pins[i];
        
        // Configure pin
        configurePin(pin.pin, pin.mode, pin.name);
        
        // Set pin value
        if (pin.mode == GPIO_MODE_PWM) {
            setPWM(pin.pin, pin.pwmFrequency, pin.value, pin.pwmResolution);
        } else {
            setPinValue(pin.pin, pin.value);
        }
    }
    
    scenarioRunning = true;
    scenarioStartTime = millis();
    
    Serial.print("Executing scenario: ");
    Serial.println(scenario->name);
    
    return true;
}

void GPIOModule::stopScenario() {
    scenarioRunning = false;
    Serial.println("Scenario stopped");
}

void GPIOModule::deleteScenario(const String& filename) {
    storageManager.deleteFile(filename);
}

int GPIOModule::getScenarioCount() {
    return storageManager.getFileCount(GPIO_DIR);
}

String GPIOModule::getScenarioFilename(int index) {
    return storageManager.getFileName(GPIO_DIR, index);
}

void GPIOModule::saveCurrentState() {
    // Save current pin states to a temporary scenario
    GPIOScenario tempScenario;
    tempScenario.name = "TempState_" + String(millis());
    tempScenario.duration = 0;
    tempScenario.repeat = false;
    tempScenario.timestamp = millis();
    tempScenario.pinCount = configuredPinCount;
    
    for (int i = 0; i < configuredPinCount; i++) {
        tempScenario.pins[i] = pins[i];
    }
    
    saveScenario(&tempScenario);
}

void GPIOModule::restoreState() {
    // This would restore from the last saved state
    // Implementation depends on how states are managed
    Serial.println("State restore not fully implemented");
}

void GPIOModule::resetAllPins() {
    for (int i = 0; i < configuredPinCount; i++) {
        if (pins[i].enabled) {
            // Reset to input mode
            pinMode(pins[i].pin, INPUT);
            
            // Detach PWM if used
            if (pins[i].pwmChannel >= 0) {
                ledcDetachPin(pins[i].pin);
            }
            
            pins[i].enabled = false;
            pins[i].value = 0;
            pins[i].pwmChannel = -1;
        }
    }
    
    configuredPinCount = 0;
    Serial.println("All GPIO pins reset");
}

GPIOPin* GPIOModule::getPin(uint8_t pin) {
    int index = pinToIndex(pin);
    if (index >= 0 && index < MAX_GPIO_PINS) {
        return &pins[index];
    }
    return nullptr;
}

int GPIOModule::getConfiguredPinCount() {
    return configuredPinCount;
}

GPIOPin* GPIOModule::getConfiguredPin(int index) {
    if (index >= 0 && index < configuredPinCount) {
        return &pins[index];
    }
    return nullptr;
}

bool GPIOModule::isValidPin(uint8_t pin) {
    return (pin >= GPIO_START_PIN && pin <= GPIO_END_PIN);
}

String GPIOModule::getModeString(GPIOMode mode) {
    switch (mode) {
        case GPIO_MODE_INPUT: return "Input";
        case GPIO_MODE_OUTPUT: return "Output";
        case GPIO_MODE_INPUT_PULLUP: return "Input Pullup";
        case GPIO_MODE_INPUT_PULLDOWN: return "Input Pulldown";
        case GPIO_MODE_ANALOG: return "Analog";
        case GPIO_MODE_PWM: return "PWM";
        default: return "Unknown";
    }
}

int GPIOModule::pinToIndex(uint8_t pin) {
    if (isValidPin(pin)) {
        return pin - GPIO_START_PIN;
    }
    return -1;
}

bool GPIOModule::isConfiguredPin(uint8_t pin) {
    for (int i = 0; i < configuredPinCount; i++) {
        if (pins[i].pin == pin && pins[i].enabled) {
            return true;
        }
    }
    return false;
}

void GPIOModule::initializePin(uint8_t pin, GPIOMode mode) {
    switch (mode) {
        case GPIO_MODE_INPUT:
            pinMode(pin, INPUT);
            break;
        case GPIO_MODE_OUTPUT:
            pinMode(pin, OUTPUT);
            break;
        case GPIO_MODE_INPUT_PULLUP:
            pinMode(pin, INPUT_PULLUP);
            break;
        case GPIO_MODE_INPUT_PULLDOWN:
            pinMode(pin, INPUT_PULLDOWN);
            break;
        case GPIO_MODE_ANALOG:
            // Analog pins don't need pinMode on ESP32
            break;
        case GPIO_MODE_PWM:
            pinMode(pin, OUTPUT);
            break;
    }
}

String GPIOModule::generatePinName(uint8_t pin, GPIOMode mode) {
    return getModeString(mode) + "_GPIO" + String((int)pin);
}

void GPIOModule::updateAnalogReading(uint8_t pin) {
    GPIOPin* pinObj = getPin(pin);
    if (pinObj && pinObj->enabled && pinObj->mode == GPIO_MODE_ANALOG) {
        int oldValue = pinObj->value;
        pinObj->value = analogRead(pin);
        
        // Log significant changes (threshold to avoid noise)
        if (analyzing && abs(pinObj->value - oldValue) > 50) {
            logPinChange(pin, oldValue, pinObj->value);
        }
    }
}

void GPIOModule::checkPinChanges() {
    // Check all configured input pins for changes
    for (int i = 0; i < configuredPinCount; i++) {
        if (pins[i].enabled && 
            (pins[i].mode == GPIO_MODE_INPUT || 
             pins[i].mode == GPIO_MODE_INPUT_PULLUP || 
             pins[i].mode == GPIO_MODE_INPUT_PULLDOWN)) {
            
            int currentValue = digitalRead(pins[i].pin);
            if (currentValue != pins[i].value) {
                logPinChange(pins[i].pin, pins[i].value, currentValue);
                pins[i].value = currentValue;
            }
        }
    }
}
