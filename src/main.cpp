#include "FS.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "string.h"
#include "Arduino.h"

#include "BOARD.h"
#include "LORA_typedefs.h"
#include "TeleMetry.h"
#include "lora.h"
#include "GNSS.h"
#include "sensors.h"
#include "SF_RSL.h"
#include "FileSys.h"
#include "preferences.h"
#include "SQL.h"
#include "PWR.h"
#include <SPI.h>
#include <Wire.h>
#include "display.h"
#include <ArduinoJson.h>

static void getChipInfo();
static void printWakeupReason();
static void BOARD_init();
static String settings_getJSON();
static bool settings_apply(const String& key, const JsonVariant& value, String& error);
static bool settings_apply_all(JsonDocument& doc, String& error);

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

  // if(OLED_init(preferences_get_OLEDdriver())){
  //   Serial.println(F("OLED init done!"));
  // }

#if DISP_ST7735_160_80
  Display_init("ST7735");
#elif HAS_OLED_DISPLAY
  {
    String driver = preferences_get_OLEDdriver();
    if (driver == "SH1106") {
      driver = "SH110X";
    }
    Display_init(driver);
  }
#endif

  if(GNSS_init()){
    Serial.println(F("GNSS init done!"));
  }  

  if(LORA_init()){
    TM_changeID(preferences_get_id());
    TM_setFilterEnabled(preferences_get_id_filter());
    LORA_changeFrequency(preferences_get_frequency()); 
    Serial.println(F("LORA init done!"));
    Display_drawString(0, 21, "LORA OK");
  } else {
    Display_drawString(0, 21, "LORA FAIL");
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
    Display_drawString(0, 29, "WiFi AP created!");

    Serial.print("Connecting to Hotspot");
    WiFi.begin(sql_lte_ssid, sql_lte_pass);             // Connect to the network

    Display_drawString(0, 37, "WiFi connecting...");
    Serial.println(" ...");

    uint8_t timeout = 255;
    while (timeout && (WiFi.status() != WL_CONNECTED))
    {
      delay(100);
      timeout--;
      Serial.print(".");
    }

    if(timeout){
      Display_drawString(0, 45, "WiFi connected!");
      Serial.println();
      Serial.println("Connected!");
      Serial.print("IP address for WiFi: ");
      Serial.println(WiFi.localIP());
      Serial.print("IP address for AP: ");
      Serial.println(WiFi.softAPIP());
    }
    else {
      Display_drawString(0, 45, "WiFi con. failed!");
      Serial.println();
      Serial.println("WiFi connection failed!");
    }

    Serial.println('\n');
    Serial.println("Connection established!");  
  #else 
    WiFi.softAP(ssid);
    Display_drawString(0, 29, "WiFi AP created!");
  #endif
  

  server.serveStatic("/tracker_list.html", SPIFFS, "/tracker_list.html");
  server.serveStatic("/styles.css", SPIFFS, "/styles.css");
  server.serveStatic("/tracker_list_script.js", SPIFFS, "/tracker_list_script.js");
  server.serveStatic("/download", SPIFFS, "/log.csv");


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html", false);
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", settings_getJSON());
  });

  server.on("/settings", HTTP_POST,
    [](AsyncWebServerRequest *request){
      String *body = static_cast<String*>(request->_tempObject);
      request->_tempObject = nullptr;
      if(body == NULL){
        request->send(400, "application/json", "{\"ok\":false,\"error\":\"Empty body\"}");
        return;
      }

      JsonDocument doc;
      DeserializationError parse_error = deserializeJson(doc, *body);
      delete body;
      if(parse_error){
        request->send(400, "application/json", "{\"ok\":false,\"error\":\"Invalid JSON\"}");
        return;
      }

      String error;
      JsonDocument response;
      if(settings_apply_all(doc, error)){
        response["ok"] = true;
      } else {
        response["ok"] = false;
        response["error"] = error;
      }
      String json;
      serializeJson(response, json);
      request->send(response["ok"] ? 200 : 400, "application/json", json);
    },
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
      if(index == 0){
        request->_tempObject = new String();
        static_cast<String*>(request->_tempObject)->reserve(total);
      }
      if(request->_tempObject != NULL){
        static_cast<String*>(request->_tempObject)->concat(reinterpret_cast<const char*>(data), len);
      }
    }
  );

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

  server.begin();

  // Draw splash screen
  Display_drawSplash();
  delay(2000);  

  // Display firmware version
  Display_clear();
  Display_drawLargeString(0, 34,"FW: " + (String)version);
  Display_flush();
  delay(2000);

  // Display WiFi info
  Display_clear();
  Display_drawLargeString(0, 15, "WiFi AP:");
  Display_drawLargeString(0, 34, ssid);
  Display_flush();
  delay(5000);

  LORA_startRX();
}

void loop() {
  LORA_RXhandler();
  LORA_PacketCounter();
  GNSS_srv();
  Display_refresh();
  PWR_loop();
}

static void settings_add_option(JsonArray options, const char* value, const char* label){
  JsonObject option = options.add<JsonObject>();
  option["value"] = value;
  option["label"] = label;
}

static bool settings_has_option(JsonArray options, const String& value){
  for(JsonObject option : options){
    if(value == option["value"].as<const char*>()){
      return true;
    }
  }
  return false;
}

