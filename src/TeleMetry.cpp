#include <math.h>
#include <string.h>
#include "LORA_typedefs.h"
#include "GNSS.h"
#include "SPIFFS.h"
#include "FileSys.h"
#include "preferences.h"
#include "SQL.h"
#include <ArduinoJson.h>
#include "TeleMetry.h"

void TM_parser_FULLSTATE(float rssi, uint8_t * buf);
void TM_parser_TRACKER(float rssi, uint8_t * buf);
void TM_file_write(char * line, uint16_t length);
void TM_updateHistory(uint16_t sender_id, float latitude, float longitude, float altitude);

static rocket_state_t rocket_state_d;
geocord_t lastvalid_lat = {0.0f, 'N'};
geocord_t lastvalid_lon = {0.0f, 'E'};
float 	  lastvalid_altitude = 0.0f;
static TM_history_table_entry_t history_table[256];

float distance2target = 0.0f;
float dir2target = 0.0f;

float TM_RSSI = -200.0f;

int TM_ID = 0;
bool TM_id_filter = false;

float verticalVel = 0.0f;

static long LORA_lastPacketMillis = 0;
uint32_t prevTimestamp = 1;
float prevAltitude = 0.0f;

void TM_parser(uint8_t * buf, uint8_t len, float RSSI){
	TM_RSSI = RSSI;
	
	//Telemetry frame - full state
	if((*((uint8_t *)buf + 0)) == PACKET_LEGACY_FULL)		
		TM_parser_FULLSTATE(RSSI, buf);

	//Tracker frame - full state retransmitted
	if((*((uint8_t *)buf + 0)) == PACKET_TRACKER)	
		TM_parser_TRACKER(RSSI, buf);
}

