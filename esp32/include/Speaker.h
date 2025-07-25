#ifndef SPEAKER_H
#define SPEAKER_H

#include <driver/i2s.h>
#include <esp_heap_caps.h>
#include <math.h>

#define I2S_SPEAKER_NUM I2S_NUM_1
#define I2S_SPEAKER_WS 13
#define I2S_SPEAKER_SCK 12
#define I2S_SPEAKER_SD 21
#define SAMPLE_RATE 16000
#define SAMPLE_COUNT (SAMPLE_RATE * 1)

extern int16_t* melody;

void setupSpeaker();
void playMelody();

#endif