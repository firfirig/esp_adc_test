#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include "esp_idf_version.h"
#include "adc_calibration.h"

static const char *TAG = "ESP_ADC_TEST";

#if CONFIG_IDF_TARGET_ESP32
#define TEST_GPIO "GPIO34"
#elif CONFIG_IDF_TARGET_ESP32S3
#define TEST_GPIO "GPIO4"
#endif

void app_main(void)
{
    adc_calibration_t adc;

    ESP_ERROR_CHECK(adc_calibration_init(&adc));

    esp_chip_info_t chip;
    esp_chip_info(&chip);

    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, "       ESP-IDF ADC KALIBRASYON TESTI");
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, "IDF: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Test pini: %s", TEST_GPIO);
#if CONFIG_IDF_TARGET_ESP32
    ESP_LOGI(TAG, "ESP32: ADC1_CH6 + Line Fitting");
#elif CONFIG_IDF_TARGET_ESP32S3
    ESP_LOGI(TAG, "ESP32-S3: ADC1_CH3 + Curve Fitting");
#endif
    ESP_LOGI(TAG, "Kalibrasyon: %s", adc.calibrated ? "AKTIF" : "YOK");
    ESP_LOGI(TAG, "----------------------------------------------");

    while (1) {
        int raw = 0;
        int voltage_mv = 0;

        if (adc_calibration_read(&adc, &raw, &voltage_mv) == ESP_OK) {
            ESP_LOGI(TAG,
                     "RAW=%4d | ADC=%4d mV | %.3f V",
                     raw, voltage_mv, voltage_mv / 1000.0f);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
