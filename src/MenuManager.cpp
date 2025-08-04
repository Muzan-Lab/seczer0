#include "MenuManager.h"
#include "DisplayManager.h"
#include "menu.h"
#include "NFCModule.h"
#include "IRModule.h"
#include "iButtonModule.h"
#include "RFModule.h"
#include "GPIOModule.h"

MenuManager menuManager;

// Menu item definitions
const MenuItem mainMenu[MENU_COUNT] = {
    {"NFC", "📡", MENU_NFC},
    {"Infrared", "📶", MENU_IR},
    {"iButton", "🔑", MENU_IBUTTON},
    {"Sub-GHz", "📻", MENU_RF},
    {"GPIO", "🔌", MENU_GPIO},
    {"Settings", "⚙️", MENU_SETTINGS},
    {"About", "ℹ️", MENU_ABOUT}
};

const MenuItem nfcSubMenu[NFC_SUBMENU_COUNT] = {
    {"Scan NFC", "🔍", NFC_SCAN},
    {"Emulate", "📡", NFC_EMULATE},
    {"History", "📋", NFC_HISTORY},
    {"< Back", "←", NFC_BACK}
};

const MenuItem irSubMenu[IR_SUBMENU_COUNT] = {
    {"Learn Remote", "📖", IR_SCAN},
    {"Send Signal", "📶", IR_EMULATE},
    {"Saved Signals", "💾", IR_HISTORY},
    {"< Back", "←", IR_BACK}
};

const MenuItem iButtonSubMenu[IBUTTON_SUBMENU_COUNT] = {
    {"Read Key", "🔍", IBUTTON_SCAN},
    {"Emulate Key", "🔑", IBUTTON_EMULATE},
    {"Saved Keys", "💾", IBUTTON_HISTORY},
    {"< Back", "←", IBUTTON_BACK}
};

const MenuItem rfSubMenu[RF_SUBMENU_COUNT] = {
    {"Frequency Scan", "📡", RF_SCAN},
    {"Transmit", "📻", RF_EMULATE},
    {"Saved Signals", "💾", RF_HISTORY},
    {"< Back", "←", RF_BACK}
};

const MenuItem gpioSubMenu[GPIO_SUBMENU_COUNT] = {
    {"Pin Reader", "📖", GPIO_READ},
    {"Pin Writer", "✏️", GPIO_WRITE},
    {"Logic Analyzer", "📊", GPIO_ANALYZE},
    {"< Back", "←", GPIO_BACK}
};

const MenuItem settingsSubMenu[SETTINGS_SUBMENU_COUNT] = {
    {"Display", "🖥️", SETTINGS_DISPLAY},
    {"Sound", "🔊", SETTINGS_SOUND},
    {"Storage", "💾", SETTINGS_STORAGE},
    {"About", "ℹ️", SETTINGS_ABOUT},
    {"< Back", "←", SETTINGS_BACK}
};

MenuManager::MenuManager() 
    : currentState(MENU_MAIN), previousState(MENU_MAIN), 
      currentSelection(0), maxItems(MENU_COUNT), 
      needsRedraw(true), lastInputTime(0) {
}

void MenuManager::init() {
    currentState = MENU_MAIN;
    currentSelection = 0;
    maxItems = MENU_COUNT;
    needsRedraw = true;
    
    Serial.println("Menu manager initialized");
}

void MenuManager::update() {
    // Auto-redraw if needed
    if (needsRedraw) {
        draw();
        needsRedraw = false;
    }
}

void MenuManager::handleInput(JoystickDirection input) {
    lastInputTime = millis();
    
    switch (input) {
        case JOYSTICK_UP:
            moveUp();
            break;
            
        case JOYSTICK_DOWN:
            moveDown();
            break;
            
        case JOYSTICK_SELECT:
            selectCurrentItem();
            break;
            
        default:
            break;
    }
    
    needsRedraw = true;
}

void MenuManager::moveUp() {
    currentSelection--;
    if (currentSelection < 0) {
        currentSelection = maxItems - 1;
    }
}

void MenuManager::moveDown() {
    currentSelection++;
    if (currentSelection >= maxItems) {
        currentSelection = 0;
    }
}

