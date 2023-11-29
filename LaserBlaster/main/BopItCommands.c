/**
 * @file BopItCommands.c
 *
 * @brief Commands for BopIt game.
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "Adc.h"
#include "BopItCommands.h"
#include "DFPlayerMini.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "Neopixel.h"

/* Defines
 ******************************************************************************/

#define BOPITCOMMANDS_SEMPHR_BLOCK_TIME 0U            /* Do not wait if mutex cannot be taken */
#define BOPITCOMMANDS_UART_NUM UART_NUM_2             /* Use UART 2 for DFPlayerMini */
#define BOPITCOMMANDS_UART_RX_PIN GPIO_NUM_23         /* GPIO to use for UART RX */
#define BOPITCOMMANDS_UART_TX_PIN GPIO_NUM_19         /* GPIO to use for UART RX */
#define BOPITCOMMANDS_PLAYERMINI_IS_ACK true          /* Enable ACK when initializing DFPlayerMini*/
#define BOPITCOMMANDS_PLAYERMINI_DO_RESET true        /* Perform a reset when initializing DFPlayerMini*/
#define BOPITCOMMANDS_PLAYERMINI_VOLUME 30U           /* DFPlayerMini volume */
#define BOPITCOMMANDS_PLAYERMINI_BEGIN_FILE 1U        /* DFPlayerMini file to play on initialization */
#define BOPITCOMMANDS_PLAYERMINI_SUCCESS_FILE 3U      /* DFPlayerMini file to play for success feedback */
#define BOPITCOMMANDS_PLAYERMINI_FAIL_FILE 2U         /* DFPlayerMini file to play for fail feedback */
#define BOPITCOMMANDS_NEOPIXEL_PIN GPIO_NUM_4         /* GPIO pin for neopixel strip */
#define BOPITCOMMANDS_NEOPIXEL_COUNT 6U               /* Neopixel strip with 6 pixels */
#define BOPITCOMMANDS_CLIP0_VOLTAGE_LOWER_BOUND 2950U /* Lower bound for voltage that should be read for clip 0 */
#define BOPITCOMMANDS_CLIP0_VOLTAGE_UPPER_BOUND 3050U /* Upper bound for voltage that should be read for clip 0 */
#define BOPITCOMMANDS_CLIP1_VOLTAGE_LOWER_BOUND 1500U /* Lower bound for voltage that should be read for clip 1 */
#define BOPITCOMMANDS_CLIP1_VOLTAGE_UPPER_BOUND 1800U /* Upper bound for voltage that should be read for clip 1 */
#define BOPITCOMMANDS_FEEDBACK_DELAY_MS 1000U         /* 1000ms delay after providing feedback */

/* Typedefs
 ******************************************************************************/

typedef enum
{
    BOPITCOMMANDS_CLIPNUMBER_0,         /* Clip 0 inserted */
    BOPITCOMMANDS_CLIPNUMBER_1,         /* Clip 1 inserted */
    BOPITCOMMANDS_CLIPNUMBER_UNDEFINED, /* Unknown clip inserted */
} BopItCommands_ClipNumber_t;           /* Number for each clip */

/* Globals
 ******************************************************************************/

static const char *BopItCommands_EspLogTag = "BopItCommands"; /* Tag for logging from BopItCommands module */

static void *BopItCommands_PlayerMini = NULL; /* Handle for DFPlayerMini */

static uint8_t BopItCommands_NeopixelBuffer[NEOPIXEL_PIXEL_BUFFER_SIZE(BOPITCOMMANDS_NEOPIXEL_COUNT)]; /* Buffer for storing neopixel channel code data */
static Neopixel_Strip_t BopItCommands_NeopixelStrip;                                                   /* Neopixel strip for visual feedback */

bool BopItCommands_TriggerInputFlag = false;                  /* Indicates if Trigger was pressed */
SemaphoreHandle_t BopItCommands_TriggerInputFlagMutex = NULL; /* Mutex for Trigger flag */
StaticSemaphore_t BopItCommands_TriggerInputFlagMutexBuffer;  /* Buffer to store mutex for Trigger flag */

