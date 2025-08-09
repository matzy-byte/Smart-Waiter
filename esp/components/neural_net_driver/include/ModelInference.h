#ifndef MODEL_INFERENCE_H
#define MODEL_INFERENCE_H

#include <stdint.h>
#include <micro_mutable_op_resolver.h>
#include <micro_interpreter.h>
#include <micro_error_reporter.h>
#include <schema_generated.h>

typedef struct {
    const uint8_t* model_data;
    size_t tensor_arena_size;
    int frame_len;
    int frame_step;
    int fft_size;
    int pool_size;
    float epsilon;
    int spectrogram_w;
    int spectrogram_h;
    int audio_len;
} ModelInferenceConfig_t;

class ModelInference {
    private:
        ModelInferenceConfig_t s_config;

        tflite::MicroErrorReporter error_reporter;
        tflite::MicroMutableOpResolver<9> op_resolver;
        const tflite::Model* model;
        tflite::MicroInterpreter* interpreter;
        TfLiteTensor* input;
        TfLiteTensor* output;

        uint8_t* tensor_arena;
        float* hann_window;
        float* fft_input;
        float* mag;
    
    public:
        ModelInference(const ModelInferenceConfig_t& config);
        ~ModelInference();

        bool runInference(int16_t* samples);
    
    private:
        void preprocessAudioToSpectrogram(int16_t* audio, int audio_len, float* output_tensor_data);
};

#endif