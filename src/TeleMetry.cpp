#include <math.h>
#include <string.h>
#include "LORA_typedefs.h"
#include "GNSS.h"
#include "SPIFFS.h"
#include "FileSys.h"
#include "TeleMetry.h"

static lora_bin_packet_t 		lora_bin_packet_d;
static lora_binmin_packet_t 	lora_binmin_packet_d;

static rocket_state_t rocket_state_d;
geocord_t lastvalid_lat = {0.0f, 'N'};
geocord_t lastvalid_lon = {0.0f, 'E'};
float 	  lastvalid_altitude = 0.0f;

float distance2target = 0.0f;
float dir2target = 0.0f;

float TM_RSSI = -200.0f;

int TM_ID = 1; //Target ID

float verticalVel = 0.0f;

static long LORA_lastPacketMillis = 0;
uint32_t prevTimestamp = 1;
float prevAltitude = 0.0f;

void TM_parser(uint8_t * buf, uint8_t len, float RSSI){
	//if(!TM_frameCheck(buf, len))
	//		return;
	// Serial.print(F("New packet!  "));
	// Serial.print("Lenght: ");
	// Serial.print(len);
	// Serial.print("  ID: ");
	// uint16_t ID = (*(uint16_t *)buf);
	// Serial.println(ID);

	

	TM_RSSI = RSSI;
	if((len == sizeof(lora_bin_packet_t)) && ((*(uint16_t *)buf) == 0x00AA))		//Telemetry frame - full state
		TM_parser_FULLSTATE(buf);

	if((len == sizeof(lora_bin_packet_t)) && ((*(uint16_t *)buf) == 0x00AB))		//Telemetry frame - full state retransmitted
		TM_parser_FULLSTATE(buf);

	if((len == sizeof(lora_binmin_packet_t)) && ((*buf) == 0x11))		//Telemetry frame - minimal state
		TM_parser_MINISTATE(buf);

	//else if((len == 6) && ((*buf) == 0x21))	//Command frame - manual control
	//	TM_parser_CMDMC(buf);

	//else if((len == 6) && ((*buf) == 0x31))	//Config frame
	//	TM_parser_CFG(buf);
}

void TM_parser_FULLSTATE(uint8_t * buf){
	//Serial.println(F("Full Telemetry!"));
	if((((lora_bin_packet_t *)buf)->id) != TM_ID) {
		return;
	}

	

	LORA_lastPacketMillis = millis();

	rocket_state_d.timestamp_ms = ((lora_bin_packet_t *)buf)->timestamp_ms;
	rocket_state_d.packet_no = ((lora_bin_packet_t *)buf)->packet_no;
	rocket_state_d.state = ((lora_bin_packet_t *)buf)->state;
	rocket_state_d.flags = ((lora_bin_packet_t *)buf)->flags;
	rocket_state_d.accX  = ((float)((lora_bin_packet_t *)buf)->accX_100) / 100.0f;
	rocket_state_d.accY  = ((float)((lora_bin_packet_t *)buf)->accY_100) / 100.0f;
	rocket_state_d.accZ  = ((float)((lora_bin_packet_t *)buf)->accZ_100) / 100.0f;
	rocket_state_d.gyroX = ((float)((lora_bin_packet_t *)buf)->gyroX_10) / 10.0f;
	rocket_state_d.gyroY = ((float)((lora_bin_packet_t *)buf)->gyroY_10) / 10.0f;
	rocket_state_d.gyroZ = ((float)((lora_bin_packet_t *)buf)->gyroZ_10) / 10.0f;
	rocket_state_d.tilt  = ((float)((lora_bin_packet_t *)buf)->tilt_100) / 100.0f;
	rocket_state_d.pressure = ((lora_bin_packet_t *)buf)->pressure;
	rocket_state_d.velocity = ((float)((lora_bin_packet_t *)buf)->velocity_10)  / 10.0f;
	rocket_state_d.altitude = ((float)((lora_bin_packet_t *)buf)->altitude);
	rocket_state_d.gnss_lat.cord = ((double)((((lora_bin_packet_t *)buf)->lat)))  / 10000000.0;
	rocket_state_d.gnss_lat.sign = (((lora_bin_packet_t *)buf)->lat < 0)?'S':'N';
	rocket_state_d.gnss_lon.cord = ((double)((((lora_bin_packet_t *)buf)->lon)))  / 10000000.0;
	rocket_state_d.gnss_lon.sign = (((lora_bin_packet_t *)buf)->lon < 0)?'W':'E';
	rocket_state_d.gnss_altitude = ((float)((lora_bin_packet_t *)buf)->alti_gps)  / 1000.0f;
	rocket_state_d.fix = ((uint8_t)((lora_bin_packet_t *)buf)->sats_fix) >> 6;
	rocket_state_d.sats = ((uint8_t)((lora_bin_packet_t *)buf)->sats_fix) & 0x3F;
	rocket_state_d.vbat = ((float)((lora_bin_packet_t *)buf)->vbat_10) / 10.0f;

	if(rocket_state_d.fix > 0){
		
		lastvalid_lat.sign = rocket_state_d.gnss_lat.sign;
		lastvalid_lat.cord = rocket_state_d.gnss_lat.cord;
		lastvalid_lon.sign = rocket_state_d.gnss_lon.sign;
		lastvalid_lon.cord = rocket_state_d.gnss_lon.cord;
		lastvalid_altitude = rocket_state_d.gnss_altitude;
<<<<<<< Updated upstream
=======

		verticalVel = (prevAltitude - lastvalid_altitude) / (float(millis()/1000) - float(prevTimestamp/1000));
		Serial.println(verticalVel);
		prevTimestamp = millis();
		prevAltitude = lastvalid_altitude;
>>>>>>> Stashed changes
	}

	distance2target = GNSS_calcDistance(lastvalid_lat.cord, lastvalid_lon.cord);
	dir2target		= GNSS_calcDir(0.0f, lastvalid_lat.cord, lastvalid_lon.cord);

	char bufer[256];
	sprintf(bufer, "%i,%i,0x%x,"
					"%.2f,%.2f,%.2f,"
					"%.2f,%.2f,%.2f,"
					"%.2f,"
					"%.0f,%.0f,%0.1f,"
					"%lf,%lf,"
					"%.1f,%i,%i"
					"%lf,%lf,%i,%i,"
					"%.3f,%.0f", 	
								rocket_state_d.timestamp_ms, rocket_state_d.state, rocket_state_d.flags,
								rocket_state_d.accX, rocket_state_d.accY, rocket_state_d.accZ,
								rocket_state_d.gyroX, rocket_state_d.gyroY, rocket_state_d.gyroZ,
								rocket_state_d.tilt,
								rocket_state_d.pressure, rocket_state_d.altitude, rocket_state_d.velocity,
								rocket_state_d.gnss_lat.cord, rocket_state_d.gnss_lon.cord,
								rocket_state_d.gnss_altitude, rocket_state_d.fix, rocket_state_d.sats,
								GNSS_getOwnLat(), GNSS_getOwnLon(), GNSS_getOwnFix(), GNSS_getOwnFix(),
								distance2target, dir2target);
	//Serial.println(bufer);


	TM_file_write();
}

