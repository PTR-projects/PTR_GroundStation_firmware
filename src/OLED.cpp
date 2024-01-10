#include "LORA_typedefs.h"
#include "TeleMetry.h"
#include "GNSS.h"
#include "sensors.h"
#include "Accessories.h"
#include "lora.h"
#include <Wire.h>
#include "SH1106Wire.h"
#include "SSD1306Wire.h"
#include "OLED.h"

//---------------- OLED declarations -------------------------
//static SSD1306Wire display(0x3c, SDA, SCL);
static SH1106Wire display(0x3c, SDA, SCL);
static char buffer[128];

static long OLED_previousMillis = 0;
static long OLED_interval = 100;

static long OLED_newPacketCounter = 0;

void OLED_init(){
    //pinMode(16,OUTPUT);
    //digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
    //delay(50); 
    //digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high、
    display.init();
    display.flipScreenVertically();  
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.clear();
    display.drawXbm(0, 0, 128, 64, splash);
    display.display();
    Serial.println(F("SSD1306 ready!"));
}

void OLED_refresh(){
    unsigned long OLED_currentMillis = millis();
    if((OLED_currentMillis - OLED_previousMillis) > OLED_interval) {
        OLED_previousMillis = OLED_currentMillis;

        display.clear();
        OLED_drawRocketLaunch();
        //OLED_drawFinder();
        display.display();
    }
}

void OLED_drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress){
  uint16_t radius = height / 2;
  uint16_t xRadius = x + radius;
  uint16_t yRadius = y + radius;
  uint16_t doubleRadius = 2 * radius;
  uint16_t innerRadius = radius - 2;

  display.setColor(WHITE);
  uint16_t maxProgressWidth = (width - doubleRadius + 1) * progress / 100;
  display.fillRect(x, y, maxProgressWidth, height);
  display.drawHorizontalLine(x, y + radius, width);

  display.setColor(BLACK);
  for(uint8_t i=0; (i*8) < width; i++){
    //drawVerticalLine(x + 8*i, y, height);
    display.drawLine(x + 8*i, y,            x + 8*i + 4,     y + radius);
    display.drawLine(x + 8*i, y+height-1,   x + 8*i + 4,     y + radius);
    display.drawLine(x + 8*i+1, y,          x + 8*i + 4 + 1, y + radius);
    display.drawLine(x + 8*i+1, y+height-1, x + 8*i + 4 + 1, y + radius);
  }

  display.setColor(WHITE);
}

void OLED_drawRocketLaunch(){
  
  // RSSI bar
  OLED_drawProgressBar(0,5, 72, 9, LORA_checkTimeout()?TM_getRSSIPercentage():0);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(ArialMT_Plain_10); 
<<<<<<< Updated upstream
  display.drawStringf(127, 5, buffer, "%3.0f Ppm", LORA_getPacketRate());
=======
>>>>>>> Stashed changes


  unsigned long currentMillis = millis();
  unsigned long dT = (currentMillis - LORA_getPacketHealth()) / 1000;

  //Serial.printf("Last packet: %d s ago \n", dT);
  if(dT > 5){
    display.drawStringf(127, 5, buffer, "%ds ", dT);
  }
  else{
    display.drawStringf(127, 5, buffer, "%3.0f Ppm", LORA_getPacketRate());
  }
 

  // // Altitude & Velocity
  // display.setTextAlignment(TEXT_ALIGN_LEFT);
  // display.setFont(ArialMT_Plain_16); 
  // display.drawStringf(0,  13, buffer,"%.1f km",   TM_getAltitudeKM());
  // display.drawStringf(63, 13, buffer,"Mach %.1f", TM_getMach());

  // GEO target
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10); 
  if(TM_getGeoAltitude() < 1000.0f){
    display.drawStringf(0, 13, buffer,"%.2f m", TM_getGeoAltitude());
  }
  else{
     display.drawStringf(0, 13, buffer,"%.2f km", TM_getGeoAltitude() / 1000.0f);
  }
  
  display.drawStringf(0, 23, buffer, "%c%.7f", TM_getGeoLatitude().sign,  fabs(TM_getGeoLatitude().cord));
  display.drawStringf(0, 33, buffer, "%c%.7f", TM_getGeoLongitude().sign, fabs(TM_getGeoLongitude().cord));

  //RF freq and ID
  display.drawStringf(0, 53, buffer, "%.3fMHz", LORA_getCurrentFrequency());
  
  // GEO own
  /*
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawStringf(0, 23, buffer, "%c%.7f", ' ', fabs(GNSS_getOwnLat()));
  display.drawStringf(0, 34, buffer, "%c%.7f", ' ', fabs(GNSS_getOwnLon()));
  
  */
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(ArialMT_Plain_10); 

  //voltages
  display.drawStringf(127, 23, buffer, "vAvi: %.2f V", TM_getVbat());
  display.drawStringf(127, 33, buffer, "ID: %d", TM_getID());

  if( isnan(TM_getVertVel())){
    display.drawStringf(0, 43, buffer, "Vel: --- m/s", TM_getVertVel());
  }
  else{
    display.drawStringf(0, 43, buffer, "Vel: %.2f m/s", TM_getVertVel());
  }
  
  
  float distance = TM_getDistance2target();
  if(distance < 0.0f){
    display.drawString(127, 43, "---- km");
    display.drawString(127, 52, "---- deg");
  } else if(distance < 1000.0f){
<<<<<<< Updated upstream
    display.drawStringf(127, 43, buffer, "Distance: %.0f m", distance * 1000.0f);
    display.drawStringf(127, 52, buffer, "Bearing: %.0f deg", TM_getDir2target());
  } else {
    display.drawStringf(127, 43, buffer, "Distance: %.1f km", distance);
    display.drawStringf(127, 52, buffer, "Bearing: %.0f deg", TM_getDir2target());
=======
    display.drawStringf(127, 43, buffer, "RNG: %.0f m", distance * 1000.0f);
    display.drawStringf(127, 52, buffer, "BRG: %.0f deg", TM_getDir2target());
  } else {
    display.drawStringf(127, 43, buffer, "RNG: %.1f km", distance);
    display.drawStringf(127, 52, buffer, "BRG: %.0f deg", TM_getDir2target());
>>>>>>> Stashed changes
  }
  
  if(LORA_newPacketReceiver()){
    OLED_newPacketCounter = 2;
  }
  //draw blinking dot
  if(OLED_newPacketCounter > 0){
<<<<<<< Updated upstream
    display.fillCircle(100,30, 5);
=======
    display.fillCircle(80, 9, 3);
>>>>>>> Stashed changes
    OLED_newPacketCounter--;
  }
  
  display.setTextAlignment(TEXT_ALIGN_LEFT);
}

