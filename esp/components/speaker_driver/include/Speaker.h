#ifndef SPEAKER_H
#define SPEAKER_H

#include <driver/i2s.h>

typedef struct {
    i2s_port_t i2s_port;
    gpio_num_t pin_ws;
    gpio_num_t pin_sck;
    gpio_num_t pin_SD;
    uint32_t sample_rate;
    int sample_count;
} SpeakerConfig_t;

class Speaker {
    private:
        SpeakerConfig_t s_config;
        int16_t *response_audio_buffer;
        size_t response_audio_size;
    
    public:
        Speaker(const SpeakerConfig_t& config);
        ~Speaker();
        
        void powerOnSpeaker();
        void powerOffSpeaker();
        bool loadAudio();
        void playMelody();
};

#endif