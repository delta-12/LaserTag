#include "BlePeripheral.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "Neopixel.h"
#include "nvs_flash.h"
#include "Rmt.h"
#include <stdio.h>

#define NEOPIXEL_PIN GPIO_NUM_14 /* GPIO pin for neopixel strip */
#define NEOPIXEL_COUNT 3U        /* Neopixel strip with 3 pixels */
#define PAIRING_SEQUENCE_DELAY_MS 500U
#define PAIRING_COMPLETE_DELAY_MS 100U
#define PAIRING_COMPLETE_BLINK_COUNT 3U

static const char *LogTag = "Target";

static uint8_t NeopixelBuffer[NEOPIXEL_PIXEL_BUFFER_SIZE(NEOPIXEL_COUNT)]; /* Buffer for storing neopixel channel code data */
static Neopixel_Strip_t NeopixelStrip;

void rmtRxEventHandler(const uint8_t *const data, const size_t size);

void app_main(void)
{
    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    Neopixel_Init(&NeopixelStrip, NeopixelBuffer, NEOPIXEL_COUNT, NEOPIXEL_PIN);
    BlePeripheral_Init();

    size_t pixelNum = 0U;
    while (!BlePeripheral_IsConnected())
    {
        Neopixel_Clear(&NeopixelStrip);
        Neopixel_SetPixelColorName(&NeopixelStrip, pixelNum, NEOPIXEL_COLORNAME_ORANGE);
        Neopixel_Show(&NeopixelStrip);

        pixelNum++;
        pixelNum %= NEOPIXEL_COUNT;

        vTaskDelay(PAIRING_SEQUENCE_DELAY_MS / portTICK_PERIOD_MS);
    }
    for (uint8_t i = 0U; i < PAIRING_COMPLETE_BLINK_COUNT; i++)
    {
        Neopixel_FillColorName(&NeopixelStrip, NEOPIXEL_COLORNAME_BLUE);
        Neopixel_Show(&NeopixelStrip);
        vTaskDelay(PAIRING_COMPLETE_DELAY_MS / portTICK_PERIOD_MS);

        Neopixel_Clear(&NeopixelStrip);
        Neopixel_Show(&NeopixelStrip);
        vTaskDelay(PAIRING_COMPLETE_DELAY_MS / portTICK_PERIOD_MS);
    }

    Rmt_RxInit();
    Rmt_RegisterRxEventHandler(rmtRxEventHandler);
}

void rmtRxEventHandler(const uint8_t *const data, const size_t size)
{
    ESP_LOGI(LogTag, "RMT RX EVENT");

    for (size_t i = 0U; i < size; i++)
    {
        printf("%x ", *(data + i));
    }
    printf("\n");
}
