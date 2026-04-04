#pragma once

bool GNSS_init();
void GNSS_srv();
float GNSS_calcDistance(float targetLat, float targetLon);
float GNSS_calcDir(float deviceAzimuth, float targetLat, float targetLon);
float GNSS_getOwnLat();
float GNSS_getOwnLon();
uint8_t GNSS_getOwnFix();
uint8_t GNSS_getOwnSat();
void GNSS_process();
void gpsHardwareReset();