static String settings_current_display(){
#if DISP_ST7735_160_80
  return String("ST7735");
#elif HAS_OLED_DISPLAY
  String driver = preferences_get_OLEDdriver();
  if(driver == "SH1106"){
    return String("SH110X");
  }
  return driver;
#else
  return String();
#endif
}

static String settings_getJSON(){
  JsonDocument doc;
  JsonArray settings = doc["settings"].to<JsonArray>();

  JsonObject frequency = settings.add<JsonObject>();
  frequency["key"] = "frequency";
  frequency["label"] = "Frequency";
  frequency["type"] = "select";
  String frequency_value = String(preferences_get_frequency());
  frequency["value"] = frequency_value;
  JsonArray frequency_options = frequency["options"].to<JsonArray>();
  settings_add_option(frequency_options, "434250", "CH0 434.250 MHz");
  settings_add_option(frequency_options, "434375", "CH1 434.375 MHz");
  settings_add_option(frequency_options, "434500", "CH2 434.500 MHz");
  settings_add_option(frequency_options, "434625", "CH3 434.625 MHz");
  if(!settings_has_option(frequency_options, frequency_value)){
    settings_add_option(frequency_options, frequency_value.c_str(), frequency_value.c_str());
  }

  JsonObject id_filter = settings.add<JsonObject>();
  id_filter["key"] = "id_filter";
  id_filter["label"] = "Filter ID";
  id_filter["type"] = "bool";
  id_filter["value"] = TM_getFilterEnabled();
  JsonArray id_filter_options = id_filter["options"].to<JsonArray>();
  settings_add_option(id_filter_options, "false", "Off");
  settings_add_option(id_filter_options, "true", "On");
  JsonObject id_input = id_filter["input"].to<JsonObject>();
  id_input["key"] = "id";
  id_input["type"] = "number";
  id_input["min"] = 0;
  id_input["max"] = 65536;
  id_input["value"] = TM_getID();

#if HAS_ANY_DISPLAY
  JsonObject display = settings.add<JsonObject>();
  display["key"] = "display";
  display["label"] = "Display";
  display["type"] = "select";
  display["value"] = settings_current_display();
  JsonArray display_options = display["options"].to<JsonArray>();
#if DISP_ST7735_160_80
  settings_add_option(display_options, "ST7735", "ST7735");
#endif
#if DISP_SSD1306_128_64
  settings_add_option(display_options, "SSD1306", "SSD1306");
#endif
#if DISP_SH110X_128_64
  settings_add_option(display_options, "SH110X", "SH110X");
#endif
#endif

  String json;
  serializeJson(doc, json);
  return json;
}

static bool settings_value_as_bool(const JsonVariant& value){
  if(value.is<bool>()){
    return value.as<bool>();
  }
  String text = value.as<String>();
  text.toLowerCase();
  return (text == "true" || text == "1" || text == "on");
}

static int settings_value_as_int(const JsonVariant& value){
  if(value.is<const char*>() || value.is<String>()){
    return value.as<String>().toInt();
  }
  return value.as<int>();
}

static bool settings_apply(const String& key, const JsonVariant& value, String& error){
  if(key == "frequency"){
    int freq = settings_value_as_int(value);
    if(freq < 430000 || freq > 440000){
      error = "Frequency not in range";
      return false;
    }
    if(!LORA_changeFrequency(freq)){
      error = "Failed to change frequency";
      return false;
    }
    return true;
  }

  if(key == "id"){
    int id = settings_value_as_int(value);
    if(id < 0 || id > 65536){
      error = "ID invalid";
      return false;
    }
    TM_changeID(id);
    return true;
  }

  if(key == "id_filter"){
    TM_setFilterEnabled(settings_value_as_bool(value));
    return true;
  }

  if(key == "display"){
    String display = value.as<String>();
    bool allowed = false;
#if DISP_ST7735_160_80
    if(display == "ST7735") allowed = true;
#endif
#if DISP_SSD1306_128_64
    if(display == "SSD1306") allowed = true;
#endif
#if DISP_SH110X_128_64
    if(display == "SH110X") allowed = true;
#endif
    if(!allowed){
      error = "Display type not available";
      return false;
    }
    String current = settings_current_display();
#if HAS_OLED_DISPLAY
    if(display == "SSD1306"){
      preferences_update_OLEDdriver("SSD1306");
    } else if(display == "SH110X"){
      preferences_update_OLEDdriver("SH1106");
    }
#endif
    if(display != current){
      Display_init(display);
    }
    return true;
  }

  error = "Unknown setting";
  return false;
}

static bool settings_apply_all(JsonDocument& doc, String& error){
  JsonArray items = doc["settings"].as<JsonArray>();
  if(items.isNull()){
    String key = doc["key"] | "";
    if(key.length() == 0){
      error = "Missing settings";
      return false;
    }
    return settings_apply(key, doc["value"], error);
  }

  JsonVariant filter_value;
  bool has_filter = false;
  for(JsonObject item : items){
    String key = item["key"] | "";
    if(key == "id_filter"){
      filter_value = item["value"];
      has_filter = true;
      continue;
    }
    if(!settings_apply(key, item["value"], error)){
      return false;
    }
  }
  if(has_filter && !settings_apply("id_filter", filter_value, error)){
    return false;
  }
  return true;
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
    digitalWrite(BOARD_LED, LED_OFF);
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
}