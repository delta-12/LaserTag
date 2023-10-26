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
#include "driver/uart.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define delay(ms) (vTaskDelay(ms / portTICK_PERIOD_MS))

/* Function Prototypes
 ******************************************************************************/

uint64_t millis(void);
// void _delay(const uint64_t ms);

/* Class Declarations
 ******************************************************************************/
class Stream
{
private:
    const uart_port_t uartNum = UART_NUM_2;
    uart_config_t uartConfig;
    const int uartBufferSize = 1024;
    uint32_t uartRxPin = 16;
    uint32_t uartTxPin = 17;

public:
    Stream(const uint32_t rxPin, const uint32_t txPin, const bool inverseLogic = false);
    Stream(const bool inverseLogic = false);
    ~Stream(void);
    void begin(void);
    void setPins(const uint32_t rxPin, const uint32_t txPin);
    size_t available(void);
    uint8_t read(void);
    uint8_t read(uint8_t *const buffer, const size_t size);
    size_t write(const uint8_t *const buffer, const size_t size);
};

#endif