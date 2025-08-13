#ifndef NEURAL_NET_H
#define NEURAL_NET_H

#include <stdint.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>

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
} NeuralNetConfig_t;

class NeuralNet {
    private:
        NeuralNetConfig_t s_config;

        const tflite::Model* model;
        tflite::MicroMutableOpResolver<9> resolver;
        tflite::MicroInterpreter* interpreter;
        TfLiteTensor* input;
        TfLiteTensor* output;

        uint8_t* tensor_arena;
        float* hann_window;
        float* fft_input;
        float* mag;

        void preprocessAudio(int16_t* samples, float* output_tensor_data);

    public:
        NeuralNet(const NeuralNetConfig_t& config);
        ~NeuralNet();

        bool runInference(int16_t* samples);
};

#endif