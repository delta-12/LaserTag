/**
 * @file BopItCommands.c
 *
 * @brief Commands for BopIt game.
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "BopItCommands.h"
#include "DFPlayerMini.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "Neopixel.h"

/* Defines
 ******************************************************************************/

#define BOPITCOMMANDS_SEMPHR_BLOCK_TIME 0U       /* Do not wait if mutex cannot be taken */
#define BOPITCOMMANDS_UART_NUM UART_NUM_2        /* Use UART 2 for DFPlayerMini */
#define BOPITCOMMANDS_UART_RX_PIN GPIO_NUM_16    /* GPIO to use for UART RX */
#define BOPITCOMMANDS_UART_TX_PIN GPIO_NUM_17    /* GPIO to use for UART RX */
#define BOPITCOMMANDS_PLAYERMINI_IS_ACK true     /* Enable ACK when initializing DFPlayerMini*/
#define BOPITCOMMANDS_PLAYERMINI_DO_RESET true   /* Perform a reset when initializing DFPlayerMini*/
#define BOPITCOMMANDS_PLAYERMINI_VOLUME 30U      /* DFPlayerMini volume */
#define BOPITCOMMANDS_PLAYERMINI_BEGIN_FILE 1U   /* DFPlayerMini file to play on initialization */
#define BOPITCOMMANDS_PLAYERMINI_SUCCESS_FILE 3U /* DFPlayerMini file to play for success feedback */
#define BOPITCOMMANDS_PLAYERMINI_FAIL_FILE 2U    /* DFPlayerMini file to play for fail feedback */
#define BOPITCOMMANDS_NEOPIXEL_PIN GPIO_NUM_15   /* GPIO pin for neopixel strip */
#define BOPITCOMMANDS_NEOPIXEL_COUNT 6U          /* Neopixel strip with 6 pixels */

/* Globals
 ******************************************************************************/

static const char *BopItCommands_EspLogTag = "BopItCommands"; /* Tag for logging from BopItCommands module */

static void *BopItCommands_PlayerMini = NULL; /* Handle for DFPlayerMini */

static uint8_t BopItCommands_NeopixelBuffer[NEOPIXEL_PIXEL_BUFFER_SIZE(BOPITCOMMANDS_NEOPIXEL_COUNT)]; /* Buffer for storing neopixel channel code data */
static Neopixel_Strip_t BopItCommands_NeopixelStrip; /* Neopixel strip for visual feedback */

bool BopItCommands_Button0InputFlag = false;                  /* Indicates if Button 0 was pressed */
SemaphoreHandle_t BopItCommands_Button0InputFlagMutex = NULL; /* Mutex for Button 0 flag */
StaticSemaphore_t BopItCommands_Button0InputFlagMutexBuffer;  /* Buffer to store mutex for Button 0 flag */

/* BopIt command for Button 0 */
BopIt_Command_t BopItCommands_Button0 = {
    .Name = "Button 0 Command",
    .IssueCommand = BopItCommands_Button0IssueCommand,
    .SuccessFeedback = BopItCommands_Button0SuccessFeedback,
    .FailFeedback = BopItCommands_Button0FailFeedback,
    .GetInput = BopItCommands_Button0GetInput,
};

bool BopItCommands_Button1InputFlag = false;                  /* Indicates if Button 1 was pressed */
SemaphoreHandle_t BopItCommands_Button1InputFlagMutex = NULL; /* Mutex for Button 1 flag */
StaticSemaphore_t BopItCommands_Button1InputFlagMutexBuffer;  /* Buffer to store mutex for Button 1 flag */

/* BopIt command for Button 1 */
BopIt_Command_t BopItCommands_Button1 = {
    .Name = "Button 1 Command",
    .IssueCommand = BopItCommands_Button1IssueCommand,
    .SuccessFeedback = BopItCommands_Button1SuccessFeedback,
    .FailFeedback = BopItCommands_Button1FailFeedback,
    .GetInput = BopItCommands_Button1GetInput,
};

bool BopItCommands_Button2InputFlag = false;                  /* Indicates if Button 2 was pressed */
SemaphoreHandle_t BopItCommands_Button2InputFlagMutex = NULL; /* Mutex for Button 2 flag */
StaticSemaphore_t BopItCommands_Button2InputFlagMutexBuffer;  /* Buffer to store mutex for Button 2 flag */

