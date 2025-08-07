#include <Microphone.h>

void setupMicrophone() {
    i2s_config_t config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
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

    i2s_pin_config_t pinConfig = {
        .bck_io_num = I2S_MICROPHONE_SCK,
        .ws_io_num = I2S_MICROPHONE_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_MICROPHONE_SD
    };

    i2s_driver_install(I2S_MICROPHONE_NUM, &config, 0, NULL);
    i2s_set_pin(I2S_MICROPHONE_NUM, &pinConfig);
    i2s_zero_dma_buffer(I2S_MICROPHONE_NUM);
}

void readMicrophoneSamples(int32_t *input, int16_t *output) {
    size_t readBytes = 0;
    i2s_read(I2S_MICROPHONE_NUM, input, MICROPHONE_SAMPLE_COUNT * sizeof(int32_t), &readBytes, portMAX_DELAY);

    for (int i = 0; i < MICROPHONE_SAMPLE_COUNT; i++) {
        output[i] = (int16_t)(((input[i] >> 16) + MICROPHONE_DC_OFFSET) * MICROPHONE_GAIN);
    }
}
