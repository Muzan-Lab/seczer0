#ifndef GPIOMODULE_H
#define GPIOMODULE_H

#include <Arduino.h>
#include <ArduinoJson.h>

// GPIO pin definitions (configurable pins)
#define GPIO_START_PIN 14
#define GPIO_END_PIN   39
#define MAX_GPIO_PINS  (GPIO_END_PIN - GPIO_START_PIN + 1)

// GPIO modes
enum GPIOMode {
    GPIO_MODE_INPUT = 0,
    GPIO_MODE_OUTPUT,
    GPIO_MODE_INPUT_PULLUP,
    GPIO_MODE_INPUT_PULLDOWN,
    GPIO_MODE_ANALOG,
    GPIO_MODE_PWM
};

// GPIO pin state
struct GPIOPin {
    uint8_t pin;
    GPIOMode mode;
    String name;
    int value;
    int pwmChannel;
    int pwmFrequency;
    int pwmResolution;
    bool enabled;
};

// GPIO scenario for automation
struct GPIOScenario {
    String name;
    GPIOPin pins[MAX_GPIO_PINS];
    int pinCount;
    unsigned long duration;
    bool repeat;
    unsigned long timestamp;
};

class GPIOModule {
public:
    GPIOModule();
    bool init();
    void update();
    
    // Pin configuration
    bool configurePin(uint8_t pin, GPIOMode mode, const String& name = "");
    bool setPinValue(uint8_t pin, int value);
    int getPinValue(uint8_t pin);
    bool setPWM(uint8_t pin, int frequency, int dutyCycle, int resolution = 8);
    
    // Pin analysis
    void startAnalysis();
    void stopAnalysis();
    bool isAnalyzing();
    void logPinChange(uint8_t pin, int oldValue, int newValue);
    
    // Scenario management
    bool saveScenario(const GPIOScenario* scenario);
    bool loadScenario(const String& filename, GPIOScenario* scenario);
    bool executeScenario(const GPIOScenario* scenario);
    void stopScenario();
    void deleteScenario(const String& filename);
    int getScenarioCount();
    String getScenarioFilename(int index);
    
    // Pin state management
    void saveCurrentState();
    void restoreState();
    void resetAllPins();
    
    // Getters
    GPIOPin* getPin(uint8_t pin);
    int getConfiguredPinCount();
    GPIOPin* getConfiguredPin(int index);
    bool isValidPin(uint8_t pin);
    String getModeString(GPIOMode mode);
    
    // Status
    bool isInitialized() { return gpioInitialized; }
    bool isScenarioRunning() { return scenarioRunning; }

private:
    bool gpioInitialized;
    GPIOPin pins[MAX_GPIO_PINS];
    int configuredPinCount;
    bool analyzing;
    bool scenarioRunning;
    GPIOScenario currentScenario;
    unsigned long scenarioStartTime;
    
    // Analysis data
    struct PinChange {
        uint8_t pin;
        int oldValue;
        int newValue;
        unsigned long timestamp;
    };
    
    static const int MAX_CHANGES = 1000;
    PinChange changes[MAX_CHANGES];
    int changeCount;
    int changeIndex;
    
    // Helper functions
    int pinToIndex(uint8_t pin);
    bool isConfiguredPin(uint8_t pin);
    void initializePin(uint8_t pin, GPIOMode mode);
    String generatePinName(uint8_t pin, GPIOMode mode);
    void updateAnalogReading(uint8_t pin);
    void checkPinChanges();
};

extern GPIOModule gpioModule;

#endif
