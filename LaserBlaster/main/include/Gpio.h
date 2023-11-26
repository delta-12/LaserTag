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

#define GPIO_BUTTON_TRIGGER GPIO_NUM_18 /* GPIO pin for trigger button */
#define GPIO_BUTTON_PRIME GPIO_NUM_5    /* GPIO pin for prime button */

#define GPIO_JOYSTICK_UP GPIO_NUM_26     /* GPIO pin for joystick up */
#define GPIO_JOYSTICK_DOWN GPIO_NUM_25   /* GPIO pin for joystick down */
#define GPIO_JOYSTICK_LEFT GPIO_NUM_32   /* GPIO pin for joystick left */
#define GPIO_JOYSTICK_RIGHT GPIO_NUM_27  /* GPIO pin for joystick right */
#define GPIO_JOYSTICK_CENTER GPIO_NUM_33 /* GPIO pin for joystick center */

/* Typedefs
 ******************************************************************************/

typedef uint32_t Gpio_GpioNum_t;                                   /* GPIO number */
typedef void (*Gpio_EventHandler_t)(const Gpio_GpioNum_t gpioNum); /* GPIO event handler */

/* Type of physical device or sensor connected to GPIO */
typedef enum
{
    GPIO_TYPE_BUTTON,   /* GPIO input for buttons */
    GPIO_TYPE_JOYSTICK, /* GPIO input type for joystick */
} Gpio_Type_t;          /* Type of physical device or sensor connected to GPIO */

/* Function Prototypes
 ******************************************************************************/

void Gpio_Init(void);
void Gpio_RegisterEventHandler(const Gpio_Type_t gpioType, Gpio_EventHandler_t eventHandler);

#endif
