/**
 * @file      utilities.h
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2024-05-12
 * @last-update 2024-08-07
 */
#pragma once


// Support board list, select in platformio.ini configuration
// #define LORA32_SX1278

// #define T_BEAM_SX1262
// #define T_BEAM_SX1276
// #define T_BEAM_SX1278

// #define T3_V1_3_SX1276
// #define T3_V1_3_SX1278

// #define T3_V1_6_SX1276
// #define T3_V1_6_SX1278

// #define T3_V1_6_SX1276_TCXO
// #define T3_V3_0_SX1262
// #define T3_V3_0_SX1276
// #define T3_V3_0_SX1278
// #define T3_V3_0_LR1121

// #define T3_S3_V1_2_SX1262
// #define T3_S3_V1_2_SX1276
// #define T3_S3_V1_2_SX1278
// #define T3_S3_V1_2_LR1121
// #define HELTEC_TRACKER
// #define HELTEC_LORA32

#define UNUSED_PIN                   (-1)

//------- T-BEAM + SX1262/SX1276/SX1278 -----------------------
// https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/schematic/LilyGo_TBeam_V1.2-lastest.pdf
#if defined(T_BEAM_SX1262) || defined(T_BEAM_SX1276) || defined(T_BEAM_SX1278)
#define HAS_GPS
#define HAS_PMU
#define HAS_RADIO
#define HAS_DISPLAY

#if   defined(T_BEAM_SX1262)
#define USING_SX1262
#elif defined(T_BEAM_SX1276)
#define USING_SX1276
#elif defined(T_BEAM_SX1278)
#define USING_SX1278
#endif // T_BEAM_SX1262

#define GPS_RX_PIN                  34
#define GPS_TX_PIN                  12
#define GPS_BAUDRATE                9600
#define BUTTON_PIN                  38
#define BUTTON_PIN_MASK             GPIO_SEL_38
#define I2C_SDA                     21
#define I2C_SCL                     22
#define OLED_RST                    UNUSED_PIN
#define PMU_IRQ                     35
#define RADIO_SCLK_PIN              5
#define RADIO_MISO_PIN              19
#define RADIO_MOSI_PIN              27
#define RADIO_CS_PIN                18
#define RADIO_DIO0_PIN              26
#define RADIO_RST_PIN               23
#define RADIO_DIO1_PIN              33
// SX1276/78
#define RADIO_DIO2_PIN              32
// SX1262
#define RADIO_BUSY_PIN              32
#define BOARD_LED                   4
#define LED_ON                      LOW
#define LED_OFF                     HIGH
#define PMU_WIRE_PORT               Wire

#define DISP_ST7735_160_80     0    // Enable TFT
#define DISP_SSD1306_128_64    1    // Enable SSD1306 OLED  
#define DISP_SH110X_128_64     1    // Disable SH110X OLED
#define BOARD_VARIANT_NAME          "T-Beam"

//------- T3 v1.3 + SX1276/SX1278 -----------------------
// https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/schematic/T3_V1.3.pdf
#elif defined(T3_V1_3_SX1276) || defined(T3_V1_3_SX1278)
#define HAS_RADIO
#define HAS_DISPLAY

#if   defined(T3_V1_3_SX1276)
#define USING_SX1276
#elif defined(T3_V1_3_SX1278)
#define USING_SX1278
#endif // T3_V1_3_SX1276

#define I2C_SDA                     21
#define I2C_SCL                     22
#define OLED_RST                    UNUSED_PIN
#define RADIO_SCLK_PIN              5
#define RADIO_MISO_PIN              19
#define RADIO_MOSI_PIN              27
#define RADIO_CS_PIN                18
#define RADIO_DIO0_PIN              26
#define RADIO_RST_PIN               14
#define RADIO_DIO1_PIN              33
// SX1276/78
#define RADIO_DIO2_PIN              32
// SX1262
#define RADIO_BUSY_PIN              32
#define BAT_ADC_PIN                 35
#define BAT_ADC_MULTIPLIER          (2.0f)
#define DISP_ST7735_160_80     0    // Enable TFT
#define DISP_SSD1306_128_64    1    // Enable SSD1306 OLED  
#define DISP_SH110X_128_64     1    // Disable SH110X OLED
#define BOARD_VARIANT_NAME          "T3 V1.3"

