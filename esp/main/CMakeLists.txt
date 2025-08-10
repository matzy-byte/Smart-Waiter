idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "include"
    REQUIRES
        ring_buffer_driver
        microphone_driver
        neural_net_driver
        speaker_driver
        esp-tflite-micro
        esp-dsp
)