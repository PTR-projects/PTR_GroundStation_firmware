#include "Arduino.h"
#include <math.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> 
#include <MicroNMEA.h>
#include "GNSS.h"


#define pi 3.14159265358979323846

//------ GNSS -------------
SFE_UBLOX_GNSS myGNSS;
char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

//----- Global vars --------
float myLat    = 0.0f;
float myLon    = 0.0f;
uint8_t myFix  = 0;
uint8_t mySats = 0;

void GNSS_init(){
    Serial1.begin(9600, SERIAL_8N1, 34, 12);

    if (myGNSS.begin(Serial1) == false) {
        Serial.println(F("Ublox init Failed. Freezing."));
        //while (1);
    }
}

double toRad(double degree) {
    return degree/180 * pi;
}

void GNSS_srv(){
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
}

float GNSS_calcDistance(float targetLat, float targetLon){
   if(myFix == 0)
        return -1.0f;
    
    /*
    float rlat1 = 3.141592f * myLat / 180.0f;
    float rlat2 = 3.141592f * targetLat / 180.0f;
    float theta = myLon - targetLon;
    float rtheta = 3.141592f * theta / 180.0f;

    float dist = sin(rlat1) * sin(rlat2) 
                + cos(rlat1) * cos(rlat2) * cos(rtheta);
    dist = acos(dist);
    dist = dist * 180.0f / 3.14f;
    dist = dist * 60.0f * 1.1515f;
    dist = dist * 1.609344f;
    // myLat
    // myLon
    */


   /*
    //http://www.movable-type.co.uk/scripts/latlong.html

    float R = 6371000.0f;
    float rlat1 = 3.141592f * myLat / 180.0f;
    float rlat2 = 3.141592f * targetLat / 180.0f;
    float dlat = 3.141592f * (targetLat - myLat) / 180.0f;
    float dlon = 3.141592f * (targetLon - myLon) / 180.0f;

    float a = sin(dlat/2.0f) * sin(dlat/2.0f) + cos(rlat1) * cos(rlat2) * sin(dlon/2)* sin(dlon/2);
    float c = 2 * atan2f(sqrt(a), sqrt(1.0f - a));
    float dist = R * c;
    */

    double dist;
    dist = sin(toRad(myLat)) * sin(toRad(targetLat)) + cos(toRad(myLat)) * cos(toRad(targetLat)) * cos(toRad(myLon - targetLon));
    dist = acos(dist);

    dist = 6371.0 * dist;

    return dist;
}

float GNSS_calcDir(float deviceAzimuth, float targetLat, float targetLon){
   if(myFix == 0)
        return 0.0f;

    
    float dy = targetLat - myLat;
    float dx = cosf(M_PI/180.0f*myLat) * (targetLon - myLon);
    float angle = atan2f(dy, dx);

    angle = angle * 180.0f/3.14f;
    angle = angle - 90.0f;
    angle = angle + deviceAzimuth;
    


    /*
    if(angle > 180.0f)
        angle = angle - 360.0f;
    */
   
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
    //Take the incoming char from the Ublox I2C port and pass it on to the MicroNMEA lib
    //for sentence cracking
    nmea.process(incoming);
}