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
#define EVENTHANDLERS_SHOT_HEADER_HI 0x4CU                        /* Shot header HI 'L' */
#define EVENTHANDLERS_SHOT_HEADER_LO 0x54U                        /* Shot header LO 'T' */
#define EVENTHANDLERS_3_BYTE_BIT_SHIFT 24U                        /* Bit shift to get byte 3 */
#define EVENTHADNLERS_2_BYTE_BIT_SHIFT 16U                        /* Bit shift to get byte 2 */
#define EVENTHADNLERS_1_BYTE_BIT_SHIFT 8U                         /* Bit shift to get byte 1 */
#define EVENTHADNLERS_BYTE_MASK 0xFFU                             /* Mask a byte */

/* Defines
 ******************************************************************************/

static uint32_t EventHandlers_ShotCount = 0U;
static uint8_t EventHandlers_ShotData[] = {EVENTHANDLERS_SHOT_HEADER_HI, EVENTHANDLERS_SHOT_HEADER_LO, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

/* Function Prototypes
 ******************************************************************************/

void EventHandlers_TriggerEventHandler(void);
void EventHandlers_PrimeEventHandler(void);

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
    bool triggerCommandIssued = false;

    if (xSemaphoreTake(BopItCommands_TriggerCommandIssuedFlagMutex, EVENTHANDLERS_SEMPHR_BLOCK_TIME) == pdTRUE)
    {
        triggerCommandIssued = BopItCommands_TriggerCommandIssuedFlag;
        xSemaphoreGive(BopItCommands_TriggerCommandIssuedFlagMutex);
    }

    printf("Trigger cmd issued: %d\n", (uint8_t)triggerCommandIssued);

    if (triggerCommandIssued)
    {
        EventHandlers_ShotCount++;

        EventHandlers_ShotData[3U] = (EventHandlers_ShotCount << 24U) & 0xFFU;
        EventHandlers_ShotData[4U] = (EventHandlers_ShotCount << 16U) & 0xFFU;
        EventHandlers_ShotData[5U] = (EventHandlers_ShotCount << 8U) & 0xFFU;
        EventHandlers_ShotData[6U] = EventHandlers_ShotCount & 0xFFU;
    }

    Rmt_Transmit(EventHandlers_ShotData, sizeof(EventHandlers_ShotData));

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
 * @brief Handle a joystick event.
 *
 * @param[in] gpioNum GPIO number of the joystick input that produced the event
 ******************************************************************************/
void EventHandlers_JoystickEventHandler(const Gpio_GpioNum_t gpioNum)
{
    /* TODO integrate with display and menu */

    /* Stub handler */
    printf("Joystick input: ");

    switch (gpioNum)
    {
    case GPIO_JOYSTICK_UP:
        printf("UP\n");
        break;
    case GPIO_JOYSTICK_DOWN:
        printf("DOWN\n");
        break;
    case GPIO_JOYSTICK_LEFT:
        printf("LEFT\n");
        break;
    case GPIO_JOYSTICK_RIGHT:
        printf("RIGHT\n");
        break;
    case GPIO_JOYSTICK_CENTER:
        printf("CENTER\n");
        break;
    default:
        break;
    }
}
