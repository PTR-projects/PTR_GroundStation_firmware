#pragma once

void LORA_init();
void LORA_startRX();
void LORA_setFlag();
void LORA_RXhandler();
uint8_t LORA_checkTimeout();
uint8_t LORA_newPacketReceiver();
void LORA_PacketCounter();
float LORA_getPacketRate();
<<<<<<< Updated upstream
bool LORA_changeFrequency(int freq);
=======
bool LORA_changeFrequency(int freq);
float LORA_getCurrentFrequency();
>>>>>>> Stashed changes
