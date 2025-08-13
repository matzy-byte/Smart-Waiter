#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <driver/i2s_std.h>

typedef struct {
    uint32_t sample_rate;
    int sample_count;
    float gain;
    int dc_offset;
    i2s_port_t port;
    gpio_num_t pin_bclk;
    gpio_num_t pin_ws;
    gpio_num_t pin_din;
} MicrophoneConfig_t;

class Microphone {
    private:
        MicrophoneConfig_t s_config;

        i2s_chan_handle_t rx_handle;
        int16_t* processed_audio;

    public:
        Microphone(const MicrophoneConfig_t& config);
        ~Microphone();

        int16_t* readMicrophoneSamples();
};

#endif