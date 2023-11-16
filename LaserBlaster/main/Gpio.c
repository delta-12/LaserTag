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

#define GPIO_EVENT_QUEUE_SIZE 10U     /* Size of event queues */
#define GPIO_ESP_INTR_FLAG_DEFAULT 0U /* Default to allocating a non-shared interrupt of level 1, 2 or 3 */
#define GPIO_TASK_STACK_DEPTH 2048U   /* Stack depth for GPIO RTOS tasks */
#define GPIO_TASK_PRIORITY 10U        /* Priority for GPIO RTOS tasks */

/* Globals
 ******************************************************************************/

static StaticQueue_t Gpio_ButtonEventQueue;                                                 /* Queue for storing button interrupt events */
static uint8_t Gpio_ButtonEventQueueBuffer[GPIO_EVENT_QUEUE_SIZE * sizeof(Gpio_GpioNum_t)]; /* Statically allocated buffer for button interrupt event queue's storage area */
static QueueHandle_t Gpio_ButtonEventQueueHandle = NULL;                                    /* Handle for queue storing button interrupt events */
static Gpio_EventHandler_t Gpio_ButtonEventHandler = NULL;                                  /* Button event handler registered by client, not be called directly */

/* Function Prototypes
 ******************************************************************************/

static void Gpio_ButtonIsrHandler(void *arg);
static void Gpio_ButtonEventHandlerTask(void *arg);
static void Gpio_RegisterButtonEventHandler(Gpio_EventHandler_t eventHandler);

/* Function Definitions
 ******************************************************************************/

/**
 * @brief Initialize all GPIO required.
 ******************************************************************************/
void Gpio_Init(void)
{
    /* Initialize button inputs with interrupt on falling edge */
    gpio_config_t buttons = {
        .pin_bit_mask = GPIO_BUTTON_PIN_SEL,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&buttons);
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
        gpio_install_isr_service(GPIO_ESP_INTR_FLAG_DEFAULT);

        /* Hook ISR handlers for specific GPIO pins */
        gpio_isr_handler_add(GPIO_BUTTON_TRIGGER, Gpio_ButtonIsrHandler, (void *)GPIO_BUTTON_TRIGGER);
        gpio_isr_handler_add(GPIO_BUTTON_PRIME, Gpio_ButtonIsrHandler, (void *)GPIO_BUTTON_PRIME);
    }
}
