#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// Display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Display regions
#define STATUS_BAR_HEIGHT 10
#define MENU_AREA_Y (STATUS_BAR_HEIGHT + 2)
#define MENU_AREA_HEIGHT (SCREEN_HEIGHT - STATUS_BAR_HEIGHT - 2)

class DisplayManager {
public:
    DisplayManager();
    bool init();
    void clear();
    void display();
    void drawStatusBar();
    void drawBattery(int percentage);
    void drawTime();
    void drawWiFiStatus(bool connected);
    void drawSDStatus(bool inserted);
    
    // Menu display functions
    void drawMenu(const char* items[], int count, int selected);
    void drawSubmenu(const char* title, const char* items[], int count, int selected);
    void drawModuleScreen(const char* title, const char* content);
    void drawProgressBar(int percentage);
    void drawScrollText(const char* text, int x, int y, int maxWidth);
    
    // Boot animation
    void showBootAnimation();
    void drawLogo();
    
    // Utility functions
    void drawCenteredText(const char* text, int y);
    void drawIcon(int x, int y, const char* icon);
    void drawFrame(int x, int y, int width, int height);
    void drawScrollbar(int totalItems, int visibleItems, int scrollPosition);
    
    // Get display instance
    Adafruit_SSD1306* getDisplay() { return &display; }

private:
    Adafruit_SSD1306 display;
    bool displayInitialized;
    unsigned long lastUpdate;
    
    void drawBatteryIcon(int x, int y, int percentage);
    void drawWiFiIcon(int x, int y, bool connected);
    void drawSDIcon(int x, int y, bool inserted);
};

extern DisplayManager displayManager;

#endif
