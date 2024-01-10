#include "Accessories.h"
void Accessories_init() {
    pinMode(BAT_ADC_PIN, INPUT);
}

float Accessories_getVBat() {
    uint32_t in = 0;
    for (int i = 0; i < ADC_BATTERY_LEVEL_SAMPLES; i++)
    {
        in += (uint32_t)analogRead(BAT_ADC_PIN);
    }
    in = (int)in / ADC_BATTERY_LEVEL_SAMPLES;
    float bat_mv = ((float)in / 4096) * 3600 * 2;
    float bat_v = bat_mv / 1000;
    return bat_v;
}