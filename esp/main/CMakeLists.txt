idf_component_register(
    SRCS "main.cpp"
    REQUIRES
        freertos
        esp_driver_i2s
        esp-tflite-micro
        esp-dsp
        ring_buffer_driver
        microphone_driver
        neural_net_driver
)