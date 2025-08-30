idf_component_register(
    SRCS "main.cpp"
    REQUIRES
        "littlefs"
        "esp_driver_i2s"
        "esp-tflite-micro"
        "esp-dsp"
        "ring_buffer_driver"
        "microphone_driver"
        "neural_net_driver"
        "speaker_driver"
)

littlefs_create_partition_image(storage ../littlefs FLASH_IN_PROJECT)