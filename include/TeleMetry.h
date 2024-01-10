#pragma once

typedef struct {
    double cord;
    char sign;
} geocord_t;

typedef struct {
    uint32_t timestamp_ms;
    uint16_t packet_no;
    uint8_t  state;
    uint8_t  flags;

    float vbat;	//Vbat

    float accX;	//Acc [g]
    float accY;
    float accZ;

    float gyroX;	//Gyro [deg/s]
    float gyroY;
    float gyroZ;

    float tilt;	    //Tilt [deg]
    float pressure;	//Pressure [Pa]
    float velocity;	//Velocity [m/s]
    float altitude; //Altitude [m]

    geocord_t gnss_lat;
    geocord_t gnss_lon;
    float     gnss_altitude;
    uint8_t sats;
    uint8_t fix;
} rocket_state_t;

void TM_parser(uint8_t * buf, uint8_t len, float RSSI);
void TM_parser_FULLSTATE(uint8_t * buf);
void TM_parser_MINISTATE(uint8_t * buf);

uint8_t TM_getRSSIPercentage();
uint8_t TM_getFlightState();
geocord_t TM_getGeoLatitude();
geocord_t TM_getGeoLongitude();
float     TM_getGeoAltitude();
float TM_getAltitudeKM();
float TM_getVelocity();
float TM_getMach();
float TM_getDistance2target();
float TM_getDir2target();
float TM_getVbat();
void  TM_file_write();
bool TM_changeID(int id);
int TM_getID();
long LORA_getPacketHealth();
float TM_getVertVel();