#include "LORA_typedefs.h"
#include "lora.h"
#include "TeleMetry.h"
#include <SPI.h>
#include <RadioLib.h>
#include "FileSys.h"

//---------------- LORA declarations -------------------------
#define RADIO_SCLK_PIN              5
#define RADIO_MISO_PIN              19
#define RADIO_MOSI_PIN              27
#define RADIO_CS_PIN                18
#define RADIO_DI0_PIN               26
#define RADIO_RST_PIN               23
#define RADIO_DIO1_PIN              33
#define RADIO_BUSY_PIN              32

static SX1278 radio = new Module(18, 26, 23);
//SX1276 radio = new Module(RADIO_CS_PIN, RADIO_DI0_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);
static volatile bool receivedFlag = false;     // flag to indicate that a packet was received
static volatile bool enableInterrupt = true;   // disable interrupt when it's not needed
static uint8_t lora_buffer[256];

//Timeout variables
static long LORA_timeout_previousMillis = 0;
static long LORA_timeout_interval = 100;
static uint8_t lora_timeout_counter = 20;

//Counter variables
static long LORA_counter_previousMillis = 0;
static long LORA_counter_interval = 1000;
static uint16_t packetCounter[5] = {0,0,0,0,0};
static float packet_rate = 0;
static uint8_t LORA_newPacketReceivedOLED = 0;

float LORA_currentFrequencyMHz = 434.25;





void LORA_init(){
    Serial.print(F("[SX1278] Initializing ... "));
    int state = radio.begin();
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!"));
    } else {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true);
    }

<<<<<<< Updated upstream
    radio.setFrequency(433.55);
=======
    radio.setFrequency(434.25);
>>>>>>> Stashed changes
    radio.setBandwidth(125);        // 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125, 250, 500
    radio.setSpreadingFactor(8);   // 6 - 12
    radio.setCodingRate(5);
    radio.setSyncWord(0x14);        // set LoRa sync word to 0x14
    radio.setOutputPower(10);       // set output power to 10 dBm (accepted range is -3 - 17 dBm)
    radio.setPreambleLength(15);    // set LoRa preamble length to 15 symbols (accepted range is 6 - 65535)
    radio.setGain(0);               // set amplifier gain to 1 (accepted range is 1 - 6, where 1 is maximum gain), 0=AGC
    radio.setCRC(true);
    radio.setDio0Action(LORA_setFlag);
    Serial.print(F("[SX1276] Starting to listen ... "));

    TM_file_write();
}

void LORA_startRX(){
    int state = radio.startReceive();
}

void LORA_setFlag(){
    // check if the interrupt is enabled
    if (!enableInterrupt) {
        return;
    }

    // we got a packet, set the flag
    receivedFlag = true;
}

void LORA_RXhandler(){
    unsigned long currentMillis = millis();
    if((currentMillis - LORA_timeout_previousMillis) > LORA_timeout_interval) {
        LORA_timeout_previousMillis = currentMillis;

        if(lora_timeout_counter) lora_timeout_counter--;
    }

  if (receivedFlag) {
        // disable the interrupt service routine while
        // processing the data
        enableInterrupt = false;

        //reset counter
        lora_timeout_counter = 20;

        // reset flag
        receivedFlag = false;
        
        int state = radio.readData(lora_buffer, radio.getPacketLength());

        if (state == RADIOLIB_ERR_NONE) {
            packetCounter[0]++;
            LORA_newPacketReceivedOLED = 1;
            TM_parser(lora_buffer, radio.getPacketLength(), radio.getRSSI()); 
        } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
            // packet was received, but is malformed
            Serial.println();
            Serial.println(F("[SX1276] CRC error!"));

        } else {
            // some other error occurred
            Serial.println();
            Serial.print(F("[SX1276] Failed, code "));
            Serial.println(state);
        }

        // put module back to listen mode
        radio.startReceive();

        // we're ready to receive more packets,
        // enable interrupt service routine
        enableInterrupt = true;
    }
}

uint8_t LORA_checkTimeout(){
    return (lora_timeout_counter != 0);
}

uint8_t LORA_newPacketReceiver(){
    uint8_t tmp = LORA_newPacketReceivedOLED;
    LORA_newPacketReceivedOLED = 0;
    return tmp;
}

void LORA_PacketCounter(){
    unsigned long currentMillis = millis();
    if((currentMillis - LORA_counter_previousMillis) > LORA_counter_interval) {
        LORA_counter_previousMillis = currentMillis;
        
        uint32_t packet_sum = packetCounter[0] + packetCounter[1] + packetCounter[2] + packetCounter[3] + packetCounter[4];

        packetCounter[4] = packetCounter[3];
        packetCounter[3] = packetCounter[2];
        packetCounter[2] = packetCounter[1];
        packetCounter[1] = packetCounter[0];
        packetCounter[0] = 0;

        packet_rate = 0.7f * packet_rate + 0.3f*(((float)packet_sum) / 5.0f * 60.0f);   //Packets per minute
    }
}

float LORA_getPacketRate(){
    return packet_rate;
}

bool LORA_changeFrequency(int freq){
    double temp = (double)freq / 1000.0;

    Serial.printf("Changing frequency to %f \n", (float)temp);
    
    if(radio.setFrequency((float)temp) != 0){
        return false;
    }
<<<<<<< Updated upstream
    LORA_startRX();
    return true;
}
=======
    LORA_currentFrequencyMHz = ((float)freq / 1000);
    LORA_startRX();
    return true;
}

float LORA_getCurrentFrequency() {
    return LORA_currentFrequencyMHz;
}

>>>>>>> Stashed changes
