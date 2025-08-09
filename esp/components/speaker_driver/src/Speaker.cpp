#include "Speaker.h"

#include <esp_heap_caps.h>
#include <esp_random.h>
#include <sys/stat.h>

Speaker::Speaker(const SpeakerConfig_t& config) {
    this->s_config = config;
    this->response_audio_buffer = nullptr;
    this->response_audio_size = 0;
}

Speaker::~Speaker() {
    if (this->response_audio_buffer) {
        heap_caps_free(this->response_audio_buffer);
    }
};

void Speaker::powerOnSpeaker() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = this->s_config.sample_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 16,
        .dma_buf_len = 512,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    i2s_pin_config_t i2s_pin_config = {
        .bck_io_num = this->s_config.pin_sck,
        .ws_io_num = this->s_config.pin_ws,
        .data_out_num = this->s_config.pin_SD,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(this->s_config.i2s_port, &i2s_config, 0, NULL);
    i2s_set_pin(this->s_config.i2s_port, &i2s_pin_config);
    i2s_zero_dma_buffer(this->s_config.i2s_port);
};

void Speaker::powerOffSpeaker() {
    i2s_driver_uninstall(this->s_config.i2s_port);
};

bool Speaker::loadAudio() {
    if (this->response_audio_buffer) {
        heap_caps_free(this->response_audio_buffer);
        this->response_audio_buffer = nullptr;
        this->response_audio_size = 0;
    }

    int randomNumber = (esp_random() % 5) + 1;
    char filename[32];
    snprintf(filename, sizeof(filename), "/spiffs/Miguel%d.wav", randomNumber);
    
    FILE *file = fopen(filename, "rb");
    fseek(file, 44, SEEK_SET);
    struct stat st;
    if (stat(filename, &st) != 0) {
        fclose(file);
        return false;
    }

    size_t dataSize = st.st_size - 44;
    this->response_audio_buffer = (int16_t*)heap_caps_malloc(dataSize, MALLOC_CAP_SPIRAM);
    this->response_audio_size = fread(this->response_audio_buffer, 1, dataSize, file);
    if (dataSize > this->s_config.sample_count * sizeof(int16_t)) {
        dataSize = this->s_config.sample_count * sizeof(int16_t);
    }
    fclose(file);
    return true;
}

void Speaker::playMelody() {
    if (!this->loadAudio()) {
        return;
    }

    this->powerOnSpeaker();

    size_t written;
    i2s_write(this->s_config.i2s_port, this->response_audio_buffer, this->response_audio_size * sizeof(int16_t), &written, portMAX_DELAY);

    this->powerOffSpeaker();
}