file(GLOB src_files "src/*.cpp")

idf_component_register(
    SRCS "${src_files}"
    INCLUDE_DIRS "include"
    REQUIRES
        "esp_driver_i2s"
)