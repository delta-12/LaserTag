/**
 * @file Adc.c
 *
 * @brief Manage ADC peripherals.
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "Adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

/* Defines
 ******************************************************************************/

#define ADC_ADC2_CHANNEL ADC_CHANNEL_6
#define ADC_ATTEN_DB ADC_ATTEN_DB_11

/* Globals
 ******************************************************************************/

const static char *Adc_LogTag = "Adc";

static adc_oneshot_unit_handle_t Adc_Adc2OneshotUnitHandle;
static adc_cali_handle_t Adc_Adc2CalibrationHandle = NULL;
static bool Adc_Adc2Calibrated = false;

/* Function Prototypes
 ******************************************************************************/

static bool Adc_CalibrationInit(const adc_unit_t adcUnit, const adc_channel_t adcChannel, const adc_atten_t adcAtten, adc_cali_handle_t *const adcCalibrationHandleOut);

/* Function Definitions
 ******************************************************************************/

void Adc_Init(void)
{
    /* Initialize ADC 2 */
    adc_oneshot_unit_init_cfg_t adc2InitConfig = {
        .unit_id = ADC_UNIT_2,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc2InitConfig, &Adc_Adc2OneshotUnitHandle));

    /* Calibrate ADC 2 */
    Adc_Adc2Calibrated = Adc_CalibrationInit(ADC_UNIT_2, ADC_ADC2_CHANNEL, ADC_ATTEN_DB, &Adc_Adc2CalibrationHandle);

    /* Configure ADC 2 */
    adc_oneshot_chan_cfg_t adc2Config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(Adc_Adc2OneshotUnitHandle, ADC_ADC2_CHANNEL, &adc2Config));
}

esp_err_t Adc_OneshotRead(Adc_mV_t *const voltage)
{
    esp_err_t err = ESP_OK;
    int raw;

    err = adc_oneshot_read(Adc_Adc2OneshotUnitHandle, ADC_ADC2_CHANNEL, &raw);

    if (Adc_Adc2Calibrated && err == ESP_OK)
    {
        err = adc_cali_raw_to_voltage(Adc_Adc2CalibrationHandle, raw, voltage);
    }

    return err;
}

void Adc_DeInit(void)
{
    ESP_ERROR_CHECK(adc_oneshot_del_unit(Adc_Adc2OneshotUnitHandle));
    if (Adc_Adc2Calibrated && Adc_Adc2CalibrationHandle != NULL)
    {
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
        ESP_LOGI(Adc_LogTag, "Deregister %s calibration scheme", "Curve Fitting");
        ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(Adc_Adc2CalibrationHandle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
        ESP_LOGI(Adc_LogTag, "Deregister %s calibration scheme", "Line Fitting");
        ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(Adc_Adc2CalibrationHandle));
#endif
    }
}

static bool Adc_CalibrationInit(const adc_unit_t adcUnit, const adc_channel_t adcChannel, const adc_atten_t adcAtten, adc_cali_handle_t *const adcCalibrationHandleOut)
{
    adc_cali_handle_t adcCalibrationHandle = NULL;
    esp_err_t err = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated)
    {
        ESP_LOGI(Adc_LogTag, "Calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t curveFittingConfig = {
            .unit_id = adcUnit,
            .chan = adcChannel,
            .atten = adcAtten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        err = adc_cali_create_scheme_curve_fitting(&curveFittingConfig, &adcCalibrationHandle);
        if (err == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated)
    {
        ESP_LOGI(Adc_LogTag, "Calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t lineFittingConfig = {
            .unit_id = adcUnit,
            .atten = adcAtten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        err = adc_cali_create_scheme_line_fitting(&lineFittingConfig, &adcCalibrationHandle);
        if (err == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

    *adcCalibrationHandleOut = adcCalibrationHandle;
    if (err == ESP_OK)
    {
        ESP_LOGI(Adc_LogTag, "Calibration Success");
    }
    else if (err == ESP_ERR_NOT_SUPPORTED || !calibrated)
    {
        ESP_LOGW(Adc_LogTag, "eFuse not burnt, skip software calibration");
    }
    else
    {
        ESP_LOGE(Adc_LogTag, "Invalid arg or no memory");
    }

    return calibrated;
}
