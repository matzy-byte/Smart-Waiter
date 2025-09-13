#include <microphone.h>

#include <esp_heap_caps.h>
#include <portmacro.h>

Microphone::Microphone(const MicrophoneConfig_t& config) {
    this->s_config = config;
    this->processed_audio = (int16_t*) heap_caps_malloc(this->s_config.sample_count * sizeof(int16_t), MALLOC_CAP_INTERNAL);

    i2s_chan_config_t i2s_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(this->s_config.port, I2S_ROLE_MASTER);
    i2s_chan_cfg.auto_clear = true;

    i2s_std_config_t i2s_config = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(s_config.sample_rate),
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_32BIT,
            .slot_mode = I2S_SLOT_MODE_MONO,
            .slot_mask = I2S_STD_SLOT_LEFT,
            .ws_width = I2S_SLOT_BIT_WIDTH_32BIT,
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
            .dout = I2S_GPIO_UNUSED,
            .din = this->s_config.pin_din,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false
            }
        }
    };

    i2s_new_channel(&i2s_chan_cfg, NULL, &this->rx_handle);
    i2s_channel_init_std_mode(this->rx_handle, &i2s_config);
    i2s_channel_enable(this->rx_handle);

    //printf("I2S channel successfully initialized and enabled\n");
};

Microphone::~Microphone() {
    if (this->processed_audio) heap_caps_free(this->processed_audio);
    
    if (this->rx_handle) {
        i2s_channel_disable(this->rx_handle);
        i2s_del_channel(this->rx_handle);
    }
    this->rx_handle = NULL;
};

int16_t* Microphone::readMicrophoneSamples() {
    size_t read_bytes = 0;
    i2s_channel_read(this->rx_handle, this->processed_audio, this->s_config.sample_count * sizeof(int16_t), &read_bytes, portMAX_DELAY);

    for (int i = 0; i < this->s_config.sample_count; i++) {
        this->processed_audio[i] -= this->s_config.dc_offset;
    }

    return this->processed_audio;
};