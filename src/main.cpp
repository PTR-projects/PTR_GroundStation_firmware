#include "FS.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "string.h"
#include "Arduino.h"

#include "BOARD.h"
#include "LORA_typedefs.h"
#include "TeleMetry.h"
#include "lora.h"
#include "OLED.h"
#include "GNSS.h"
#include "sensors.h"
#include "SF_RSL.h"
#include "FileSys.h"
#include "preferences.h"
#include "SQL.h"
#include "PWR.h"
#include <SPI.h>
#include <Wire.h>

static void getChipInfo();
static void printWakeupReason();
static void BOARD_init();

static long RSL_previousMillis = 0;

const char *ssid_base = "PTR-GS";
//const char *password = "KPPTR";

AsyncWebServer server(80);

#ifdef VERSION_TAG
  const char *version = VERSION_TAG;
#else
  const char *version = "?";
#endif


void setup() {
  uint8_t mac[6];
  char ssid[12];

  BOARD_init(); Serial.println(F("BOARD init done!"));
  PWR_init(); Serial.println(F("PWR init done!"));

  if(FS_init()){
    Serial.println(F("FS init done!"));
  }

  preferences_init();

  if(OLED_init(preferences_get_OLEDdriver())){
    Serial.println(F("OLED init done!"));
    OLED_clear();
    OLED_drawString(0, 5, "OLED OK");
  }

  if(GNSS_init()){
    Serial.println(F("GNSS init done!"));
  }  

  if(LORA_init()){
    TM_changeID(preferences_get_id());
    LORA_changeFrequency(preferences_get_frequency()); 
    Serial.println(F("LORA init done!"));
    OLED_drawString(0, 21, "LORA OK");
  } else {
    OLED_drawString(0, 21, "LORA FAIL");
    while(1){ delay(100); }
  }

  //SQL init
  SQL_init();

  WiFi.macAddress(mac);
  snprintf(ssid, sizeof(ssid), "%s-%02X%02X", ssid_base, mac[4], mac[5]);

  Serial.printf("\n[*] WiFi MAC: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.printf("\n[*] WiFi SSID: %s\n", ssid);

  //Server stuff
  #if SQL_IMPLEMENTED
    WiFi.mode(WIFI_AP_STA);

    Serial.println("\n[*] Creating ESP32 AP");
    WiFi.softAP(ssid);
    OLED_drawString(0, 29, "WiFi AP created!");

    Serial.print("Connecting to Hotspot");
    WiFi.begin(sql_lte_ssid, sql_lte_pass);             // Connect to the network

    OLED_drawString(0, 37, "WiFi connecting...");
    Serial.println(" ...");

    uint8_t timeout = 255;
    while (timeout && (WiFi.status() != WL_CONNECTED))
    {
      delay(100);
      timeout--;
      Serial.print(".");
    }

    if(timeout){
      OLED_drawString(0, 45, "WiFi connected!");
      Serial.println();
      Serial.println("Connected!");
      Serial.print("IP address for WiFi: ");
      Serial.println(WiFi.localIP());
      Serial.print("IP address for AP: ");
      Serial.println(WiFi.softAPIP());
    }
    else {
      OLED_drawString(0, 45, "WiFi con. failed!");
      Serial.println();
      Serial.println("WiFi connection failed!");
    }

    Serial.println('\n');
    Serial.println("Connection established!");  
  #else 
    WiFi.softAP(ssid);
    OLED_drawString(0, 29, "WiFi AP created!");
  #endif
  

  server.serveStatic("/list_trackers.html", SPIFFS, "/list_trackers.html");
  server.serveStatic("/styles.css", SPIFFS, "/styles.css");
  server.serveStatic("/list_trackers.js", SPIFFS, "/list_trackers.js");
  server.serveStatic("/download", SPIFFS, "/log.csv");
  server.serveStatic("/map_trackers.html", SPIFFS, "/map_trackers.html");
  server.serveStatic("/map_trackers.js", SPIFFS, "/map_trackers.js");


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html", false);
  });

  server.on("/tracker_list_api", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", TM_getJSON());
  });
 
  server.on("/parse", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/log.csv", "text/plain", false);
  });

  server.on("/help", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "KPPTR telemetry receiver:");
  });

  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request){
    deleteFile(SPIFFS, "/log.csv");
    request->send(200, "text/plain", "Succesfully deleted file!");
  });

  server.on("/setFreq", HTTP_POST, [](AsyncWebServerRequest *request){
    String temp;
    int freq;

    temp = request->getParam("freq", true)->value();
    freq = temp.toInt();
    Serial.printf("Received method: %s \n", temp);
    if(freq >= 430000 && freq <= 440000){
      if(LORA_changeFrequency(freq)){
        request->send(200, "text/plain", "Succesfully changed frequency to " + (String)(freq) );
      }
      else{
        request->send(200, "text/plain", "Failed to change frequency!");
      }
    }
    else{
      request->send(200, "text/plain", "Frequency not in range!");
    }
  });

  server.on("/setID", HTTP_POST, [](AsyncWebServerRequest *request){
    String temp;
    int id;
    temp = request->getParam("code", true)->value();
    id = temp.toInt();
    Serial.printf("Received method: %s \n", temp);
    TM_changeID(id);
    request->send(200, "text/plain", "Succesfully changed ID to " + (String)(id) );
  });

  server.on("/setOLED", HTTP_POST, [](AsyncWebServerRequest *request){
    String temp;

    if(request->getParam("driver", true)->value() != NULL){
      temp = request->getParam("driver", true)->value();
      if(temp == NULL){
        request->send(200, "text/plain", "Args Error");
        return;
      }

      String driver = temp;
      Serial.printf("Received method: %s \n", driver);
      OLED_changeDriver(driver);
      request->send(200, "text/plain", "Succesfully changed OLED driver to " + driver);
    }
  });

  server.begin();
  OLED_drawString(0, 53,"FW: " + (String)version);
  delay(2000);

  OLED_clear();
  OLED_drawSplash();
  delay(2000);  

  OLED_clear();
  OLED_drawLargeString(0, 15, "WiFi AP:");
  OLED_drawLargeString(0, 34, ssid);
  delay(5000);

  LORA_startRX();
}

