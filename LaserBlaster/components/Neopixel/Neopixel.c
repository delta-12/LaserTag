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

#define NEOPIXEL_I2S_SAMPLE_RATE 93750U            /* Sample rate need to transmit neopixel data at the correct frequency */
#define NEOPIXEL_RED_CHANNEL_OFFSET 4U             /* Offset from the start of a RGB value in a I2S TX buffer for the red channel*/
#define NEOPIXEL_GREEN_CHANNEL_OFFSET 0U           /* Offset from the start of a RGB value in a I2S TX buffer for the green channel*/
#define NEOPIXEL_BLUE_CHANNEL_OFFSET 8U            /* Offset from the start of a RGB value in a I2S TX buffer for the blue channel*/
#define NEOPIXEL_ZERO_CHANNEL 0x88888888U          /* 8 4-bit codes for zeroed out RGB channel */
#define NEOPIXEL_ONE_CHANNEL_CODE 0xEU             /* 4-bit channel code for 0b1 */
#define NEOPIXEL_CHANNEL_INTENSITY_BIT 0x1U        /* Mask a single bit */
#define NEOPIXEL_CHANNEL_INTENSITY_BIT_COUNT 8U    /* RGB channel intensity is 8 bits, values 0 to 255 */
#define NEOPIXEL_BITS_PER_CHANNEL_CODE 4U          /* Each channel code has 4 bits */
#define NEOPIXEL_BYTE_MASK 0xFFU                   /* Mask a byte */
#define NEOPIXEL_CHANNEL_CODE_BYTE_0 0U            /* Buffer offset for byte 0 of a channel code */
#define NEOPIXEL_CHANNEL_CODE_BYTE_1 1U            /* Buffer offset for byte 1 of a channel code */
#define NEOPIXEL_CHANNEL_CODE_BYTE_2 2U            /* Buffer offset for byte 2 of a channel code */
#define NEOPIXEL_CHANNEL_CODE_BYTE_3 3U            /* Buffer offset for byte 3 of a channel code */
#define NEOPIXEL_CHANNEL_CODE_BYTE_0_BIT_SHIFT 24U /* Bit shift to get byte 0 of a channel code */
#define NEOPIXEL_CHANNEL_CODE_BYTE_1_BIT_SHIFT 16U /* Bit shift to get byte 1 of a channel code */
#define NEOPIXEL_CHANNEL_CODE_BYTE_2_BIT_SHIFT 8U  /* Bit shift to get byte 2 of a channel code */
#define NEOPIXEL_CHANNEL_CODE_BYTE_3_BIT_SHIFT 0U  /* Bit shift to get byte 3 of a channel code */

/* Globals
 ******************************************************************************/

/* List of RGB values corresponding to Neopixel_ColorName_t */
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

/**
 * @brief Initialize a neopixel strip.
 *
 * @param[in,out] neopixelStrip Pointer to the neopxiel strip to initialize
 * @param[in]     pixelBuffer   Pointer to the I2S TX buffer to store pixel RGB
 *                              values
 * @param[in]     pixelCount    Number of pixels in strip, must not exceed the
 *                              number of RGB values that can be stored in the
 *                              pixel buffer
 * @param[in]     gpioNum       GPIO pin strip is attached to
 ******************************************************************************/
