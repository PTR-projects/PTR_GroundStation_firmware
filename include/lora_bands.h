#pragma once

#include "BOARD.h"

struct LoraChannel {
    int freq_khz;
    const char *label;
};

#if defined(USING_SX1278) && (defined(LORA_BAND_868) || defined(LORA_BAND_915))
#error "SX1278 hardware only supports the 433 MHz band"
#endif

#if defined(LORA_BAND_868)
static const LoraChannel LORA_CHANNELS[] = {
    {868250, "CH0 868.250 MHz"},
    {868375, "CH1 868.375 MHz"},
    {868500, "CH2 868.500 MHz"},
    {868625, "CH3 868.625 MHz"},
    {868750, "CH4 868.750 MHz"},
    {868875, "CH5 868.875 MHz"},
    {869000, "CH6 869.000 MHz"},
};
#elif defined(LORA_BAND_915)
static const LoraChannel LORA_CHANNELS[] = {
    {915250, "CH0 915.250 MHz"},
    {915375, "CH1 915.375 MHz"},
    {915500, "CH2 915.500 MHz"},
    {915625, "CH3 915.625 MHz"},
    {915750, "CH4 915.750 MHz"},
    {915875, "CH5 915.875 MHz"},
    {915000, "CH6 915.000 MHz"},
};
#else
static const LoraChannel LORA_CHANNELS[] = {
    {434250, "CH0 434.250 MHz"},
    {434375, "CH1 434.375 MHz"},
    {434500, "CH2 434.500 MHz"},
    {434625, "CH3 434.625 MHz"},
};
#endif

static const int LORA_CHANNEL_COUNT = (int)(sizeof(LORA_CHANNELS) / sizeof(LORA_CHANNELS[0]));
static const int LORA_BAND_DEFAULT_KHZ = LORA_CHANNELS[0].freq_khz;

static inline bool lora_frequency_is_valid(int freq_khz) {
    for (int i = 0; i < LORA_CHANNEL_COUNT; i++) {
        if (LORA_CHANNELS[i].freq_khz == freq_khz) {
            return true;
        }
    }
    return false;
}
