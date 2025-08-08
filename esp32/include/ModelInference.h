#ifndef MODEL_INFERENCE_H
#define MODEL_INFERENCE_H

#include <Global.h>
#include <Arduino.h>
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/micro/system_setup.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <QLiteModel.h>
#include <esp_dsp.h>
#include <math.h>

#define FRAME_LEN 256
#define FRAME_STEP 128
#define FFT_SIZE 256
#define POOL_SIZE 6
#define EPSILON 1e-6f
#define SPECTROGRAM_W 124
#define SPECTROGRAM_H 22

void setupModel();
void preprocessAudioToSpectrogram(int16_t* audio, int audio_len, float* output_tensor_data);
bool runInference(int16_t* samples);

#endif