void OLED_drawFinder(){
  
  // RSSI bar
  display.drawProgressBar(0,0, 72, 9, LORA_checkTimeout()?TM_getRSSIPercentage():0);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(ArialMT_Plain_10); 
  display.drawStringf(127, 0, buffer, "%3.0f Ppm", LORA_getPacketRate());

  // GEO
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10); 
  display.drawStringf(0, 43, buffer, "%c%.7f", TM_getGeoLatitude().sign, fabs(TM_getGeoLatitude().cord));
  display.drawStringf(0, 52, buffer, "%c%.7f", TM_getGeoLongitude().sign, fabs(TM_getGeoLongitude().cord));

  // Distance
  display.setFont(ArialMT_Plain_16); 
  float distance = GNSS_calcDistance(TM_getGeoLatitude().cord, TM_getGeoLongitude().cord);
  float direction = GNSS_calcDir(0.0f, TM_getGeoLatitude().cord, TM_getGeoLongitude().cord);
  if(distance < 0.0f){
    display.drawString(0, 23, "---- km");
  } else if(distance < 1000.0f){
    display.drawStringf(0, 23, buffer, "%.0f m", distance * 1000.0f);
  } else {
    display.drawStringf(0, 23, buffer, "%.1f km", distance);
  }
  
  // Compass
  if(LORA_newPacketReceiver()){
    OLED_newPacketCounter = 2;
  }
  //draw blinking dot
  if(OLED_newPacketCounter > 0){
    display.fillCircle(75, 57, 4);
    OLED_newPacketCounter--;
  }
  
  display.setTextAlignment(TEXT_ALIGN_LEFT); 
}

void OLED_drawCompass(int16_t x, int16_t y, float angle, float pitch, float roll){
  display.drawCircle(x, y, 23);
  display.drawCircle(x, y, 10);
  display.fillCircle(x+pitch*(50.0f), y+(roll*50.0f), 8);

  uint8_t xp = x;
  uint8_t yp = y;

  uint8_t x0 = xp + cos(angle) * 24; // tip of the arrow on circle
  uint8_t y0 = yp - sin(angle) * 24;
  uint8_t x1 = xp + cos(angle + 0.2) * 15; // triangle point
  uint8_t y1 = yp - sin(angle + 0.2) * 15;
  uint8_t x2 = xp + cos(angle - 0.2) * 15; // triangle point
  uint8_t y2 = yp - sin(angle - 0.2) * 15;

  //display.drawCircle(x + xp, y + yp, 23);
  display.drawLine(2*xp - x0, 2*yp - y0, x0, y0);
  display.drawLine(2*xp - x0 +1, 2*yp - y0, x0+1, y0);
  //Arrow
  display.drawLine(x0, y0, x1, y1);
  display.drawLine(x1, y1, x2, y2);
  display.drawLine(x2, y2, x0, y0);
  //display.drawTriangle(x0, y0, x1, y1, x2, y2);
}