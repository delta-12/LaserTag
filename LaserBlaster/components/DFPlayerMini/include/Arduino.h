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
void delay(uint64_t ms);

/* Class Declarations
 ******************************************************************************/
class Stream
{
public:
    Stream(uint8_t rxPin, uint8_t txPin, bool inverseLogic = false);
    ~Stream(void);
    bool available(void);
    uint8_t read(void);
    uint8_t read(uint8_t *buffer, size_t size);
    size_t write(const uint8_t *buffer, size_t size);
};

#endif