void TM_parser_MINISTATE(uint8_t * buf){
	//Serial.println(F("Mini Telemetry!"));
	//((lora_binmin_packet_t *)buf)->id;
	//((lora_binmin_packet_t *)buf)->vbat_10;
}

uint8_t TM_getRSSIPercentage(){
	float RSSI_percentage = 100.0f * (164.0f + TM_RSSI) / 164.0f;
  	if(RSSI_percentage < 0.0f)   RSSI_percentage = 0.0f;
  	if(RSSI_percentage > 100.0f) RSSI_percentage = 100.0f;

	return (uint8_t)RSSI_percentage;
}

uint8_t TM_getFlightState(){
	return rocket_state_d.state;
}

geocord_t TM_getGeoLatitude(){
	return lastvalid_lat;
	//return rocket_state_d.gnss_lat;
}

geocord_t TM_getGeoLongitude(){
	return lastvalid_lon;
	//return rocket_state_d.gnss_lon;
}

float TM_getGeoAltitude(){
	return lastvalid_altitude;
}

float TM_getAltitudeKM(){
	return rocket_state_d.altitude/1000.0f;
}

float TM_getVelocity(){
	return rocket_state_d.velocity;
}

float getMachAtAltitude(float altitude){
	if(altitude < 0.0f){
		return 340.0f;
	} else if(altitude < 11110.0f){
		return 340.0f - altitude * 0.00405041f;
	} else if(altitude < 25098.0f){
		return 295.0f;
	} else if(altitude < 47350.0f){
		return 247.628f + altitude * 0.00188747f;
	} else if(altitude < 53445.0f){
		return 337.0f;
	} else if(altitude < 79994.0f){
		return 473.889 - altitude * 0.0025613f;
	} else {
		return 269.0f;
	}

	return 340.0f;
}

float TM_getMach(){
	return rocket_state_d.velocity / getMachAtAltitude(rocket_state_d.altitude);
}

float TM_getDistance2target(){
	return distance2target;
}

float TM_getDir2target(){
	return dir2target;
}

float TM_getVbat(){
	return rocket_state_d.vbat;
}

void TM_file_write(){
	char s[1000];
	sprintf(s, "%ld,%d,%i,%i,"	//sys state
				"%f,%f,%f,"		//acc
				"%f,%f,%f,"		//gyro
				"%f,%f,%f,%f,%f,"	//tilt, press, velo, alti
				"%c,%f,%c,%f,%d,%d,"//geo
				"%f,%f,%d,%d,"		//own geo
				"%f,%f\n", 			//distance, dir
	rocket_state_d.timestamp_ms, 
	rocket_state_d.packet_no, 
	rocket_state_d.state, 
	rocket_state_d.flags, 
	rocket_state_d.accX, 
	rocket_state_d.accY, 
	rocket_state_d.accZ,  
	rocket_state_d.gyroX,
	rocket_state_d.gyroY,
	rocket_state_d.gyroZ,
	rocket_state_d.tilt, 
	rocket_state_d.pressure, 
	rocket_state_d.velocity, 
	rocket_state_d.altitude,
	rocket_state_d.gnss_lat.cord, rocket_state_d.gnss_lat.sign, 
	rocket_state_d.gnss_lon.cord, rocket_state_d.gnss_lon.sign, 
	rocket_state_d.gnss_altitude, 
	rocket_state_d.fix, rocket_state_d.sats,
	GNSS_getOwnLat(), GNSS_getOwnLon(),
	GNSS_getOwnFix(), GNSS_getOwnSat(),
	distance2target, dir2target);
	appendFile(SPIFFS, "/log.csv", s);
}

bool TM_changeID(int id) {
	Serial.printf("Changing ID to %d \n", id);
	TM_ID = id;
	return true;
}

int TM_getID() {
	return TM_ID;
}

long LORA_getPacketHealth(){
    return LORA_lastPacketMillis;
}

float TM_getVertVel(){
	return verticalVel;
}