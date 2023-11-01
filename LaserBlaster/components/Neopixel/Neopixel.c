/**
 * @file Neopixel.c
 *
 * @brief I2S Neopixel driver.
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "esp_check.h"
#include "Neopixel.h"

/* Defines
 ******************************************************************************/

#define NEOPIXEL_I2S_SAMPLE_RATE 93750U
#define NEOPIXEL_RED_CHANNEL_OFFSET 4U
#define NEOPIXEL_GREEN_CHANNEL_OFFSET 0U
#define NEOPIXEL_BLUE_CHANNEL_OFFSET 8U
#define NEOPIXEL_WRITE_TIMEOUT_MS 1000U
#define NEOPIXEL_ZERO_CHANNEL 0x88888888U
#define NEOPIXEL_ONE_CHANNEL_CODE 0xEU
#define NEOPIXEL_CHANNEL_INTENSITY_BIT 0x1U
#define NEOPIXEL_CHANNEL_INTENSITY_BIT_COUNT 8U
#define NEOPIXEL_BITS_PER_CHANNEL_CODE 4U
#define NEOPIXEL_TASK_STACK_DEPTH 2048U /* Stack depth for neopixel RTOS tasks */
#define NEOPIXEL_TASK_PRIORITY 10U      /* Priority for neopixel RTOS tasks */

/* Globals
 ******************************************************************************/

static Neopixel_Color_t Neopixel_Colors[] = {
    {0U, 0U, 0U},       /* Black */
    {255U, 255U, 255U}, /* White */
    {255U, 0U, 0U},     /* Red */
    {0U, 255U, 0U},     /* Green */
    {0U, 0U, 255U},     /* Blue */
    {255U, 255U, 0U},   /* Yellow */
    {255U, 165U, 0U},   /* Orange */
    {255U, 0U, 255U},   /* Magenta */
};

/* Function Prototypes
 ******************************************************************************/

static void Neopixel_SetChannelCodes(uint8_t *const channelBuffer, const Neopixel_ChannelIntensity_t channelIntensity);

/* Function Definitions
 ******************************************************************************/

void Neopixel_Init(Neopixel_Strip_t *const neopixelStrip)
{
    if (neopixelStrip != NULL)
    {
        i2s_chan_config_t channelConfig = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
        i2s_new_channel(&channelConfig, &neopixelStrip->I2sChannelHandle, NULL);

        i2s_std_config_t stdTxConfig = {
            .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(NEOPIXEL_I2S_SAMPLE_RATE),
            .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
            .gpio_cfg = {
                .mclk = I2S_GPIO_UNUSED,
                .bclk = I2S_GPIO_UNUSED,
                .ws = I2S_GPIO_UNUSED,
                .dout = neopixelStrip->GpioNum,
                .din = I2S_GPIO_UNUSED,
                .invert_flags = {
                    .mclk_inv = false,
                    .bclk_inv = false,
                    .ws_inv = false,
                },
            },
        };
        ESP_ERROR_CHECK(i2s_channel_init_std_mode(neopixelStrip->I2sChannelHandle, &stdTxConfig));
        ESP_ERROR_CHECK(i2s_channel_enable(neopixelStrip->I2sChannelHandle));

        Neopixel_Clear(neopixelStrip);
        Neopixel_Show(neopixelStrip);
    }
}

void Neopixel_Clear(Neopixel_Strip_t *const neopixelStrip)
{
    if (neopixelStrip != NULL)
    {
        Neopixel_FillColorName(neopixelStrip, NEOPIXEL_COLORNAME_BLACK);
    }
}

void Neopixel_SetPixelColor(Neopixel_Strip_t *const neopixelStrip, const size_t pixelNum, const Neopixel_Color_t *const color)
{
    if (neopixelStrip != NULL)
    {
        Neopixel_SetPixelChannels(neopixelStrip, pixelNum, color->Red, color->Green, color->Blue);
    }
}

void Neopixel_SetPixelColorName(Neopixel_Strip_t *const neopixelStrip, const size_t pixelNum, const Neopixel_ColorName_t colorName)
{
    if (neopixelStrip != NULL)
    {
        Neopixel_SetPixelColor(neopixelStrip, pixelNum, &Neopixel_Colors[colorName]);
    }
}

