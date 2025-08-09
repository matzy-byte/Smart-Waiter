#include <Microphone.h>

Microphone::Microphone(const MicrophoneConfig_t& config) {
    this->s_config = config;

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = s_config.sample_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = this->s_config.pin_sck,
        .ws_io_num = this->s_config.pin_ws,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = this->s_config.pin_SD
    };

    i2s_driver_install(s_config.i2s_port, &i2s_config, 0, NULL);
    i2s_set_pin(s_config.i2s_port, &pin_config);
    i2s_zero_dma_buffer(s_config.i2s_port);
}

Microphone::~Microphone() {
    i2s_driver_uninstall(this->s_config.i2s_port); 
}

void Microphone::readMicrophoneSamples(int32_t* input, int16_t* output) {
    size_t readBytes = 0;
    i2s_read(this->s_config.i2s_port, input, this->s_config.sample_count * sizeof(int32_t), &readBytes, portMAX_DELAY);

    for (int i = 0; i < this->s_config.sample_count; i++) {
        output[i] = (int16_t)(((input[i] >> 16) + this->s_config.dc_offset) * this->s_config.gain);
    }
}