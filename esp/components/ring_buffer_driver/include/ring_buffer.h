#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    size_t data_buffer_sample_count;
    size_t microphone_buffer_sample_count;
    size_t inference_buffer_sample_count;
} RingBufferConfig_t;

class RingBuffer {
    private:
        RingBufferConfig_t s_config;

        size_t index;
        int16_t* data_buffer;
        int16_t* read_buffer;

    public:
        RingBuffer(const RingBufferConfig_t& config);
        ~RingBuffer();

        void writeBuffer(int16_t* input);
        int16_t* readBuffer();
};

#endif