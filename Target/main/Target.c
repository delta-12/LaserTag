#include "BlePeripheral.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "Neopixel.h"
#include "nvs_flash.h"
#include "Rmt.h"

#define NEOPIXEL_PIN GPIO_NUM_14        /* GPIO pin for neopixel strip */
#define NEOPIXEL_COUNT 3U               /* Neopixel strip with 3 pixels */
#define PAIRING_SEQUENCE_DELAY_MS 500U  /* Wait 500ms when updating the pairing sequence */
#define PAIRING_COMPLETE_DELAY_MS 100U  /* Wait 100ms when updating the pairing complete sequence */
#define PAIRING_COMPLETE_BLINK_COUNT 3U /* Blink neopixel three times when pairing complete */
#define CHECK_CONNECTION_DELAY_MS 50U   /* Check connection every 50ms after pairing complete */

static uint8_t NeopixelBuffer[NEOPIXEL_PIXEL_BUFFER_SIZE(NEOPIXEL_COUNT)]; /* Buffer for storing neopixel channel code data */
static Neopixel_Strip_t NeopixelStrip;

void RmtRxEventHandler(const uint8_t *const data, const size_t size);

void app_main(void)
{
    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    /* Initialize peripherals */
    Neopixel_Init(&NeopixelStrip, NeopixelBuffer, NEOPIXEL_COUNT, NEOPIXEL_PIN);
    BlePeripheral_Init();
    Rmt_RxInit();
    Rmt_RegisterRxEventHandler(RmtRxEventHandler);

    size_t pixelNum;
    while (true)
    {
        /* Display pairing sequence, blink neopixels orange sequentially */
        pixelNum = 0U;
        while (!BlePeripheral_IsConnected())
        {
            Neopixel_Clear(&NeopixelStrip);
            Neopixel_SetPixelColorName(&NeopixelStrip, pixelNum, NEOPIXEL_COLORNAME_ORANGE);
            Neopixel_Show(&NeopixelStrip);

            pixelNum++;
            pixelNum %= NEOPIXEL_COUNT;

            vTaskDelay(PAIRING_SEQUENCE_DELAY_MS / portTICK_PERIOD_MS);
        }

        /* Display pairing complete sequence, blink neopixels blue in unison */
        for (uint8_t i = 0U; i < PAIRING_COMPLETE_BLINK_COUNT; i++)
        {
            Neopixel_FillColorName(&NeopixelStrip, NEOPIXEL_COLORNAME_BLUE);
            Neopixel_Show(&NeopixelStrip);
            vTaskDelay(PAIRING_COMPLETE_DELAY_MS / portTICK_PERIOD_MS);

            Neopixel_Clear(&NeopixelStrip);
            Neopixel_Show(&NeopixelStrip);
            vTaskDelay(PAIRING_COMPLETE_DELAY_MS / portTICK_PERIOD_MS);
        }

        /* Wait until no longer connected */
        while (BlePeripheral_IsConnected())
        {
            vTaskDelay(CHECK_CONNECTION_DELAY_MS / portTICK_PERIOD_MS);
        }
    }
}

void RmtRxEventHandler(const uint8_t *const data, const size_t size)
{
    ESP_LOGI("Target", "RMT RX EVENT");

    for (size_t i = 0U; i < size; i++)
    {
        printf("%x ", *(data + i));
    }
    printf("\n");

    /* Hack to discard const qualifier */
    /* TODO update Rmt module to remove const qualifier from RX event handler typedef */
    uint8_t *dataPtr = (uint8_t *)(uint32_t)data;

    BlePeripheral_Notify(dataPtr, size);
}
