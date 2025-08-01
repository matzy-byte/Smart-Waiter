#include <Ringbuffer.h>

int32_t *rawMicrophoneSamples = nullptr;
int16_t *processedMicrophoneSamples = nullptr;
int16_t *buffer = nullptr;
int16_t *rBuffer = nullptr;
int indexBuffer = 0;

void setupRingbuffer() {
    rawMicrophoneSamples = (int32_t *)heap_caps_malloc(MICROPHONE_SAMPLE_COUNT * sizeof(int32_t), MALLOC_CAP_SPIRAM);
    processedMicrophoneSamples = (int16_t *)heap_caps_malloc(MICROPHONE_SAMPLE_COUNT * sizeof(int16_t), MALLOC_CAP_SPIRAM);
    buffer = (int16_t *)heap_caps_malloc(DATABUFFER_SAMPLE_COUNT * sizeof(int16_t), MALLOC_CAP_SPIRAM);
    rBuffer = (int16_t *)heap_caps_malloc(INFERENCE_SAMPLE_COUNT * sizeof(int16_t), MALLOC_CAP_SPIRAM);
}

void writeBuffer(int16_t *input) {
    memcpy(&buffer[indexBuffer], input, MICROPHONE_SAMPLE_COUNT * sizeof(int16_t));
    indexBuffer += MICROPHONE_SAMPLE_COUNT;
}

int16_t* readBuffer() {
    if (indexBuffer >= DATABUFFER_SAMPLE_COUNT) {
        memcpy(rBuffer, &buffer[indexBuffer - MICROPHONE_SAMPLE_COUNT], MICROPHONE_SAMPLE_COUNT * sizeof(int16_t));
        memcpy(rBuffer + MICROPHONE_SAMPLE_COUNT, buffer, (INFERENCE_SAMPLE_COUNT - MICROPHONE_SAMPLE_COUNT) * sizeof(int16_t));
        indexBuffer = 0;
    } else if (indexBuffer < INFERENCE_SAMPLE_COUNT) {
        memcpy(rBuffer, &buffer[DATABUFFER_SAMPLE_COUNT - (INFERENCE_SAMPLE_COUNT - indexBuffer)], (INFERENCE_SAMPLE_COUNT - indexBuffer) * sizeof(int16_t));
        memcpy(rBuffer + (INFERENCE_SAMPLE_COUNT - indexBuffer), buffer, indexBuffer * sizeof(int16_t));
    } else {
        memcpy(rBuffer, &buffer[indexBuffer - INFERENCE_SAMPLE_COUNT], INFERENCE_SAMPLE_COUNT * sizeof(int16_t));
    }
    return rBuffer;
}