//------- T3 v1.6 + SX1276/8 -----------------------
// https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/schematic/T3_V1.6.1.pdf
#elif defined(T3_V1_6_SX1276) || defined(T3_V1_6_SX1278)
#define HAS_RADIO
#define HAS_SDCARD
#define HAS_DISPLAY

#if   defined(T3_V1_6_SX1276)
#define USING_SX1276
#elif defined(T3_V1_6_SX1278)
#define USING_SX1278
#endif // T3_V1_6_SX1276

#define I2C_SDA                     21
#define I2C_SCL                     22
#define OLED_RST                    UNUSED_PIN
#define RADIO_SCLK_PIN              5
#define RADIO_MISO_PIN              19
#define RADIO_MOSI_PIN              27
#define RADIO_CS_PIN                18
#define RADIO_DIO0_PIN              26
#define RADIO_RST_PIN               23
#define RADIO_DIO1_PIN              33
// SX1276/78
#define RADIO_DIO2_PIN              32
// SX1262
#define RADIO_BUSY_PIN              32
#define SDCARD_MOSI                 15
#define SDCARD_MISO                 2
#define SDCARD_SCLK                 14
#define SDCARD_CS                   13
#define SDCARD_DAT2                 12
#define SDCARD_DAT1                 4
#define BOARD_LED                   25
#define LED_ON                      HIGH
#define LED_OFF                     LOW
#define BAT_ADC_PIN                 35
#define BAT_ADC_MULTIPLIER          (2.0f)
#define DISP_ST7735_160_80     0    // Enable TFT
#define DISP_SSD1306_128_64    1    // Enable SSD1306 OLED  
#define DISP_SH110X_128_64     1    // Disable SH110X OLED
#define BOARD_VARIANT_NAME          "T3 V1.6"

//------- T3 v1.6 + SX1276 + TCXO -----------------------
// https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/schematic/T3_V1.6.1.pdf
#elif defined(T3_V1_6_SX1276_TCXO)
#define HAS_RADIO
#define HAS_SDCARD
#define HAS_DISPLAY

// SX1276
#define USING_SX1276
#define I2C_SDA                     21
#define I2C_SCL                     22
#define OLED_RST                    UNUSED_PIN
#define RADIO_SCLK_PIN              5
#define RADIO_MISO_PIN              19
#define RADIO_MOSI_PIN              27
#define RADIO_CS_PIN                18
#define RADIO_DIO0_PIN              26
#define RADIO_RST_PIN               23
#define RADIO_DIO1_PIN              -1//33
/*
* In the T3 V1.6.1 TCXO version, Radio DIO1 is connected to Radio’s
* internal temperature-compensated crystal oscillator enable
* */
// TCXO pin must be set to HIGH before enabling Radio
#define RADIO_TCXO_ENABLE           33
#define RADIO_BUSY_PIN              32
#define SDCARD_MOSI                 15
#define SDCARD_MISO                 2
#define SDCARD_SCLK                 14
#define SDCARD_CS                   13
#define SDCARD_DAT2                 12
#define SDCARD_DAT1                 4
#define BOARD_LED                   25
#define LED_ON                      HIGH
#define LED_OFF                     LOW
#define BAT_ADC_PIN                 35
#define BAT_ADC_MULTIPLIER          (2.0f)
#define DISP_ST7735_160_80     0    // Enable TFT
#define DISP_SSD1306_128_64    1    // Enable SSD1306 OLED  
#define DISP_SH110X_128_64     1    // Disable SH110X OLED
#define BOARD_VARIANT_NAME          "T3 V1.6 TCXO"

