#pragma once

typedef enum{
    HELTEC_WIRELESS_TRACKER_TFT,
    TTGO_OLED_SSD1306,
    TTGO_OLED_SH110X
} display_type_t;

void Display_init(String model);
void Display_clear();
void Display_drawString(uint16_t x, uint16_t y, const String &text);
void Display_drawLargeString(uint16_t x, uint16_t y, const String &text);
void Display_drawSplash();
void Display_refresh();