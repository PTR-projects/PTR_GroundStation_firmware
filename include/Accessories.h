#pragma once

#include <Arduino.h>

#define BAT_ADC_PIN 35
#define ADC_BATTERY_LEVEL_SAMPLES 5

void Accessories_init();
float Accessories_getVBat();