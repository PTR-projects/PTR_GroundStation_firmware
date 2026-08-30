#include "BOARD.h"
#include "Accessories.h"

static void Accessories_enableBatADC(bool state);

void Accessories_init() {
#if defined(BAT_ADC_PIN)
    pinMode(BAT_ADC_PIN, INPUT);
#endif

#if defined(BAT_ADC_EN_PIN) && !defined(BAT_ADC_EN_HELTEC_FIX)
    pinMode(BAT_ADC_EN_PIN, OUTPUT);
    digitalWrite(BAT_ADC_EN_PIN, !BAT_ADC_EN_LEVEL);
#endif

#if defined(BAT_ADC_EN_PIN) && defined(BAT_ADC_EN_HELTEC_FIX)
    pinMode(BAT_ADC_EN_PIN, INPUT_PULLDOWN);
#endif
}

float Accessories_getVBat() {
    float bat_v = 0.0f;

#if defined(BAT_ADC_EN_PIN)
    Accessories_enableBatADC(true);
    delay(10);
#endif

#if defined(BAT_ADC_PIN)
    uint32_t in = 0;
    for (int i = 0; i < ADC_BATTERY_LEVEL_SAMPLES; i++)
    {
        in += (uint32_t)analogRead(BAT_ADC_PIN);
    }
    in = (int)in / ADC_BATTERY_LEVEL_SAMPLES;
    float bat_mv = ((float)in / 4095.0f) * 3300.0f * 1.1f * (float)BAT_ADC_MULTIPLIER;
    bat_v = bat_mv / 1000.0f;
#endif

Accessories_enableBatADC(false);

    return bat_v;
}

static void Accessories_enableBatADC(bool state){
#if defined(BAT_ADC_EN_PIN) && !defined(BAT_ADC_EN_HELTEC_FIX)
    digitalWrite(BAT_ADC_EN_PIN, state?BAT_ADC_EN_LEVEL:(!BAT_ADC_EN_LEVEL));
#endif

#if defined(BAT_ADC_EN_PIN) && defined(BAT_ADC_EN_HELTEC_FIX)
    pinMode(BAT_ADC_EN_PIN, state?INPUT_PULLUP:INPUT_PULLDOWN);
#endif
}