#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

// Joystick directions
enum JoystickDirection {
    JOYSTICK_NONE = 0,
    JOYSTICK_UP,
    JOYSTICK_DOWN,
    JOYSTICK_SELECT
};

// Joystick pin definitions
#define JOYSTICK_UP_PIN     4
#define JOYSTICK_DOWN_PIN   5
#define JOYSTICK_SELECT_PIN 6

class Joystick {
public:
    Joystick();
    void init();
    JoystickDirection read();
    bool isPressed(JoystickDirection direction);
    void update();

private:
    unsigned long lastReadTime;
    unsigned long debounceDelay;
    JoystickDirection lastDirection;
    bool buttonStates[3];
    bool lastButtonStates[3];
    unsigned long lastDebounceTime[3];
    
    bool readPin(int pin);
};

extern Joystick joystick;

#endif
