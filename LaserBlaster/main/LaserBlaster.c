#include "BleCentral.h"
#include "BopIt.h"
#include "BopItCommands.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "EventHandlers.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#define BOPIT_COMMAND_COUNT 3U
#define BOPIT_RUN_DELAY_MS 10U
#define US_PER_MS 1000ULL

static const char *BopItTag = "BopIt";

static BopIt_Command_t *BopItCommands[BOPIT_COMMAND_COUNT] = {&BopItCommands_Trigger, &BopItCommands_Prime, &BopItCommands_Reload};

static void BopItLogger(const char *const message);
static BopIt_TimeMs_t BopItTime(void);

void app_main(void)
{
    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    /* Initialize peripherals and modules */
    Gpio_Init();
    BopItCommands_Init();
    EventHandlers_Init();
    BleCentral_Init();

    /* Register handlers for GPIO inputs */
    Gpio_RegisterEventHandler(GPIO_TYPE_BUTTON, EventHandlers_ButtonEventHandler);

    /* TODO Testing */
    Gpio_RegisterEventHandler(GPIO_TYPE_JOYSTICK, EventHandlers_JoystickEventHandler);

    /* Initialize BopIt game */
    BopIt_GameContext_t bopItGameContext = {
        .Commands = BopItCommands,
        .CommandCount = BOPIT_COMMAND_COUNT,
        .OnGameStart = NULL,
        .OnGameEnd = NULL,
    };
    BopIt_RegisterLogger(BopItLogger);
    BopIt_RegisterTime(BopItTime);
    BopIt_Init(&bopItGameContext);

    /* Wait for BLE connection to Target */
    while (!BleCentral_IsConnected())
    {
        vTaskDelay(BOPIT_RUN_DELAY_MS / portTICK_PERIOD_MS);
    }

    /* Run BopIt game */
    while (bopItGameContext.GameState != BOPIT_GAMESTATE_END)
    {
        BopIt_Run(&bopItGameContext);

        /* TODO check if trigger was pressed and state is success, then wait for shot be received in remaining time to complete command */
        // if (bopItGameContext.CurrentCommand == &BopItCommands_Trigger && bopItGameContext.GameState == BOPIT_GAMESTATE_SUCCESS)
        // {
        //     bool shotReceived = false;
        //     while ((BopItTime() - bopItGameContext.WaitStart) < bopItGameContext.WaitTime && !shotReceived && BleCentral_IsConnected())
        //     {
        //         /* TODO check for shot received */
        //         shotReceived = true;
        //     }

        //     if (!shotReceived)
        //     {
        //         bopItGameContext.GameState = BOPIT_GAMESTATE_FAIL;
        //     }

        // }

        vTaskDelay(BOPIT_RUN_DELAY_MS / portTICK_PERIOD_MS);
    }
    BopIt_Run(&bopItGameContext);
    vTaskDelay(BOPIT_RUN_DELAY_MS / portTICK_PERIOD_MS);

    /* De-initialization */
    BopItCommands_DeInit();
    BleCentral_DeInit();
}

static void BopItLogger(const char *const message)
{
    ESP_LOGI(BopItTag, "%s", message);
}

static BopIt_TimeMs_t BopItTime(void)
{
    return (BopIt_TimeMs_t)(esp_timer_get_time() / US_PER_MS);
}
