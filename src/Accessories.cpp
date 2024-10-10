#include "BOARD.h"
#include "Accessories.h"
void Accessories_init() {
#if defined(BAT_ADC_PIN)
    pinMode(BAT_ADC_PIN, INPUT);
#endif
#if defined(BAT_ADC_EN_PIN)
    pinMode(BAT_ADC_EN_PIN, OUTPUT);
    digitalWrite(BAT_ADC_EN_PIN, LOW);
#endif
}

float Accessories_getVBat() {
    float bat_v = 0.0f;

#if defined(BAT_ADC_EN_PIN)
    digitalWrite(BAT_ADC_EN_PIN, HIGH);
    delay(10);
#endif

#if defined(BAT_ADC_PIN)
    uint32_t in = 0;
    for (int i = 0; i < ADC_BATTERY_LEVEL_SAMPLES; i++)
    {
        in += (uint32_t)analogRead(BAT_ADC_PIN);
    }
    in = (int)in / ADC_BATTERY_LEVEL_SAMPLES;
    float bat_mv = ((float)in / 4096) * 3600 * 2;
    bat_v = bat_mv / 1000;
#endif

#if defined(BAT_ADC_EN_PIN)
    digitalWrite(BAT_ADC_EN_PIN, LOW);
#endif

    return bat_v;
}