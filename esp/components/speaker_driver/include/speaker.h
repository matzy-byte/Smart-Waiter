#ifndef SPEAKER_H
#define SPEAKER_H

#include <driver/i2s_std.h>

typedef struct {
    uint32_t sample_rate;
    int sample_count;
    i2s_port_t port;
    gpio_num_t pin_bclk;
    gpio_num_t pin_ws;
    gpio_num_t pin_dout;
} SpeakerConfig_t;

class Speaker {
    private:
        SpeakerConfig_t s_config;

        i2s_chan_handle_t tx_handle;
        int16_t* audio;

        void activateSpeaker();
        void deactivateSpeaker();
        void playAudio();
    
    public:
        Speaker(const SpeakerConfig_t& config);
        ~Speaker();

        void playMelody();
};

#endif