void MenuManager::selectCurrentItem() {
    switch (currentState) {
        case MENU_MAIN:
            // Handle main menu selection
            switch (currentSelection) {
                case MENU_NFC:
                    goToSubmenu(MENU_NFC_SUB);
                    break;
                case MENU_IR:
                    goToSubmenu(MENU_IR_SUB);
                    break;
                case MENU_IBUTTON:
                    goToSubmenu(MENU_IBUTTON_SUB);
                    break;
                case MENU_RF:
                    goToSubmenu(MENU_RF_SUB);
                    break;
                case MENU_GPIO:
                    goToSubmenu(MENU_GPIO_SUB);
                    break;
                case MENU_SETTINGS:
                    goToSubmenu(MENU_SETTINGS_SUB);
                    break;
                case MENU_ABOUT:
                    // Show about screen
                    displayManager.clear();
                    displayManager.drawModuleScreen("About", "FlipperS3 v1.0\nESP32-S3 Multi-tool\n\nPress SELECT to return");
                    displayManager.display();
                    delay(3000);
                    needsRedraw = true;
                    break;
            }
            break;
            
        case MENU_NFC_SUB:
            if (currentSelection == NFC_BACK) {
                goBack();
            } else {
                runModule(MENU_NFC, currentSelection);
            }
            break;
            
        case MENU_IR_SUB:
            if (currentSelection == IR_BACK) {
                goBack();
            } else {
                runModule(MENU_IR, currentSelection);
            }
            break;
            
        case MENU_IBUTTON_SUB:
            if (currentSelection == IBUTTON_BACK) {
                goBack();
            } else {
                runModule(MENU_IBUTTON, currentSelection);
            }
            break;
            
        case MENU_RF_SUB:
            if (currentSelection == RF_BACK) {
                goBack();
            } else {
                runModule(MENU_RF, currentSelection);
            }
            break;
            
        case MENU_GPIO_SUB:
            if (currentSelection == GPIO_BACK) {
                goBack();
            } else {
                runModule(MENU_GPIO, currentSelection);
            }
            break;
            
        case MENU_SETTINGS_SUB:
            if (currentSelection == SETTINGS_BACK) {
                goBack();
            } else {
                // Handle settings options
                displayManager.clear();
                displayManager.drawModuleScreen("Settings", "Feature not implemented\n\nPress SELECT to return");
                displayManager.display();
                delay(2000);
                needsRedraw = true;
            }
            break;
            
        default:
            break;
    }
}

void MenuManager::goToMainMenu() {
    previousState = currentState;
    currentState = MENU_MAIN;
    currentSelection = 0;
    maxItems = MENU_COUNT;
}

void MenuManager::goToSubmenu(MenuState submenu) {
    previousState = currentState;
    currentState = submenu;
    currentSelection = 0;
    
    // Update max items based on submenu
    switch (submenu) {
        case MENU_NFC_SUB:
            maxItems = NFC_SUBMENU_COUNT;
            break;
        case MENU_IR_SUB:
            maxItems = IR_SUBMENU_COUNT;
            break;
        case MENU_IBUTTON_SUB:
            maxItems = IBUTTON_SUBMENU_COUNT;
            break;
        case MENU_RF_SUB:
            maxItems = RF_SUBMENU_COUNT;
            break;
        case MENU_GPIO_SUB:
            maxItems = GPIO_SUBMENU_COUNT;
            break;
        case MENU_SETTINGS_SUB:
            maxItems = SETTINGS_SUBMENU_COUNT;
            break;
        default:
            maxItems = MENU_COUNT;
            break;
    }
}

void MenuManager::goBack() {
    if (currentState != MENU_MAIN) {
        currentState = MENU_MAIN;
        currentSelection = 0;
        maxItems = MENU_COUNT;
    }
}

void MenuManager::draw() {
    displayManager.clear();
    
    switch (currentState) {
        case MENU_MAIN:
            drawMainMenu();
            break;
        default:
            drawCurrentSubmenu();
            break;
    }
    
    displayManager.drawStatusBar();
    displayManager.display();
}

void MenuManager::drawMainMenu() {
    const char* menuItems[MENU_COUNT];
    
    for (int i = 0; i < MENU_COUNT; i++) {
        menuItems[i] = mainMenu[i].name;
    }
    
    displayManager.drawMenu(menuItems, MENU_COUNT, currentSelection);
}

