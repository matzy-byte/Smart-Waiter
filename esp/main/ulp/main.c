#include <ulp_riscv_utils.h>
#include <ulp_riscv_adc_ulp_core.h>

uint32_t adc_threshold = 2200;
int32_t wakeup_result;

int main(void) {
    int32_t last_result = ulp_riscv_adc_read_channel(ADC_UNIT_1, ADC_CHANNEL_0);
    
    if (last_result >= adc_threshold) {
        wakeup_result = last_result;
        ulp_riscv_wakeup_main_processor();
    }
    return 0;
}