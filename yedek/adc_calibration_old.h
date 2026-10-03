#pragma once

#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"

typedef struct {
    adc_oneshot_unit_handle_t adc_handle;
    adc_cali_handle_t cali_handle;
    adc_channel_t channel;
    bool calibrated;
} adc_calibration_t;

esp_err_t adc_calibration_init(adc_calibration_t *ctx);
esp_err_t adc_calibration_read(adc_calibration_t *ctx, int *raw, int *voltage_mv);
void adc_calibration_deinit(adc_calibration_t *ctx);
