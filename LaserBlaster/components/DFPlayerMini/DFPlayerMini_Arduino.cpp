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
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

/* Defines
 ******************************************************************************/

#define US_PER_MS 1000ULL /* 1000 microseconds per millisecond */

/* Globals
 ******************************************************************************/

static const char *TAG = "Uart Event Handler";
static QueueHandle_t DFPlayerMini_Arduino_UartQueueHandle;
static size_t DFPlayerMini_Arduino_BytesAvailable = 0U;
SemaphoreHandle_t DFPlayerMini_Arduino_BytesAvailableMutex = NULL;
StaticSemaphore_t DFPlayerMini_Arduino_BytesAvailableMutexBuffer;

/* Function Prototypes
 ******************************************************************************/

static void DFPlayerMini_Arduino_UartEventHandlerTask(void *arg);

/* Function Definitions
 ******************************************************************************/

uint64_t millis(void)
{
    return (uint64_t)(esp_timer_get_time() / US_PER_MS);
}

// void delay(const uint16_t ms)
// {
//     vTaskDelay(ms / portTICK_PERIOD_MS);
// }

Stream::Stream(const uint32_t rxPin, const uint32_t txPin, const bool inverseLogic)
{
    uartConfig.baud_rate = 9600;
    uartConfig.data_bits = UART_DATA_8_BITS;
    uartConfig.parity = UART_PARITY_DISABLE;
    uartConfig.stop_bits = UART_STOP_BITS_1;
    uartConfig.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uartConfig.rx_flow_ctrl_thresh = 0U;
    uartConfig.source_clk = UART_SCLK_DEFAULT;

    DFPlayerMini_Arduino_BytesAvailableMutex = xSemaphoreCreateMutexStatic(&DFPlayerMini_Arduino_BytesAvailableMutexBuffer);
}

Stream::Stream(const bool inverseLogic)
{
    uartConfig.baud_rate = 9600;
    uartConfig.data_bits = UART_DATA_8_BITS;
    uartConfig.parity = UART_PARITY_DISABLE;
    uartConfig.stop_bits = UART_STOP_BITS_1;
    uartConfig.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uartConfig.rx_flow_ctrl_thresh = 0U;
    uartConfig.source_clk = UART_SCLK_DEFAULT;

    DFPlayerMini_Arduino_BytesAvailableMutex = xSemaphoreCreateMutexStatic(&DFPlayerMini_Arduino_BytesAvailableMutexBuffer);
}

Stream::~Stream(void)
{
    uart_driver_delete(uartNum);
}

void Stream::begin(void)
{
    // Install UART driver using an event queue here
    ESP_ERROR_CHECK(uart_driver_install(uartNum, uartBufferSize, uartBufferSize, 20U, &DFPlayerMini_Arduino_UartQueueHandle, 0U));
    ESP_ERROR_CHECK(uart_param_config(uartNum, &uartConfig));
    ESP_ERROR_CHECK(uart_set_pin(uartNum, uartTxPin, uartRxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    /* TODO start uart event handler task */
    xTaskCreate(DFPlayerMini_Arduino_UartEventHandlerTask, "uart_event_task", 2048, NULL, 12, NULL);
}

void Stream::setPins(const uint32_t rxPin, const uint32_t txPin)
{
    uartRxPin = rxPin;
    uartTxPin = txPin;

    ESP_ERROR_CHECK(uart_set_pin(uartNum, uartTxPin, uartRxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

size_t Stream::available(void)
{
    size_t bytesAvailable = 0U;

    if (xSemaphoreTake(DFPlayerMini_Arduino_BytesAvailableMutex, portMAX_DELAY) == pdTRUE)
    {
        bytesAvailable = DFPlayerMini_Arduino_BytesAvailable;
        xSemaphoreGive(DFPlayerMini_Arduino_BytesAvailableMutex);
    }

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
    size_t uartBufferedDatalength = 0U;

    ESP_ERROR_CHECK(uart_get_buffered_data_len(uartNum, (size_t *)&uartBufferedDatalength));

    if (size > uartBufferedDatalength)
    {
        bytesRead = uart_read_bytes(uartNum, buffer, uartBufferedDatalength, 100U);
    }
    else
    {
        bytesRead = uart_read_bytes(uartNum, buffer, size, 100U);
    }

    if (bytesRead < 0)
    {
        bytesRead = 0U;
    }

    if (xSemaphoreTake(DFPlayerMini_Arduino_BytesAvailableMutex, portMAX_DELAY) == pdTRUE)
    {
        DFPlayerMini_Arduino_BytesAvailable -= bytesRead;
        xSemaphoreGive(DFPlayerMini_Arduino_BytesAvailableMutex);
    }

    return (uint8_t)bytesRead;
}

size_t Stream::write(const uint8_t *const buffer, const size_t size)
{
    return uart_write_bytes(uartNum, buffer, size);
}

static void DFPlayerMini_Arduino_UartEventHandlerTask(void *arg)
{
    uart_event_t event;

    for (;;)
    {
        // Waiting for UART event.
        if (xQueueReceive(DFPlayerMini_Arduino_UartQueueHandle, (void *)&event, (TickType_t)portMAX_DELAY))
        {
            ESP_LOGI(TAG, "uart[%d] event:", UART_NUM_2);

            switch (event.type)
            {
            // Event of UART receving data
            /*We'd better handler data event fast, there would be much more data events than
            other types of events. If we take too much time on data event, the queue might
            be full.*/
            case UART_DATA:
                ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                if (xSemaphoreTake(DFPlayerMini_Arduino_BytesAvailableMutex, portMAX_DELAY) == pdTRUE)
                {
                    DFPlayerMini_Arduino_BytesAvailable += event.size;
                    xSemaphoreGive(DFPlayerMini_Arduino_BytesAvailableMutex);
                }
                break;
            // Event of HW FIFO overflow detected
            case UART_FIFO_OVF:
                ESP_LOGI(TAG, "hw fifo overflow");
                // If fifo overflow happened, you should consider adding flow control for your application.
                // The ISR has already reset the rx FIFO,
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(UART_NUM_2);
                xQueueReset(DFPlayerMini_Arduino_UartQueueHandle);
                break;
            // Event of UART ring buffer full
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");
                // If buffer full happened, you should consider increasing your buffer size
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(UART_NUM_2);
                xQueueReset(DFPlayerMini_Arduino_UartQueueHandle);
                break;
            // Event of UART RX break detected
            case UART_BREAK:
                ESP_LOGI(TAG, "uart rx break");
                break;
            // Event of UART parity check error
            case UART_PARITY_ERR:
                ESP_LOGI(TAG, "uart parity error");
                break;
            // Event of UART frame error
            case UART_FRAME_ERR:
                ESP_LOGI(TAG, "uart frame error");
                break;
            // UART_PATTERN_DET
            case UART_PATTERN_DET:
                ESP_LOGI(TAG, "[UART PATTERN DETECTED]");
                break;
            // Others
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }

    vTaskDelete(NULL);
}