void loop() {
  LORA_RXhandler();
  LORA_PacketCounter();
  GNSS_srv();
  OLED_refresh();
  PWR_loop();
}



static void getChipInfo(){
  struct {
      String          chipModel;
      float           psramSize;
      uint8_t         chipModelRev;
      uint8_t         chipFreq;
      uint8_t         flashSize;
      uint8_t         flashSpeed;
  } devInfo;

#if defined(ARDUINO_ARCH_ESP32)

    Serial.println("-----------------------------------");

    printWakeupReason();

#if defined(CONFIG_IDF_TARGET_ESP32)  ||  defined(CONFIG_IDF_TARGET_ESP32S3)

    if (psramFound()) {
        uint32_t psram = ESP.getPsramSize();
        devInfo.psramSize = psram / 1024.0 / 1024.0;
        Serial.printf("PSRAM is enable! PSRAM: %.2fMB\n", devInfo.psramSize);
    } else {
        Serial.println("PSRAM is disable!");
        devInfo.psramSize = 0;
    }

#endif

    Serial.print("Flash:");
    devInfo.flashSize       = ESP.getFlashChipSize() / 1024.0 / 1024.0;
    devInfo.flashSpeed      = ESP.getFlashChipSpeed() / 1000 / 1000;
    devInfo.chipModel       = ESP.getChipModel();
    devInfo.chipModelRev    = ESP.getChipRevision();
    devInfo.chipFreq        = ESP.getCpuFreqMHz();

    Serial.print(devInfo.flashSize);
    Serial.println(" MB");
    Serial.print("Flash speed:");
    Serial.print(devInfo.flashSpeed);
    Serial.println(" M");
    Serial.print("Model:");

    Serial.println(devInfo.chipModel);
    Serial.print("Chip Revision:");
    Serial.println(devInfo.chipModelRev);
    Serial.print("Freq:");
    Serial.print(devInfo.chipFreq);
    Serial.println(" MHZ");
    Serial.print("SDK Ver:");
    Serial.println(ESP.getSdkVersion());
    Serial.print("DATE:");
    Serial.println(__DATE__);
    Serial.print("TIME:");
    Serial.println(__TIME__);

    Serial.print("EFUSE MAC: ");
    Serial.print( ESP.getEfuseMac(), HEX);
    Serial.println();

    Serial.println("-----------------------------------");

#endif
}

static void printWakeupReason(){
#ifdef ESP32
    Serial.print("Reset reason:");
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_UNDEFINED:
        Serial.println(" In case of deep sleep, reset was not caused by exit from deep sleep");
        break;
    case ESP_SLEEP_WAKEUP_ALL :
        break;
    case ESP_SLEEP_WAKEUP_EXT0 :
        Serial.println("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1 :
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        break;
    case ESP_SLEEP_WAKEUP_TIMER :
        Serial.println("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD :
        Serial.println("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP :
        Serial.println("Wakeup caused by ULP program");
        break;
    default :
        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
        break;
    }
#endif
}

static void BOARD_init(){
    Serial.begin(115200); //UART
    //Serial.begin();     //USB

    // while (!Serial);

    Serial.println("Board Setup start!");

    getChipInfo();

    SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);

#ifdef I2C_SDA
    Wire.begin(I2C_SDA, I2C_SCL);
#endif

#ifdef HAS_GPS
    Serial1.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
#endif // HAS_GPS


#ifdef BOARD_LED
    /*
    * T-Beam LED defaults to low level as turn on,
    * so it needs to be forced to pull up
    * * * * */
#if LED_ON == LOW
#if defined(ARDUINO_ARCH_ESP32)
    gpio_hold_dis((gpio_num_t)BOARD_LED);
#endif //ARDUINO_ARCH_ESP32
#endif

    pinMode(BOARD_LED, OUTPUT);
    digitalWrite(BOARD_LED, LED_ON);
#endif

#ifdef GPS_EN_PIN
    pinMode(GPS_EN_PIN, OUTPUT);
    digitalWrite(GPS_EN_PIN, HIGH);
#endif

#ifdef GPS_RST_PIN
    pinMode(GPS_RST_PIN, OUTPUT);
    digitalWrite(GPS_RST_PIN, HIGH);
#endif


#ifdef RADIO_LDO_EN
    pinMode(RADIO_LDO_EN, OUTPUT);
    digitalWrite(RADIO_LDO_EN, HIGH);
#endif


// #ifdef HAS_GPS
// #ifdef T_BEAM_S3_BPF
//     find_gps = beginGPS();
// #endif
// #endif
}