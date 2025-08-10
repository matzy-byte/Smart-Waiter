#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

typedef struct {
    size_t databuffer_sample_count;
    size_t microphone_sample_count;
    size_t inference_sample_count;
} RingBufferConfig_t;

class RingBuffer {
private:
    RingBufferConfig_t s_config;

    int16_t *data_buffer;
    int16_t *read_buffer;
    size_t index_buffer;
    //SemaphoreHandle_t mutex;

public:
    RingBuffer(const RingBufferConfig_t& config);
    ~RingBuffer();

    void write(const int16_t* input);
    int16_t* read();
};

#endif