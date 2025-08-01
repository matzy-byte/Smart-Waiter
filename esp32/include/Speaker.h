#ifndef SPEAKER_H
#define SPEAKER_H

#include <Global.h>
#include <driver/i2s.h>
#include <esp_heap_caps.h>
#include <math.h>
#include <SPIFFS.h>
#include <esp_random.h>

#define I2S_SPEAKER_NUM I2S_NUM_1
#define I2S_SPEAKER_WS 13
#define I2S_SPEAKER_SCK 12
#define I2S_SPEAKER_SD 21

extern int16_t* responseAudio;

void setupSpeaker();
void playMelody();

#endif