/* BopIt command for Button 2 */
BopIt_Command_t BopItCommands_Button2 = {
    .Name = "Button 2 Command",
    .IssueCommand = BopItCommands_Button2IssueCommand,
    .SuccessFeedback = BopItCommands_Button2SuccessFeedback,
    .FailFeedback = BopItCommands_Button2FailFeedback,
    .GetInput = BopItCommands_Button2GetInput,
};

/* Function Prototypes
 ******************************************************************************/

static void BopItCommands_ResetInputFlags(void);

/* Function Definitions
 ******************************************************************************/

/**
 * @brief Perform initialization needed for BopIt commands.  Must be called
 * before calling any other functions in module.  Creates mutexes for button
 * event flags and initializes DFPlayerMini and neopixel strip.
 ******************************************************************************/
void BopItCommands_Init(void)
{
    BopItCommands_Button0InputFlagMutex = xSemaphoreCreateMutexStatic(&BopItCommands_Button0InputFlagMutexBuffer);
    BopItCommands_Button1InputFlagMutex = xSemaphoreCreateMutexStatic(&BopItCommands_Button1InputFlagMutexBuffer);
    BopItCommands_Button2InputFlagMutex = xSemaphoreCreateMutexStatic(&BopItCommands_Button2InputFlagMutexBuffer);

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
}

/**
 * @brief Perform deinitialization needed for BopIt commands.  Frees handle
 * for DFPlayerMini and clears neopixel strip.
 ******************************************************************************/
void BopItCommands_DeInit(void)
{
    if (BopItCommands_PlayerMini != NULL)
    {
        DFPlayerMini_FreeHandle(BopItCommands_PlayerMini);
        Neopixel_Clear(&BopItCommands_NeopixelStrip);
        Neopixel_Show(&BopItCommands_NeopixelStrip);
    }
}

/**
 * @brief Issue command to press Button 0.
 ******************************************************************************/
void BopItCommands_Button0IssueCommand(void)
{
    BopItCommands_ResetInputFlags();
    ESP_LOGI(BopItCommands_EspLogTag, "Press Button 0");
}

/**
 * @brief Provide feedback to indicate Button 0 was successfully pressed.
 ******************************************************************************/
void BopItCommands_Button0SuccessFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Successfully pressed Button 0");
    DFPlayerMini_Play(BopItCommands_PlayerMini, BOPITCOMMANDS_PLAYERMINI_SUCCESS_FILE);
    Neopixel_FillColorName(&BopItCommands_NeopixelStrip, NEOPIXEL_COLORNAME_GREEN);
    Neopixel_Show(&BopItCommands_NeopixelStrip);
}

/**
 * @brief Provide feedback to indicate failure to press Button 0.
 ******************************************************************************/
void BopItCommands_Button0FailFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Failed to pressed Button 0");
    DFPlayerMini_Play(BopItCommands_PlayerMini, BOPITCOMMANDS_PLAYERMINI_FAIL_FILE);
    Neopixel_FillColorName(&BopItCommands_NeopixelStrip, NEOPIXEL_COLORNAME_RED);
    Neopixel_Show(&BopItCommands_NeopixelStrip);
}

/**
 * @brief Check if Button 0 was pressed.
 *
 * @return Whether Button 0 was pressed or not
 *
 * @retval true Button 0 was pressed
 * @retval false Button 0 was not pressed
 ******************************************************************************/
bool BopItCommands_Button0GetInput(void)
{
    bool input = false;

    if (xSemaphoreTake(BopItCommands_Button0InputFlagMutex, BOPITCOMMANDS_SEMPHR_BLOCK_TIME) == pdTRUE)
    {
        input = BopItCommands_Button0InputFlag;
        BopItCommands_Button0InputFlag = false;
        xSemaphoreGive(BopItCommands_Button0InputFlagMutex);
    }

    return input;
}

/**
 * @brief Issue command to press Button 1.
 ******************************************************************************/
void BopItCommands_Button1IssueCommand(void)
{
    BopItCommands_ResetInputFlags();
    ESP_LOGI(BopItCommands_EspLogTag, "Press Button 1");
}

/**
 * @brief Provide feedback to indicate Button 1 was successfully pressed.
 ******************************************************************************/
void BopItCommands_Button1SuccessFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Successfully pressed Button 1");
    DFPlayerMini_Play(BopItCommands_PlayerMini, BOPITCOMMANDS_PLAYERMINI_SUCCESS_FILE);
    Neopixel_FillColorName(&BopItCommands_NeopixelStrip, NEOPIXEL_COLORNAME_GREEN);
    Neopixel_Show(&BopItCommands_NeopixelStrip);
}

