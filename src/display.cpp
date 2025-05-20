#include "Arduino.h"
#include <SPI.h>
#include "BOARD.h"
#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7735.h>  // Hardware-specific library for ST7735
#include <Adafruit_SSD1306.h> // Hardware-specific library for SSD1306
#include <Adafruit_SH110X.h>  // Hardware-specific library for SH110X
#include "display.h"

static display_type_t display_type = HELTEC_WIRELESS_TRACKER_TFT;
static long Display_previousMillis = 0;
static long Display_interval = 100;
static Adafruit_SSD1306 * display;

static SPIClass spiDisp;

static void Display_display();

struct {
    void * init;
    void * clear;
    void * display;
    void * draw_pixel;
} display_adapter;

void Display_init(String model){
    Serial.print("Display init start... ");

#ifdef LCD_CS
    if(model == "ST7735"){
        display_type = HELTEC_WIRELESS_TRACKER_TFT;
        pinMode(LCD_LED, OUTPUT);
        pinMode(VEXT_EN_PIN, OUTPUT);   // przenieść gdzie indziej np. PWR
        digitalWrite(VEXT_EN_PIN, HIGH);
        digitalWrite(LCD_LED, HIGH);
        delay(10);

        //spiDisp = SPIClass(HSPI);
        //spiDisp.begin(LCD_SCLK, 4, LCD_MOSI);  // MISO = -1 but must be something so it is 4
        //display = new Adafruit_ST7735(&spiDisp, LCD_CS, LCD_RS, LCS_RES);   //LCD_CS, LCD_RS, LCD_MOSI, LCD_SCLK, LCS_RES);
        display = new Adafruit_ST7735(LCD_CS, LCD_RS, LCD_MOSI, LCD_SCLK, LCS_RES);   //LCD_CS, LCD_RS, LCD_MOSI, LCD_SCLK, LCS_RES);
        display->initR(INITR_MINI160x80_PLUGIN);
        display->setRotation(1);
        Display_clear();
        Serial.println("Display init done!");
        return;
    }
#endif

    
#ifdef OLED_RST
    if(model == "SSD1306"){
        display_type = TTGO_OLED_SSD1306;
        display = new Adafruit_SSD1306(128, 64, &Wire, OLED_RST);
        display->begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDR);
        delay(100);
        Display_clear();
        Serial.println("Display init done!");
        return;
    }
    // if(model == "SH110X"){
    //     display_type = TTGO_OLED_SH110X;
    //     display = new new Adafruit_SH110X(OLED_RST);
    //     Serial.println("Display init done!");
    //     return;
    // }
#endif

    Serial.println("Display init failed!!!");
}

void Display_clear(){
    if(display_type == HELTEC_WIRELESS_TRACKER_TFT){
        display->fillScreen(ST77XX_BLACK);
        return;
    }

    if(display_type == TTGO_OLED_SSD1306){
        display->clearDisplay();
        return;
    }

    if(display_type == TTGO_OLED_SH110X){
        display->clearDisplay();
        return;
    }
}
// Kolor jest zapisywany inaczej dla OLED i dla LCD !!! Zrobić to na ładnie
void Display_drawString(uint16_t x, uint16_t y, const String &text){
    if(display_type == HELTEC_WIRELESS_TRACKER_TFT){
        display->setCursor(x, y);
        display->setTextColor(ST77XX_WHITE);
        display->setTextSize(1);
        display->setTextWrap(false);
        display->print(text);
        return;
    }
    
    if(display_type == TTGO_OLED_SSD1306){
        display->setCursor(x, y);
        display->setTextSize(1);
        display->setTextWrap(false);
        display->setTextColor(SSD1306_WHITE);
        display->print(text);
        Serial.println("Display write ok");
        display->display();             //wywalić później
        Serial.println("Display refresh ok");
        return;
    }

    if(display_type == TTGO_OLED_SH110X){
        return;
    }

    Serial.println("Display write failed!");
}

void Display_drawLargeString(uint16_t x, uint16_t y, const String &text){
    if(display_type == HELTEC_WIRELESS_TRACKER_TFT){
        display->setCursor(x, y);
        display->setTextColor(ST77XX_WHITE);
        display->setTextSize(2);
        display->setTextWrap(false);
        display->print(text);
        return;
    }

    if(display_type == TTGO_OLED_SSD1306){
        display->setCursor(x, y);
        display->setTextSize(2);
        display->setTextWrap(false);
        display->setTextColor(SSD1306_WHITE);
        display->print(text);
        Serial.println("Display write ok");
        display->display();             //wywalić później
        Serial.println("Display refresh ok");
        return;
    }

    if(display_type == TTGO_OLED_SH110X){
        return;
    }

    Serial.println("Display write failed!");
}

void Display_drawSplash(){

}



void Display_refresh(){
//#if defined(HAS_DISPLAY)
    unsigned long Display_currentMillis = millis();
    if((Display_currentMillis - Display_previousMillis) > Display_interval) {
        Display_previousMillis = Display_currentMillis;

        //Display_clear();
        //OLED_drawRocketLaunch();
        //Display_display();
    }
//#endif
}

//---------------- Private ------------------
static void Display_display(){
    if(display_type == TTGO_OLED_SSD1306){
        display->display();
        return;
    }

    if(display_type == TTGO_OLED_SH110X){
        display->display();
        return;
    }
}