//------- T3 v3.0 SX1262/SX1276/SX1278/LR1121 + TCXO -----------------------
// https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/schematic/T3_V3.0.pdf
#elif defined(T3_V3_0_SX1262) || defined(T3_V3_0_SX1276) || defined(T3_V3_0_SX1278) || defined(T3_V3_0_LR1121)
#define HAS_RADIO
#define HAS_SDCARD
#define HAS_DISPLAY

#if   defined(T3_V3_0_SX1262)
#define USING_SX1262
#elif defined(T3_V3_0_SX1276)
#define USING_SX1276
#elif defined(T3_V3_0_SX1278)
#define USING_SX1278
#elif defined(T3_V3_0_LR1121)
#define USING_LR1121
#endif // T3_V3_0_SX1262

#define I2C_SDA                     21
#define I2C_SCL                     22
#define OLED_RST                    4
#define RADIO_SCLK_PIN              5
#define RADIO_MISO_PIN              19
#define RADIO_MOSI_PIN              27
#define RADIO_CS_PIN                18
#define RADIO_RST_PIN               23
// TCXO pin must be set to HIGH before enabling Radio
#define RADIO_TCXO_ENABLE           12  //only sx1276 tcxo version
#define RADIO_BUSY_PIN              32
// SX1262
#if defined(USING_SX1262)
#define RADIO_DIO1_PIN              26
#define RADIO_BUSY_PIN              32
// SX1276/78
#elif defined(USING_SX1276) || defined(USING_SX1278)
#define RADIO_DIO0_PIN              26
#define RADIO_DIO1_PIN              32
// LR1121
#elif defined(USING_LR1121)
#define RADIO_DIO9_PIN              26      //LR1121 DIO9  
#define RADIO_BUSY_PIN              32      //LR1121 BUSY  
#endif
#define SDCARD_MOSI                 15
#define SDCARD_MISO                 2
#define SDCARD_SCLK                 14
#define SDCARD_CS                   13
#define BOARD_LED                   25
#define LED_ON                      HIGH
#define LED_OFF                     LOW
#define BAT_ADC_PIN                 35
#define BAT_ADC_MULTIPLIER          (2.0f)
#define DISP_ST7735_160_80     0    // Enable TFT
#define DISP_SSD1306_128_64    1    // Enable SSD1306 OLED  
#define DISP_SH110X_128_64     1    // Disable SH110X OLED
#define BOARD_VARIANT_NAME          "T3 V3.0"
#define BUTTON_PIN                  0

//------- T3 S3 v1.2 SX1262/SX1276/SX1278/LR1121 -----------------------
// https://github.com/Xinyuan-LilyGO/LilyGo-LoRa-Series/blob/master/schematic/T3_S3_V1.2.pdf
#elif   defined(T3_S3_V1_2_SX1262)    ||   \
        defined(T3_S3_V1_2_SX1276)    ||   \
        defined(T3_S3_V1_2_SX1278)    ||   \
        defined(T3_S3_V1_2_LR1121)
#define HAS_RADIO
#define HAS_SDCARD
#define HAS_DISPLAY

#if   defined(T3_S3_V1_2_SX1262)
#define USING_SX1262
#elif defined(T3_S3_V1_2_SX1276)
#define USING_SX1276
#elif defined(T3_S3_V1_2_SX1278)
#define USING_SX1278
#elif defined(T3_S3_V1_2_LR1121)
#define USING_LR1121
#endif // T3_S3_V1_2_SX1262

