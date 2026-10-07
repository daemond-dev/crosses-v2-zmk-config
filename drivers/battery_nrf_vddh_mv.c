/*
 * Diagnostic copy of ZMK's battery_nrf_vddh.c. Same VDDHDIV5 measurement, but the state of
 * charge channel carries the voltage: level = (mV - 2500) / 20, clamped to 1..100, so
 * mV = 2500 + 20 * level. 1 means 2520 mV or less, 100 means 4500 mV or more (USB). 0 is never
 * sent, so a 0 on the host still means no reading or a split disconnect.
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_battery_nrf_vddh_mv

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define VDDHDIV (5)

static const struct device *adc = DEVICE_DT_GET(DT_NODELABEL(adc));

struct vddh_mv_data {
    struct adc_channel_cfg acc;
    struct adc_sequence as;
    uint16_t adc_raw;
    uint16_t millivolts;
    uint8_t code;
};

static uint8_t mv_to_code(int mv) {
    int code = (mv - 2500) / 20;

    if (code < 1) {
        return 1;
    }
    if (code > 100) {
        return 100;
    }
    return code;
}

static int vddh_mv_sample_fetch(const struct device *dev, enum sensor_channel chan) {
    if (chan != SENSOR_CHAN_GAUGE_VOLTAGE && chan != SENSOR_CHAN_GAUGE_STATE_OF_CHARGE &&
        chan != SENSOR_CHAN_ALL) {
        return -ENOTSUP;
    }

    struct vddh_mv_data *drv_data = dev->data;
    struct adc_sequence *as = &drv_data->as;

    int rc = adc_read(adc, as);
    as->calibrate = false;

    if (rc != 0) {
        LOG_ERR("Failed to read ADC: %d", rc);
        return rc;
    }

    int32_t val = drv_data->adc_raw;
    rc = adc_raw_to_millivolts(adc_ref_internal(adc), drv_data->acc.gain, as->resolution, &val);
    if (rc != 0) {
        LOG_ERR("Failed to convert raw ADC to mV: %d", rc);
        return rc;
    }

    drv_data->millivolts = val * VDDHDIV;
    drv_data->code = mv_to_code(drv_data->millivolts);

    return 0;
}

static int vddh_mv_channel_get(const struct device *dev, enum sensor_channel chan,
                               struct sensor_value *val) {
    const struct vddh_mv_data *drv_data = dev->data;

    switch (chan) {
    case SENSOR_CHAN_GAUGE_VOLTAGE:
        val->val1 = drv_data->millivolts / 1000;
        val->val2 = (drv_data->millivolts % 1000) * 1000U;
        break;
    case SENSOR_CHAN_GAUGE_STATE_OF_CHARGE:
        val->val1 = drv_data->code;
        val->val2 = 0;
        break;
    default:
        return -ENOTSUP;
    }

    return 0;
}

static const struct sensor_driver_api vddh_mv_api = {
    .sample_fetch = vddh_mv_sample_fetch,
    .channel_get = vddh_mv_channel_get,
};

static int vddh_mv_init(const struct device *dev) {
    struct vddh_mv_data *drv_data = dev->data;

    if (!device_is_ready(adc)) {
        LOG_ERR("ADC device is not ready %s", adc->name);
        return -ENODEV;
    }

    drv_data->as = (struct adc_sequence){
        .channels = BIT(0),
        .buffer = &drv_data->adc_raw,
        .buffer_size = sizeof(drv_data->adc_raw),
        .oversampling = 4,
        .calibrate = true,
        .resolution = 12,
    };

    drv_data->acc = (struct adc_channel_cfg){
        .gain = ADC_GAIN_1_2,
        .reference = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40),
        .input_positive = SAADC_CH_PSELN_PSELN_VDDHDIV5,
    };

    return adc_channel_setup(adc, &drv_data->acc);
}

static struct vddh_mv_data vddh_mv_data;

DEVICE_DT_INST_DEFINE(0, &vddh_mv_init, NULL, &vddh_mv_data, NULL, POST_KERNEL,
                      CONFIG_SENSOR_INIT_PRIORITY, &vddh_mv_api);