void Neopixel_Init(Neopixel_Strip_t *const neopixelStrip, uint8_t *const pixelBuffer, const size_t pixelCount, const gpio_num_t gpioNum)
{
    if (neopixelStrip != NULL)
    {
        neopixelStrip->PixelBuffer = pixelBuffer;
        neopixelStrip->PixelCount = pixelCount;
        neopixelStrip->GpioNum = gpioNum;

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

/**
 * @brief Clear a neopixel strip.
 *
 * @note Neopixel_Show must be called after to take effect.
 *
 * @param[in,out] neopixelStrip Pointer to the neopxiel strip to clear
 ******************************************************************************/
void Neopixel_Clear(Neopixel_Strip_t *const neopixelStrip)
{
    if (neopixelStrip != NULL)
    {
        Neopixel_FillColorName(neopixelStrip, NEOPIXEL_COLORNAME_BLACK);
    }
}

/**
 * @brief Set the color of a neopixel in a strip by RGB value.
 *
 * @note Neopixel_Show must be called after to take effect.
 *
 * @param[in,out] neopixelStrip Pointer to the neopxiel strip
 * @param[in]     pixelNum      The number of the pixel in the strip
 * @param[in]     color         RGB values of the color to set
 ******************************************************************************/
void Neopixel_SetPixelColor(Neopixel_Strip_t *const neopixelStrip, const size_t pixelNum, const Neopixel_Color_t color)
{
    if (neopixelStrip != NULL)
    {
        Neopixel_SetPixelChannels(neopixelStrip, pixelNum, color.Red, color.Green, color.Blue);
    }
}

/**
 * @brief Set the color of a neopixel in a strip by name of the color.
 *
 * @note Neopixel_Show must be called after to take effect.
 *
 * @param[in,out] neopixelStrip Pointer to the neopxiel strip
 * @param[in]     pixelNum      The number of the pixel in the strip
 * @param[in]     colorName     Name of the color to set
 ******************************************************************************/
void Neopixel_SetPixelColorName(Neopixel_Strip_t *const neopixelStrip, const size_t pixelNum, const Neopixel_ColorName_t colorName)
{
    if (neopixelStrip != NULL)
    {
        Neopixel_SetPixelColor(neopixelStrip, pixelNum, Neopixel_Colors[colorName]);
    }
}

/**
 * @brief Set the color of a neopixel in a strip by RGB channel values.
 *
 * @note Neopixel_Show must be called after to take effect.
 *
 * @param[in,out] neopixelStrip Pointer to the neopxiel strip
 * @param[in]     pixelNum      The number of the pixel in the strip
 * @param[in]     red           Intensity of the red channel
 * @param[in]     green         Intensity of the green channel
 * @param[in]     blue          Intensity of the blue channel
 ******************************************************************************/
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

/**
 * @brief Set the color of all neopixels in a strip by RGB value.
 *
 * @note Neopixel_Show must be called after to take effect.
 *
 * @param[in,out] neopixelStrip Pointer to the neopxiel strip
 * @param[in]     color         RGB values of the color to set
 ******************************************************************************/
void Neopixel_FillColor(Neopixel_Strip_t *const neopixelStrip, const Neopixel_Color_t *const color)
{
    if (neopixelStrip != NULL)
    {
        Neopixel_FillChannels(neopixelStrip, color->Red, color->Green, color->Blue);
    }
}

/**
 * @brief Set the color of all neopixels in a strip by name of the color.
 *
 * @note Neopixel_Show must be called after to take effect.
 *
 * @param[in,out] neopixelStrip Pointer to the neopxiel strip
 * @param[in]     colorName     Name of the color to set
 ******************************************************************************/
void Neopixel_FillColorName(Neopixel_Strip_t *const neopixelStrip, const Neopixel_ColorName_t colorName)
{
    if (neopixelStrip != NULL)
    {
        Neopixel_FillColor(neopixelStrip, &Neopixel_Colors[colorName]);
    }
}

/**
 * @brief Set the color of all neopixels in a strip by RGB channel values.
 *
 * @note Neopixel_Show must be called after to take effect.
 *
 * @param[in,out] neopixelStrip Pointer to the neopxiel strip
 * @param[in]     red           Intensity of the red channel
 * @param[in]     green         Intensity of the green channel
 * @param[in]     blue          Intensity of the blue channel
 ******************************************************************************/
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

/**
 * @brief Transmit the RGB values in a neopixel strip's I2S TX buffer to the
 *        neopixel strip itself over I2S.  Must be called after other functions
 *        to set color in order for the change to take effect.
 *
 * @param[in] neopixelStrip Pointer to the neopxiel strip
 ******************************************************************************/
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

/**
 * @brief Set the 4-bit codes corresponding to a given intensity for a single
 *        RGB channel.
 *
 * @param[in,out] channelBuffer    Pointer to the buffer of codes for a single
 *                                 channel
 * @param[in]     channelIntensity Intensity to set the channel to
 ******************************************************************************/
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

        *(channelBuffer + NEOPIXEL_CHANNEL_CODE_BYTE_0) = (channelCodes >> NEOPIXEL_CHANNEL_CODE_BYTE_0_BIT_SHIFT) & NEOPIXEL_BYTE_MASK;
        *(channelBuffer + NEOPIXEL_CHANNEL_CODE_BYTE_1) = (channelCodes >> NEOPIXEL_CHANNEL_CODE_BYTE_1_BIT_SHIFT) & NEOPIXEL_BYTE_MASK;
        *(channelBuffer + NEOPIXEL_CHANNEL_CODE_BYTE_2) = (channelCodes >> NEOPIXEL_CHANNEL_CODE_BYTE_2_BIT_SHIFT) & NEOPIXEL_BYTE_MASK;
        *(channelBuffer + NEOPIXEL_CHANNEL_CODE_BYTE_3) = (channelCodes >> NEOPIXEL_CHANNEL_CODE_BYTE_3_BIT_SHIFT) & NEOPIXEL_BYTE_MASK;
    }
}
