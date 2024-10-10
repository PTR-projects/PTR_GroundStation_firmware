
#if defined(TARGET_ESP32)
//#include "TeleMetry.h"
#include "BluetoothSerial.h"
#include "standard/mavlink.h"
#include "mavlink_driver.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

void BT_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

bool BTconnected = false;

static BluetoothSerial SerialBT;
static unsigned long previousMillisMAVLink  = 0;
static unsigned long next_interval_MAVLink = 300;
static mavlink_message_t msg;
static uint8_t buf[MAVLINK_MAX_PACKET_LEN];


// UAV definitions
static int sysid = 1;                                  ///< ID 20 for this airplane. 1 PX, 255 ground station
static int compid = 158;                               ///< The component sending the message
static int type = MAV_TYPE_QUADROTOR;                  ///< This system is an airplane / fixed wing
static uint8_t system_type = MAV_TYPE_GENERIC;         //airplane
static uint8_t autopilot_type = MAV_AUTOPILOT_INVALID; //n-board controller
static uint8_t system_mode = MAV_MODE_MANUAL_ARMED;    ///< Booting up
static uint32_t custom_mode = 0;                       ///< Custom mode, can be defined by user/adopter
static uint8_t system_state = MAV_STATE_ACTIVE;        ///< System ready for flight


void MAVLink_init(){
    SerialBT.register_callback(BT_callback);
    SerialBT.begin("TTGO"); //Bluetooth device name
}

void MAVLink_srv(){
    if (SerialBT.available()) {
        SerialBT.read();
    }

    unsigned long currentMillisMAVLink = millis();
    if (currentMillisMAVLink - previousMillisMAVLink >= next_interval_MAVLink){
        previousMillisMAVLink = currentMillisMAVLink;

        uint16_t len = 0;

        // Pack the message
        mavlink_msg_heartbeat_pack(sysid, 0, &msg, type, autopilot_type, system_mode, custom_mode, system_state);
        len = mavlink_msg_to_send_buffer(buf, &msg);
        if(BTconnected)
            SerialBT.write(buf, len);

        mavlink_msg_altitude_pack(sysid, 0, &msg, 0, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f);
        len = mavlink_msg_to_send_buffer(buf, &msg);
        if(BTconnected)
            SerialBT.write(buf, len);

        //------- System status ------------
        mavlink_msg_sys_status_pack(sysid, 0, &msg,
                               MAV_SYS_STATUS_SENSOR_3D_GYRO | MAV_SYS_STATUS_SENSOR_3D_ACCEL | MAV_SYS_STATUS_SENSOR_3D_MAG | MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE 
                               | MAV_SYS_STATUS_SENSOR_3D_GYRO2 | MAV_SYS_STATUS_SENSOR_3D_ACCEL2 | MAV_SYS_STATUS_AHRS | MAV_SYS_STATUS_LOGGING | MAV_SYS_STATUS_SENSOR_BATTERY
                               | MAV_SYS_STATUS_PREARM_CHECK | MAV_SYS_STATUS_EXTENSION_USED, 
                               MAV_SYS_STATUS_SENSOR_3D_GYRO | MAV_SYS_STATUS_SENSOR_3D_ACCEL | MAV_SYS_STATUS_SENSOR_3D_MAG | MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE 
                               | MAV_SYS_STATUS_SENSOR_3D_GYRO2 | MAV_SYS_STATUS_SENSOR_3D_ACCEL2 | MAV_SYS_STATUS_AHRS | MAV_SYS_STATUS_LOGGING | MAV_SYS_STATUS_SENSOR_BATTERY
                               | MAV_SYS_STATUS_PREARM_CHECK, 
                               MAV_SYS_STATUS_SENSOR_3D_GYRO | MAV_SYS_STATUS_SENSOR_3D_ACCEL | MAV_SYS_STATUS_SENSOR_3D_MAG | MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE 
                               | MAV_SYS_STATUS_SENSOR_3D_GYRO2 | MAV_SYS_STATUS_SENSOR_3D_ACCEL2 | MAV_SYS_STATUS_AHRS | MAV_SYS_STATUS_LOGGING | MAV_SYS_STATUS_SENSOR_BATTERY
                               | MAV_SYS_STATUS_PREARM_CHECK, 
                               300, 7200, 24, 90, 100, 0, 0, 0, 0, 0, 
                               MAV_SYS_STATUS_RECOVERY_SYSTEM, MAV_SYS_STATUS_RECOVERY_SYSTEM, MAV_SYS_STATUS_RECOVERY_SYSTEM); 
        len = mavlink_msg_to_send_buffer(buf, &msg);
        if(BTconnected)
            SerialBT.write(buf, len);

        //--------- Atitude --------------
        mavlink_msg_attitude_pack(sysid, 0, &msg,
                                    0, 0.1f, 0.1f, 0.2f, 0.0f, 0.0f, 0.0f);
        len = mavlink_msg_to_send_buffer(buf, &msg);
        if(BTconnected)
            SerialBT.write(buf, len);

        //---------- RC channels raw --------
        mavlink_msg_rc_channels_raw_pack(sysid, 0, &msg,
                               0, 0, 11, 22, 33, 44, 55, 66, 77, 88, UINT8_MAX);
        len = mavlink_msg_to_send_buffer(buf, &msg);
        if(BTconnected)
            SerialBT.write(buf, len);

        //---------- VFR HUD --------------
        mavlink_msg_vfr_hud_pack(sysid, 0, &msg,
                               14.0f, 1.34f, 20, 10, 123, 2);
        len = mavlink_msg_to_send_buffer(buf, &msg);
        if(BTconnected)
            SerialBT.write(buf, len);

        //---------- GPS RAW --------------
        mavlink_msg_gps_raw_int_pack(sysid, 0, &msg,
                               0, GPS_FIX_TYPE_3D_FIX, 500000000, 200000000, 100000, UINT16_MAX, UINT16_MAX, 2200, UINT16_MAX, 6, 101000, 0, 0, 0, 0, 0);
        len = mavlink_msg_to_send_buffer(buf, &msg);
        if(BTconnected)
            SerialBT.write(buf, len);

        //--------- Radio status ----------
        mavlink_msg_radio_status_pack(sysid, 0, &msg,
                               200, 102, 95, 10, 12, 34, 23);
        len = mavlink_msg_to_send_buffer(buf, &msg);
        if(BTconnected)
            SerialBT.write(buf, len);

        //---------- GPS orgin ------------
        mavlink_msg_gps_global_origin_pack(sysid, 0, &msg,
                               500100000, 200015000, 0, 0);
        len = mavlink_msg_to_send_buffer(buf, &msg);
        if(BTconnected){
            SerialBT.write(buf, len);
            Serial.println("Sent");
        }

        //TODO:
        //https://github.com/mavlink/c_library_v2/tree/master/common
        //https://mavlink.io/en/messages/common.html
        //MAV_PACKET_RC_CHANNELS_ID = 65
        //MAV_PACKET_HOME_POSITION_ID = 242
        //MAV_PACKET_STATUSTEXT_ID = 253
    }
}

//---------------------------
//      Low Level
//---------------------------
void BT_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT){
    Serial.println("Client Connected");
    BTconnected = true;
  }
  else if(event == ESP_SPP_CLOSE_EVT ){
    Serial.println("Client Disconnected");
    BTconnected = false;
  }
}
#endif