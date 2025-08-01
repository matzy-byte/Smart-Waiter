#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <Global.h>
#include <driver/i2s.h>
#include <esp_heap_caps.h>

#define I2S_MICROPHONE_NUM I2S_NUM_0
#define I2S_MICROPHONE_WS 15
#define I2S_MICROPHONE_SCK 14
#define I2S_MICROPHONE_SD 39

void setupMicrophone();
void readMicrophoneSamples(int32_t* input, int16_t* output);

#endif
