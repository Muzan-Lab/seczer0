#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include <Arduino.h>
#include "menu.h"
#include "Joystick.h"

// Menu states
enum MenuState {
    MENU_MAIN = 0,
    MENU_NFC_SUB,
    MENU_IR_SUB,
    MENU_IBUTTON_SUB,
    MENU_RF_SUB,
    MENU_GPIO_SUB,
    MENU_SETTINGS_SUB,
    MENU_MODULE_RUNNING
};

class MenuManager {
public:
    MenuManager();
    void init();
    void update();
    void handleInput(JoystickDirection input);
    void draw();
    
    // Menu navigation
    void goToMainMenu();
    void goToSubmenu(MenuState submenu);
    void goBack();
    void selectCurrentItem();
    
    // Getters
    MenuState getCurrentState() { return currentState; }
    int getCurrentSelection() { return currentSelection; }
    bool isInSubmenu() { return currentState != MENU_MAIN; }
    
    // Module execution
    void runModule(int moduleId, int actionId);

private:
    MenuState currentState;
    MenuState previousState;
    int currentSelection;
    int maxItems;
    bool needsRedraw;
    unsigned long lastInputTime;
    
    void updateMaxItems();
    void drawMainMenu();
    void drawCurrentSubmenu();
    const MenuItem* getCurrentMenuItems();
    int getCurrentMenuCount();
    void executeMenuAction();
    
    // Menu helpers
    void moveUp();
    void moveDown();
    void wrapSelection();
};

extern MenuManager menuManager;

#endif
