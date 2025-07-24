#include <ModelInference.h>

constexpr int kTensorArenaSize = 70 * 1024;
static uint8_t tensor_arena[kTensorArenaSize];

static const tflite::Model *model = nullptr;
static tflite::MicroInterpreter *interpreter = nullptr;
static TfLiteTensor *input = nullptr;
static TfLiteTensor *output = nullptr;

const float input_scale = 0.0300050f;
const int input_zero_point = 72;
const float output_scale = 0.0039063f;
const int output_zero_point = -128;

float hann_window[FRAME_LEN];
float fft_input[2 * FFT_SIZE];
float fft_output[FFT_SIZE];
float mag[FFT_SIZE / 2];

void setupModel() {
    model = tflite::GetModel(detection_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        Serial.printf("Model schema mismatch: %d vs %d\n", model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    static tflite::MicroMutableOpResolver<7> resolver;
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

void preprocessAudioToSpectrogram(const int16_t *audio, int audio_len, int8_t *output_tensor_data) {
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
        dsps_cplx2reC_fc32((float *)fft_input, FFT_SIZE);

        for (int i = 0; i < FFT_SIZE / 2; i++) {
            float real = fft_input[i * 2];
            float imag = fft_input[i * 2 + 1];
            mag[i] = sqrt(real * real + imag * imag);
        }

        for (int j = 0; j < SPECTROGRAM_H; j++) {
            float avg = 0.0f;
            for (int k = 0; k < POOL_SIZE; k++) {
                int bin = j * POOL_SIZE + k;
                if (bin < FFT_SIZE / 2) {
                    avg += mag[bin];
                }
            }
            avg /= POOL_SIZE;

            float log_val = log10f(avg + EPSILON);

            int8_t q = static_cast<int8_t>(round((log_val / input_scale) + input_zero_point));
            q = std::max((int8_t)-128, std::min((int8_t)127, q));

            output_tensor_data[t * SPECTROGRAM_H + j] = q;
        }
    }
}

bool runInference(int16_t *samples) {
    preprocessAudioToSpectrogram(samples, SAMPLE_RATE, input->data.int8);
    
    if (interpreter->Invoke() != kTfLiteOk) {
        Serial.println("Invoke failed");
        return false;
    }

    int8_t q = output->data.int8[0];
    float prob_juan = (q - output_zero_point) * output_scale;
    prob_juan = std::min(1.0f, std::max(0.0f, prob_juan));

    Serial.print("Prediction (juan): ");
    Serial.print(prob_juan * 100.0f);
    Serial.println(" %");

    Serial.print("Prediction (none): ");
    Serial.print((1.0f - prob_juan) * 100.0f);
    Serial.println(" %");

    return prob_juan > 0.8f;
}