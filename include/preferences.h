#pragma once

#include <Arduino.h>
#include "BOARD.h"

#if HAS_OLED_DISPLAY
typedef enum{
    SSD1306 = 0,
    SH1106 = 1
} OLED_driver_e;
#endif

typedef struct{
    int frequency;
    int id;
    bool id_filter;
#if HAS_OLED_DISPLAY
    OLED_driver_e oled_driver;
#endif
}config_data_t;


int preferences_init();
int preferences_save();

int preferences_get_frequency();
int preferences_get_id();
bool preferences_get_id_filter();
#if HAS_OLED_DISPLAY
String preferences_get_OLEDdriver();
void preferences_update_OLEDdriver(String driver);
#endif

void preferences_update_frequency(int frequency);
void preferences_update_id(int id);
void preferences_update_id_filter(bool enabled);
