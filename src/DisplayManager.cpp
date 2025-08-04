#include "DisplayManager.h"
#include "logo.h"

DisplayManager displayManager;

DisplayManager::DisplayManager() 
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
      displayInitialized(false), lastUpdate(0) {
}

bool DisplayManager::init() {
    Wire.begin();
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println("SSD1306 allocation failed");
        return false;
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    
    displayInitialized = true;
    Serial.println("Display initialized");
    
    return true;
}

void DisplayManager::clear() {
    if (!displayInitialized) return;
    display.clearDisplay();
}

void DisplayManager::display() {
    if (!displayInitialized) return;
    display.display();
    lastUpdate = millis();
}

void DisplayManager::drawStatusBar() {
    if (!displayInitialized) return;
    
    // Draw status bar background
    display.drawLine(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, STATUS_BAR_HEIGHT, SSD1306_WHITE);
    
    // Draw battery (right side)
    drawBattery(85); // Mock battery level
    
    // Draw SD card status
    drawSDStatus(true); // Mock SD status
    
    // Draw WiFi status
    drawWiFiStatus(false); // Mock WiFi status
    
    // Draw time (center)
    drawTime();
}

void DisplayManager::drawBattery(int percentage) {
    int x = SCREEN_WIDTH - 20;
    int y = 2;
    
    // Battery outline
    display.drawRect(x, y, 16, 6, SSD1306_WHITE);
    display.drawRect(x + 16, y + 1, 2, 4, SSD1306_WHITE);
    
    // Battery fill
    int fillWidth = (14 * percentage) / 100;
    if (fillWidth > 0) {
        display.fillRect(x + 1, y + 1, fillWidth, 4, SSD1306_WHITE);
    }
}

void DisplayManager::drawTime() {
    // Simple time display (hours:minutes)
    unsigned long seconds = millis() / 1000;
    int minutes = (seconds / 60) % 60;
    int hours = (seconds / 3600) % 24;
    
    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", hours, minutes);
    
    int textWidth = strlen(timeStr) * 6;
    int x = (SCREEN_WIDTH - textWidth) / 2;
    
    display.setCursor(x, 1);
    display.print(timeStr);
}

void DisplayManager::drawWiFiStatus(bool connected) {
    int x = SCREEN_WIDTH - 40;
    int y = 2;
    
    if (connected) {
        // WiFi connected icon
        display.drawLine(x, y + 5, x + 2, y + 3, SSD1306_WHITE);
        display.drawLine(x + 2, y + 3, x + 4, y + 5, SSD1306_WHITE);
        display.drawLine(x + 1, y + 4, x + 3, y + 2, SSD1306_WHITE);
    } else {
        // WiFi disconnected icon
        display.drawLine(x, y + 5, x + 4, y + 1, SSD1306_WHITE);
        display.drawLine(x, y + 1, x + 4, y + 5, SSD1306_WHITE);
    }
}

void DisplayManager::drawSDStatus(bool inserted) {
    int x = SCREEN_WIDTH - 60;
    int y = 2;
    
    // SD card outline
    display.drawRect(x, y, 8, 6, SSD1306_WHITE);
    display.drawLine(x + 6, y, x + 8, y + 2, SSD1306_WHITE);
    display.drawLine(x + 8, y + 2, x + 8, y + 6, SSD1306_WHITE);
    
    if (inserted) {
        // Fill when inserted
        display.fillRect(x + 1, y + 1, 5, 4, SSD1306_WHITE);
    }
}

