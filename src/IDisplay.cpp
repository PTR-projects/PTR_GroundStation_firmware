#include "IDisplay.h"

// High-level drawing functions implemented once using low-level primitives

void IDisplay::drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress) {
    uint16_t radius = height / 2;
    uint16_t doubleRadius = 2 * radius;
    
    // Fill progress
    uint16_t maxProgressWidth = (width - doubleRadius + 1) * progress / 100;
    setColor(COLOR_WHITE);
    fillRect(x, y, maxProgressWidth, height);
    drawLine(x, y + radius, width, y + radius);
    
    // Draw diagonal lines for progress effect
    setColor(COLOR_BLACK);
    for(uint8_t i=0; (i*8) < width; i++){
        drawLine(x + 8*i, y,            x + 8*i + 4,     y + radius);
        drawLine(x + 8*i, y+height-1,   x + 8*i + 4,     y + radius);
        drawLine(x + 8*i+1, y,          x + 8*i + 4 + 1, y + radius);
        drawLine(x + 8*i+1, y+height-1, x + 8*i + 4 + 1, y + radius);
    }

    setColor(COLOR_WHITE);
}

void IDisplay::drawRocketLaunch() {
    static unsigned long newPacketCounter = 0;
    
    setColor(COLOR_WHITE);

    // RSSI bar
    drawProgressBar(0, 5, 72, 7, LORA_checkTimeout() ? TM_getRSSIPercentage() : 0);
    
    unsigned long currentMillis = millis();
    unsigned long dT = (currentMillis - LORA_getPacketHealth()) / 1000;

    // Packet rate or time since last packet
    Display_setTextAlignment(TEXT_ALIGN_RIGHT);
    if(dT > 5){
        drawString(127, 3, String(dT) + "s ", 1);
    }
    else{
        drawString(127, 3, String(LORA_getPacketRate(), 0) + " Ppm", 1);
    }

    // GEO data
    Display_setTextAlignment(TEXT_ALIGN_LEFT);
    if(TM_getGeoAltitude() < 1000.0f){
        drawString(0, 13, String(TM_getGeoAltitude(), 2) + " m", 1);
    }
    else{
        drawString(0, 13, String(TM_getGeoAltitude() / 1000.0f, 2) + " km", 1);
    }
    
    drawString(0, 23, String(TM_getGeoLatitude().sign) + String(fabs(TM_getGeoLatitude().cord), 6), 1);
    drawString(0, 33, String(TM_getGeoLongitude().sign) + String(fabs(TM_getGeoLongitude().cord), 6), 1);

    // RF freq and ID
    drawString(0, 53, String(LORA_getCurrentFrequency(), 2) + "MHz", 1);
    drawString(0, 43, "ID: " + String(TM_getID()), 1);

    // Battery and velocity
    Display_setTextAlignment(TEXT_ALIGN_RIGHT);
    drawString(127, 13, "BAT:  " + String(PWR_getBAT(), 2) + " V", 1);
    drawString(127, 23, "vAvi: " + String(TM_getVbat(), 2) + " V", 1);
    
    if(isnan(TM_getVertVel())){
        drawString(127, 33, "Vel: --- m/s", 1);
    }
    else{
        drawString(127, 33, "Vel: " + String(TM_getVertVel(), 2) + " m/s", 1);
    }
    
    // Distance and bearing
    float distance = TM_getDistance2target();
    if(distance <= 0.0f){
        drawString(127, 43, "---- km", 1);
        drawString(127, 52, "---- deg", 1);
    } else if(distance < 1000.0f){
        drawString(127, 43, "RNG: " + String(distance * 1000.0f, 0) + " m", 1);
        drawString(127, 52, "BRG: " + String(TM_getDir2target(), 0) + " deg", 1);
    } else {
        drawString(127, 43, "RNG: " + String(distance, 1) + " km", 1);
        drawString(127, 52, "BRG: " + String(TM_getDir2target(), 0) + " deg", 1);
    }
    
    // New packet indicator
    if(LORA_newPacketReceiver()){
        newPacketCounter = 2;
    }

    if(newPacketCounter > 0){
        fillCircle(80, 7, 3);
        newPacketCounter--;
    }
    Display_setTextAlignment(TEXT_ALIGN_LEFT);
}

void IDisplay::drawFinder() {
    static unsigned long newPacketCounter = 0;
    
    setColor(COLOR_WHITE);

    // RSSI bar
    Display_setTextAlignment(TEXT_ALIGN_RIGHT);
    drawProgressBar(0, 0, 72, 9, LORA_checkTimeout() ? TM_getRSSIPercentage() : 0);
    drawString(127, 0, String(LORA_getPacketRate(), 0) + " Ppm", 1);

    // GEO coordinates
    Display_setTextAlignment(TEXT_ALIGN_LEFT);
    drawString(0, 43, String(TM_getGeoLatitude().sign) + String(fabs(TM_getGeoLatitude().cord), 7), 1);
    drawString(0, 52, String(TM_getGeoLongitude().sign) + String(fabs(TM_getGeoLongitude().cord), 7), 1);

    // Distance
    float distance = GNSS_calcDistance(TM_getGeoLatitude().cord, TM_getGeoLongitude().cord);
    if(distance < 0.0f){
        drawString(0, 23, "---- km", 2);
    } else if(distance < 1000.0f){
        drawString(0, 23, String(distance * 1000.0f, 0) + " m", 2);
    } else {
        drawString(0, 23, String(distance, 1) + " km", 2);
    }
    
    // New packet indicator
    if(LORA_newPacketReceiver()){
        newPacketCounter = 2;
    }
    if(newPacketCounter > 0){
        fillCircle(75, 57, 4);
        newPacketCounter--;
    }
}

void IDisplay::drawCompass(int16_t x, int16_t y, float angle, float pitch, float roll) {
    setColor(COLOR_WHITE);
    drawCircle(x, y, 23);
    drawCircle(x, y, 10);
    fillCircle(x + pitch * 50.0f, y + roll * 50.0f, 8);

    uint8_t xp = x;
    uint8_t yp = y;

    uint8_t x0 = xp + cos(angle) * 24;
    uint8_t y0 = yp - sin(angle) * 24;
    uint8_t x1 = xp + cos(angle + 0.2) * 15;
    uint8_t y1 = yp - sin(angle + 0.2) * 15;
    uint8_t x2 = xp + cos(angle - 0.2) * 15;
    uint8_t y2 = yp - sin(angle - 0.2) * 15;

    drawLine(2*xp - x0, 2*yp - y0, x0, y0);
    drawLine(2*xp - x0 + 1, 2*yp - y0, x0 + 1, y0);
    drawLine(x0, y0, x1, y1);
    drawLine(x1, y1, x2, y2);
    drawLine(x2, y2, x0, y0);
}

void IDisplay::drawSplash() {
  drawSplash();
}