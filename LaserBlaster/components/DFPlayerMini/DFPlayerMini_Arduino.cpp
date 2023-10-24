/**
 * @file DFPlayerMini_Arduino.cpp
 *
 * @brief Implements parts of Arduino core needed to compile
 * DFRobotDFPlayerMini library
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "Arduino.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Defines
 ******************************************************************************/

#define US_PER_MS 1000ULL /* 1000 microseconds per millisecond */

/* Function Definitions
 ******************************************************************************/

uint16_t millis(void)
{
    return (uint16_t)(esp_timer_get_time() / US_PER_MS);
}

void delay(const uint16_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

Stream::Stream(const uint32_t rxPin, const uint32_t txPin, const bool inverseLogic) {}

Stream::Stream(const bool inverseLogic) {}

Stream::~Stream(void) {}

void Stream::begin(void) {}

void Stream::setPins(const uint32_t rxPin, const uint32_t txPin) {}

bool Stream::available(void)
{
    return false;
}

uint8_t Stream::read(void)
{
    return 0U;
}

uint8_t Stream::read(uint8_t *const buffer, const size_t size)
{
    return 0U;
}

size_t Stream::write(const uint8_t *const buffer, const size_t size)
{
    return 0U;
}
