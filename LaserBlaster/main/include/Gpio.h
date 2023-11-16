/**
 * @file Gpio.h
 *
 * @brief Manage GPIO peripherals.
 *
 ******************************************************************************/

#ifndef GPIO_H
#define GPIO_H

/* Includes
 ******************************************************************************/
#include "driver/gpio.h"
#include <stdint.h>

/* Defines
 ******************************************************************************/

#define GPIO_BUTTON_TRIGGER GPIO_NUM_18
#define GPIO_BUTTON_PRIME GPIO_NUM_5
#define GPIO_BUTTON_PIN_SEL ((1UL << GPIO_BUTTON_TRIGGER) | (1UL << GPIO_BUTTON_PRIME))

/* Typedefs
 ******************************************************************************/

typedef uint32_t Gpio_GpioNum_t;                                   /* GPIO number */
typedef void (*Gpio_EventHandler_t)(const Gpio_GpioNum_t gpioNum); /* GPIO event handler */

typedef enum
{
    GPIO_TYPE_BUTTON, /* GPIO input for buttons */
} Gpio_Type_t;        /* Type of physical device or sensor connected to GPIO */

/* Function Prototypes
 ******************************************************************************/

void Gpio_Init(void);
void Gpio_RegisterEventHandler(const Gpio_Type_t gpioType, Gpio_EventHandler_t eventHandler);

#endif
