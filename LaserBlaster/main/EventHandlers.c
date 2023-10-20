/**
 * @file EventHandlers.c
 *
 * @brief Collection of handlers for various events.
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
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

void EventHandlers_Button0EventHandler(void)
{
    if (xSemaphoreTake(BopItCommands_Button0InputFlagMutex, EVENTHANDLERS_SEMPHR_BLOCK_TIME) == pdTRUE)
    {
        BopItCommands_Button0InputFlag = true;
        xSemaphoreGive(BopItCommands_Button0InputFlagMutex);
    }
}

void EventHandlers_Button1EventHandler(void)
{
    if (xSemaphoreTake(BopItCommands_Button1InputFlagMutex, EVENTHANDLERS_SEMPHR_BLOCK_TIME) == pdTRUE)
    {
        BopItCommands_Button1InputFlag = true;
        xSemaphoreGive(BopItCommands_Button1InputFlagMutex);
    }
}

void EventHandlers_Button2EventHandler(void)
{
    if (xSemaphoreTake(BopItCommands_Button2InputFlagMutex, EVENTHANDLERS_SEMPHR_BLOCK_TIME) == pdTRUE)
    {
        BopItCommands_Button2InputFlag = true;
        xSemaphoreGive(BopItCommands_Button2InputFlagMutex);
    }
}
