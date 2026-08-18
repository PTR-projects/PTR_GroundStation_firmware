#include "BOARD.h"
#include "LORA_typedefs.h"
#include "lora.h"
#include "TeleMetry.h"
#include <SPI.h>
#include <RadioLib.h>
#include "FileSys.h"
#include "preferences.h"

// RadioLib uses the global SPI object. On ESP32-S3 that is FSPI (SPI2),
// started in BOARD_init() with the radio pins. TFT uses a separate HSPI
// (SPI3) instance — do not pass FSPI to the display.
#if defined(USING_SX1276)
static SX1276 radio = new Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_DIO1_PIN, SPI);

#elif defined(USING_SX1278)
static SX1278 radio = new Module(RADIO_CS_PIN, RADIO_DIO0_PIN, RADIO_RST_PIN, RADIO_DIO1_PIN, SPI);

#elif defined(USING_SX1262)
static SX1262 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN, SPI);

#elif defined(USING_LR1121)
static LR1121 radio = new Module(RADIO_CS_PIN, RADIO_DIO9_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN, SPI);

#endif

static volatile bool LED_blink = false;
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

float LORA_currentFrequencyMHz = 434.25f;

bool LORA_init(){
    Serial.print(F("[LORA] Initializing ... "));

    int state = radio.begin();
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!"));
    } else {
        Serial.print(F("failed, code "));
        Serial.println(state);
    }

    radio.setFrequency(433.0f);
    radio.setBandwidth(125);        // 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125, 250, 500
    radio.setSpreadingFactor(8);   // 6 - 12
    radio.setCodingRate(5);
    radio.setSyncWord(0x14);        // set LoRa sync word to 0x14
    radio.setOutputPower(10);       // set output power to 10 dBm (accepted range is -3 - 17 dBm)
    radio.setPreambleLength(15);    // set LoRa preamble length to 15 symbols (accepted range is 6 - 65535)
    radio.setCRC(true);
    radio.setPacketReceivedAction(LORA_setFlag);
    Serial.print(F("[LORA] Starting to listen ... "));

    return true;
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

#ifdef BOARD_LED
        if(LED_blink){
            LED_blink = false;
            digitalWrite(BOARD_LED, LED_ON);
        } else {
            digitalWrite(BOARD_LED, LED_OFF);
        }
#endif
    }

    if (receivedFlag) {
        // disable the interrupt service routine while
        // processing the data
        enableInterrupt = false;

        //reset counter
        lora_timeout_counter = 50;

        // reset flag
        receivedFlag = false;
        
        int state = radio.readData(lora_buffer, radio.getPacketLength());

        if (state == RADIOLIB_ERR_NONE) {
            LED_blink = true;
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

    preferences_update_frequency(freq);

    if(radio.setFrequency((float)temp) != 0){
        return false;
    }
    LORA_currentFrequencyMHz = ((float)freq / 1000);
    LORA_startRX();
    return true;
}

float LORA_getCurrentFrequency() {
    return LORA_currentFrequencyMHz;
}

