/**
 * @file Neopixel.h
 *
 * @brief I2S Neopixel driver.
 *
 ******************************************************************************/

#ifndef NEOPIXEL_H
#define NEOPIXEL_H

/* Includes
 ******************************************************************************/
#include "driver/gpio.h"
#include "driver/i2s_std.h"

/* Defines
 ******************************************************************************/

#define NEOPIXEL_CHANNEL_BYTES 4U                                                           /* Each RGB channel has 8 4-bit codes */
#define NEOPIXEL_CHANNELS_PER_PIXEL 3U                                                      /* Each pixel has RGB channels */
#define NEOPIXEL_PIXEL_CHANNELS_SIZE (NEOPIXEL_CHANNEL_BYTES * NEOPIXEL_CHANNELS_PER_PIXEL) /* Total size needed to store RGB channels for a single pixel */

/**
 * @brief Size of the buffer needed to store the RGB channels for a strip of
 * pixels.
 *
 * @param[in] pixelCount Number of pixels in the strip
 ******************************************************************************/
#define NEOPIXEL_PIXEL_BUFFER_SIZE(pixelCount) (size_t)(pixelCount * NEOPIXEL_PIXEL_CHANNELS_SIZE)

/* Typedefs
 ******************************************************************************/

typedef uint8_t Neopixel_ChannelIntensity_t;

typedef enum
{
    NEOPIXEL_COLORNAME_BLACK,
    NEOPIXEL_COLORNAME_WHITE,
    NEOPIXEL_COLORNAME_RED,
    NEOPIXEL_COLORNAME_GREEN,
    NEOPIXEL_COLORNAME_BLUE,
    NEOPIXEL_COLORNAME_YELLOW,
    NEOPIXEL_COLORNAME_ORANGE,
    NEOPIXEL_COLORNAME_MAGENTA
} Neopixel_ColorName_t;

typedef struct
{
    Neopixel_ChannelIntensity_t Red;
    Neopixel_ChannelIntensity_t Green;
    Neopixel_ChannelIntensity_t Blue;
} Neopixel_Color_t;

typedef struct
{
    uint8_t *PixelBuffer;
    size_t PixelCount;
    gpio_num_t GpioNum;
    i2s_chan_handle_t I2sChannelHandle;
} Neopixel_Strip_t;

/* Function Prototypes
 ******************************************************************************/

void Neopixel_Init(Neopixel_Strip_t *const neopixelStrip);
void Neopixel_Clear(Neopixel_Strip_t *const neopixelStrip);
void Neopixel_SetPixelColor(Neopixel_Strip_t *const neopixelStrip, const size_t pixelNum, const Neopixel_Color_t *const color);
void Neopixel_SetPixelColorName(Neopixel_Strip_t *const neopixelStrip, const size_t pixelNum, const Neopixel_ColorName_t colorName);
void Neopixel_SetPixelChannels(Neopixel_Strip_t *const neopixelStrip, const size_t pixelNum, const Neopixel_ChannelIntensity_t red, const Neopixel_ChannelIntensity_t green, const Neopixel_ChannelIntensity_t blue);
void Neopixel_FillColor(Neopixel_Strip_t *const neopixelStrip, const Neopixel_Color_t *const color);
void Neopixel_FillColorName(Neopixel_Strip_t *const neopixelStrip, const Neopixel_ColorName_t colorName);
void Neopixel_FillChannels(Neopixel_Strip_t *const neopixelStrip, const Neopixel_ChannelIntensity_t red, const Neopixel_ChannelIntensity_t green, const Neopixel_ChannelIntensity_t blue);
void Neopixel_Show(const Neopixel_Strip_t *const neopixelStrip);

#endif
