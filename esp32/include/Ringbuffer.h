#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <Global.h>
#include <esp_heap_caps.h>
#include <cstring>

extern int32_t *rawMicrophoneSamples;
extern int16_t *processedMicrophoneSamples;

void setupRingbuffer();
void writeBuffer(int16_t *input);
int16_t* readBuffer();

#endif