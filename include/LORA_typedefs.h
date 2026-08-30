#ifndef UTILS_H_
#define UTILS_H_

#include "Arduino.h"
#include <stdint.h>


// MSG types definition
typedef enum{
	PACKET_HEARTBEAT	= 0x00,
	PACKET_LEGACY_FULL	= 0xAA, // kppacket_payload_legacyfull_t
	PACKET_SENSORS		= 0x01, // kppacket_payload_rocket_meas_t
	PACKET_ADCS		    = 0x02,	// kppacket_payload_rocket_ADCS_t
	PACKET_TRACKER		= 0x03,	// kppacket_payload_rocket_tracker_t

	// Custom data packets
	PACKET_CUSTOM_8B	= 0xFC,	// payload = 8B
	PACKET_CUSTOM_16B	= 0xFB,	// payload = 16B
	PACKET_CUSTOM_32B	= 0xFC,	// payload = 32B
	PACKET_CUSTOM_64B	= 0xFD,	// payload = 64B
	PACKET_CUSTOM_128B	= 0xFE,	// payload = 128B
	PACKET_CUSTOM_240B	= 0xFF,	// payload = 240B
} msg_type_e;

// Packet ID definition
typedef union{
	uint16_t ID;
	struct __attribute__((__packed__)){
		uint8_t msg_type;		    // Message type
		uint8_t msg_ver		: 2;	// Message version (should be 0, for future use)
		uint8_t retransmit	: 1;	// Retransmission flag (1=ON, 0=OFF)
		uint8_t encoded		: 1;	// Data encryption (1=ON, 0=OFF)
		uint8_t redu		: 4;	// Redundant bits for future use
	};
} packet_id_t;

// Header structure
typedef struct __attribute__((__packed__)){
    packet_id_t packet_id;
    uint16_t sender_id;
    uint16_t dest_id;
    uint16_t packet_no;
    uint32_t timestamp_ms;
    uint16_t redu;
} kppacket_header_t;

// Sensors measurements
typedef struct __attribute__((__packed__)){
    int16_t vbat_100;    // Vbat*100 (min. -327V, max. 327V)

    int16_t accX_100;    // Acc*100 [g]
    int16_t accY_100;
    int16_t accZ_100;

    int16_t gyroX_10;    // Gyro*10 [deg/s]
    int16_t gyroY_10;
    int16_t gyroZ_10;

    uint16_t pressure;   // Pressure * 0.5 [Pa]

    int32_t lat;         // Latitude  [1e-7 deg]
    int32_t lon;         // Longitude [1e-7 deg]
    int32_t alti_gps;    // Height above ellipsoid [- mm]
    uint8_t sats_fix;    // 6b - sat_cnt + 2b fix
} kppacket_payload_rocket_meas_t;

// Min. telemetry for trackers
typedef struct __attribute__((__packed__)){
    uint8_t retransmission_cnt;
    uint8_t vbat_10;    // Battery voltage (in decivolts [V*10])
    int32_t lat;	    // Latitude  [1e-7 deg]
    int32_t lon;	    // Longitude [1e-7 deg]
    uint16_t alti_gps;	// Height above ellipsoid [+1000]
    uint16_t max_alti;	// Max altitude [+1000]
    uint8_t sats_fix;	// 6b - sat_cnt + 2b fix
} kppacket_payload_rocket_tracker_t;

// Attitude, altitude, velocity, flight state
typedef struct __attribute__((__packed__)){
    uint8_t state;
    uint8_t flags;

	// Quaternions
    float q0;
    float q1;
    float q2;
    float q3;

    int16_t  tilt_100;       // Tilt*100 [deg]
    int16_t  velocity_10;    // Velocity*10 [m/s]
    uint16_t altitude;       // Altitude [m]
} kppacket_payload_rocket_ADCS_t;


// Legacy header structure
typedef struct __attribute__((__packed__)){
    packet_id_t packet_id;
    uint16_t sender_id;
    uint16_t packet_no;
    uint32_t timestamp_ms;
} kppacket_legacyheader_t;

typedef struct __attribute__((__packed__)){
	uint8_t state;
	uint8_t flags;

	uint8_t vbat_10;	//Vbat*10

	int16_t accX_100;	//Acc*100 [g]
	int16_t accY_100;
	int16_t accZ_100;

	int16_t gyroX_10;	//Gyro*10 [deg/s]
	int16_t gyroY_10;
	int16_t gyroZ_10;

	int16_t tilt_100;	//Tilt*100 [deg]
	float pressure;		//Pressure [Pa]
	int16_t velocity_10;	//Velocity*10 [m/s]
	uint16_t altitude;		//Altitude [m]

	int32_t lat;		//[1e-7 deg]
	int32_t lon;		//[1e-7 deg]
	int32_t alti_gps;	//Height above ellipsoid [- mm]
	uint8_t sats_fix;	//6b - sats + 2b fix
} kppacket_payload_rocket_t;

typedef struct __attribute__((__packed__)){
    uint8_t packet_len;
    union{
        struct{
            kppacket_header_t header;
            uint8_t  payload[255 - sizeof(kppacket_header_t)];
        };
       struct{
           kppacket_legacyheader_t legacyheader;
           uint8_t  legacypayload[255 - sizeof(kppacket_legacyheader_t)];
       };
    };
} kppacket_t;


#endif /* UTILS_H_ */
