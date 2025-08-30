#include <ring_buffer.h>
#include <microphone.h>
#include <neural_net.h>
#include <q_lite_model.h>
#include <speaker.h>

#include <esp_littlefs.h>

#define SAMPLE_RATE 16000
#define AUDIO_SLICES 8
#define MICROPHONE_DC_OFFSET -3440
#define MICROPHONE_GAIN 6.5f
#define DATABUFFER_SAMPLE_COUNT ((SAMPLE_RATE * 3) / 2)
#define MICROPHONE_SAMPLE_COUNT (SAMPLE_RATE / AUDIO_SLICES)
#define INFERENCE_SAMPLE_COUNT (SAMPLE_RATE * 1)
#define SPEAKER_SAMPLE_COUNT (SAMPLE_RATE * 5)

static const RingBufferConfig_t ring_buffer_config = {
    .microphone_buffer_sample_count = MICROPHONE_SAMPLE_COUNT,
    .inference_buffer_sample_count = INFERENCE_SAMPLE_COUNT
};

static const MicrophoneConfig_t microphone_config = {
    .sample_rate = SAMPLE_RATE,
    .sample_count = MICROPHONE_SAMPLE_COUNT,
    .gain = MICROPHONE_GAIN,
    .dc_offset = MICROPHONE_DC_OFFSET,
    .port = I2S_NUM_0,
    .pin_bclk = (gpio_num_t) 14,
    .pin_ws = (gpio_num_t) 15,
    .pin_din = (gpio_num_t) 39
};

static const NeuralNetConfig_t neural_net_config = {
    .model_data = q_lite_model,
    .tensor_arena_size = 100480,
    .frame_len = 256,
    .frame_step = 128,
    .fft_size = 256,
    .pool_size = 6,
    .epsilon = 1e-6f,
    .spectrogram_w = 124,
    .spectrogram_h = 22,
    .audio_len = INFERENCE_SAMPLE_COUNT
};

static const SpeakerConfig_t speaker_config = {
    .sample_rate = SAMPLE_RATE,
    .sample_count = SPEAKER_SAMPLE_COUNT,
    .port = I2S_NUM_1,
    .pin_bclk = (gpio_num_t) 12,
    .pin_ws = (gpio_num_t) 13,
    .pin_dout = (gpio_num_t) 21
};

void fillBuffer(Microphone& microphone, RingBuffer& ring_buffer) {
    for (int i = 0; i < 3 * AUDIO_SLICES; i++) {
        ring_buffer.writeBuffer(microphone.readMicrophoneSamples());
    }
}

extern "C" void app_main(void) {
    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/littlefs",
        .partition_label = "storage",
    };

    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK) {
        printf("Failed to mount LittleFS (%s)\n", esp_err_to_name(ret));
        return;
    }
    printf("LittleFS mounted successfully\n");

    NeuralNet *neural_net = new NeuralNet(neural_net_config);
    RingBuffer *ring_buffer = new RingBuffer(ring_buffer_config);
    Microphone *microphone = new Microphone(microphone_config);
    Speaker *speaker = new Speaker(speaker_config);

    if (!neural_net || !ring_buffer || !microphone || !speaker) {
        printf("Failed to allocate one or more objects!\n");
        return;
    }

    fillBuffer(*microphone, *ring_buffer);
    while (true) {
        ring_buffer->writeBuffer(microphone->readMicrophoneSamples());
        if (neural_net->runInference(ring_buffer->readBuffer())) {
            //printf("\nPLAY MUSIC!\n");
            speaker->playMelody();
            ring_buffer->reset();
            fillBuffer(*microphone, *ring_buffer);
        }
    }
}