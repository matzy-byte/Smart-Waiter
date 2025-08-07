#include <ModelInference.h>

constexpr int kTensorArenaSize = 100 * 1024;
static uint8_t tensor_arena[kTensorArenaSize];

static const tflite::Model *model = nullptr;
static tflite::MicroInterpreter *interpreter = nullptr;
static TfLiteTensor *input = nullptr;
static TfLiteTensor *output = nullptr;

float hann_window[FRAME_LEN];
float fft_input[2 * FFT_SIZE];
float mag[FFT_SIZE / 2 + 1];

void setupModel() {
    model = tflite::GetModel(lite_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        Serial.printf("Model schema mismatch: %d vs %d\n", model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    static tflite::MicroMutableOpResolver<9> resolver;
    resolver.AddFullyConnected();
    resolver.AddConv2D();
    resolver.AddMaxPool2D();
    resolver.AddSoftmax();
    resolver.AddReshape();
    resolver.AddRelu();
    resolver.AddLogistic();

    static tflite::MicroErrorReporter micro_error_reporter;
    tflite::ErrorReporter *error_reporter = &micro_error_reporter;

    static tflite::MicroInterpreter static_interpreter(model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        Serial.println("Tensor allocation failed");
        return;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

    // For processing audio later on
    dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    dsps_wind_hann_f32(hann_window, FRAME_LEN);
    Serial.println("Model loaded and interpreter initialized.");
}

void preprocessAudioToSpectrogram(int16_t *audio, int audio_len, float *output_tensor_data) {
    for (int t = 0; t < SPECTROGRAM_W; t++) {
        int offset = t * FRAME_STEP;

        for (int i = 0; i < FRAME_LEN; i++) {
            float sample = (offset + i < audio_len) ? ((float)audio[offset + i] / 32768.0f) : 0.0f;
            sample *= hann_window[i];
            fft_input[2 * i] = sample;
            fft_input[2 * i + 1] = 0.0f; 
        }

        dsps_fft2r_fc32((float *)fft_input, FFT_SIZE);
        dsps_bit_rev_fc32((float *)fft_input, FFT_SIZE);

        for (int i = 0; i <= FFT_SIZE / 2; i++) {
            float real = fft_input[i * 2];
            float imag = fft_input[i * 2 + 1];
            mag[i] = sqrt(real * real + imag * imag);
        }

        for (int j = 0; j < SPECTROGRAM_H; j++) {
            float avg = 0.0f;
            int count = 0;

            for (int k = 0; k < POOL_SIZE; k++) {
                int bin = j * POOL_SIZE + k;
                if (bin >= (FFT_SIZE / 2 + 1)) break;
                avg += mag[bin];
                count++;
            }

            avg = (count > 0) ? (avg / count) : 0.0f;

            float log_val = log10f(avg + EPSILON);
            output_tensor_data[t * SPECTROGRAM_H + j] = log_val;
        }
    }
}

bool runInference(int16_t *samples) {
    preprocessAudioToSpectrogram(samples, INFERENCE_SAMPLE_COUNT, input->data.f);

    if (interpreter->Invoke() != kTfLiteOk) {
        Serial.println("Invoke failed");
        return false;
    }

    float prob_juan = output->data.f[0];
    prob_juan = std::min(1.0f, std::max(0.0f, prob_juan));
    Serial.println(prob_juan);

    if (prob_juan > 0.95f) {
        return true;
    }
    return false;
}