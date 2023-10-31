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

/* Defines
 ******************************************************************************/

#define EVENTHANDLERS_SEMPHR_BLOCK_TIME (1U / portTICK_PERIOD_MS) /* Wait for up to 1ms when taking mutex */

/* Function Prototypes
 ******************************************************************************/

void EventHandlers_Button0EventHandler(void);
void EventHandlers_Button1EventHandler(void);
void EventHandlers_Button2EventHandler(void);

/* Function Definitions
 ******************************************************************************/

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
    case GPIO_BUTTON_0:
        EventHandlers_Button0EventHandler();
        break;
    case GPIO_BUTTON_1:
        EventHandlers_Button1EventHandler();
        break;
    case GPIO_BUTTON_2:
        EventHandlers_Button2EventHandler();
        break;
    default:
        break;
    }
}

/**
 * @brief Update Button 0 input flag to indicate Button 0 was pressed.
 *
 * @note BopItCommands must have been initialized to create the mutex.
 ******************************************************************************/
void EventHandlers_Button0EventHandler(void)
{
    if (xSemaphoreTake(BopItCommands_Button0InputFlagMutex, EVENTHANDLERS_SEMPHR_BLOCK_TIME) == pdTRUE)
    {
        BopItCommands_Button0InputFlag = true;
        xSemaphoreGive(BopItCommands_Button0InputFlagMutex);
    }
}

/**
 * @brief Update Button 1 input flag to indicate Button 1 was pressed.
 *
 * @note BopItCommands must have been initialized to create the mutex.
 ******************************************************************************/
void EventHandlers_Button1EventHandler(void)
{
    if (xSemaphoreTake(BopItCommands_Button1InputFlagMutex, EVENTHANDLERS_SEMPHR_BLOCK_TIME) == pdTRUE)
    {
        BopItCommands_Button1InputFlag = true;
        xSemaphoreGive(BopItCommands_Button1InputFlagMutex);
    }
}

/**
 * @brief Update Button 2 input flag to indicate Button 2 was pressed.
 *
 * @note BopItCommands must have been initialized to create the mutex.
 ******************************************************************************/
void EventHandlers_Button2EventHandler(void)
{
    if (xSemaphoreTake(BopItCommands_Button2InputFlagMutex, EVENTHANDLERS_SEMPHR_BLOCK_TIME) == pdTRUE)
    {
        BopItCommands_Button2InputFlag = true;
        xSemaphoreGive(BopItCommands_Button2InputFlagMutex);
    }
}
