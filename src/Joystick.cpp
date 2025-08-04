#include "Joystick.h"

Joystick joystick;

Joystick::Joystick() 
    : lastReadTime(0), debounceDelay(50), lastDirection(JOYSTICK_NONE) {
    for (int i = 0; i < 3; i++) {
        buttonStates[i] = false;
        lastButtonStates[i] = false;
        lastDebounceTime[i] = 0;
    }
}

void Joystick::init() {
    pinMode(JOYSTICK_UP_PIN, INPUT_PULLUP);
    pinMode(JOYSTICK_DOWN_PIN, INPUT_PULLUP);
    pinMode(JOYSTICK_SELECT_PIN, INPUT_PULLUP);
    
    Serial.println("Joystick initialized");
}

JoystickDirection Joystick::read() {
    unsigned long currentTime = millis();
    
    // Debounce timing
    if (currentTime - lastReadTime < debounceDelay) {
        return JOYSTICK_NONE;
    }
    
    // Read current states (inverted because of pull-up)
    bool upPressed = !digitalRead(JOYSTICK_UP_PIN);
    bool downPressed = !digitalRead(JOYSTICK_DOWN_PIN);
    bool selectPressed = !digitalRead(JOYSTICK_SELECT_PIN);
    
    JoystickDirection direction = JOYSTICK_NONE;
    
    // Determine direction (priority: select > up > down)
    if (selectPressed) {
        direction = JOYSTICK_SELECT;
    } else if (upPressed) {
        direction = JOYSTICK_UP;
    } else if (downPressed) {
        direction = JOYSTICK_DOWN;
    }
    
    // Only return direction if it's different from last reading
    if (direction != JOYSTICK_NONE && direction != lastDirection) {
        lastDirection = direction;
        lastReadTime = currentTime;
        return direction;
    }
    
    // Reset last direction if no button is pressed
    if (direction == JOYSTICK_NONE) {
        lastDirection = JOYSTICK_NONE;
    }
    
    return JOYSTICK_NONE;
}

bool Joystick::isPressed(JoystickDirection direction) {
    switch (direction) {
        case JOYSTICK_UP:
            return !digitalRead(JOYSTICK_UP_PIN);
        case JOYSTICK_DOWN:
            return !digitalRead(JOYSTICK_DOWN_PIN);
        case JOYSTICK_SELECT:
            return !digitalRead(JOYSTICK_SELECT_PIN);
        default:
            return false;
    }
}

void Joystick::update() {
    unsigned long currentTime = millis();
    
    // Update button states with debouncing
    bool currentStates[3] = {
        !digitalRead(JOYSTICK_UP_PIN),
        !digitalRead(JOYSTICK_DOWN_PIN),
        !digitalRead(JOYSTICK_SELECT_PIN)
    };
    
    for (int i = 0; i < 3; i++) {
        if (currentStates[i] != lastButtonStates[i]) {
            lastDebounceTime[i] = currentTime;
        }
        
        if ((currentTime - lastDebounceTime[i]) > debounceDelay) {
            if (currentStates[i] != buttonStates[i]) {
                buttonStates[i] = currentStates[i];
            }
        }
        
        lastButtonStates[i] = currentStates[i];
    }
}

bool Joystick::readPin(int pin) {
    return !digitalRead(pin); // Inverted because of pull-up resistor
}