void TM_parser_FULLSTATE(float rssi, uint8_t * buf){
	//Serial.println(F("Full Telemetry!"));

	kppacket_legacyheader_t * pHeader;
	kppacket_payload_legacyfull_t * pPlayload;

	pHeader = (kppacket_legacyheader_t*)buf;
	pPlayload = (kppacket_payload_legacyfull_t*)(buf + sizeof(kppacket_legacyheader_t));

	// Check if packet is destined to this device
	if(TM_id_filter && (pHeader->sender_id != TM_ID)) {
		return;
	}

	LORA_lastPacketMillis = millis();

	rocket_state_d.timestamp_ms = pHeader->timestamp_ms;
	rocket_state_d.packet_no = pHeader->packet_no;
	rocket_state_d.sender_ID = pHeader->sender_id;
	rocket_state_d.dest_ID = 0;
	rocket_state_d.state = pPlayload->state;
	rocket_state_d.flags = pPlayload->flags;
	rocket_state_d.accX  = pPlayload->accX_100 / 100.0f;
	rocket_state_d.accY  = ((float)pPlayload->accY_100) / 100.0f;
	rocket_state_d.accZ  = ((float)pPlayload->accZ_100) / 100.0f;
	rocket_state_d.gyroX = ((float)pPlayload->gyroX_10) / 10.0f;
	rocket_state_d.gyroY = ((float)pPlayload->gyroY_10) / 10.0f;
	rocket_state_d.gyroZ = ((float)pPlayload->gyroZ_10) / 10.0f;
	rocket_state_d.tilt  = ((float)pPlayload->tilt_100) / 100.0f;
	rocket_state_d.pressure = pPlayload->pressure;
	rocket_state_d.velocity = ((float)pPlayload->velocity_10)  / 10.0f;
	rocket_state_d.altitude = ((float)pPlayload->altitude);
	rocket_state_d.gnss_lat.cord = ((float)((pPlayload->lat)))  / 10000000.0;
	rocket_state_d.gnss_lat.sign = (pPlayload->lat < 0)?'S':'N';
	rocket_state_d.gnss_lon.cord = ((float)((pPlayload->lon)))  / 10000000.0;
	rocket_state_d.gnss_lon.sign = (pPlayload->lon < 0)?'W':'E';
	rocket_state_d.gnss_altitude = ((float)pPlayload->alti_gps)  / 1000.0f;
	rocket_state_d.fix = ((uint8_t)pPlayload->sats_fix) >> 6;
	rocket_state_d.sats = ((uint8_t)pPlayload->sats_fix) & 0x3F;
	rocket_state_d.vbat = ((float)pPlayload->vbat_10) / 10.0f;

	if(rocket_state_d.fix > 0){
		
		lastvalid_lat.sign = rocket_state_d.gnss_lat.sign;
		lastvalid_lat.cord = rocket_state_d.gnss_lat.cord;
		lastvalid_lon.sign = rocket_state_d.gnss_lon.sign;
		lastvalid_lon.cord = rocket_state_d.gnss_lon.cord;
		lastvalid_altitude = rocket_state_d.gnss_altitude;

		verticalVel = (prevAltitude - lastvalid_altitude) / ((float)(millis() - prevTimestamp)/1000.0f);
		prevTimestamp = millis();
		prevAltitude = lastvalid_altitude;
	}

	distance2target = GNSS_calcDistance(lastvalid_lat.cord, lastvalid_lon.cord);
	dir2target		= GNSS_calcDir(0.0f, lastvalid_lat.cord, lastvalid_lon.cord);
	TM_updateHistory(rocket_state_d.sender_ID, lastvalid_lat.cord, lastvalid_lon.cord, lastvalid_altitude);

	char raw_packet[512];
	memset(raw_packet, 0, sizeof(raw_packet));
	int packet_len = sizeof(kppacket_payload_rocket_tracker_t) + sizeof(kppacket_header_t);
	for(uint8_t i = 0; i < packet_len; i++){
		sprintf(raw_packet + 2 * i, "%02X", *(buf+i));
	}

	char buffer[1024] = {0};
	uint16_t len = sprintf(buffer, 
				"0x%X;%.1f;%ld;%d;%i;%i;%i;"	//sys state
				"%.1f;"				// Vbat
				"%.2f;%.2f;%.2f;"	// acc
				"%.1f;%.1f;%.1f;"	// gyro
				"%.2f;"				// tilt
				"%.0f;%.2f;%.1f;"	// press, velo, alti
				"%c;%f;%c;%f;%.1f;"	// geo + alt
				"%d;%d;"			// fix + sats
				"%f;%f;%d;%d;"		// own geo
				"%.0f;%.1f;"			// distance, dir
				"%s\n", 			// raw
	PACKET_LEGACY_FULL,
	rssi,
	rocket_state_d.timestamp_ms, 
	rocket_state_d.packet_no, 
	rocket_state_d.sender_ID,
	rocket_state_d.state, 
	rocket_state_d.flags, 
	rocket_state_d.vbat,
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
	rocket_state_d.gnss_lat.sign, rocket_state_d.gnss_lat.cord,  
	rocket_state_d.gnss_lon.sign, rocket_state_d.gnss_lon.cord, 
	rocket_state_d.gnss_altitude, 
	rocket_state_d.fix, rocket_state_d.sats,
	GNSS_getOwnLat(), GNSS_getOwnLon(),
	GNSS_getOwnFix(), GNSS_getOwnSat(),
	distance2target, dir2target,
	raw_packet);
	
	Serial.print(buffer);
	TM_file_write(buffer, len);

	// SQL
	packet_generic_t packet_generic_d = {
		.receiver_id = 0,
		.rssi = rssi,
		.timestamp = 0L,	//LORA_lastPacketMillis,
		.sender_id = pHeader->sender_id,
		.packet_no = pHeader->packet_no,
		.vbat = ((float)pPlayload->vbat_10) / 10.0f,
		.sats_fix = (uint8_t)pPlayload->sats_fix,
		.latitude = ((float)((pPlayload->lat)))  / 10000000.0f,
		.longitude = ((float)((pPlayload->lon)))  / 10000000.0f,
		.altitude = (float)pPlayload->alti_gps / 1000.0f,
		.max_altitude = 0.0f,
		.packet_length = sizeof(kppacket_payload_legacyfull_t) + sizeof(kppacket_legacyheader_t)
	};

	memcpy(packet_generic_d.raw, pHeader, packet_generic_d.packet_length);
	SQL_addToBuffer(&packet_generic_d);
}

