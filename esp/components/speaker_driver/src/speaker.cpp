#include <speaker.h>

#include <driver/gpio.h>
#include <esp_heap_caps.h>
#include <portmacro.h>
#include <esp_random.h>

Speaker::Speaker(const SpeakerConfig_t& config) {
    this->s_config = config;
    this->audio = (int16_t*) heap_caps_malloc(4000 * sizeof(int16_t), MALLOC_CAP_INTERNAL);
    if (!this->audio) printf("NOT ALLOCATED");

    gpio_set_direction(this->s_config.pin_bclk, GPIO_MODE_OUTPUT);
    gpio_set_direction(this->s_config.pin_ws, GPIO_MODE_OUTPUT);
    gpio_set_direction(this->s_config.pin_dout, GPIO_MODE_OUTPUT);

    gpio_set_level(this->s_config.pin_bclk, 0);
    gpio_set_level(this->s_config.pin_ws, 0);
    gpio_set_level(this->s_config.pin_dout, 0);
}

Speaker::~Speaker() {
    if (this->audio) heap_caps_free(this->audio);
}

void Speaker::playMelody() {
    this->activateSpeaker();
    this->playAudio();
    this->deactivateSpeaker();
}

void Speaker::activateSpeaker() {
    i2s_chan_config_t i2s_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(this->s_config.port, I2S_ROLE_MASTER);
    i2s_chan_cfg.auto_clear = true;

    i2s_std_config_t i2s_config = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(s_config.sample_rate),
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_16BIT,
            .slot_mode = I2S_SLOT_MODE_MONO,
            .slot_mask = I2S_STD_SLOT_LEFT,
            .ws_width = I2S_SLOT_BIT_WIDTH_16BIT,
            .ws_pol = false,
            .bit_shift = true,
            .left_align = true,
            .big_endian = false,
            .bit_order_lsb = false
        },
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = this->s_config.pin_bclk,
            .ws = this->s_config.pin_ws,
            .dout = this->s_config.pin_dout,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false
            }
        }
    };

    i2s_new_channel(&i2s_chan_cfg, &this->tx_handle, NULL);
    i2s_channel_init_std_mode(this->tx_handle, &i2s_config);
    i2s_channel_enable(this->tx_handle);
}

void Speaker::deactivateSpeaker() {
    if (this->tx_handle) {
        i2s_channel_disable(this->tx_handle);
        i2s_del_channel(this->tx_handle);
        this->tx_handle = nullptr;
    }
}

void Speaker::playAudio() {
    int randomNumber = (random() % 5) + 1;
    char filename[32];
    snprintf(filename, sizeof(filename), "/littlefs/Miguel%d.wav", randomNumber);

    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("Failed to open file: %s", filename);
        return;
    }

    size_t bytes_read;
    esp_err_t ret;
    size_t bytes_written = 0;

    while ((bytes_read = fread(this->audio, 1, 4000 * sizeof(int16_t), f)) > 0) {
        ret = i2s_channel_write(this->tx_handle, this->audio, bytes_read, &bytes_written, portMAX_DELAY);
        if (ret != ESP_OK) {
            printf("i2s_channel_write failed: %s\n", esp_err_to_name(ret));
            break;
        }
    }

    fclose(f);
}
