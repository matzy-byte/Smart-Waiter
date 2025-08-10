#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <driver/i2s.h>
#include <driver/gpio.h>

typedef struct {
    i2s_port_t i2s_port;
    gpio_num_t pin_ws;
    gpio_num_t pin_sck;
    gpio_num_t pin_SD;
    uint32_t sample_rate;
    int sample_count;
    int dc_offset;
    float gain;
} MicrophoneConfig_t;

class Microphone {
    private:
        MicrophoneConfig_t s_config;

        int32_t* raw_audio;
        int16_t* processed_audio;

    public:
        Microphone(const MicrophoneConfig_t& config);
        ~Microphone();

        int16_t* readMicrophoneSamples();
};

#endif
