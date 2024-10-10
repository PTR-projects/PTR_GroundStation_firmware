#include "Arduino.h"
#include <math.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> 
#include <MicroNMEA.h>
#include "BOARD.h"
#include "GNSS.h"


//------ GNSS -------------
SFE_UBLOX_GNSS myGNSS;
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));


//----- Global vars --------
float myLat    = 0.0f;
float myLon    = 0.0f;
uint8_t myFix  = 0;
uint8_t mySats = 0;

bool GNSS_init(){
#if defined (GPS_RST_PIN)
    pinMode(GPS_RST_PIN, OUTPUT);
    digitalWrite(GPS_RST_PIN, LOW);
    delay(20);
    pinMode(GPS_RST_PIN, INPUT);
    delay(100);
#endif

#if defined (GPS_PPS_PIN)
    pinMode(GPS_PPS_PIN, INPUT);
#endif

#if defined(HAS_GPS)
    if (myGNSS.begin(Serial1) == false) {
        Serial.println(F("Ublox init Failed."));
        //while (1);
        return false;
    }
#endif
    return true;
}

double toRad(double degree) {
    return degree/180 * M_PI;
}

void GNSS_srv(){
    #if defined(HAS_GPS)
    if(myGNSS.checkUblox()){
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

void SFE_UBLOX_GNSS::processNMEA(char incoming)
{
    nmea.process(incoming);
}