/**
 * @file Gpio.c
 *
 * @brief Manage GPIO peripherals.
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "Gpio.h"

/* Defines
 ******************************************************************************/

#define GPIO_BIT 1ULL                                                                                                                                                                                         /* Bit for masking, unsigned long long for pins greater than 31 */
#define GPIO_BUTTON_PIN_SEL ((GPIO_BIT << GPIO_BUTTON_TRIGGER) | (GPIO_BIT << GPIO_BUTTON_PRIME))                                                                                                             /* Bit mask for selecting button pins  */
#define GPIO_JOYSTICK_PIN_SEL ((GPIO_BIT << GPIO_JOYSTICK_UP) | (GPIO_BIT << GPIO_JOYSTICK_DOWN) | (GPIO_BIT << GPIO_JOYSTICK_LEFT) | (GPIO_BIT << GPIO_JOYSTICK_RIGHT) | (GPIO_BIT << GPIO_JOYSTICK_CENTER)) /* Bit mask for selecting joystick pins */

#define GPIO_EVENT_QUEUE_SIZE 10U     /* Size of event queues */
#define GPIO_ESP_INTR_FLAG_DEFAULT 0U /* Default to allocating a non-shared interrupt of level 1, 2 or 3 */
#define GPIO_TASK_STACK_DEPTH 2048U   /* Stack depth for GPIO RTOS tasks */
#define GPIO_TASK_PRIORITY 10U        /* Priority for GPIO RTOS tasks */

/* Globals
 ******************************************************************************/

static bool Gpio_IsrServiceInstalled = false; /* Flag set if ISR service was installed */

static StaticQueue_t Gpio_ButtonEventQueue;                                                 /* Queue for storing button interrupt events */
static uint8_t Gpio_ButtonEventQueueBuffer[GPIO_EVENT_QUEUE_SIZE * sizeof(Gpio_GpioNum_t)]; /* Statically allocated buffer for button interrupt event queue's storage area */
static QueueHandle_t Gpio_ButtonEventQueueHandle = NULL;                                    /* Handle for queue storing button interrupt events */
static Gpio_EventHandler_t Gpio_ButtonEventHandler = NULL;                                  /* Button event handler registered by client, not be called directly */

static StaticQueue_t Gpio_JoystickEventQueue;                                                 /* Queue for storing joystick interrupt events */
static uint8_t Gpio_JoystickEventQueueBuffer[GPIO_EVENT_QUEUE_SIZE * sizeof(Gpio_GpioNum_t)]; /* Statically allocated buffer for joystick interrupt event queue's storage area */
static QueueHandle_t Gpio_JoystickEventQueueHandle = NULL;                                    /* Handle for queue storing joystick interrupt events */
static Gpio_EventHandler_t Gpio_JoystickEventHandler = NULL;                                  /* Joystick event handler registered by client, not be called directly */

/* Function Prototypes
 ******************************************************************************/

static void Gpio_ButtonIsrHandler(void *arg);
static void Gpio_ButtonEventHandlerTask(void *arg);
static void Gpio_RegisterButtonEventHandler(Gpio_EventHandler_t eventHandler);

static void Gpio_JoystickIsrHandler(void *arg);
static void Gpio_JoystickEventHandlerTask(void *arg);
static void Gpio_RegisterJoystickEventHandler(Gpio_EventHandler_t eventHandler);

/* Function Definitions
 ******************************************************************************/

/**
 * @brief Initialize all GPIO required.
 ******************************************************************************/
void Gpio_Init(void)
{
    /* Initialize button inputs with interrupt on falling edge */
    gpio_config_t buttonsGpioConfig = {
        .pin_bit_mask = GPIO_BUTTON_PIN_SEL,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&buttonsGpioConfig);

    /* Initialize joysticks inputs with interrupt on falling edge */
    gpio_config_t joystickGpioConfig = {
        .pin_bit_mask = GPIO_JOYSTICK_PIN_SEL,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&joystickGpioConfig);
}

/**
 * @brief Register handlers for different types of GPIO events.
 *
 * @param[in] gpioType     Type of GPIO to register a handler for
 * @param[in] eventHandler Handler for GPIO event
 ******************************************************************************/
void Gpio_RegisterEventHandler(const Gpio_Type_t gpioType, Gpio_EventHandler_t eventHandler)
{
    if (eventHandler != NULL)
    {
        switch (gpioType)
        {
        case GPIO_TYPE_BUTTON:
            Gpio_RegisterButtonEventHandler(eventHandler);
            break;
        case GPIO_TYPE_JOYSTICK:
            Gpio_RegisterJoystickEventHandler(eventHandler);
            break;
        default:
            break;
        }
    }
}

/**
 * @brief GPIO button ISR.  Adds button interrupt event to the button event
 * queue.
 *
 * @param[in] arg GPIO number
 ******************************************************************************/
static void IRAM_ATTR Gpio_ButtonIsrHandler(void *arg)
{
    Gpio_GpioNum_t gpioNum = (Gpio_GpioNum_t)arg;

    xQueueSendFromISR(Gpio_ButtonEventQueueHandle, &gpioNum, NULL);
}

/**
 * @brief Task to handle events in the button event queue.
 *
 * @param[in] arg GPIO number
 ******************************************************************************/
