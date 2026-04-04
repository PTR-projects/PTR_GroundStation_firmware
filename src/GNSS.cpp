#include "Arduino.h"
#include <math.h>
#include "BOARD.h"
#include "GNSS.h"

#if defined(HAS_GPS)
//------ GNSS -------------
#include <MicroNMEA.h>
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));
HardwareSerial& gps_serial = Serial1;
#endif

//----- Global vars --------
float myLat    = 0.0f;
float myLon    = 0.0f;
uint8_t myFix  = 0;
uint8_t mySats = 0;

bool GNSS_init(){
#if defined(HAS_GPS)
    gpsHardwareReset();

#if defined (GPS_PPS_PIN)
    pinMode(GPS_PPS_PIN, INPUT);
#endif

    Serial1.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
#endif
    return true;
}

double toRad(double degree) {
    return degree/180 * M_PI;
}

void GNSS_srv(){
    #if defined(HAS_GPS)
    GNSS_process();
    
    if (nmea.isValid() == true) {
        long latitude_mdeg = nmea.getLatitude();
        long longitude_mdeg = nmea.getLongitude();
        myLat = latitude_mdeg  / 1000000.0f;
        myLon = longitude_mdeg / 1000000.0f;
        myFix = 1;
        mySats = nmea.getNumSatellites();
    } else {
        myFix = 0;
        mySats = 0;
    }
    #endif
}

float GNSS_calcDistance(float targetLat, float targetLon){
   if(myFix == 0)
        return -1.0f;
    
    double dist;
    dist = sin(toRad(myLat)) * sin(toRad(targetLat)) + cos(toRad(myLat)) * cos(toRad(targetLat)) * cos(toRad(myLon - targetLon));
    dist = acos(dist);

    dist = 6371.0 * dist;

    return (float)dist;
}

float GNSS_calcDir(float deviceAzimuth, float targetLat, float targetLon){
   if(myFix == 0)
        return 0.0f;

    
    float dy = targetLat - myLat;
    float dx = cosf(M_PI/180.0f * myLat) * (targetLon - myLon);
    float angle = atan2f(dy, dx);

    angle = angle * 180.0f / M_PI;
    angle = angle - 90.0f;
    angle = angle + deviceAzimuth;
    
    return angle;
}

float GNSS_getOwnLat(){
    return myLat;
}

float GNSS_getOwnLon(){
    return myLon;
}

uint8_t GNSS_getOwnFix(){
    return myFix;
}

uint8_t GNSS_getOwnSat(){
    return mySats;
}

void GNSS_process(){
#if defined(HAS_GPS)
    while (gps_serial.available()) {
        char c = gps_serial.read();
        nmea.process(c);
    }
#endif
}

void gpsHardwareReset(){
    
#if defined (GPS_RST_PIN) && defined(HAS_GPS)
    pinMode(GPS_RST_PIN, OUTPUT);

    // Empty input buffer
    while (gps_serial.available())
        gps_serial.read();

    digitalWrite(GPS_RST_PIN, LOW);
    delay(50);
    digitalWrite(GPS_RST_PIN, HIGH);

    // Reset is complete when the first valid message is received
    while (1) {
        while (gps_serial.available()) {
        char c = gps_serial.read();
        if (nmea.process(c))
            return;

        }
    }
#endif
}