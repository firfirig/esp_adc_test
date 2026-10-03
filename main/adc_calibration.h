#pragma once

#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

// Filtre için varsayılan örnekleme sayısı (Anlık gürültüyü yok eder)
#define ADC_FILTER_SAMPLE_COUNT     16

// ESP32 Line Fitting sapmasını sıfırlamak için 2-Noktalı Düzeltme Anahtarı
#define ENABLE_ESP32_CUSTOM_OFFSET_CORRECTION  1

typedef struct {
    adc_oneshot_unit_handle_t adc_handle;
    adc_cali_handle_t cali_handle;
    adc_channel_t channel;
    bool calibrated;
    uint32_t sample_count;  // Filtre örnek sayısı
} adc_calibration_t;

/**
 * @brief ADC Birimini ve Kalibrasyon Yapısını Başlatır
 */
esp_err_t adc_calibration_init(adc_calibration_t *ctx);

/**
 * @brief Filtrelenmiş ve Kalibre Edilmiş ADC Değerini Oku
 * @note main.c içerisinden yapılan mevcut çağrıyı değiştirmeden doğrudan filtreli çalışır.
 */
esp_err_t adc_calibration_read(adc_calibration_t *ctx, int *raw, int *voltage_mv);

/**
 * @brief ADC Sürücülerini ve Kalibrasyonu Temizler
 */
void adc_calibration_deinit(adc_calibration_t *ctx);