void MenuManager::drawCurrentSubmenu() {
    const MenuItem* items = getCurrentMenuItems();
    const char* menuItems[maxItems];
    const char* title = "";
    
    // Get submenu title
    switch (currentState) {
        case MENU_NFC_SUB:
            title = "NFC Tools";
            break;
        case MENU_IR_SUB:
            title = "Infrared";
            break;
        case MENU_IBUTTON_SUB:
            title = "iButton";
            break;
        case MENU_RF_SUB:
            title = "Sub-GHz";
            break;
        case MENU_GPIO_SUB:
            title = "GPIO Tools";
            break;
        case MENU_SETTINGS_SUB:
            title = "Settings";
            break;
        default:
            title = "Menu";
            break;
    }
    
    // Convert menu items to string array
    for (int i = 0; i < maxItems; i++) {
        menuItems[i] = items[i].name;
    }
    
    displayManager.drawSubmenu(title, menuItems, maxItems, currentSelection);
}

const MenuItem* MenuManager::getCurrentMenuItems() {
    switch (currentState) {
        case MENU_NFC_SUB:
            return nfcSubMenu;
        case MENU_IR_SUB:
            return irSubMenu;
        case MENU_IBUTTON_SUB:
            return iButtonSubMenu;
        case MENU_RF_SUB:
            return rfSubMenu;
        case MENU_GPIO_SUB:
            return gpioSubMenu;
        case MENU_SETTINGS_SUB:
            return settingsSubMenu;
        default:
            return mainMenu;
    }
}

void MenuManager::runModule(int moduleId, int actionId) {
    displayManager.clear();
    
    switch (moduleId) {
        case MENU_NFC:
            switch (actionId) {
                case NFC_SCAN:
                    displayManager.drawModuleScreen("NFC Scan", "Scanning for NFC cards...\n\nHold card near device\nPress SELECT to stop");
                    break;
                case NFC_EMULATE:
                    displayManager.drawModuleScreen("NFC Emulate", "Select card to emulate\n\nNo saved cards found\nPress SELECT to return");
                    break;
                case NFC_HISTORY:
                    displayManager.drawModuleScreen("NFC History", "Recent NFC cards:\n\nNo history available\nPress SELECT to return");
                    break;
            }
            break;
            
        case MENU_IR:
            switch (actionId) {
                case IR_SCAN:
                    displayManager.drawModuleScreen("IR Learn", "Learning IR signal...\n\nPoint remote at device\nPress SELECT to stop");
                    break;
                case IR_EMULATE:
                    displayManager.drawModuleScreen("IR Send", "Select signal to send\n\nNo saved signals\nPress SELECT to return");
                    break;
                case IR_HISTORY:
                    displayManager.drawModuleScreen("IR History", "Recent IR signals:\n\nNo history available\nPress SELECT to return");
                    break;
            }
            break;
            
        case MENU_IBUTTON:
            switch (actionId) {
                case IBUTTON_SCAN:
                    displayManager.drawModuleScreen("iButton Read", "Reading iButton key...\n\nTouch key to device\nPress SELECT to stop");
                    break;
                case IBUTTON_EMULATE:
                    displayManager.drawModuleScreen("iButton Emulate", "Select key to emulate\n\nNo saved keys found\nPress SELECT to return");
                    break;
                case IBUTTON_HISTORY:
                    displayManager.drawModuleScreen("iButton History", "Recent iButton keys:\n\nNo history available\nPress SELECT to return");
                    break;
            }
            break;
            
        case MENU_RF:
            switch (actionId) {
                case RF_SCAN:
                    displayManager.drawModuleScreen("RF Scan", "Scanning frequencies...\n\n433.92 MHz\nPress SELECT to stop");
                    break;
                case RF_EMULATE:
                    displayManager.drawModuleScreen("RF Transmit", "Select signal to send\n\nNo saved signals\nPress SELECT to return");
                    break;
                case RF_HISTORY:
                    displayManager.drawModuleScreen("RF History", "Recent RF signals:\n\nNo history available\nPress SELECT to return");
                    break;
            }
            break;
            
        case MENU_GPIO:
            switch (actionId) {
                case GPIO_READ:
                    displayManager.drawModuleScreen("GPIO Read", "Pin states:\n\nSelect pins to monitor\nPress SELECT to return");
                    break;
                case GPIO_WRITE:
                    displayManager.drawModuleScreen("GPIO Write", "Pin control:\n\nSelect pins to control\nPress SELECT to return");
                    break;
                case GPIO_ANALYZE:
                    displayManager.drawModuleScreen("Logic Analyzer", "Analyzing signals...\n\nNo activity detected\nPress SELECT to stop");
                    break;
            }
            break;
    }
    
    displayManager.display();
    
    // Wait for user input to return
    while (true) {
        joystick.update();
        JoystickDirection input = joystick.read();
        
        if (input == JOYSTICK_SELECT) {
            break;
        }
        
        delay(50);
    }
    
    needsRedraw = true;
}