bool BopItCommands_TriggerCommandIssuedFlag = false;                  /* Indicates if Trigger command was issued */
SemaphoreHandle_t BopItCommands_TriggerCommandIssuedFlagMutex = NULL; /* Mutex for Trigger Command Issued flag */
StaticSemaphore_t BopItCommands_TriggerCommandIssuedFlagMutexBuffer;  /* Buffer to store mutex for Trigger Command Issued flag */

/* BopIt command for Trigger */
BopIt_Command_t BopItCommands_Trigger = {
    .Name = "Trigger Command",
    .IssueCommand = BopItCommands_TriggerIssueCommand,
    .SuccessFeedback = BopItCommands_TriggerSuccessFeedback,
    .FailFeedback = BopItCommands_TriggerFailFeedback,
    .GetInput = BopItCommands_TriggerGetInput,
};

bool BopItCommands_PrimeInputFlag = false;                  /* Indicates if Prime was done */
SemaphoreHandle_t BopItCommands_PrimeInputFlagMutex = NULL; /* Mutex for Prime flag */
StaticSemaphore_t BopItCommands_PrimeInputFlagMutexBuffer;  /* Buffer to store mutex for Prime flag */

/* BopIt command for Prime */
BopIt_Command_t BopItCommands_Prime = {
    .Name = "Prime Command",
    .IssueCommand = BopItCommands_PrimeIssueCommand,
    .SuccessFeedback = BopItCommands_PrimeSuccessFeedback,
    .FailFeedback = BopItCommands_PrimeFailFeedback,
    .GetInput = BopItCommands_PrimeGetInput,
};

static BopItCommands_ClipNumber_t BopItCommands_InsertedClip = BOPITCOMMANDS_CLIPNUMBER_0; /* Clip currently inserted */

/* BopIt command for Reload */
BopIt_Command_t BopItCommands_Reload = {
    .Name = "Reload Command",
    .IssueCommand = BopItCommands_ReloadIssueCommand,
    .SuccessFeedback = BopItCommands_ReloadSuccessFeedback,
    .FailFeedback = BopItCommands_ReloadFailFeedback,
    .GetInput = BopItCommands_ReloadGetInput,
};

/* Function Prototypes
 ******************************************************************************/

static void BopItCommands_SuccessFeedback(void);
static void BopItCommands_FailFeedback(void);
static void BopItCommands_ResetInputFlags(void);

/* Function Definitions
 ******************************************************************************/

/**
 * @brief Perform initialization needed for BopIt commands.  Must be called
 * before calling any other functions in module.  Creates mutexes for button
 * event flags and initializes DFPlayerMini, neopixel strip, and ADC.
 ******************************************************************************/
void BopItCommands_Init(void)
{
    BopItCommands_TriggerInputFlagMutex = xSemaphoreCreateMutexStatic(&BopItCommands_TriggerInputFlagMutexBuffer);
    BopItCommands_TriggerCommandIssuedFlagMutex = xSemaphoreCreateMutexStatic(&BopItCommands_TriggerCommandIssuedFlagMutexBuffer);
    BopItCommands_PrimeInputFlagMutex = xSemaphoreCreateMutexStatic(&BopItCommands_PrimeInputFlagMutexBuffer);

    BopItCommands_PlayerMini = DFPlayerMini_CreateHandle(BOPITCOMMANDS_UART_NUM, BOPITCOMMANDS_UART_RX_PIN, BOPITCOMMANDS_UART_TX_PIN);
    if (!DFPlayerMini_Begin(BopItCommands_PlayerMini, BOPITCOMMANDS_PLAYERMINI_IS_ACK, BOPITCOMMANDS_PLAYERMINI_DO_RESET))
    {
        ESP_LOGE(BopItCommands_EspLogTag, "DFPlayerMini failed to begin.");
    }
    else
    {
        DFPlayerMini_Volume(BopItCommands_PlayerMini, BOPITCOMMANDS_PLAYERMINI_VOLUME);
        DFPlayerMini_Play(BopItCommands_PlayerMini, BOPITCOMMANDS_PLAYERMINI_BEGIN_FILE);
    }

    Neopixel_Init(&BopItCommands_NeopixelStrip, BopItCommands_NeopixelBuffer, BOPITCOMMANDS_NEOPIXEL_COUNT, BOPITCOMMANDS_NEOPIXEL_PIN);

    Adc_Init();
}

