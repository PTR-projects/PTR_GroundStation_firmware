
#include "FS.h"
#include "SPIFFS.h"
#include <string>
#include <ArduinoJson.h>
#include "preferences.h"

using namespace std;

const char *path = "/config.json";

config_data_t config_data_d;

JsonDocument config;

void preferences_update();

int preferences_init(){
    File file;

    if(!SPIFFS.exists(path)){
        file = SPIFFS.open(path, FILE_WRITE);
        if(!file){
            Serial.println("- failed to open file for writing");
            return -1;
        }

        config_data_d.frequency = 433250;
        config_data_d.id = 0;
        config_data_d.id_filter = false;
#if HAS_OLED_DISPLAY
        config_data_d.oled_driver = SH1106;
#endif

        config["configuration"]["frequency"] = 434250;
        config["configuration"]["id"] = 0;
        config["configuration"]["id_filter"] = false;
#if HAS_OLED_DISPLAY
        config["configuration"]["oled_driver"] = SSD1306;
#endif

        serializeJson(config, file);
        file.close();
    }
    
    file = SPIFFS.open(path, FILE_READ);
    if(!file){
        Serial.println("- failed to open file for reading");
        return -1;
    }

    deserializeJson(config, file);

    config_data_d.frequency = config["configuration"]["frequency"];
    config_data_d.id = config["configuration"]["id"];
    if(config["configuration"]["id_filter"].is<bool>()){
        config_data_d.id_filter = config["configuration"]["id_filter"];
    } else {
        config_data_d.id_filter = (config_data_d.id != 0);
    }
#if HAS_OLED_DISPLAY
    config_data_d.oled_driver = config["configuration"]["oled_driver"];
#endif

    return 0;
}

int preferences_get_frequency(){  
    return config_data_d.frequency;
}

int preferences_get_id(){
    return config_data_d.id;
}

bool preferences_get_id_filter(){
    return config_data_d.id_filter;
}

#if HAS_OLED_DISPLAY
String preferences_get_OLEDdriver(){
    OLED_driver_e tmp = config_data_d.oled_driver;
    if(tmp == SSD1306)
        return "SSD1306";
    
    if(tmp == SH1106)
        return "SH1106";

    return "SH1106";
}

void preferences_update_OLEDdriver(String driver){
    OLED_driver_e tmp = SH1106;

    if(driver == "SSD1306")
        tmp = SSD1306;
    
    if(driver == "SH1106")
        tmp = SH1106;

    config_data_d.oled_driver = tmp;
    preferences_update();
}
#endif

void preferences_update_frequency(int frequency){
    config_data_d.frequency = frequency;

    preferences_update();
}

void preferences_update_id(int id){
    config_data_d.id = id;
    preferences_update();
}

void preferences_update_id_filter(bool enabled){
    config_data_d.id_filter = enabled;
    preferences_update();
}

void preferences_update(){
    File file;
    file = SPIFFS.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }

    config["configuration"]["frequency"] = config_data_d.frequency;
    config["configuration"]["id"] = config_data_d.id;
    config["configuration"]["id_filter"] = config_data_d.id_filter;
#if HAS_OLED_DISPLAY
    config["configuration"]["oled_driver"] = config_data_d.oled_driver;
#endif

    serializeJson(config, file);
    file.close(); 

}
