#include "Arduino.h"
#include "BOARD.h"
#include "sleep.h"
#include "lora.h"
#include "PWR.h"
#include "display.h"
#include <WiFi.h>
#include <Wire.h>
#include <esp_sleep.h>
#include <driver/gpio.h>
#include <driver/rtc_io.h>

#ifdef BUTTON_PIN
#include <OneButton.h>
#endif

#ifdef BUTTON_PIN
static OneButton userButton(BUTTON_PIN, true, true);
#endif

static void sleep_hold_pin(int pin, int mode, int level) {
    pinMode(pin, mode);
    if (mode == OUTPUT) {
        digitalWrite(pin, level);
    }
    if (GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
        gpio_hold_en((gpio_num_t)pin);
    }
}

static void sleep_release_holds() {
    gpio_deep_sleep_hold_dis();

    for (int i = 0; i <= GPIO_NUM_MAX; i++) {
        if (rtc_gpio_is_valid_gpio((gpio_num_t)i)) {
            rtc_gpio_hold_dis((gpio_num_t)i);
        } else if (GPIO_IS_VALID_OUTPUT_GPIO((gpio_num_t)i)) {
            gpio_hold_dis((gpio_num_t)i);
        }
    }
}

static void onLongPress() {
    Serial.println(F("[SLEEP] Button long press"));
    doDeepSleep();
}

void Sleep_init() {
    sleep_release_holds();

#ifdef BUTTON_PIN
    userButton.setPressMs(2000);
    userButton.attachLongPressStart(onLongPress);
    Serial.printf("[SLEEP] Long-press GPIO%d for 2s to deep sleep\n", BUTTON_PIN);
#endif
}

void Sleep_loop() {
#ifdef BUTTON_PIN
    userButton.tick();
#endif
}

void doDeepSleep() {
    Serial.println(F("[SLEEP] Entering deep sleep (wake on button)"));

    Display_clear();
    Display_drawLargeString(0, 20, "Sleep");
    Display_flush();
    delay(250);

#ifdef BUTTON_PIN
    // Long-press is still held. Wait for release so ext1 (active-low)
    // does not wake the chip immediately.
    while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
    }
    delay(50);
#endif

    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);

    LORA_sleep();

#ifdef RADIO_CS_PIN
    sleep_hold_pin(RADIO_CS_PIN, OUTPUT, HIGH);
#endif

#ifdef BOARD_LED
    sleep_hold_pin(BOARD_LED, OUTPUT, LED_OFF);
#endif

#ifdef LCD_LED
    sleep_hold_pin(LCD_LED, OUTPUT, LOW);
#endif

#ifdef OLED_RST
    if (OLED_RST != UNUSED_PIN) {
        pinMode(OLED_RST, OUTPUT);
        digitalWrite(OLED_RST, HIGH);
    }
#endif

#ifdef VEXT_EN_PIN
    sleep_hold_pin(VEXT_EN_PIN, OUTPUT, !VEXT_EN_LEVEL);
#endif

#ifdef GPS_RST_PIN
#ifdef GPS_RST_LEVEL
    sleep_hold_pin(GPS_RST_PIN, OUTPUT, GPS_RST_LEVEL);
#else
    sleep_hold_pin(GPS_RST_PIN, OUTPUT, LOW);
#endif
#endif

#ifdef GPS_EN_PIN
    sleep_hold_pin(GPS_EN_PIN, OUTPUT, LOW);
#endif

#ifdef RADIO_LDO_EN
    sleep_hold_pin(RADIO_LDO_EN, OUTPUT, LOW);
#endif

#ifdef HAS_GPS
    Serial1.end();
#endif

    PWR_prepareSleep();

#ifdef I2C_SDA
    Wire.end();
    pinMode(I2C_SDA, ANALOG);
    pinMode(I2C_SCL, ANALOG);
#endif

#ifdef BUTTON_PIN
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    if (GPIO_IS_VALID_OUTPUT_GPIO(BUTTON_PIN)) {
        gpio_hold_en((gpio_num_t)BUTTON_PIN);
    }

    if (rtc_gpio_is_valid_gpio((gpio_num_t)BUTTON_PIN)) {
        rtc_gpio_pullup_en((gpio_num_t)BUTTON_PIN);
        rtc_gpio_pulldown_dis((gpio_num_t)BUTTON_PIN);
#if defined(CONFIG_IDF_TARGET_ESP32)
        esp_sleep_enable_ext1_wakeup(1ULL << BUTTON_PIN, ESP_EXT1_WAKEUP_ALL_LOW);
#else
        esp_sleep_enable_ext1_wakeup(1ULL << BUTTON_PIN, ESP_EXT1_WAKEUP_ANY_LOW);
#endif
    }
#endif

    gpio_deep_sleep_hold_en();
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    Serial.flush();
    delay(50);
    esp_deep_sleep_start();
}
