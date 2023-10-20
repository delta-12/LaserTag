/**
 * @file BopItCommands.c
 *
 * @brief Commands for BopIt game.
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "BopItCommands.h"
#include "esp_log.h"
#include <stddef.h>

/* Defines
 ******************************************************************************/

#define BOPITCOMMANDS_SEMPHR_BLOCK_TIME 0U

/* Globals
 ******************************************************************************/

static const char *BopItCommands_EspLogTag = "BopItCommands";

bool BopItCommands_Button0InputFlag = false;
SemaphoreHandle_t BopItCommands_Button0InputFlagMutex = NULL;
StaticSemaphore_t BopItCommands_Button0InputFlagMutexBuffer;
BopIt_Command_t BopItCommands_Button0 = {
    .Name = "Button 0 Command",
    .IssueCommand = BopItCommands_Button0IssueCommand,
    .SuccessFeedback = BopItCommands_Button0SuccessFeedback,
    .FailFeedback = BopItCommands_Button0FailFeedback,
    .GetInput = BopItCommands_Button0GetInput,
};

bool BopItCommands_Button1InputFlag = false;
SemaphoreHandle_t BopItCommands_Button1InputFlagMutex = NULL;
StaticSemaphore_t BopItCommands_Button1InputFlagMutexBuffer;
BopIt_Command_t BopItCommands_Button1 = {
    .Name = "Button 0 Command",
    .IssueCommand = BopItCommands_Button1IssueCommand,
    .SuccessFeedback = BopItCommands_Button1SuccessFeedback,
    .FailFeedback = BopItCommands_Button1FailFeedback,
    .GetInput = BopItCommands_Button1GetInput,
};

bool BopItCommands_Button2InputFlag = false;
SemaphoreHandle_t BopItCommands_Button2InputFlagMutex = NULL;
StaticSemaphore_t BopItCommands_Button2InputFlagMutexBuffer;
BopIt_Command_t BopItCommands_Button2 = {
    .Name = "Button 0 Command",
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

/* Must be called before calling any other functions in module */
void BopItCommands_Init(void)
{
    BopItCommands_Button0InputFlagMutex = xSemaphoreCreateMutexStatic(&BopItCommands_Button0InputFlagMutexBuffer);
    BopItCommands_Button1InputFlagMutex = xSemaphoreCreateMutexStatic(&BopItCommands_Button1InputFlagMutexBuffer);
    BopItCommands_Button2InputFlagMutex = xSemaphoreCreateMutexStatic(&BopItCommands_Button2InputFlagMutexBuffer);
}

void BopItCommands_Button0IssueCommand(void)
{
    BopItCommands_ResetInputFlags();
    ESP_LOGI(BopItCommands_EspLogTag, "Press Button 0");
}

void BopItCommands_Button0SuccessFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Successfully pressed Button 0");
}

void BopItCommands_Button0FailFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Failed to pressed Button 0");
}

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

void BopItCommands_Button1IssueCommand(void)
{
    BopItCommands_ResetInputFlags();
    ESP_LOGI(BopItCommands_EspLogTag, "Press Button 1");
}

void BopItCommands_Button1SuccessFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Successfully pressed Button 1");
}

void BopItCommands_Button1FailFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Successfully pressed Button 1");
}

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

void BopItCommands_Button2IssueCommand(void)
{
    BopItCommands_ResetInputFlags();
    ESP_LOGI(BopItCommands_EspLogTag, "Press Button 2");
}

void BopItCommands_Button2SuccessFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Successfully pressed Button 2");
}

void BopItCommands_Button2FailFeedback(void)
{
    ESP_LOGI(BopItCommands_EspLogTag, "Successfully pressed Button 2");
}

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
