#include "BlePeripheral.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Neopixel.h"
#include "nvs_flash.h"
#include "Rmt.h"

#define NEOPIXEL_PIN GPIO_NUM_14        /* GPIO pin for neopixel strip */
#define NEOPIXEL_COUNT 3U               /* Neopixel strip with 3 pixels */
#define PAIRING_SEQUENCE_DELAY_MS 500U  /* Wait 500ms when updating the pairing sequence */
#define PAIRING_COMPLETE_DELAY_MS 100U  /* Wait 100ms when updating the pairing complete sequence */
#define PAIRING_COMPLETE_BLINK_COUNT 3U /* Blink neopixel three times when pairing complete */
#define CHECK_CONNECTION_DELAY_MS 50U   /* Check connection every 50ms after pairing complete */
#define SHOT_RECEIVED_BLINK_DELAY 50U
#define SHOT_RECEIVED_BLINK_COUNT 10U
#define SHOT_RECEIVED_TASK_STACK_DEPTH 2048U /* Stack depth for Shot Received RTOS tasks */
#define SHOT_RECEIVED_TASK_PRIORITY 10U      /* Priority for Shot Received RTOS tasks */

static uint8_t NeopixelBuffer[NEOPIXEL_PIXEL_BUFFER_SIZE(NEOPIXEL_COUNT)]; /* Buffer for storing neopixel channel code data */
static Neopixel_Strip_t NeopixelStrip;
static size_t PixelNum;
static uint32_t NextShot = 1U;
Neopixel_ColorName_t NeopixelColor;

void RmtRxEventHandler(const uint8_t *const data, const size_t size);
void ShotReceivedTask(void *arg);

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

    while (true)
    {
        /* Display pairing sequence, blink neopixels orange sequentially */
        PixelNum = 0U;
        while (!BlePeripheral_IsConnected())
        {
            Neopixel_Clear(&NeopixelStrip);
            Neopixel_SetPixelColorName(&NeopixelStrip, PixelNum, NEOPIXEL_COLORNAME_ORANGE);
            Neopixel_Show(&NeopixelStrip);

            PixelNum++;
            PixelNum %= NEOPIXEL_COUNT;

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
    for (size_t i = 0U; i < size; i++)
    {
        printf("%x ", *(data + i));
    }
    printf("\n");

    if (size == 7U)
    {
        if (data[0U] == 0x4CU && data[1U] == 0x54U && data[2U] == 0x00U)
        {
            uint32_t shotNumber = data[3U];
            shotNumber = (shotNumber << 8U) | data[4U];
            shotNumber = (shotNumber << 8U) | data[5U];
            shotNumber = (shotNumber << 8U) | data[6U];

            /* Hack to discard const qualifier */
            /* TODO update Rmt module to remove const qualifier from RX event handler typedef */
            uint8_t *dataPtr = (uint8_t *)(uint32_t)data;
            BlePeripheral_Notify(dataPtr, size);

            NeopixelColor = NEOPIXEL_COLORNAME_RED;
            if (shotNumber == NextShot)
            {
                NeopixelColor = NEOPIXEL_COLORNAME_GREEN;
                NextShot++;
            }
            xTaskCreate(ShotReceivedTask, "ShotReceivedTask", SHOT_RECEIVED_TASK_STACK_DEPTH, NULL, SHOT_RECEIVED_TASK_PRIORITY, NULL);
        }
    }
}

void ShotReceivedTask(void *arg)
{
    uint16_t blinkCount = 0U;

    PixelNum = 0U;
    while (blinkCount < SHOT_RECEIVED_BLINK_COUNT)
    {
        Neopixel_Clear(&NeopixelStrip);
        Neopixel_SetPixelColorName(&NeopixelStrip, PixelNum, NeopixelColor);
        Neopixel_Show(&NeopixelStrip);

        PixelNum++;
        PixelNum %= NEOPIXEL_COUNT;

        vTaskDelay(SHOT_RECEIVED_BLINK_DELAY / portTICK_PERIOD_MS);

        blinkCount++;
    }

    Neopixel_Clear(&NeopixelStrip);
    Neopixel_Show(&NeopixelStrip);

    vTaskDelete(NULL);
}
