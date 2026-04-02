#pragma once

// Hardware-agnostic display API.
// Call Display_init() once with the driver name, then use the rest freely.
// 
// Display types are configured in BOARD.h using these defines:
// - DISP_ST7735_160_80: Enable ST7735 TFT display (160x80)
// - DISP_SSD1306_128_64: Enable SSD1306 OLED display (128x64)  
// - DISP_SH110X_128_64: Enable SH110X OLED display (128x64)
//
// Multiple OLED displays can be supported simultaneously.
// Supported models for Display_init(): "ST7735", "SSD1306", "SH110X"

// Color definitions for displays
enum DisplayColor {
    COLOR_BLACK = 0,
    COLOR_WHITE = 1,
    COLOR_RED = 2,
    COLOR_GREEN = 3,
    COLOR_BLUE = 4,
    COLOR_CYAN = 5,
    COLOR_MAGENTA = 6,
    COLOR_YELLOW = 7,
    COLOR_ORANGE = 8
};

void Display_init(String model);
void Display_clear();
void Display_drawString(uint16_t x, uint16_t y, const String &text);
void Display_drawLargeString(uint16_t x, uint16_t y, const String &text);
void Display_drawSplash();
void Display_refresh();

// Color functions
DisplayColor Display_getColor();
void Display_setColor(DisplayColor color);
void Display_setPixel(uint16_t x, uint16_t y);
void Display_setPixel(uint16_t x, uint16_t y, DisplayColor color);
void Display_clear(DisplayColor color);

// Advanced drawing functions
void Display_drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress);
void Display_drawRocketLaunch();
void Display_drawFinder();
void Display_drawCompass(int16_t x, int16_t y, float angle, float pitch, float roll);
void Display_fillCircle(uint16_t x, uint16_t y, uint16_t radius);
