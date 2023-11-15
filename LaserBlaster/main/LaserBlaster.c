#include "BopIt.h"
#include "BopItCommands.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "EventHandlers.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Rmt.h"

#define BOPIT_COMMAND_COUNT 3U
#define BOPIT_RUN_DELAY 10U
#define US_PER_MS 1000ULL

static const char *BopItTag = "BopIt";

static BopIt_Command_t *BopItCommands[BOPIT_COMMAND_COUNT] = {&BopItCommands_Trigger, &BopItCommands_Prime, &BopItCommands_Reload};

static void BopItLogger(const char *const message);
static BopIt_TimeMs_t BopItTime(void);
static void TestParser(const uint8_t *const buffer, const size_t size);

void app_main(void)
{
    Gpio_Init();
    BopItCommands_Init();
    EventHandlers_Init();

    Gpio_RegisterEventHandler(GPIO_TYPE_BUTTON, EventHandlers_ButtonEventHandler);

    // Rmt_RxInit();
    // Rmt_RegisterRxEventHandler(TestParser);

    /* Test RMT and IR encoder */
    while (1)
    {
        // Rmt_Transmit(data, 4U);
        vTaskDelay(2000U / portTICK_PERIOD_MS);
    }

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
        vTaskDelay(BOPIT_RUN_DELAY / portTICK_PERIOD_MS);
    }
    BopIt_Run(&bopItGameContext);
    vTaskDelay(BOPIT_RUN_DELAY / portTICK_PERIOD_MS);

    BopItCommands_DeInit();
}

static void BopItLogger(const char *const message)
{
    ESP_LOGI(BopItTag, "%s", message);
}

static BopIt_TimeMs_t BopItTime(void)
{
    return (BopIt_TimeMs_t)(esp_timer_get_time() / US_PER_MS);
}

static void TestParser(const uint8_t *const buffer, const size_t size)
{
    ESP_LOGI("TEST PARSER", "Shot received!");
    for (size_t i = 0U; i < size; i++)
    {
        printf("%x ", *(buffer + i));
    }
    printf("\n");
}