static void Gpio_ButtonEventHandlerTask(void *arg)
{
    Gpio_GpioNum_t gpioNum;

    for (;;)
    {
        if (xQueueReceive(Gpio_ButtonEventQueueHandle, &gpioNum, portMAX_DELAY))
        {
            (*Gpio_ButtonEventHandler)(gpioNum); /* Assumes Gpio_RegisterButtonEventHandler checked for NULL pointer */
        }
    }

    vTaskDelete(NULL);
}

/**
 * @brief Register a GPIO handler for button events.
 *
 * @param[in] eventHandler Handler for GPIO button events
 ******************************************************************************/
static void Gpio_RegisterButtonEventHandler(Gpio_EventHandler_t eventHandler)
{
    if (eventHandler != NULL)
    {
        /* Assign event handler */
        Gpio_ButtonEventHandler = eventHandler;

        /* Create a queue to handle GPIO event from ISR */
        Gpio_ButtonEventQueueHandle = xQueueCreateStatic(GPIO_EVENT_QUEUE_SIZE, sizeof(Gpio_GpioNum_t), Gpio_ButtonEventQueueBuffer, &Gpio_ButtonEventQueue);

        /* Start task to handle events in the queue */
        xTaskCreate(Gpio_ButtonEventHandlerTask, "Gpio_ButtonEventHandlerTask", GPIO_TASK_STACK_DEPTH, NULL, GPIO_TASK_PRIORITY, NULL);

        /* Install GPIO ISR service */
        if (!Gpio_IsrServiceInstalled)
        {
            gpio_install_isr_service(GPIO_ESP_INTR_FLAG_DEFAULT);
            Gpio_IsrServiceInstalled = true;
        }

        /* Hook ISR handlers for specific GPIO pins */
        gpio_isr_handler_add(GPIO_BUTTON_TRIGGER, Gpio_ButtonIsrHandler, (void *)GPIO_BUTTON_TRIGGER);
        gpio_isr_handler_add(GPIO_BUTTON_PRIME, Gpio_ButtonIsrHandler, (void *)GPIO_BUTTON_PRIME);
    }
}

/**
 * @brief GPIO joystick ISR.  Adds joystick interrupt event to the joystick
 * event queue.
 *
 * @param[in] arg GPIO number
 ******************************************************************************/
static void IRAM_ATTR Gpio_JoystickIsrHandler(void *arg)
{
    Gpio_GpioNum_t gpioNum = (Gpio_GpioNum_t)arg;

    xQueueSendFromISR(Gpio_JoystickEventQueueHandle, &gpioNum, NULL);
}

/**
 * @brief Task to handle events in the joystick event queue.
 *
 * @param[in] arg GPIO number
 ******************************************************************************/
static void Gpio_JoystickEventHandlerTask(void *arg)
{
    Gpio_GpioNum_t gpioNum;

    for (;;)
    {
        if (xQueueReceive(Gpio_JoystickEventQueueHandle, &gpioNum, portMAX_DELAY))
        {
            (*Gpio_JoystickEventHandler)(gpioNum); /* Assumes Gpio_RegisterJoystickEventHandler checked for NULL pointer */
        }
    }

    vTaskDelete(NULL);
}

/**
 * @brief Register a GPIO handler for joystick events.
 *
 * @param[in] eventHandler Handler for GPIO joystick events
 ******************************************************************************/
static void Gpio_RegisterJoystickEventHandler(Gpio_EventHandler_t eventHandler)
{
    if (eventHandler != NULL)
    {
        /* Assign event handler */
        Gpio_JoystickEventHandler = eventHandler;

        /* Create a queue to handle GPIO event from ISR */
        Gpio_JoystickEventQueueHandle = xQueueCreateStatic(GPIO_EVENT_QUEUE_SIZE, sizeof(Gpio_GpioNum_t), Gpio_JoystickEventQueueBuffer, &Gpio_JoystickEventQueue);

        /* Start task to handle events in the queue */
        xTaskCreate(Gpio_JoystickEventHandlerTask, "Gpio_JoystickEventHandlerTask", GPIO_TASK_STACK_DEPTH, NULL, GPIO_TASK_PRIORITY, NULL);

        /* Install GPIO ISR service */
        if (!Gpio_IsrServiceInstalled)
        {
            gpio_install_isr_service(GPIO_ESP_INTR_FLAG_DEFAULT);
            Gpio_IsrServiceInstalled = true;
        }

        /* Hook ISR handlers for specific GPIO pins */
        gpio_isr_handler_add(GPIO_JOYSTICK_UP, Gpio_JoystickIsrHandler, (void *)GPIO_JOYSTICK_UP);
        gpio_isr_handler_add(GPIO_JOYSTICK_DOWN, Gpio_JoystickIsrHandler, (void *)GPIO_JOYSTICK_DOWN);
        gpio_isr_handler_add(GPIO_JOYSTICK_LEFT, Gpio_JoystickIsrHandler, (void *)GPIO_JOYSTICK_LEFT);
        gpio_isr_handler_add(GPIO_JOYSTICK_RIGHT, Gpio_JoystickIsrHandler, (void *)GPIO_JOYSTICK_RIGHT);
        gpio_isr_handler_add(GPIO_JOYSTICK_CENTER, Gpio_JoystickIsrHandler, (void *)GPIO_JOYSTICK_CENTER);
    }
}
