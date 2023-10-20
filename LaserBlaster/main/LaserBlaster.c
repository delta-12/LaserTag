#include "Gpio.h"
#include <stdio.h>

static void ButtonEventHandler(const Gpio_GpioNum_t gpioNum);

void app_main(void)
{
    Gpio_Init();

    Gpio_RegisterEventHandler(GPIO_TYPE_BUTTON, ButtonEventHandler);
}

static void ButtonEventHandler(const Gpio_GpioNum_t gpioNum)
{
    printf("Button %ld pressed.\n", gpioNum);
}