#define I2C_SDA                     18
#define I2C_SCL                     17
#define OLED_RST                    UNUSED_PIN
#define RADIO_SCLK_PIN              5
#define RADIO_MISO_PIN              3
#define RADIO_MOSI_PIN              6
#define RADIO_CS_PIN                7
#define SDCARD_MOSI                 11
#define SDCARD_MISO                 2
#define SDCARD_SCLK                 14
#define SDCARD_CS                   13
#define SDCARD_DAT1                 4
#define SDCARD_DAT2                 12
#define BOARD_LED                   37
#define LED_ON                      HIGH
#define LED_OFF                     LOW
#define BUTTON_PIN                  0
#define BAT_ADC_PIN                 1
#define BAT_ADC_MULTIPLIER          (2.0f)
#define RADIO_RST_PIN               8
// SX1262
#if defined(USING_SX1262)
#define RADIO_DIO1_PIN              33
#define RADIO_BUSY_PIN              34
// SX1276/78
#elif defined(USING_SX1276) || defined(USING_SX1278)
#define RADIO_BUSY_PIN              33      //DIO1
#define RADIO_DIO0_PIN              9
#define RADIO_DIO1_PIN              33
#define RADIO_DIO2_PIN              34
#define RADIO_DIO3_PIN              21
#define RADIO_DIO4_PIN              10
#define RADIO_DIO5_PIN              36
// LR1121
#elif defined(USING_LR1121)
#define RADIO_DIO9_PIN              36      //LR1121 DIO9  = IO36
#define RADIO_BUSY_PIN              34      //LR1121 BUSY  = IO34
#endif

#define DISP_ST7735_160_80     0    // Enable TFT
#define DISP_SSD1306_128_64    1    // Enable SSD1306 OLED  
#define DISP_SH110X_128_64     1    // Disable SH110X OLED
#define BOARD_VARIANT_NAME          "T3 S3 V1.X"

//------- Lilygo Lora32 + SX1278 -----------------------
// https://github.com/LilyGO/TTGO-LORA32/blob/master/schematic1in6.pdf
#elif defined(LORA32_SX1278) 
#define HAS_RADIO
#define HAS_SDCARD
#define HAS_DISPLAY

#define USING_SX1278
#define GPS_RX_PIN                  34
#define GPS_TX_PIN                  12
#define GPS_BAUDRATE                9600  
#define BUTTON_PIN                  38
#define BUTTON_PIN_MASK             GPIO_SEL_38
#define I2C_SDA                     21
#define I2C_SCL                     22
#define OLED_RST                    UNUSED_PIN
#define RADIO_SCLK_PIN              5
#define RADIO_MISO_PIN              19
#define RADIO_MOSI_PIN              27
#define RADIO_CS_PIN                18
#define RADIO_DIO0_PIN              26
#define RADIO_RST_PIN               23
#define RADIO_DIO1_PIN              33
// SX1276/78
#define RADIO_DIO2_PIN              32
#define BAT_ADC_PIN                 35
#define BAT_ADC_MULTIPLIER          (2.0f)
#define BOARD_LED                   4
#define LED_ON                      LOW
#define LED_OFF                     HIGH
#define SDCARD_MOSI                 15
#define SDCARD_MISO                 2
#define SDCARD_SCLK                 14
#define SDCARD_CS                   13
#define SDCARD_DAT2                 12
#define SDCARD_DAT1                 4

#define DISP_ST7735_160_80     0    // Enable TFT
#define DISP_SSD1306_128_64    1    // Enable SSD1306 OLED  
#define DISP_SH110X_128_64     1    // Disable SH110X OLED                
#define BOARD_VARIANT_NAME          "Lora32"

//------- Heltec Tracker -----------------------
// https://resource.heltec.cn/download/Wireless_Tracker/Wireless_Tacker1.1/HTIT-Tracker_V0.5.pdf
#elif defined(HELTEC_TRACKER)
#define HAS_GPS
#define HAS_DISPLAY