void DisplayManager::drawMenu(const char* items[], int count, int selected) {
    if (!displayInitialized) return;
    
    int startY = MENU_AREA_Y;
    int itemHeight = 10;
    int visibleItems = MENU_AREA_HEIGHT / itemHeight;
    
    // Calculate scroll position
    int scrollOffset = 0;
    if (selected >= visibleItems) {
        scrollOffset = selected - visibleItems + 1;
    }
    
    // Draw menu items
    for (int i = 0; i < visibleItems && (i + scrollOffset) < count; i++) {
        int itemIndex = i + scrollOffset;
        int y = startY + (i * itemHeight);
        
        // Highlight selected item
        if (itemIndex == selected) {
            display.fillRect(0, y, SCREEN_WIDTH, itemHeight - 1, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
        } else {
            display.setTextColor(SSD1306_WHITE);
        }
        
        display.setCursor(5, y + 1);
        display.print(items[itemIndex]);
        
        // Draw selection arrow
        if (itemIndex == selected) {
            display.setCursor(1, y + 1);
            display.print(">");
        }
    }
    
    // Draw scrollbar if needed
    if (count > visibleItems) {
        drawScrollbar(count, visibleItems, scrollOffset);
    }
    
    display.setTextColor(SSD1306_WHITE); // Reset text color
}

void DisplayManager::drawSubmenu(const char* title, const char* items[], int count, int selected) {
    if (!displayInitialized) return;
    
    // Draw title
    display.setCursor(5, MENU_AREA_Y);
    display.print(title);
    display.drawLine(0, MENU_AREA_Y + 10, SCREEN_WIDTH, MENU_AREA_Y + 10, SSD1306_WHITE);
    
    // Draw menu items below title
    int startY = MENU_AREA_Y + 15;
    int itemHeight = 10;
    int maxItems = (MENU_AREA_HEIGHT - 15) / itemHeight;
    
    for (int i = 0; i < maxItems && i < count; i++) {
        int y = startY + (i * itemHeight);
        
        // Highlight selected item
        if (i == selected) {
            display.fillRect(0, y, SCREEN_WIDTH, itemHeight - 1, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
        } else {
            display.setTextColor(SSD1306_WHITE);
        }
        
        display.setCursor(5, y + 1);
        display.print(items[i]);
        
        // Draw selection arrow
        if (i == selected) {
            display.setCursor(1, y + 1);
            display.print(">");
        }
    }
    
    display.setTextColor(SSD1306_WHITE); // Reset text color
}

void DisplayManager::drawModuleScreen(const char* title, const char* content) {
    if (!displayInitialized) return;
    
    // Draw title bar
    display.fillRect(0, MENU_AREA_Y, SCREEN_WIDTH, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(5, MENU_AREA_Y + 2);
    display.print(title);
    
    // Draw content
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5, MENU_AREA_Y + 20);
    display.print(content);
}

void DisplayManager::drawProgressBar(int percentage) {
    if (!displayInitialized) return;
    
    int barWidth = 100;
    int barHeight = 8;
    int x = (SCREEN_WIDTH - barWidth) / 2;
    int y = SCREEN_HEIGHT - 20;
    
    // Draw progress bar outline
    display.drawRect(x, y, barWidth, barHeight, SSD1306_WHITE);
    
    // Draw progress fill
    int fillWidth = (barWidth - 2) * percentage / 100;
    if (fillWidth > 0) {
        display.fillRect(x + 1, y + 1, fillWidth, barHeight - 2, SSD1306_WHITE);
    }
    
    // Draw percentage text
    char percentStr[5];
    sprintf(percentStr, "%d%%", percentage);
    int textWidth = strlen(percentStr) * 6;
    display.setCursor((SCREEN_WIDTH - textWidth) / 2, y - 12);
    display.print(percentStr);
}

void DisplayManager::drawScrollText(const char* text, int x, int y, int maxWidth) {
    if (!displayInitialized) return;
    
    int textLen = strlen(text);
    int textWidth = textLen * 6;
    
    if (textWidth <= maxWidth) {
        // Text fits, draw normally
        display.setCursor(x, y);
        display.print(text);
    } else {
        // Text is too long, implement scrolling
        static unsigned long lastScrollTime = 0;
        static int scrollOffset = 0;
        
        if (millis() - lastScrollTime > 200) { // Scroll every 200ms
            scrollOffset++;
            if (scrollOffset > textLen) {
                scrollOffset = -maxWidth / 6;
            }
            lastScrollTime = millis();
        }
        
        display.setCursor(x - (scrollOffset * 6), y);
        display.print(text);
    }
}

void DisplayManager::showBootAnimation() {
    if (!displayInitialized) return;
    
    // Clear display
    display.clearDisplay();
    
    // Draw wolf logo with animation
    for (int frame = 0; frame < 10; frame++) {
        display.clearDisplay();
        
        // Draw logo centered
        int logoX = (SCREEN_WIDTH - LOGO_WIDTH) / 2;
        int logoY = (SCREEN_HEIGHT - LOGO_HEIGHT) / 2;
        
        display.drawBitmap(logoX, logoY, wolf_logo_bitmap, LOGO_WIDTH, LOGO_HEIGHT, SSD1306_WHITE);
        
        // Add fade effect or animation
        if (frame < 5) {
            // Growing effect
            int size = (frame + 1) * LOGO_HEIGHT / 5;
            int offsetY = (LOGO_HEIGHT - size) / 2;
            display.fillRect(logoX, logoY + offsetY + size, LOGO_WIDTH, offsetY, SSD1306_BLACK);
            display.fillRect(logoX, logoY, LOGO_WIDTH, offsetY, SSD1306_BLACK);
        }
        
        display.display();
        delay(200);
    }
    
    // Show "FlipperS3" text
    display.clearDisplay();
    drawCenteredText("FlipperS3", 25);
    drawCenteredText("v1.0", 40);
    display.display();
    delay(1500);
}

void DisplayManager::drawLogo() {
    if (!displayInitialized) return;
    
    int logoX = (SCREEN_WIDTH - LOGO_WIDTH) / 2;
    int logoY = (SCREEN_HEIGHT - LOGO_HEIGHT) / 2;
    
    display.drawBitmap(logoX, logoY, wolf_logo_bitmap, LOGO_WIDTH, LOGO_HEIGHT, SSD1306_WHITE);
}

void DisplayManager::drawCenteredText(const char* text, int y) {
    if (!displayInitialized) return;
    
    int textWidth = strlen(text) * 6; // Assuming 6 pixels per character
    int x = (SCREEN_WIDTH - textWidth) / 2;
    
    display.setCursor(x, y);
    display.print(text);
}

void DisplayManager::drawIcon(int x, int y, const char* icon) {
    if (!displayInitialized) return;
    
    display.setCursor(x, y);
    display.print(icon);
}

void DisplayManager::drawFrame(int x, int y, int width, int height) {
    if (!displayInitialized) return;
    
    display.drawRect(x, y, width, height, SSD1306_WHITE);
}

void DisplayManager::drawScrollbar(int totalItems, int visibleItems, int scrollPosition) {
    if (!displayInitialized || totalItems <= visibleItems) return;
    
    int scrollbarX = SCREEN_WIDTH - 3;
    int scrollbarY = MENU_AREA_Y;
    int scrollbarHeight = MENU_AREA_HEIGHT;
    
    // Draw scrollbar track
    display.drawLine(scrollbarX, scrollbarY, scrollbarX, scrollbarY + scrollbarHeight, SSD1306_WHITE);
    
    // Calculate thumb size and position
    int thumbHeight = (scrollbarHeight * visibleItems) / totalItems;
    if (thumbHeight < 3) thumbHeight = 3; // Minimum thumb size
    
    int thumbPosition = (scrollbarHeight - thumbHeight) * scrollPosition / (totalItems - visibleItems);
    
    // Draw scrollbar thumb
    display.fillRect(scrollbarX - 1, scrollbarY + thumbPosition, 3, thumbHeight, SSD1306_WHITE);
}
