#include <Speaker.h>

int16_t *responseAudio;

void setupSpeaker() {
    i2s_config_t config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
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

    i2s_pin_config_t pinConfig = {
        .bck_io_num = I2S_SPEAKER_SCK,
        .ws_io_num = I2S_SPEAKER_WS,
        .data_out_num = I2S_SPEAKER_SD,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_SPEAKER_NUM, &config, 0, NULL);
    i2s_set_pin(I2S_SPEAKER_NUM, &pinConfig);
    i2s_zero_dma_buffer(I2S_SPEAKER_NUM);

    responseAudio = (int16_t *)heap_caps_malloc(SPEAKER_SAMPLE_COUNT * sizeof(int16_t), MALLOC_CAP_SPIRAM);
}

void loadAudio() {
    int randomNumber = (esp_random() % (5 - 1 + 1)) + 1;
    char filename[32];
    snprintf(filename, sizeof(filename), "/%s%d.wav", "Miguel", randomNumber);

    
    File file = SPIFFS.open(filename, "rb");
    if (!file || file.isDirectory()) {
        Serial.println("File open failed");
        return;
    }
    file.seek(44);

    size_t dataSize = file.size() - 44;
    if (dataSize > SPEAKER_SAMPLE_COUNT * sizeof(int16_t)) {
        dataSize = SPEAKER_SAMPLE_COUNT * sizeof(int16_t);
    }
    file.read((uint8_t*)responseAudio, dataSize);
    file.close();
}

void playMelody() {
    loadAudio();

    size_t written;
    i2s_write(I2S_SPEAKER_NUM, responseAudio, SPEAKER_SAMPLE_COUNT * sizeof(int16_t), &written, portMAX_DELAY);
}