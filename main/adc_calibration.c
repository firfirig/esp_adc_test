#include "adc_calibration.h"

#include "esp_log.h"
#include "esp_check.h"
#include "esp_idf_version.h"
#include "soc/soc_caps.h"

static const char *TAG = "ADC_CAL";

#if CONFIG_IDF_TARGET_ESP32
#define ADC_UNIT_USED       ADC_UNIT_1
#define ADC_CHANNEL_USED    ADC_CHANNEL_6   // GPIO34 = ADC1_CH6
#define ADC_ATTEN_USED      ADC_ATTEN_DB_12
#elif CONFIG_IDF_TARGET_ESP32S3
#define ADC_UNIT_USED       ADC_UNIT_1
#define ADC_CHANNEL_USED    ADC_CHANNEL_3   // GPIO4 = ADC1_CH3
#define ADC_ATTEN_USED      ADC_ATTEN_DB_12
#else
#error "Bu proje yalnızca ESP32 veya ESP32-S3 hedefleri için hazırlanmıştır."
#endif

static esp_err_t create_calibration(adc_calibration_t *ctx)
{
#if CONFIG_IDF_TARGET_ESP32
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_USED,
        .atten = ADC_ATTEN_USED,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .default_vref = 1100, // eFuse boşsa varsayılan 1100mV referansı kullanır
    };

    esp_err_t ret = adc_cali_create_scheme_line_fitting(
        &cali_config, &ctx->cali_handle);

    if (ret == ESP_OK) {
        ctx->calibrated = true;
        ESP_LOGI(TAG, "ESP32 Line Fitting kalibrasyonu aktif.");
    } else {
        ESP_LOGW(TAG, "Line Fitting kalibrasyonu kullanilamadi (%s). Varsayılan ölçekleme kullanılacak.",
                 esp_err_to_name(ret));
        ctx->calibrated = false;
    }
    return ESP_OK;

#elif CONFIG_IDF_TARGET_ESP32S3
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_USED,
        .chan = ADC_CHANNEL_USED,
        .atten = ADC_ATTEN_USED,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    esp_err_t ret = adc_cali_create_scheme_curve_fitting(
        &cali_config, &ctx->cali_handle);

    if (ret == ESP_OK) {
        ctx->calibrated = true;
        ESP_LOGI(TAG, "ESP32-S3 Curve Fitting kalibrasyonu aktif.");
    } else {
        ESP_LOGW(TAG, "Curve Fitting kalibrasyonu kullanilamadi (%s). Varsayılan ölçekleme kullanılacak.",
                 esp_err_to_name(ret));
        ctx->calibrated = false;
    }
    return ESP_OK;
#endif
}

esp_err_t adc_calibration_init(adc_calibration_t *ctx)
{
    if (!ctx) return ESP_ERR_INVALID_ARG;

    *ctx = (adc_calibration_t){0};
    ctx->channel = ADC_CHANNEL_USED;
    ctx->sample_count = ADC_FILTER_SAMPLE_COUNT; // 16 örnekli ortalama filtresi

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_USED,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_RETURN_ON_ERROR(
        adc_oneshot_new_unit(&init_config, &ctx->adc_handle),
        TAG, "ADC unit olusturulamadi");

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_USED,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_RETURN_ON_ERROR(
        adc_oneshot_config_channel(ctx->adc_handle,
                                    ADC_CHANNEL_USED,
                                    &chan_config),
        TAG, "ADC channel ayarlanamadi");

    return create_calibration(ctx);
}

esp_err_t adc_calibration_read(adc_calibration_t *ctx, int *raw, int *voltage_mv)
{
    if (!ctx || !raw || !voltage_mv) return ESP_ERR_INVALID_ARG;

    uint32_t raw_sum = 0;
    int temp_raw = 0;
    uint32_t samples = (ctx->sample_count > 0) ? ctx->sample_count : 1;

    // 1. Gürültü Filtresi (Oversampling / Moving Average)
    for (uint32_t i = 0; i < samples; i++) {
        ESP_RETURN_ON_ERROR(
            adc_oneshot_read(ctx->adc_handle, ctx->channel, &temp_raw),
            TAG, "ADC okuma hatasi");
        raw_sum += temp_raw;
    }

    *raw = (int)(raw_sum / samples); // Ortalaması alınmış ham değer

    // 2. Kalibrasyon Dönüşümü
    if (ctx->calibrated) {
        ESP_RETURN_ON_ERROR(
            adc_cali_raw_to_voltage(ctx->cali_handle, *raw, voltage_mv),
            TAG, "Kalibrasyon donusum hatasi");
    } else {
        // eFuse yoksa donanımsal oranlama (yaklaşık)
        *voltage_mv = (*raw * 3300) / 4095;
    }

    // 3. ESP32 DevKitC v4 Özel 2-Noktalı Sapma Düzeltmesi (Sadece ESP32 Target için)
#if CONFIG_IDF_TARGET_ESP32 && ENABLE_ESP32_CUSTOM_OFFSET_CORRECTION
    if (ctx->calibrated && *voltage_mv > 0) {
        // Testlerinizde tespit edilen +40mV lineer ofset kaymasını sıfırlar:
        // V_gercek = (V_ESP * 0.985) - 15 mV
        int corrected_mv = (int)((*voltage_mv * 0.985f) - 15.0f);
        *voltage_mv = (corrected_mv < 0) ? 0 : corrected_mv;
    }
#endif

    return ESP_OK;
}

void adc_calibration_deinit(adc_calibration_t *ctx)
{
    if (!ctx) return;

    if (ctx->calibrated) {
#if CONFIG_IDF_TARGET_ESP32
        adc_cali_delete_scheme_line_fitting(ctx->cali_handle);
#elif CONFIG_IDF_TARGET_ESP32S3
        adc_cali_delete_scheme_curve_fitting(ctx->cali_handle);
#endif
    }

    if (ctx->adc_handle) {
        adc_oneshot_del_unit(ctx->adc_handle);
    }
}