void Neopixel_SetPixelChannels(Neopixel_Strip_t *const neopixelStrip, const size_t pixelNum, const Neopixel_ChannelIntensity_t red, const Neopixel_ChannelIntensity_t green, const Neopixel_ChannelIntensity_t blue)
{
    if (neopixelStrip != NULL)
    {
        uint8_t *pixelChannelsStart = neopixelStrip->PixelBuffer + (pixelNum * NEOPIXEL_PIXEL_CHANNELS_SIZE);

        Neopixel_SetChannelCodes(pixelChannelsStart + NEOPIXEL_RED_CHANNEL_OFFSET, red);
        Neopixel_SetChannelCodes(pixelChannelsStart + NEOPIXEL_GREEN_CHANNEL_OFFSET, green);
        Neopixel_SetChannelCodes(pixelChannelsStart + NEOPIXEL_BLUE_CHANNEL_OFFSET, blue);
    }
}

void Neopixel_FillColor(Neopixel_Strip_t *const neopixelStrip, const Neopixel_Color_t *const color)
{
    if (neopixelStrip != NULL)
    {
        Neopixel_FillChannels(neopixelStrip, color->Red, color->Green, color->Blue);
    }
}

void Neopixel_FillColorName(Neopixel_Strip_t *const neopixelStrip, const Neopixel_ColorName_t colorName)
{
    if (neopixelStrip != NULL)
    {
        Neopixel_FillColor(neopixelStrip, &Neopixel_Colors[colorName]);
    }
}
void Neopixel_FillChannels(Neopixel_Strip_t *const neopixelStrip, const Neopixel_ChannelIntensity_t red, const Neopixel_ChannelIntensity_t green, const Neopixel_ChannelIntensity_t blue)
{
    if (neopixelStrip != NULL)
    {
        for (size_t i = 0U; i < neopixelStrip->PixelCount; i++)
        {
            Neopixel_SetPixelChannels(neopixelStrip, i, red, green, blue);
        }
    }
}

void Neopixel_Show(const Neopixel_Strip_t *const neopixelStrip)
{
    if (neopixelStrip != NULL)
    {
        size_t bytesLoaded = 0U;

        i2s_channel_disable(neopixelStrip->I2sChannelHandle);
        i2s_channel_preload_data(neopixelStrip->I2sChannelHandle, neopixelStrip->PixelBuffer, NEOPIXEL_PIXEL_BUFFER_SIZE(neopixelStrip->PixelCount), &bytesLoaded);
        i2s_channel_enable(neopixelStrip->I2sChannelHandle);
    }
}

static void Neopixel_SetChannelCodes(uint8_t *const channelBuffer, const Neopixel_ChannelIntensity_t channelIntensity)
{
    if (channelBuffer != NULL)
    {
        uint32_t channelCodes = NEOPIXEL_ZERO_CHANNEL;

        for (size_t i = 0U; i < NEOPIXEL_CHANNEL_INTENSITY_BIT_COUNT; i++)
        {
            if (((channelIntensity >> i) & NEOPIXEL_CHANNEL_INTENSITY_BIT) == NEOPIXEL_CHANNEL_INTENSITY_BIT)
            {
                channelCodes |= (NEOPIXEL_ONE_CHANNEL_CODE << (i * NEOPIXEL_BITS_PER_CHANNEL_CODE));
            }
        }

        *(channelBuffer) = (channelCodes >> (6 * NEOPIXEL_BITS_PER_CHANNEL_CODE)) & 0xFF;
        *(channelBuffer + 1U) = (channelCodes >> (4 * NEOPIXEL_BITS_PER_CHANNEL_CODE)) & 0xFF;
        *(channelBuffer + 2U) = (channelCodes >> (2 * NEOPIXEL_BITS_PER_CHANNEL_CODE)) & 0xFF;
        *(channelBuffer + 3U) = (channelCodes >> (0 * NEOPIXEL_BITS_PER_CHANNEL_CODE)) & 0xFF;
    }
}
