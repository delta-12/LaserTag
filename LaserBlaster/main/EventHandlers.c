/**
 * @file EventHandlers.c
 *
 * @brief Collection of handlers for various events.
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "BopItCommands.h"
#include "EventHandlers.h"
#include "Rmt.h"

/* Defines
 ******************************************************************************/

#define EVENTHANDLERS_SEMPHR_BLOCK_TIME (1U / portTICK_PERIOD_MS) /* Wait for up to 1ms when taking mutex */

/* Function Prototypes
 ******************************************************************************/

void EventHandlers_TriggerEventHandler(void);
void EventHandlers_PrimeEventHandler(void);
void EventHandlers_ReloadEventHandler(void);

/* Function Definitions
 ******************************************************************************/

/**
 * @brief Perform initialization needed for event handlers.  Must be called
 * before calling any other functions in module.  Initializes RMT module.
 ******************************************************************************/
void EventHandlers_Init(void)
{
    Rmt_TxInit();
}

/**
 * @brief Handle a button event.  Calls event handler corresponding to button
 * that produced the event.
 *
 * @param[in] gpioNum GPIO number of the button that produced the event
 ******************************************************************************/
void EventHandlers_ButtonEventHandler(const Gpio_GpioNum_t gpioNum)
{
    switch (gpioNum)
    {
    case GPIO_BUTTON_TRIGGER:
        EventHandlers_TriggerEventHandler();
        break;
    case GPIO_BUTTON_PRIME:
        EventHandlers_PrimeEventHandler();
        break;
    case GPIO_BUTTON_2:
        EventHandlers_ReloadEventHandler();
        break;
    default:
        break;
    }
}

/**
 * @brief Update Trigger input flag to indicate Trigger was pressed.
 *
 * @note BopItCommands must have been initialized to create the mutex and RMT
 * TX channel.
 ******************************************************************************/
void EventHandlers_TriggerEventHandler(void)
{
    static uint8_t data[] = {0xAAU, 0xBBU, 0xEEU, 0xFFU, 0x00, 0x11, 0x22, 0x33, 0x44};
    Rmt_Transmit(data, sizeof(data));

    if (xSemaphoreTake(BopItCommands_TriggerInputFlagMutex, EVENTHANDLERS_SEMPHR_BLOCK_TIME) == pdTRUE)
    {
        BopItCommands_TriggerInputFlag = true;
        xSemaphoreGive(BopItCommands_TriggerInputFlagMutex);
    }
}

/**
 * @brief Update Prime input flag to indicate Prime was done.
 *
 * @note BopItCommands must have been initialized to create the mutex.
 ******************************************************************************/
void EventHandlers_PrimeEventHandler(void)
{
    if (xSemaphoreTake(BopItCommands_PrimeInputFlagMutex, EVENTHANDLERS_SEMPHR_BLOCK_TIME) == pdTRUE)
    {
        BopItCommands_PrimeInputFlag = true;
        xSemaphoreGive(BopItCommands_PrimeInputFlagMutex);
    }
}

/**
 * @brief Update Reload input flag to indicate Reload was done.
 *
 * @note BopItCommands must have been initialized to create the mutex.
 ******************************************************************************/
void EventHandlers_ReloadEventHandler(void)
{
    if (xSemaphoreTake(BopItCommands_ReloadInputFlagMutex, EVENTHANDLERS_SEMPHR_BLOCK_TIME) == pdTRUE)
    {
        BopItCommands_ReloadInputFlag = true;
        xSemaphoreGive(BopItCommands_ReloadInputFlagMutex);
    }
}
