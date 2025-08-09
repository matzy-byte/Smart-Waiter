#include <Microphone.h>
#include <RingBuffer.h>
#include <Speaker.h>
#include <ModelInference.h>
#include <QLiteModel.h>

#define SAMPLE_RATE 16000
#define AUDIO_SLICES 8
#define MICROPHONE_DC_OFFSET 3430
#define MICROPHONE_GAIN 5.6234f
#define DATABUFFER_SAMPLE_COUNT ((SAMPLE_RATE * 3) / 2)
#define MICROPHONE_SAMPLE_COUNT (SAMPLE_RATE / AUDIO_SLICES)
#define INFERENCE_SAMPLE_COUNT (SAMPLE_RATE * 1)
#define SPEAKER_SAMPLE_COUNT (SAMPLE_RATE * 5)

MicrophoneConfig_t mic_config = {
    .i2s_port = I2S_NUM_0,
    .pin_ws = (gpio_num_t)15,
    .pin_sck = (gpio_num_t)14,
    .pin_SD = (gpio_num_t)39,
    .sample_rate = SAMPLE_RATE,
    .sample_count = MICROPHONE_SAMPLE_COUNT,
    .dc_offset = MICROPHONE_DC_OFFSET,
    .gain = MICROPHONE_GAIN
};

SpeakerConfig_t spk_config = {
    .i2s_port = I2S_NUM_0,
    .pin_ws = (gpio_num_t)13,
    .pin_sck = (gpio_num_t)12,
    .pin_SD = (gpio_num_t)21,
    .sample_rate = SAMPLE_RATE,
    .sample_count = SPEAKER_SAMPLE_COUNT
};

ModelInferenceConfig_t model_config = {
    .model_data = q_lite_model,
    .tensor_arena_size = 100 * 1024,
    .frame_len = 256,
    .frame_step = 128,
    .fft_size = 256,
    .pool_size = 6,
    .epsilon = 1e-6f,
    .spectrogram_w = 124,
    .spectrogram_h = 22,
    .audio_len = INFERENCE_SAMPLE_COUNT
};

RingBuffer ring_buffer(MICROPHONE_SAMPLE_COUNT, DATABUFFER_SAMPLE_COUNT, INFERENCE_SAMPLE_COUNT);
Microphone microphone(mic_config);
Speaker speaker(spk_config);
ModelInference model(model_config);

void inference_task(void *pvParameters) {
    
}

extern "C" void app_main() {
    xTaskCreate(
        inference_task,
        "InferenceTask",
        4096,
        NULL,
        5,
        NULL
    );
};