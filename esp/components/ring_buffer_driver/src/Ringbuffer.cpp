#include <Ringbuffer.h>

#include <esp_heap_caps.h>
#include <string.h>

RingBuffer::RingBuffer(const RingBufferConfig_t& config) {
    this->s_config = config;
    this->index_buffer = 0;

    this->data_buffer = (int16_t *)heap_caps_malloc(this->s_config.databuffer_sample_count * sizeof(int16_t), MALLOC_CAP_SPIRAM);
    this->read_buffer = (int16_t *)heap_caps_malloc(this->s_config.inference_sample_count * sizeof(int16_t), MALLOC_CAP_SPIRAM);
    //this->mutex = xSemaphoreCreateMutex();
}

RingBuffer::~RingBuffer() {
    if (this->data_buffer) heap_caps_free(this->data_buffer);
    if (this->read_buffer) heap_caps_free(this->read_buffer);
    //if (this->mutex) vSemaphoreDelete(this->mutex);
}

void RingBuffer::write(const int16_t* input) {
    //if (xSemaphoreTake(this->mutex, portMAX_DELAY) != pdTRUE) return;
    
    size_t samples_to_copy = this->s_config.microphone_sample_count;
    size_t current_index = this->index_buffer;
    size_t total_size = this->s_config.databuffer_sample_count;

    if (current_index + samples_to_copy > total_size) {
        size_t first_part_len = total_size - current_index;
        memcpy(&this->data_buffer[current_index], input, first_part_len * sizeof(int16_t));
        memcpy(this->data_buffer, &input[first_part_len], (samples_to_copy - first_part_len) * sizeof(int16_t));
        this->index_buffer = samples_to_copy - first_part_len;
    } else {
        memcpy(&this->data_buffer[current_index], input, samples_to_copy * sizeof(int16_t));
        this->index_buffer += samples_to_copy;
    }
    //xSemaphoreGive(this->mutex);
}

int16_t* RingBuffer::read() {
    //if (xSemaphoreTake(this->mutex, portMAX_DELAY) != pdTRUE) return;
    int16_t* output_buffer = nullptr;

    size_t samples_to_read = this->s_config.inference_sample_count;
    size_t total_size = this->s_config.databuffer_sample_count;

    if (this->index_buffer >= samples_to_read) {
        size_t read_start_index = this->index_buffer - samples_to_read;
        memcpy(this->read_buffer, &this->data_buffer[read_start_index], samples_to_read * sizeof(int16_t));
    } else {
        size_t first_part_len = samples_to_read - this->index_buffer;
        size_t read_start_index = total_size - first_part_len;
        
        memcpy(this->read_buffer, &this->data_buffer[read_start_index], first_part_len * sizeof(int16_t));
        memcpy(&this->read_buffer[first_part_len], this->data_buffer, this->index_buffer * sizeof(int16_t));
    }
    output_buffer = this->read_buffer;
    //xSemaphoreGive(this->mutex);
    return output_buffer;
}
