#include <esp_sleep.h>
#include <ulp_riscv.h>
#include <ulp_adc.h>
#include <ulp_main.h>
#include <esp_adc/adc_continuous.h>

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");
static void init_ulp_program(void) {
    ulp_adc_cfg_t cfg = {
        .adc_n    = ADC_UNIT_1,
        .channel  = ADC_CHANNEL_0,
        .atten    = ADC_ATTEN_DB_12,
        .width    = ADC_BITWIDTH_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_RISCV,
    };

    ESP_ERROR_CHECK(ulp_adc_init(&cfg));

    esp_err_t err = ulp_riscv_load_binary(ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start));
    ESP_ERROR_CHECK(err);

    ulp_set_wakeup_period(0, 20000);

    err = ulp_riscv_run();
    ESP_ERROR_CHECK(err);
}

#include <ring_buffer.h>
#include <microphone.h>
#include <neural_net.h>
#include <q_lite_model.h>
#include <speaker.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
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

static const SpeakerConfig_t speaker_config = {
    .sample_rate = SAMPLE_RATE,
    .sample_count = SPEAKER_SAMPLE_COUNT,
    .port = I2S_NUM_1,
    .pin_bclk = (gpio_num_t) 12,
    .pin_ws = (gpio_num_t) 13,
    .pin_dout = (gpio_num_t) 21
};

void fillBuffer(Microphone& microphone, RingBuffer& ring_buffer) {
    for (int i = 0; i < AUDIO_SLICES - 1; i++) {
        ring_buffer.writeBuffer(microphone.readMicrophoneSamples());
        vTaskDelay(1);
    }
}

void detection_task(void* args) {
    NeuralNet *neural_net = new NeuralNet(neural_net_config);
    RingBuffer *ring_buffer = new RingBuffer(ring_buffer_config);
    Microphone *microphone = new Microphone(microphone_config);
    Speaker *speaker = new Speaker(speaker_config);

    if (!neural_net || !ring_buffer || !microphone || !speaker) {
        printf("Failed to allocate one or more objects!\n");
        vTaskDelete(NULL);
        return;
    }
    
    fillBuffer(*microphone, *ring_buffer);

    while (true) {
        ring_buffer->writeBuffer(microphone->readMicrophoneSamples());

        if (neural_net->runInference(ring_buffer->readBuffer())) {
            speaker->playMelody();
            ring_buffer->reset();
            esp_sleep_enable_ulp_wakeup();
            esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
            esp_deep_sleep_start();
        }

        vTaskDelay(1);
    }

    vTaskDelete(NULL);
}

void sleep_task(void* args) {
    vTaskDelay(pdMS_TO_TICKS(30000));
    esp_sleep_enable_ulp_wakeup();
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_deep_sleep_start();

    vTaskDelete(NULL);
}

extern "C" void app_main(void) {
    esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
    
    if (wakeup_cause == ESP_SLEEP_WAKEUP_ULP) {
        esp_vfs_littlefs_conf_t conf = {
            .base_path = "/littlefs",
            .partition_label = "storage",
        };
        esp_err_t ret = esp_vfs_littlefs_register(&conf);
        if (ret != ESP_OK) {
            printf("Failed to mount LittleFS (%s)\n", esp_err_to_name(ret));
            return;
        }
        
        xTaskCreate(detection_task, "DetectionTask", 8192, NULL, 5, NULL);
        xTaskCreate(sleep_task, "SleepTask", 2048, NULL, 1, NULL);
    } else {
        init_ulp_program();
        esp_sleep_enable_ulp_wakeup();
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
        esp_deep_sleep_start();
    }
    return;
}