void TM_parser_TRACKER(float rssi, uint8_t * buf){
	//Serial.println(F("Full Telemetry!"));

	kppacket_header_t * pHeader;
	kppacket_payload_rocket_tracker_t * pPlayload;

	pHeader = (kppacket_header_t*)buf;
	pPlayload = (kppacket_payload_rocket_tracker_t*)(buf + sizeof(kppacket_header_t));

	// Check if packet is destined to this device
	if(TM_id_filter && (pHeader->sender_id != TM_ID)) {
		return;
	}

	LORA_lastPacketMillis = millis();

	rocket_state_d.timestamp_ms = pHeader->timestamp_ms;
	rocket_state_d.packet_no = pHeader->packet_no;
	rocket_state_d.sender_ID = pHeader->sender_id;
	rocket_state_d.dest_ID = 0;
	rocket_state_d.state = 0;
	rocket_state_d.flags = 0;
	rocket_state_d.accX  = 0.0f;
	rocket_state_d.accY  = 0.0f;
	rocket_state_d.accZ  = 0.0f;
	rocket_state_d.gyroX = 0.0f;
	rocket_state_d.gyroY = 0.0f;
	rocket_state_d.gyroZ = 0.0f;
	rocket_state_d.tilt  = 0.0f;
	rocket_state_d.pressure = 0.0f;
	rocket_state_d.velocity = 0.0f;
	rocket_state_d.altitude = 0.0f;
	rocket_state_d.gnss_lat.cord = ((float)((pPlayload->lat)))  / 10000000.0f;
	rocket_state_d.gnss_lat.sign = (pPlayload->lat < 0)?'S':'N';
	rocket_state_d.gnss_lon.cord = ((float)((pPlayload->lon)))  / 10000000.0f;
	rocket_state_d.gnss_lon.sign = (pPlayload->lon < 0)?'W':'E';
	rocket_state_d.gnss_altitude = ((float)(pPlayload->alti_gps)) - 1000.0f;
	rocket_state_d.fix = ((uint8_t)pPlayload->sats_fix) >> 6;
	rocket_state_d.sats = ((uint8_t)pPlayload->sats_fix) & 0x3F;
	rocket_state_d.vbat = ((float)pPlayload->vbat_10) / 10.0f;

	if(rocket_state_d.fix > 0){
		
		lastvalid_lat.sign = rocket_state_d.gnss_lat.sign;
		lastvalid_lat.cord = rocket_state_d.gnss_lat.cord;
		lastvalid_lon.sign = rocket_state_d.gnss_lon.sign;
		lastvalid_lon.cord = rocket_state_d.gnss_lon.cord;
		lastvalid_altitude = rocket_state_d.gnss_altitude;

		verticalVel = (prevAltitude - lastvalid_altitude) / ((float)(millis() - prevTimestamp)/1000.0f);
		prevTimestamp = millis();
		prevAltitude = lastvalid_altitude;
	}

	distance2target = GNSS_calcDistance(lastvalid_lat.cord, lastvalid_lon.cord);
	dir2target		= GNSS_calcDir(0.0f, lastvalid_lat.cord, lastvalid_lon.cord);
	TM_updateHistory(rocket_state_d.sender_ID, lastvalid_lat.cord, lastvalid_lon.cord, lastvalid_altitude);

	char raw_packet[512];
	memset(raw_packet, 0, sizeof(raw_packet));
	int packet_len = sizeof(kppacket_payload_rocket_tracker_t) + sizeof(kppacket_header_t);
	for(uint8_t i = 0; i < packet_len; i++){
		sprintf(raw_packet + 2 * i, "%02X", *(buf+i));
	}

	char buffer[1024] = {0};
	uint16_t len = sprintf(buffer, "0x%X;%.1f;%ld;%d;%i;"	//Packet ID, timestamp, packet no, sender ID
									"%.1f;"					// Vbat
									"%c;%f;%c;%f;%.1f;"		// geo + alt
									"%d;%d;"				// fix, sats
									"%f;%f;%d;%d;"			// own geo
									"%.0f;%.1f;"			// distance, dir
									"%s\n", 				// raw
	PACKET_TRACKER,
	rssi,
	rocket_state_d.timestamp_ms, 
	rocket_state_d.packet_no, 
	rocket_state_d.sender_ID,
	rocket_state_d.vbat,
	rocket_state_d.gnss_lat.sign, rocket_state_d.gnss_lat.cord, 
	rocket_state_d.gnss_lon.sign, rocket_state_d.gnss_lon.cord,  
	rocket_state_d.gnss_altitude, 
	rocket_state_d.fix, rocket_state_d.sats,
	GNSS_getOwnLat(), GNSS_getOwnLon(),
	GNSS_getOwnFix(), GNSS_getOwnSat(),
	distance2target, dir2target,
	raw_packet);

	Serial.print(buffer);
	TM_file_write(buffer, len);

	// SQL
	packet_generic_t packet_generic_d = {
		.receiver_id = 0,
		.rssi = rssi,
		.timestamp = 0L,	//LORA_lastPacketMillis,
		.sender_id = pHeader->sender_id,
		.packet_no = pHeader->packet_no,
		.vbat = ((float)pPlayload->vbat_10) / 10.0f,
		.sats_fix = (uint8_t)pPlayload->sats_fix,
		.latitude = ((float)((pPlayload->lat)))  / 10000000.0f,
		.longitude = ((float)((pPlayload->lon)))  / 10000000.0f,
		.altitude = ((float)pPlayload->alti_gps)  - 1000.0f,
		.max_altitude = ((float)pPlayload->max_alti)  - 1000.0f,
		.packet_length = sizeof(kppacket_payload_rocket_tracker_t) + sizeof(kppacket_header_t)
	};

	memcpy(packet_generic_d.raw, buf, packet_generic_d.packet_length);
	SQL_addToBuffer(&packet_generic_d);
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

void TM_file_write(char * line, uint16_t length){
	// if(*(line+length) == 0){
	// 	appendFile(SPIFFS, "/log.csv", line);
	// }
}

bool TM_changeID(int id) {
	Serial.printf("Changing ID to %d \n", id);
	TM_ID = id;
	
	preferences_update_id(id);
	
	return true;
}

bool TM_setFilterEnabled(bool enabled) {
	Serial.printf("ID filter %s \n", enabled ? "ON" : "OFF");
	TM_id_filter = enabled;
	preferences_update_id_filter(enabled);
	return true;
}

bool TM_getFilterEnabled() {
	return TM_id_filter;
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

void TM_updateHistory(uint16_t sender_id, float latitude, float longitude, float altitude){
	uint16_t table_size = sizeof(history_table) / sizeof(TM_history_table_entry_t);
    uint16_t index = 0;

	// check if sender_id entry exists
    uint16_t i = 0;
    while(i < table_size){
		// Check if id exist or add it at the first empty position
        if((history_table[i].sender_id == sender_id)
				|| (history_table[i].sender_id == 0)){
            index = i;
            break;
        }
        i++;
    }

	history_table[index].sender_id 	= sender_id;
    history_table[index].last_receive_time_ms = millis();
	history_table[index].latitude 	= latitude;
	history_table[index].longitude 	= longitude;
	history_table[index].altitude 	= altitude;
}


int TM_getKnownSenderIDs(uint16_t *ids, int max_ids){
	if(ids == NULL || max_ids <= 0){
		return 0;
	}

	uint16_t table_size = sizeof(history_table) / sizeof(TM_history_table_entry_t);
	int count = 0;
	for(uint16_t i = 0; i < table_size && count < max_ids; i++){
		if(history_table[i].sender_id == 0){
			break;
		}
		ids[count++] = history_table[i].sender_id;
	}
	return count;
}

String TM_getJSON() {
  JsonDocument doc;
  JsonArray array = doc.to<JsonArray>();
  int time_now = millis();

  for (int i = 0; i < 255; i++) {
	// Check if entry is valid
	if(history_table[i].sender_id == 0){
		break;
	}

    JsonObject item = array.add<JsonObject>();
    item["id"] = history_table[i].sender_id;
    item["age"] = (time_now - history_table[i].last_receive_time_ms) / 1000;
    item["latitude"] = history_table[i].latitude;
    item["longitude"] = history_table[i].longitude;
    item["altitude"] = history_table[i].altitude;
  }

  String json;
  serializeJson(doc, json);

  return json;
}
