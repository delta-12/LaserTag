#include "BopIt.h"
#include "BopItCommands.h"
#include "DFPlayerMini.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "EventHandlers.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Gpio.h"
#include <stdio.h>

#define BOPIT_COMMAND_COUNT 3U
#define BOPIT_RUN_DELAY 10U
#define US_PER_MS 1000ULL
#define UART_RX_PIN GPIO_NUM_16
#define UART_TX_PIN GPIO_NUM_17

static const char *BopItTag = "BopIt";

static BopIt_Command_t *BopItCommands[BOPIT_COMMAND_COUNT] = {&BopItCommands_Button0, &BopItCommands_Button1, &BopItCommands_Button2};

static void BopItLogger(const char *const message);
static BopIt_TimeMs_t BopItTime(void);

void app_main(void)
{
    Gpio_Init();

    Gpio_RegisterEventHandler(GPIO_TYPE_BUTTON, EventHandlers_ButtonEventHandler);

    BopIt_GameContext_t bopItGameContext = {
        .Commands = BopItCommands,
        .CommandCount = BOPIT_COMMAND_COUNT,
        .OnGameStart = NULL,
        .OnGameEnd = NULL,
    };

    BopIt_RegisterLogger(BopItLogger);
    BopIt_RegisterTime(BopItTime);
    BopIt_Init(&bopItGameContext);

    BopItCommands_Init();

    while (bopItGameContext.GameState != BOPIT_GAMESTATE_END)
    {
        BopIt_Run(&bopItGameContext);
        vTaskDelay(BOPIT_RUN_DELAY / portTICK_PERIOD_MS);
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
