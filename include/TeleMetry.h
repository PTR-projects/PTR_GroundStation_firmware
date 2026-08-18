#pragma once

typedef struct {
    float cord;
    char sign;
} geocord_t;

typedef struct {
    uint32_t timestamp_ms;
    uint16_t packet_no;
    uint16_t dest_ID;
    uint16_t sender_ID;
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

typedef struct{
    uint16_t sender_id;
    uint32_t last_receive_time_ms;
    float latitude;
    float longitude;
    float altitude;
} TM_history_table_entry_t;

void TM_parser(uint8_t * buf, uint8_t len, float RSSI);

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

bool TM_changeID(int id);
int TM_getID();
bool TM_setFilterEnabled(bool enabled);
bool TM_getFilterEnabled();
int TM_getKnownSenderIDs(uint16_t *ids, int max_ids);
long LORA_getPacketHealth();
float TM_getVertVel();
String TM_getJSON();