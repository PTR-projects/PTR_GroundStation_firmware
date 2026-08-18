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

    // Przywroc fabryczna konfiguracje modulu - kasuje ustawienia
    // zostawione np. przez inny firmware (ograniczony zestaw zdan NMEA,
    // tryb oszczedzania energii itp.), ktore zapisuja sie trwale w
    // pamieci modulu (BBR) i przetrwaja zmiane firmware na ESP32.
    myGNSS.factoryDefault();
    delay(1000); // daj modulowi chwile na restart i ponowne wystartowanie NMEA

    // Wymus tryb ciaglego namierzania (bez oszczedzania energii).
    myGNSS.powerSaveMode(false);

    // Wylacz nadzorce anteny (UBX-CFG-ANT). Fabryczna konfiguracja chipa
    // domyslnie wlacza sterowanie zasilaniem/wykrywanie zwarcia anteny,
    // co na tej plytce nie pasuje do rzeczywistego podlaczenia anteny
    // (nadzorca potrafil utykac w stanie INIT i blokowac odbior).
    // flags=0x0000 = wylacz caly nadzor, modul ma po prostu nasluchiwac.
    {
        uint8_t antPayload[4] = {0x00, 0x00, 0x00, 0x00}; // flags=0x0000, pins=0x0000
        ubxPacket antPacket;
        antPacket.cls = UBX_CLASS_CFG;
        antPacket.id  = UBX_CFG_ANT;
        antPacket.len = 4;
        antPacket.startingSpot = 0;
        antPacket.payload = antPayload;
        myGNSS.sendCommand(&antPacket, 1100);
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

    float dy = targetLat - myLat;                                // skladowa polnocna
    float dx = cosf(M_PI/180.0f * myLat) * (targetLon - myLon);  // skladowa wschodnia
    float angle = atan2f(dx, dy); // 0=N, 90=E, 180=S, 270=W (standardowy azymut kompasowy)

    angle = angle * 180.0f / M_PI;
    angle = angle + deviceAzimuth;

    // znormalizuj do zakresu 0-360
    angle = fmodf(angle, 360.0f);
    if(angle < 0.0f) angle += 360.0f;

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
