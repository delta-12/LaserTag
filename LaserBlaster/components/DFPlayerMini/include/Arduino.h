/**
 * @file Arduino.h
 *
 * @brief Stand-in for Arduino header needed to compile DFRobotDFPlayerMini
 * library
 *
 ******************************************************************************/

#ifndef DFPLAYERMINI_ARDUINO_H
#define DFPLAYERMINI_ARDUINO_H

/* Includes
 ******************************************************************************/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Function Prototypes
 ******************************************************************************/

uint16_t millis(void);
void delay(const uint64_t ms);

/* Class Declarations
 ******************************************************************************/
class Stream
{
public:
    Stream(const uint32_t rxPin, const uint32_t txPin, const bool inverseLogic = false);
    Stream(const bool inverseLogic = false);
    ~Stream(void);
    void begin(void);
    void setPins(const uint32_t rxPin, const uint32_t txPin);
    bool available(void);
    uint8_t read(void);
    uint8_t read(uint8_t *const buffer, const size_t size);
    size_t write(const uint8_t *const buffer, const size_t size);
};

#endif