/**
 * @brief Perform deinitialization needed for BopIt commands.  Frees handle
 * for DFPlayerMini, clears neopixel strip, and deinitializes ADC.
 ******************************************************************************/
void BopItCommands_DeInit(void)
{
    if (BopItCommands_PlayerMini != NULL)
    {
        DFPlayerMini_FreeHandle(BopItCommands_PlayerMini);
    }

    Neopixel_Clear(&BopItCommands_NeopixelStrip);
    Neopixel_Show(&BopItCommands_NeopixelStrip);

    Adc_DeInit();
}

/**
 * @brief Issue command to press Trigger.
 ******************************************************************************/
void BopItCommands_TriggerIssueCommand(void)
{
    BopItCommands_ResetInputFlags();

    if (xSemaphoreTake(BopItCommands_TriggerCommandIssuedFlagMutex, portMAX_DELAY) == pdTRUE)
    {
        BopItCommands_TriggerCommandIssuedFlag = true;
        xSemaphoreGive(BopItCommands_TriggerCommandIssuedFlagMutex);
    }

    ESP_LOGI(BopItCommands_EspLogTag, "Press Trigger");
}

/**
 * @brief Provide feedback to indicate Trigger was successfully pressed.
 ******************************************************************************/
void BopItCommands_TriggerSuccessFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Successfully pressed Trigger");
    BopItCommands_SuccessFeedback();
}

/**
 * @brief Provide feedback to indicate failure to press Trigger.
 ******************************************************************************/
void BopItCommands_TriggerFailFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Failed to pressed Trigger");
    BopItCommands_FailFeedback();
}

/**
 * @brief Check if Trigger was pressed.
 *
 * @return bool Whether Trigger was pressed or not
 *
 * @retval true  Trigger was pressed
 * @retval false Trigger was not pressed
 ******************************************************************************/
bool BopItCommands_TriggerGetInput(void)
{
    bool input = false;

    if (xSemaphoreTake(BopItCommands_TriggerInputFlagMutex, BOPITCOMMANDS_SEMPHR_BLOCK_TIME) == pdTRUE)
    {
        input = BopItCommands_TriggerInputFlag;
        BopItCommands_TriggerInputFlag = false;
        xSemaphoreGive(BopItCommands_TriggerInputFlagMutex);
    }

    return input;
}

/**
 * @brief Issue command to Prime.
 ******************************************************************************/
void BopItCommands_PrimeIssueCommand(void)
{
    BopItCommands_ResetInputFlags();
    ESP_LOGI(BopItCommands_EspLogTag, "Prime the blaster");
}

/**
 * @brief Provide feedback to indicate Prime was successfully done.
 ******************************************************************************/
void BopItCommands_PrimeSuccessFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Successfully Primed");
    BopItCommands_SuccessFeedback();
}

/**
 * @brief Provide feedback to indicate failure to Prime.
 ******************************************************************************/
void BopItCommands_PrimeFailFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Failed to Prime");
    BopItCommands_FailFeedback();
}

/**
 * @brief Check if Prime was done.
 *
 * @return bool Whether Prime was done or not
 *
 * @retval true  Prime was done
 * @retval false Prime was not done
 ******************************************************************************/
bool BopItCommands_PrimeGetInput(void)
{
    bool input = false;

    if (xSemaphoreTake(BopItCommands_PrimeInputFlagMutex, BOPITCOMMANDS_SEMPHR_BLOCK_TIME) == pdTRUE)
    {
        input = BopItCommands_PrimeInputFlag;
        BopItCommands_PrimeInputFlag = false;
        xSemaphoreGive(BopItCommands_PrimeInputFlagMutex);
    }

    return input;
}

/**
 * @brief Issue command to Reload.
 ******************************************************************************/
void BopItCommands_ReloadIssueCommand(void)
{
    BopItCommands_ResetInputFlags();
    ESP_LOGI(BopItCommands_EspLogTag, "Reload the blaster");
}

/**
 * @brief Provide feedback to indicate Reload was successfully done.
 ******************************************************************************/
void BopItCommands_ReloadSuccessFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Successfully Reloaded");
    BopItCommands_SuccessFeedback();
}

/**
 * @brief Provide feedback to indicate failure to Reload.
 ******************************************************************************/