/**
 * @brief Provide feedback to indicate failure to press Button 1.
 ******************************************************************************/
void BopItCommands_Button1FailFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Failed to pressed Button 1");
    DFPlayerMini_Play(BopItCommands_PlayerMini, BOPITCOMMANDS_PLAYERMINI_FAIL_FILE);
    Neopixel_FillColorName(&BopItCommands_NeopixelStrip, NEOPIXEL_COLORNAME_RED);
    Neopixel_Show(&BopItCommands_NeopixelStrip);
}

/**
 * @brief Check if Button 1 was pressed.
 *
 * @return Whether Button 1 was pressed or not
 *
 * @retval true Button 1 was pressed
 * @retval false Button 1 was not pressed
 ******************************************************************************/
bool BopItCommands_Button1GetInput(void)
{
    bool input = false;

    if (xSemaphoreTake(BopItCommands_Button1InputFlagMutex, BOPITCOMMANDS_SEMPHR_BLOCK_TIME) == pdTRUE)
    {
        input = BopItCommands_Button1InputFlag;
        BopItCommands_Button1InputFlag = false;
        xSemaphoreGive(BopItCommands_Button1InputFlagMutex);
    }

    return input;
}

/**
 * @brief Issue command to press Button 2.
 ******************************************************************************/
void BopItCommands_Button2IssueCommand(void)
{
    BopItCommands_ResetInputFlags();
    ESP_LOGI(BopItCommands_EspLogTag, "Press Button 2");
}

/**
 * @brief Provide feedback to indicate Button 2 was successfully pressed.
 ******************************************************************************/
void BopItCommands_Button2SuccessFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Successfully pressed Button 2");
    DFPlayerMini_Play(BopItCommands_PlayerMini, BOPITCOMMANDS_PLAYERMINI_SUCCESS_FILE);
    Neopixel_FillColorName(&BopItCommands_NeopixelStrip, NEOPIXEL_COLORNAME_GREEN);
    Neopixel_Show(&BopItCommands_NeopixelStrip);
}

/**
 * @brief Provide feedback to indicate failure to press Button 2.
 ******************************************************************************/
void BopItCommands_Button2FailFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Failed to pressed Button 2");
    DFPlayerMini_Play(BopItCommands_PlayerMini, BOPITCOMMANDS_PLAYERMINI_FAIL_FILE);
    Neopixel_FillColorName(&BopItCommands_NeopixelStrip, NEOPIXEL_COLORNAME_RED);
    Neopixel_Show(&BopItCommands_NeopixelStrip);
}

/**
 * @brief Check if Button 2 was pressed.
 *
 * @return Whether Button 2 was pressed or not
 *
 * @retval true Button 2 was pressed
 * @retval false Button 2 was not pressed
 ******************************************************************************/
bool BopItCommands_Button2GetInput(void)
{
    bool input = false;

    if (xSemaphoreTake(BopItCommands_Button2InputFlagMutex, BOPITCOMMANDS_SEMPHR_BLOCK_TIME) == pdTRUE)
    {
        input = BopItCommands_Button2InputFlag;
        BopItCommands_Button2InputFlag = false;
        xSemaphoreGive(BopItCommands_Button2InputFlagMutex);
    }

    return input;
}

/**
 * @brief Reset flags for all inputs.  Flags will indicate all inputs were not
 * triggered after reset.
 ******************************************************************************/
static void BopItCommands_ResetInputFlags(void)
{
    if (xSemaphoreTake(BopItCommands_Button0InputFlagMutex, portMAX_DELAY) == pdTRUE)
    {
        BopItCommands_Button0InputFlag = false;
        xSemaphoreGive(BopItCommands_Button0InputFlagMutex);
    }

    if (xSemaphoreTake(BopItCommands_Button1InputFlagMutex, portMAX_DELAY) == pdTRUE)
    {
        BopItCommands_Button1InputFlag = false;
        xSemaphoreGive(BopItCommands_Button1InputFlagMutex);
    }

    if (xSemaphoreTake(BopItCommands_Button2InputFlagMutex, portMAX_DELAY) == pdTRUE)
    {
        BopItCommands_Button2InputFlag = false;
        xSemaphoreGive(BopItCommands_Button2InputFlagMutex);
    }
}
