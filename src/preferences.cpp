
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
        config_data_d.oled_driver = SH1106;

        config["configuration"]["frequency"] = 434250;
        config["configuration"]["id"] = 0;
        config["configuration"]["oled_driver"] = SSD1306;

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
    config_data_d.oled_driver = config["configuration"]["oled_driver"];

    return 0;
}

int preferences_get_frequency(){  
    return config_data_d.frequency;
}

int preferences_get_id(){
    return config_data_d.id;
}

String preferences_get_OLEDdriver(){
    OLED_driver_e tmp = config_data_d.oled_driver;
    if(tmp == SSD1306)
        return "SSD1306";
    
    if(tmp == SH1106)
        return "SH1106";

    return "SH1106";
}

void preferences_update_frequency(int frequency){
    config_data_d.frequency = frequency;

    preferences_update();
}

void preferences_update_id(int id){
    config_data_d.id = id;
    preferences_update();
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

void preferences_update(){
    File file;
    file = SPIFFS.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }

    config["configuration"]["frequency"] = config_data_d.frequency;
    config["configuration"]["id"] = config_data_d.id;
    config["configuration"]["oled_driver"] = config_data_d.oled_driver;

    serializeJson(config, file);
    file.close(); 

}