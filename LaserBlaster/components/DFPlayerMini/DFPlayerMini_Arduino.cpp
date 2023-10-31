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
#include "freertos/task.h"
#include "esp_timer.h"

/* Defines
 ******************************************************************************/

#define DFPLAYERMINI_ARDUINO_UART_BUFFER_SIZE 1024U
#define DFPLAYERMINI_ARDUINO_US_PER_MS 1000ULL
#define DFPLAYERMINI_ARDUINO_ESP_INTR_FLAG_DEFAULT 0U
#define DFPLAYERMINI_ARDUINO_UART_QUEUE_SIZE 20U
#define DFPLAYERMINI_ARDUINO_AVAILABLE_DELAY 10U
#define DFPLAYERMINI_ARDUINO_TASK_STACK_DEPTH 2048U
#define DFPLAYERMINI_ARDUINO_TASK_PRIORITY 10U

/* Function Prototypes
 ******************************************************************************/

static void DFPlayerMini_Arduino_UartEventHandlerTask(void *arg);

/* Function Definitions
 ******************************************************************************/

uint64_t millis(void)
{
    return (uint64_t)(esp_timer_get_time() / DFPLAYERMINI_ARDUINO_US_PER_MS);
}

Stream::Stream(const uart_port_t uart, const uint32_t rxPin, const uint32_t txPin, const bool inverseLogic)
{
    uartConfig.baud_rate = 9600;
    uartConfig.data_bits = UART_DATA_8_BITS;
    uartConfig.parity = UART_PARITY_DISABLE;
    uartConfig.stop_bits = UART_STOP_BITS_1;
    uartConfig.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uartConfig.rx_flow_ctrl_thresh = 0U;
    uartConfig.source_clk = UART_SCLK_DEFAULT;

    setUart(uart);
    setPins(rxPin, txPin);

    if (inverseLogic)
    {
        /* TODO invert signals if necessary */
    }
}

Stream::~Stream(void)
{
    uart_driver_delete(uartNum);
}

void Stream::begin(void)
{
    ESP_ERROR_CHECK(uart_driver_install(uartNum, DFPLAYERMINI_ARDUINO_UART_BUFFER_SIZE, DFPLAYERMINI_ARDUINO_UART_BUFFER_SIZE, DFPLAYERMINI_ARDUINO_UART_QUEUE_SIZE, &uartQueueHandle, DFPLAYERMINI_ARDUINO_ESP_INTR_FLAG_DEFAULT));
    ESP_ERROR_CHECK(uart_param_config(uartNum, &uartConfig));
    ESP_ERROR_CHECK(uart_set_pin(uartNum, uartTxPin, uartRxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    xTaskCreate(DFPlayerMini_Arduino_UartEventHandlerTask, "DFPlayerMini_Arduino_UartEventHandlerTask", DFPLAYERMINI_ARDUINO_TASK_STACK_DEPTH, this, DFPLAYERMINI_ARDUINO_TASK_PRIORITY, NULL);
}

void Stream::setUart(const uart_port_t uart)
{
    uartNum = uart;
}

void Stream::setPins(const uint32_t rxPin, const uint32_t txPin)
{
    uartRxPin = rxPin;
    uartTxPin = txPin;
}

size_t Stream::available(void)
{
    size_t bytesAvailable = 0U;

    ESP_ERROR_CHECK(uart_get_buffered_data_len(uartNum, static_cast<size_t *>(&bytesAvailable)));
    vTaskDelay(DFPLAYERMINI_ARDUINO_AVAILABLE_DELAY / portTICK_PERIOD_MS);

    /* TODO replace fixed delay with variable backoff delay depending on
    how many times available is called sequentially */

    return bytesAvailable;
}

uint8_t Stream::read(void)
{
    uint8_t byteRead = 0U;

    read(&byteRead, 1U);

    return byteRead;
}

uint8_t Stream::read(uint8_t *const buffer, const size_t size)
{
    int32_t bytesRead = 0U;
    size_t uartBufferedDatalength = available();

    if (size > uartBufferedDatalength)
    {
        bytesRead = uart_read_bytes(uartNum, buffer, uartBufferedDatalength, portMAX_DELAY);
    }
    else
    {
        bytesRead = uart_read_bytes(uartNum, buffer, size, portMAX_DELAY);
    }

    if (bytesRead < 0)
    {
        bytesRead = 0U;
    }

    return (uint8_t)bytesRead;
}

size_t Stream::write(const uint8_t *const buffer, const size_t size)
{
    return uart_write_bytes(uartNum, buffer, size);
}

uart_port_t Stream::getUartNum(void)
{
    return uartNum;
}

QueueHandle_t Stream::getUartQueueHandle(void)
{
    return uartQueueHandle;
}

static void DFPlayerMini_Arduino_UartEventHandlerTask(void *arg)
{
    Stream *stream = static_cast<Stream *>(arg);
    uart_event_t event;

    for (;;)
    {
        if (xQueueReceive(stream->getUartQueueHandle(), static_cast<void *>(&event), (TickType_t)portMAX_DELAY))
        {
            switch (event.type)
            {
            case UART_FIFO_OVF:
                uart_flush_input(stream->getUartNum());
                xQueueReset(stream->getUartQueueHandle());
                break;
            case UART_BUFFER_FULL:
                uart_flush_input(stream->getUartNum());
                xQueueReset(stream->getUartQueueHandle());
                break;
            default:
                break;
            }
        }
    }

    vTaskDelete(NULL);
}
