#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <driver/i2s.h>
#include <esp_heap_caps.h>

#define I2S_MICROPHONE_NUM I2S_NUM_0
#define I2S_MICROPHONE_WS 15
#define I2S_MICROPHONE_SCK 14
#define I2S_MICROPHONE_SD 39
#define SAMPLE_RATE 16000
#define SAMPLE_COUNT (SAMPLE_RATE * 1)

extern int32_t* rawSamples;
extern int16_t* processesSamples;

void setupMicrophone();
void readMicrophoneSamples(int32_t* buffer, size_t sampleCount);
void downsample(int32_t *input, int16_t *output);

#endif