void BopItCommands_ReloadFailFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Failed to Reload");
    BopItCommands_FailFeedback();
}

/**
 * @brief Check if Reload was done.
 *
 * @return bool Whether Reload was done or not
 *
 * @retval true  Reload was done
 * @retval false Reload was not done
 ******************************************************************************/
bool BopItCommands_ReloadGetInput(void)
{
    bool input = false;
    int voltage = 0U;
    BopItCommands_ClipNumber_t insertedClip = BOPITCOMMANDS_CLIPNUMBER_UNDEFINED;

    if (Adc_OneshotRead(&voltage) == ESP_OK)
    {
        if (voltage >= BOPITCOMMANDS_CLIP0_VOLTAGE_LOWER_BOUND && voltage < BOPITCOMMANDS_CLIP0_VOLTAGE_UPPER_BOUND)
        {
            insertedClip = BOPITCOMMANDS_CLIPNUMBER_0;
        }
        else if (voltage >= BOPITCOMMANDS_CLIP1_VOLTAGE_LOWER_BOUND && voltage < BOPITCOMMANDS_CLIP1_VOLTAGE_UPPER_BOUND)
        {
            insertedClip = BOPITCOMMANDS_CLIPNUMBER_1;
        }

        if (insertedClip != BOPITCOMMANDS_CLIPNUMBER_UNDEFINED && insertedClip != BopItCommands_InsertedClip)
        {
            BopItCommands_InsertedClip = insertedClip;
            input = true;
        }
    }

    return input;
}

/**
 * @brief Provide feedback to indicate command was successfully completed.
 * Common to all BopIt commands.
 ******************************************************************************/
static void BopItCommands_SuccessFeedback(void)
{
    DFPlayerMini_Play(BopItCommands_PlayerMini, BOPITCOMMANDS_PLAYERMINI_SUCCESS_FILE);
    Neopixel_FillColorName(&BopItCommands_NeopixelStrip, NEOPIXEL_COLORNAME_GREEN);
    Neopixel_Show(&BopItCommands_NeopixelStrip);
    vTaskDelay(BOPITCOMMANDS_FEEDBACK_DELAY_MS / portTICK_PERIOD_MS);
    Neopixel_Clear(&BopItCommands_NeopixelStrip);
    Neopixel_Show(&BopItCommands_NeopixelStrip);
}

/**
 * @brief Provide feedback to indicate command was not successfully completed.
 * Common to all BopIt commands.
 ******************************************************************************/
static void BopItCommands_FailFeedback(void)
{
    DFPlayerMini_Play(BopItCommands_PlayerMini, BOPITCOMMANDS_PLAYERMINI_FAIL_FILE);
    Neopixel_FillColorName(&BopItCommands_NeopixelStrip, NEOPIXEL_COLORNAME_RED);
    Neopixel_Show(&BopItCommands_NeopixelStrip);
    vTaskDelay(BOPITCOMMANDS_FEEDBACK_DELAY_MS / portTICK_PERIOD_MS);
    Neopixel_Clear(&BopItCommands_NeopixelStrip);
    Neopixel_Show(&BopItCommands_NeopixelStrip);
}

/**
 * @brief Reset flags for all inputs.  Flags will indicate all inputs were not
 * triggered after reset.
 ******************************************************************************/
static void BopItCommands_ResetInputFlags(void)
{
    if (xSemaphoreTake(BopItCommands_TriggerInputFlagMutex, portMAX_DELAY) == pdTRUE)
    {
        BopItCommands_TriggerInputFlag = false;
        xSemaphoreGive(BopItCommands_TriggerInputFlagMutex);
    }

    if (xSemaphoreTake(BopItCommands_TriggerCommandIssuedFlagMutex, portMAX_DELAY) == pdTRUE)
    {
        BopItCommands_TriggerCommandIssuedFlag = false;
        xSemaphoreGive(BopItCommands_TriggerCommandIssuedFlagMutex);
    }

    if (xSemaphoreTake(BopItCommands_PrimeInputFlagMutex, portMAX_DELAY) == pdTRUE)
    {
        BopItCommands_PrimeInputFlag = false;
        xSemaphoreGive(BopItCommands_PrimeInputFlagMutex);
    }
}
