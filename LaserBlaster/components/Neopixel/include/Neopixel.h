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

typedef uint8_t Neopixel_ChannelIntensity_t; /* Intensity of a single RGB channel */

/* Select RGB values by name of color */
typedef enum
{
    NEOPIXEL_COLORNAME_BLACK,  /* Select RGB values for black */
    NEOPIXEL_COLORNAME_WHITE,  /* Select RGB values for white */
    NEOPIXEL_COLORNAME_RED,    /* Select RGB values for red */
    NEOPIXEL_COLORNAME_GREEN,  /* Select RGB values for green */
    NEOPIXEL_COLORNAME_BLUE,   /* Select RGB values for blue */
    NEOPIXEL_COLORNAME_YELLOW, /* Select RGB values for yellow */
    NEOPIXEL_COLORNAME_ORANGE, /* Select RGB values for orange */
    NEOPIXEL_COLORNAME_MAGENTA /* Select RGB values for magenta */
} Neopixel_ColorName_t;

/* RGB channel values */
typedef struct
{
    Neopixel_ChannelIntensity_t Red;   /* Intensity of red channel */
    Neopixel_ChannelIntensity_t Green; /* Intensity of green channel */
    Neopixel_ChannelIntensity_t Blue;  /* Intensity of blue channel */
} Neopixel_Color_t;

/* Handle for strip of neopixels */
typedef struct
{
    uint8_t *PixelBuffer;               /* I2S TX buffer to store pixel RGB values */
    size_t PixelCount;                  /* Number of pixels in strip, must not exceed the number of RGB values that can be stored in PixelBuffer */
    gpio_num_t GpioNum;                 /* GPIO pin strip is attached to */
    i2s_chan_handle_t I2sChannelHandle; /* Handle for I2S channel used to control strip */
} Neopixel_Strip_t;

/* Function Prototypes
 ******************************************************************************/

void Neopixel_Init(Neopixel_Strip_t *const neopixelStrip, uint8_t *const pixelBuffer, const size_t pixelCount, const gpio_num_t gpioNum);
void Neopixel_Clear(Neopixel_Strip_t *const neopixelStrip);
void Neopixel_SetPixelColor(Neopixel_Strip_t *const neopixelStrip, const size_t pixelNum, const Neopixel_Color_t color);
void Neopixel_SetPixelColorName(Neopixel_Strip_t *const neopixelStrip, const size_t pixelNum, const Neopixel_ColorName_t colorName);
void Neopixel_SetPixelChannels(Neopixel_Strip_t *const neopixelStrip, const size_t pixelNum, const Neopixel_ChannelIntensity_t red, const Neopixel_ChannelIntensity_t green, const Neopixel_ChannelIntensity_t blue);
void Neopixel_FillColor(Neopixel_Strip_t *const neopixelStrip, const Neopixel_Color_t *const color);
void Neopixel_FillColorName(Neopixel_Strip_t *const neopixelStrip, const Neopixel_ColorName_t colorName);
void Neopixel_FillChannels(Neopixel_Strip_t *const neopixelStrip, const Neopixel_ChannelIntensity_t red, const Neopixel_ChannelIntensity_t green, const Neopixel_ChannelIntensity_t blue);
void Neopixel_Show(const Neopixel_Strip_t *const neopixelStrip);

#endif
