#include <ring_buffer.h>
#include <microphone.h>
#include <neural_net.h>
#include <q_lite_model.h>

#define SAMPLE_RATE 16000
#define AUDIO_SLICES 8
#define MICROPHONE_DC_OFFSET -3440
#define MICROPHONE_GAIN 6.5f
#define MIN_DETECTIONS_REQUIRED 3
#define DATABUFFER_SAMPLE_COUNT ((SAMPLE_RATE * 3) / 2)
#define MICROPHONE_SAMPLE_COUNT (SAMPLE_RATE / AUDIO_SLICES)
#define INFERENCE_SAMPLE_COUNT (SAMPLE_RATE * 1)
#define SPEAKER_SAMPLE_COUNT (SAMPLE_RATE * 5)

RingBufferConfig_t ring_buffer_config = {
    .data_buffer_sample_count = DATABUFFER_SAMPLE_COUNT,
    .microphone_buffer_sample_count = MICROPHONE_SAMPLE_COUNT,
    .inference_buffer_sample_count = INFERENCE_SAMPLE_COUNT
};

MicrophoneConfig_t microphone_config = {
    .sample_rate = SAMPLE_RATE,
    .sample_count = MICROPHONE_SAMPLE_COUNT,
    .gain = MICROPHONE_GAIN,
    .dc_offset = MICROPHONE_DC_OFFSET,
    .port = I2S_NUM_0,
    .pin_bclk = (gpio_num_t) 14,
    .pin_ws = (gpio_num_t) 15,
    .pin_din = (gpio_num_t) 39
};

NeuralNetConfig_t neural_net_config = {
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

static Microphone microphone(microphone_config);
static RingBuffer ring_buffer(ring_buffer_config);
static NeuralNet neural_net(neural_net_config);
int detections = 0;

void fillBuffer() {
    for (int i = 0; i < AUDIO_SLICES - 1; i++) {
        ring_buffer.writeBuffer(microphone.readMicrophoneSamples());
    }
}

extern "C" void app_main(void) {
    fillBuffer();
    while (true) {
        ring_buffer.writeBuffer(microphone.readMicrophoneSamples());
        if (neural_net.runInference(ring_buffer.readBuffer())) {
            detections++;

            if (detections >= MIN_DETECTIONS_REQUIRED) {
                printf("\nPLAY MUSIC!\n");
                fillBuffer();
            }
        } else {
            detections = 0;
        }
    }
}