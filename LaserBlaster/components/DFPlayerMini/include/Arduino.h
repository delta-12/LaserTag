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
#include "driver/gpio.h"
#include "driver/uart.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Defines
 ******************************************************************************/

#define DFPLAYERMINI_ARDUINO_DEFAULT_UART_NUM UART_NUM_2
#define DFPLAYERMINI_ARDUINO_DEFAULT_UART_RX_PIN GPIO_NUM_16
#define DFPLAYERMINI_ARDUINO_DEFAULT_UART_TX_PIN GPIO_NUM_17

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
    uart_config_t uartConfig;
    uart_port_t uartNum;
    uint32_t uartRxPin;
    uint32_t uartTxPin;
    QueueHandle_t uartQueueHandle;

public:
    Stream(const uart_port_t uart = DFPLAYERMINI_ARDUINO_DEFAULT_UART_NUM, const uint32_t rxPin = DFPLAYERMINI_ARDUINO_DEFAULT_UART_RX_PIN, const uint32_t txPin = DFPLAYERMINI_ARDUINO_DEFAULT_UART_TX_PIN, const bool inverseLogic = false);
    ~Stream(void);
    void begin(void);
    void setUart(const uart_port_t uart);
    void setPins(const uint32_t rxPin, const uint32_t txPin);
    size_t available(void);
    uint8_t read(void);
    uint8_t read(uint8_t *const buffer, const size_t size);
    size_t write(const uint8_t *const buffer, const size_t size);
};

#endif