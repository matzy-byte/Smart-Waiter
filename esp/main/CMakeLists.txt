idf_component_register(
    SRCS "main.cpp"
    REQUIRES
        "soc"
        "ulp"
        "esp_adc"
        "freertos"
        "littlefs"
        "esp_driver_gpio"
        "esp_driver_i2s"
        "esp-tflite-micro"
        "esp-dsp"
        "ring_buffer_driver"
        "microphone_driver"
        "neural_net_driver"
        "speaker_driver"
)

littlefs_create_partition_image(storage ../littlefs FLASH_IN_PROJECT)

set(ulp_app_name ulp_${COMPONENT_NAME})
set(ulp_riscv_sources "ulp/main.c")
set(ulp_exp_dep_srcs "main.cpp")
ulp_embed_binary("${ulp_app_name}" "${ulp_riscv_sources}" "${ulp_exp_dep_srcs}")
