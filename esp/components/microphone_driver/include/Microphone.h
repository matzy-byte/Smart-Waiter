#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <driver/i2s.h>

typedef struct {
    i2s_port_t i2s_port;
    gpio_num_t pin_ws;
    gpio_num_t pin_sck;
    gpio_num_t pin_SD;
    int sample_rate;
    int sample_count;
    int dc_offset;
    float gain;
} MicrophoneConfig_t;

class Microphone {
    private:
        MicrophoneConfig_t s_config;

    public:
        Microphone(const MicrophoneConfig_t& config);
        ~Microphone();

        void readMicrophoneSamples(int32_t* input, int16_t* output);
};

#endif