#define USING_SX1262
#define GPS_RX_PIN                  33
#define GPS_TX_PIN                  34
#define GPS_PPS_PIN                 36
#define GPS_RST_PIN                 35
#define GPS_BAUDRATE                115200   
#define BUTTON_PIN                  0
#define BUTTON_PIN_MASK             GPIO_SEL_0
#define LCD_MOSI                    42
#define LCD_SCLK                    41
#define LCD_RS                      40
#define LCS_RES                     39
#define LCD_CS                      38
#define LCD_LED                     21
#define RADIO_SCLK_PIN              9
#define RADIO_MISO_PIN              11
#define RADIO_MOSI_PIN              10
#define RADIO_CS_PIN                8
#define RADIO_RST_PIN               12
#define RADIO_DIO0_PIN             (-1)
#define RADIO_DIO1_PIN              14
#define RADIO_BUSY_PIN              13
#define BAT_ADC_PIN                 1
#define BAT_ADC_EN_PIN              2
#define BAT_ADC_EN_LEVEL            HIGH
#define BAT_ADC_MULTIPLIER          (4.9f)
#define VEXT_EN_PIN                 3
#define VEXT_EN_LEVEL               HIGH
#define GPS_BAUDRATE                115200io
#define BOARD_LED                   18
#define LED_ON                      HIGH
#define LED_OFF                     LOW
#define HAS_DISPLAY
#define DISP_ST7735_160_80     1
#define DISP_SSD1306_128_64    0
#define DISP_SH110X_128_64     0

//------- Heltec Lora32 -----------------------
// https://resource.heltec.cn/download/WiFi_LoRa_32_V3/HTIT-WB32LA(F)_V3.1_Schematic_Diagram.pdf
#elif defined(HELTEC_LORA32)
#define USING_SX1262
#define BUTTON_PIN                  0
#define BUTTON_PIN_MASK             GPIO_SEL_0
#define I2C_SDA                     17
#define I2C_SCL                     18
#define OLED_RST                    21
#define RADIO_SCLK_PIN              9
#define RADIO_MISO_PIN              11
#define RADIO_MOSI_PIN              10
#define RADIO_CS_PIN                8
#define RADIO_RST_PIN               12
#define RADIO_DIO1_PIN              14
#define RADIO_BUSY_PIN              13
#define BAT_ADC_PIN                 1
#define BAT_ADC_EN_PIN              37
#define BAT_ADC_EN_LEVEL            LOW
#define BAT_ADC_MULTIPLIER          (4.9f)
#define VEXT_EN_PIN                 36
#define VEXT_EN_LEVEL               LOW
#define GPS_BAUDRATE                115200
#define BOARD_LED                   35
#define LED_ON                      HIGH
#define LED_OFF                     LOW
#define HAS_DISPLAY
#define DISP_ST7735_160_80     0
#define DISP_SSD1306_128_64    1  
#define DISP_SH110X_128_64     1

// No board selected - error
#else
#error "When using it for the first time, please define the board model in <platformio.ini> and select build configuration"
#endif


#ifndef OLED_WIRE_PORT
#define OLED_WIRE_PORT          Wire
#endif

#ifndef PMU_WIRE_PORT
#define PMU_WIRE_PORT           Wire
#endif

#ifndef DISPLAY_ADDR
#define DISPLAY_ADDR            0x3C
#endif

// *****************************************************************************
// DISPLAY TYPE DEFINITIONS
// Define which display types are supported on this board
// *****************************************************************************
// TFT Display - ST7735 160x80
#ifndef DISP_ST7735_160_80
#define DISP_ST7735_160_80     0
#endif

// OLED Displays - 128x64 (can be supported simultaneously)
#ifndef DISP_SSD1306_128_64
#define DISP_SSD1306_128_64    0
#endif

#ifndef DISP_SH110X_128_64
#define DISP_SH110X_128_64     0
#endif

// Helper macros to check if any display is enabled
#define HAS_TFT_DISPLAY         (DISP_ST7735_160_80)
#define HAS_OLED_DISPLAY        (DISP_SSD1306_128_64 || DISP_SH110X_128_64)
#define HAS_ANY_DISPLAY         (HAS_TFT_DISPLAY || HAS_OLED_DISPLAY)