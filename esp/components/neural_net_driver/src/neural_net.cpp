#include <neural_net.h>

#include <esp_heap_caps.h>
#include <esp_dsp.h>
#include <math.h>

NeuralNet::NeuralNet(const NeuralNetConfig_t& config) {
    this->s_config = config;
    this->tensor_arena = (uint8_t*) heap_caps_malloc(this->s_config.tensor_arena_size * sizeof(uint8_t), MALLOC_CAP_INTERNAL);

    this->model = tflite::GetModel(this->s_config.model_data);

    this->resolver.AddFullyConnected();
    this->resolver.AddConv2D();
    this->resolver.AddMaxPool2D();
    this->resolver.AddSoftmax();
    this->resolver.AddReshape();
    this->resolver.AddRelu();
    this->resolver.AddLogistic();
    this->resolver.AddQuantize();
    this->resolver.AddDequantize();

    static tflite::MicroInterpreter static_interpreter(model, resolver, this->tensor_arena, this->s_config.tensor_arena_size, nullptr, nullptr);
    this->interpreter = &static_interpreter;
    if (this->interpreter->AllocateTensors() != kTfLiteOk) {
        printf("Cannot allocate tensors");
        return;
    }

    this->input = this->interpreter->input(0);
    this->output = this->interpreter->output(0);

    this->hann_window = (float*)heap_caps_malloc(this->s_config.frame_len * sizeof(float), MALLOC_CAP_INTERNAL);
    this->fft_input = (float*)heap_caps_malloc(2 * this->s_config.fft_size * sizeof(float), MALLOC_CAP_INTERNAL);
    this->mag = (float*)heap_caps_malloc((this->s_config.fft_size / 2 + 1) * sizeof(float), MALLOC_CAP_INTERNAL);
    dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    dsps_wind_hann_f32(this->hann_window, this->s_config.frame_len);
};

NeuralNet::~NeuralNet() {
    if (this->interpreter) delete this->interpreter;
    if (this->tensor_arena) heap_caps_free(this->tensor_arena);
    if (this->hann_window) heap_caps_free(this->hann_window);
    if (this->fft_input) heap_caps_free(this->fft_input);
    if (this->mag) heap_caps_free(this->mag);
};

void NeuralNet::preprocessAudio(int16_t* samples, float* output_tensor_data) {
    float min_val = std::numeric_limits<float>::max();
    float max_val = std::numeric_limits<float>::lowest();

    for (int t = 0; t < this->s_config.spectrogram_w; t++) {
        int offset = t * this->s_config.frame_step;

        for (int i = 0; i < this->s_config.frame_len; i++) {
            float sample = (offset + i < this->s_config.audio_len) ? ((float) samples[offset + i] / 32768.0f) : 0.0f;
            sample *= this->hann_window[i];
            this->fft_input[2 * i] = sample;
            this->fft_input[2 * i + 1] = 0.0f; 
        }

        dsps_fft2r_fc32((float*) this->fft_input, this->s_config.fft_size);
        dsps_bit_rev_fc32((float*) this->fft_input, this->s_config.fft_size);

        for (int i = 0; i <= this->s_config.fft_size / 2; i++) {
            float real = this->fft_input[i * 2];
            float imag = this->fft_input[i * 2 + 1];
            this->mag[i] = sqrt(real * real + imag * imag);
        }

        for (int j = 0; j < this->s_config.spectrogram_h; j++) {
            float avg = 0.0f;
            int count = 0;
            
            for (int k = 0; k < this->s_config.pool_size; k++) {
                int bin = j * this->s_config.pool_size + k;
                if (bin >= (this->s_config.fft_size / 2 + 1)) break;
                avg += this->mag[bin];
                count++;
            }
            
            avg = (count > 0) ? (avg / count) : 0.0f;
            
            float log_val = log10f(avg + this->s_config.epsilon);
            
            output_tensor_data[t * this->s_config.spectrogram_h + j] = log_val;
            
            if (log_val < min_val) min_val = log_val;
            if (log_val > max_val) max_val = log_val;
        }
    }

    float range = max_val - min_val;
    if (range < this->s_config.epsilon) range = this->s_config.epsilon;

    for (int idx = 0; idx < this->s_config.spectrogram_w * this->s_config.spectrogram_h; idx++) {
        output_tensor_data[idx] = (output_tensor_data[idx] - min_val) / range;
    }
};

bool NeuralNet::runInference(int16_t* samples) {
    this->preprocessAudio(samples, this->input->data.f);

    if (this->interpreter->Invoke() != kTfLiteOk) {
        printf("NO INVOKATION");
        return false;
    }

    float prob_juan = this->output->data.f[0];
    printf("%f\n", prob_juan);

    if (prob_juan > 0.98f) {
        return true;
    }
    return false;
}