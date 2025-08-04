#include <Arduino.h>
#include "DisplayManager.h"
#include "MenuManager.h"
#include "Joystick.h"
#include "StorageManager.h"
#include "SettingsManager.h"
#include "NFCModule.h"
#include "IRModule.h"
#include "iButtonModule.h"
#include "RFModule.h"
#include "GPIOModule.h"

// Buzzer pin
#define BUZZER_PIN 3

// Global instances
DisplayManager displayManager;
MenuManager menuManager;
Joystick joystick;
StorageManager storageManager;
SettingsManager settingsManager;
NFCModule nfcModule;
IRModule irModule;
iButtonModule iButtonModule;
RFModule rfModule;
GPIOModule gpioModule;

// System state
bool systemInitialized = false;
unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 50; // 20 FPS

void playBootSound() {
    if (settingsManager.isSoundEnabled()) {
        tone(BUZZER_PIN, 1000, 200);
        delay(250);
        tone(BUZZER_PIN, 1200, 200);
        delay(250);
        tone(BUZZER_PIN, 1500, 300);
    }
}

void playBeep() {
    if (settingsManager.isSoundEnabled()) {
        tone(BUZZER_PIN, settingsManager.getSettings()->beepFrequency, 
             settingsManager.getSettings()->beepDuration);
    }
}

bool initializeSystem() {
    Serial.begin(115200);
    Serial.println("FlipperS3 Starting...");
    
    // Initialize buzzer
    pinMode(BUZZER_PIN, OUTPUT);
    
    // Initialize display first
    if (!displayManager.init()) {
        Serial.println("Failed to initialize display!");
        return false;
    }
    
    // Show boot animation
    displayManager.showBootAnimation();
    playBootSound();
    
    // Initialize storage
    if (!storageManager.init()) {
        Serial.println("Failed to initialize storage!");
        displayManager.drawCenteredText("SD Card Error", 32);
        displayManager.display();
        delay(2000);
    }
    
    // Initialize settings
    if (!settingsManager.init()) {
        Serial.println("Failed to initialize settings, using defaults");
    }
    
    // Initialize joystick
    joystick.init();
    
    // Initialize modules based on settings
    if (settingsManager.isModuleEnabled("nfc")) {
        if (!nfcModule.init()) {
            Serial.println("Failed to initialize NFC module");
        }
    }
    
    if (settingsManager.isModuleEnabled("ir")) {
        if (!irModule.init()) {
            Serial.println("Failed to initialize IR module");
        }
    }
    
    if (settingsManager.isModuleEnabled("ibutton")) {
        if (!iButtonModule.init()) {
            Serial.println("Failed to initialize iButton module");
        }
    }
    
    if (settingsManager.isModuleEnabled("rf")) {
        if (!rfModule.init()) {
            Serial.println("Failed to initialize RF module");
        }
    }
    
    if (settingsManager.isModuleEnabled("gpio")) {
        if (!gpioModule.init()) {
            Serial.println("Failed to initialize GPIO module");
        }
    }
    
    // Initialize menu manager
    menuManager.init();
    
    Serial.println("System initialized successfully!");
    return true;
}

void setup() {
    systemInitialized = initializeSystem();
    
    if (!systemInitialized) {
        Serial.println("System initialization failed!");
        // Show error screen
        displayManager.clear();
        displayManager.drawCenteredText("INIT ERROR", 20);
        displayManager.drawCenteredText("Check connections", 35);
        displayManager.display();
        while (true) {
            delay(1000);
        }
    }
    
    // Clear display and show main menu
    displayManager.clear();
    menuManager.draw();
    displayManager.display();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Update at fixed interval
    if (currentTime - lastUpdate >= UPDATE_INTERVAL) {
        lastUpdate = currentTime;
        
        // Update joystick
        joystick.update();
        JoystickDirection input = joystick.read();
        
        // Handle input
        if (input != JOYSTICK_NONE) {
            playBeep();
            menuManager.handleInput(input);
        }
        
        // Update menu manager
        menuManager.update();
        
        // Update modules
        if (settingsManager.isModuleEnabled("nfc")) {
            nfcModule.update();
        }
        
        if (settingsManager.isModuleEnabled("ir")) {
            irModule.update();
        }
        
        if (settingsManager.isModuleEnabled("ibutton")) {
            iButtonModule.update();
        }
        
        if (settingsManager.isModuleEnabled("rf")) {
            rfModule.update();
        }
        
        if (settingsManager.isModuleEnabled("gpio")) {
            gpioModule.update();
        }
        
        // Update storage manager
        storageManager.update();
        
        // Update display if needed
        displayManager.drawStatusBar();
        displayManager.display();
    }
    
    // Small delay to prevent watchdog issues
    delay(1);
}
