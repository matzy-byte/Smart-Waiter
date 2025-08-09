#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

class RingBuffer {
private:
    size_t microphone_sample_count;
    size_t databuffer_sample_count;
    size_t inference_sample_count;

    int16_t *data_buffer;
    int16_t *read_buffer;
    size_t index_buffer;
    SemaphoreHandle_t mutex;

public:
    RingBuffer(size_t mic_samples, size_t databuf_samples, size_t inference_samples);
    ~RingBuffer();

    void write(const int16_t* input);
    int16_t* read();
};

#endif