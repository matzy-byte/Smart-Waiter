#include <ring_buffer.h>

#include <esp_heap_caps.h>
#include <string.h>

RingBuffer::RingBuffer(const RingBufferConfig_t& config) {
    this->s_config = config;
    this->index = 0;
    this->data_buffer = (int16_t*) heap_caps_malloc(this->s_config.data_buffer_sample_count * sizeof(int16_t), MALLOC_CAP_INTERNAL);
    this->read_buffer = (int16_t*) heap_caps_malloc(this->s_config.inference_buffer_sample_count * sizeof(int16_t), MALLOC_CAP_INTERNAL);
};

RingBuffer::~RingBuffer() {
    if (this->data_buffer) heap_caps_free(this->data_buffer);
    if (this->read_buffer) heap_caps_free(this->read_buffer);
};

void RingBuffer::writeBuffer(int16_t* input) {
    size_t new_index = (this->index + this->s_config.microphone_buffer_sample_count) % this->s_config.data_buffer_sample_count;

    if (new_index < this->index) {
        size_t first_part = (this->s_config.data_buffer_sample_count - this->index);
        memcpy(this->data_buffer + this->index, input, first_part * sizeof(int16_t));
        memcpy(this->data_buffer, input + first_part, new_index * sizeof(int16_t));
        index = new_index;
        return;
    }

    memcpy(this->data_buffer + this->index, input, s_config.microphone_buffer_sample_count * sizeof(int16_t));
    index = new_index;
}

int16_t* RingBuffer::readBuffer() {
    size_t read_start_index = (this->index - this->s_config.inference_buffer_sample_count + this->s_config.data_buffer_sample_count) % this->s_config.data_buffer_sample_count;

    if (read_start_index + this->s_config.inference_buffer_sample_count > this->s_config.data_buffer_sample_count) {
        size_t first_part = this->s_config.data_buffer_sample_count - read_start_index;
        size_t second_part = this->s_config.inference_buffer_sample_count - first_part;
        memcpy(this->read_buffer, this->data_buffer + read_start_index, first_part * sizeof(int16_t));
        memcpy(this->read_buffer + first_part, this->data_buffer, second_part * sizeof(int16_t));
    } else {
        memcpy(this->read_buffer, this->data_buffer + read_start_index, this->s_config.inference_buffer_sample_count * sizeof(int16_t));
    }

    return this->read_buffer;
}
