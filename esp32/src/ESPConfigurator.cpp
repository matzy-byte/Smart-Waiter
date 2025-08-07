#include <ESPConfigurator.h>

void configure_esp() {
    esp_pm_config_esp32s3_t pm_config = {
        .max_freq_mhz = 160,
        .min_freq_mhz = 60,
        .light_sleep_enable = true
    };
    esp_pm_configure(&pm_config);

    esp_wifi_stop();
    esp_wifi_deinit();
    esp_bt_controller_disable();
    esp_bt_mem_release(ESP_BT_MODE_BTDM);
}