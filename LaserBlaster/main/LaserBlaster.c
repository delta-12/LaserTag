#include "BopIt.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BOPIT_COMMAND_COUNT 3U
#define US_PER_MS 1000ULL
#define LSB 1ULL

static const char *MainTag = "Main";
static const char *BopItTag = "BopIt";

static void BopItLogger(const char *const message);
static BopIt_TimeMs_t BopItTime(void);
static void IssueCommand0(void);
static void IssueCommand1(void);
static void IssueCommand2(void);
static void SuccessFeedback(void);
static void FailFeedback(void);
static bool GetInput(void);

static BopIt_Command_t command0 = {
    .Name = "Command 0",
    .IssueCommand = IssueCommand0,
    .SuccessFeedback = SuccessFeedback,
    .FailFeedback = FailFeedback,
    .GetInput = GetInput,
};

static BopIt_Command_t command1 = {
    .Name = "Command 1",
    .IssueCommand = IssueCommand1,
    .SuccessFeedback = SuccessFeedback,
    .FailFeedback = FailFeedback,
    .GetInput = GetInput,
};

static BopIt_Command_t command2 = {
    .Name = "Command 2",
    .IssueCommand = IssueCommand2,
    .SuccessFeedback = SuccessFeedback,
    .FailFeedback = FailFeedback,
    .GetInput = GetInput,
};

static BopIt_Command_t *BopItCommands[BOPIT_COMMAND_COUNT] = {&command0, &command1, &command2};

void app_main(void)
{
    BopIt_GameContext_t bopItGameContext = {
        .Commands = BopItCommands,
        .CommandCount = BOPIT_COMMAND_COUNT,
        .OnGameStart = NULL,
        .OnGameEnd = NULL,
    };

    BopIt_RegisterLogger(BopItLogger);
    BopIt_RegisterTime(BopItTime);
    BopIt_Init(&bopItGameContext);

    while (bopItGameContext.GameState != BOPIT_GAMESTATE_END)
    {
        BopIt_Run(&bopItGameContext);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    BopIt_Run(&bopItGameContext);
}

static void BopItLogger(const char *const message)
{
    ESP_LOGI(BopItTag, "%s", message);
}

static BopIt_TimeMs_t BopItTime(void)
{
    return (BopIt_TimeMs_t)(esp_timer_get_time() / US_PER_MS);
}

static void IssueCommand0(void)
{
    ESP_LOGI(MainTag, "COMMAND 0");
}

static void IssueCommand1(void)
{
    ESP_LOGI(MainTag, "COMMAND 1");
}

static void IssueCommand2(void)
{
    ESP_LOGI(MainTag, "COMMAND 2");
}

static void SuccessFeedback(void)
{
    ESP_LOGI(MainTag, "SUCCESS");
}

static void FailFeedback(void)
{
    ESP_LOGI(MainTag, "FAIL");
}

static bool GetInput(void)
{
    return false;
}
