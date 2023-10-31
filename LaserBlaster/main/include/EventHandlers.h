/**
 * @file EventHandlers.h
 *
 * @brief Collection of handlers for various events.
 *
 ******************************************************************************/

#ifndef EVENT_HANDLERS_H
#define EVENT_HANDLERS_H

/* Includes
 ******************************************************************************/
#include "Gpio.h"

/* Function Prototypes
 ******************************************************************************/

void EventHandlers_ButtonEventHandler(const Gpio_GpioNum_t gpioNum);

#endif
