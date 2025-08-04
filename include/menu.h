#ifndef MENU_H
#define MENU_H

#include <Arduino.h>

// Menu item structure
struct MenuItem {
    const char* name;
    const char* icon;
    int id;
};

// Main menu items
enum MainMenuItems {
    MENU_NFC = 0,
    MENU_IR,
    MENU_IBUTTON,
    MENU_RF,
    MENU_GPIO,
    MENU_SETTINGS,
    MENU_ABOUT,
    MENU_COUNT
};

// Submenu items for each module
enum NFCSubMenu {
    NFC_SCAN = 0,
    NFC_EMULATE,
    NFC_HISTORY,
    NFC_BACK,
    NFC_SUBMENU_COUNT
};

enum IRSubMenu {
    IR_SCAN = 0,
    IR_EMULATE,
    IR_HISTORY,
    IR_BACK,
    IR_SUBMENU_COUNT
};

enum iButtonSubMenu {
    IBUTTON_SCAN = 0,
    IBUTTON_EMULATE,
    IBUTTON_HISTORY,
    IBUTTON_BACK,
    IBUTTON_SUBMENU_COUNT
};

enum RFSubMenu {
    RF_SCAN = 0,
    RF_EMULATE,
    RF_HISTORY,
    RF_BACK,
    RF_SUBMENU_COUNT
};

enum GPIOSubMenu {
    GPIO_READ = 0,
    GPIO_WRITE,
    GPIO_ANALYZE,
    GPIO_BACK,
    GPIO_SUBMENU_COUNT
};

enum SettingsSubMenu {
    SETTINGS_DISPLAY = 0,
    SETTINGS_SOUND,
    SETTINGS_STORAGE,
    SETTINGS_ABOUT,
    SETTINGS_BACK,
    SETTINGS_SUBMENU_COUNT
};

// Menu definitions
extern const MenuItem mainMenu[MENU_COUNT];
extern const MenuItem nfcSubMenu[NFC_SUBMENU_COUNT];
extern const MenuItem irSubMenu[IR_SUBMENU_COUNT];
extern const MenuItem iButtonSubMenu[IBUTTON_SUBMENU_COUNT];
extern const MenuItem rfSubMenu[RF_SUBMENU_COUNT];
extern const MenuItem gpioSubMenu[GPIO_SUBMENU_COUNT];
extern const MenuItem settingsSubMenu[SETTINGS_SUBMENU_COUNT];

#endif
