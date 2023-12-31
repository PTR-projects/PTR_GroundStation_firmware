#include "LORA_typedefs.h"
#include "TeleMetry.h"
#include "lora.h"
#include "OLED.h"
#include "GNSS.h"
#include "sensors.h"
#include "SF_RSL.h"
#include "FS.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "FileSys.h"
#include "mavlink_driver.h"

//#include "OLEDDisplayUi.h"

// 0x34  - AXP192
// 0x3C  - OLED	
// 0x53  - ADXL345
// 0x68  - MPU
// 0x76  - BMP280

static long RSL_previousMillis = 0;

const char *ssid = "TTGO";
//const char *password = "KPPTR";

AsyncWebServer server(80);



void setup() {
  //while (!Serial) delay(10); 
  Serial.begin(115200);
  Serial.println(F("App start!"));
  FS_init();    Serial.println(F("FS init done!"));
  OLED_init();  Serial.println(F("OLED init done!"));
  GNSS_init();  Serial.println(F("GNSS init done!"));
  LORA_init();  Serial.println(F("LORA init done!"));
  //Sensors_init();  Serial.println(F("Sensors init done!"));
  
 

  

  //Server stuff

  WiFi.softAP(ssid);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html", false);
  });
  
  server.serveStatic("/download", SPIFFS, "/log.csv");
 
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
  
  MAVLink_init();


  LORA_startRX();
}

void loop() {
  //LORA RX handler
  LORA_RXhandler();
  LORA_PacketCounter();

  //GNSS service
  GNSS_srv();

  //Sensors service
  //Sensors_task();

  //OLED handler
  OLED_refresh(); 

  MAVLink_srv();

  
}










/*
            // packet was successfully received
            Serial.println();
            //Serial.println(F("[SX1276] Received packet!"));

            Serial.print(F("[SX1276] Data length:\t"));
            Serial.println(radio.getPacketLength());

            // print data of the packet
            Serial.print(F("[SX1276] Data:\t\t"));
            for(int i=0; i<radio.getPacketLength(); i++){
              if(*(lora_bin_packet_d.data+i) < 0x10)
                Serial.print("0");
              Serial.print(*(lora_bin_packet_d.data+i), HEX);
              Serial.print(" ");
            }
            Serial.println();
            Serial.print(F("[SX1276] Packet ID:\t"));
            Serial.print("0x");
            Serial.println(lora_binmin_packet_d.packet_id, HEX);

            Serial.print(F("[SX1276] ID:\t\t"));
            Serial.print("0x");
            Serial.println(lora_binmin_packet_d.id, HEX);

            Serial.print(F("[SX1276] Vbat:\t\t"));
            Serial.print(lora_binmin_packet_d.vbat_10/10.0f);
            Serial.println(" V");

            Serial.print(F("[SX1276] Altitude:\t"));
            Serial.println(lora_binmin_packet_d.altitude);

            Serial.print(F("[SX1276] Latitude:\t"));
            Serial.println(lora_binmin_packet_d.lat);

            Serial.print(F("[SX1276] Longitude:\t"));
            Serial.println(lora_binmin_packet_d.lon);

            // print RSSI (Received Signal Strength Indicator)
            Serial.print(F("[SX1276] RSSI:\t\t"));
            Serial.print(radio.getRSSI());
            Serial.println(F(" dBm"));

            // print SNR (Signal-to-Noise Ratio)
            Serial.print(F("[SX1276] SNR:\t\t"));
            Serial.print(radio.getSNR());
            Serial.println(F(" dB"));

            // print frequency error
            Serial.print(F("[SX1276] Freq. error:\t"));
            Serial.print(radio.getFrequencyError());
            Serial.println(F(" Hz"));
            // if (u8g2) {
            //     u8g2->setFont(u8g2_font_7x14_tr);
            //     u8g2->clearBuffer();
            //     char buf[256];
            //     snprintf(buf, sizeof(buf), "Alti: %+i m", lora_binmin_packet_d.altitude);
            //     u8g2->drawStr(0, 12, buf);
            //     snprintf(buf, sizeof(buf), "Lat:  %+2.5f", lora_binmin_packet_d.lat/10000000.0);
            //     u8g2->drawStr(0, 25, buf);
            //     snprintf(buf, sizeof(buf), "Lon:  %+2.5f", lora_binmin_packet_d.lon/10000000.0);
            //     u8g2->drawStr(0, 38, buf);
            //     snprintf(buf, sizeof(buf), "State %2i Bat %2.1fV", 4, lora_binmin_packet_d.vbat_10/10.0);
            //     u8g2->drawStr(0, 51, buf);
            //     snprintf(buf, sizeof(buf), "RSSI: %+.2f dbm", radio.getRSSI());
            //     u8g2->drawStr(0, 64, buf);
            //     u8g2->sendBuffer();
            // }
            
           */