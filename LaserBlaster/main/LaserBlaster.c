#include "BleCentral.h"
#include "BopIt.h"
#include "BopItCommands.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "EventHandlers.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "MenuScreens.h"
#include "ssd1306.h"
#include "nvs_flash.h"

#define BOPIT_COMMAND_COUNT 3U
#define BOPIT_RUN_DELAY_MS 10U
#define US_PER_MS 1000ULL
#define I2C_MASTER_SCL_GPIO GPIO_NUM_22 /* Gpio number for I2C master clock */
#define I2C_MASTER_SDA_GPIO GPIO_NUM_21 /* Gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1		/* I2C port number for master */
#define I2C_MASTER_FREQ_HZ 100000U		/* I2C master clock frequency */

static const char *BopItTag = "BopIt";

static BopIt_Command_t *BopItCommands[BOPIT_COMMAND_COUNT] = {&BopItCommands_Trigger, &BopItCommands_Prime, &BopItCommands_Reload};

static ssd1306_handle_t Ssd1306Handle = NULL;

static bool StartGame = false;

static bool ShotReceived = false;
static SemaphoreHandle_t ShotReceivedMutex = NULL;
static StaticSemaphore_t ShotReceivedMutexBuffer;

static void BopItLogger(const char *const message);
static BopIt_TimeMs_t BopItTime(void);

static void JoystickEventHandler(const Gpio_GpioNum_t gpioNum);

static void Ssd1306Init(void);
static void Ssd1306MainScreen(void);
static void Ssd1306WaitForBleConnectScreen(void);
static void Ssd1306StartGameScreen(void);
static void Ssd1306SInitGameScreen(void);
static void Ssd1306SetScore(const uint8_t score);
static void Ssd1306SetLives(const uint8_t lives);

static void NotifyCallback(uint8_t *const data, const size_t size);

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

    ShotReceivedMutex = xSemaphoreCreateMutexStatic(&ShotReceivedMutexBuffer);

    /* Initialize peripherals and modules */
	Gpio_Init();
	Ssd1306Init();
	BopItCommands_Init();
	EventHandlers_Init();
    BleCentral_Init();

	Gpio_RegisterEventHandler(GPIO_TYPE_BUTTON, EventHandlers_ButtonEventHandler);
	Gpio_RegisterEventHandler(GPIO_TYPE_JOYSTICK, JoystickEventHandler);

    BleCentral_RegisterNotifyCallback(NotifyCallback);

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
	Ssd1306MainScreen();
	Ssd1306StartGameScreen();
	while (!StartGame)
	{
		vTaskDelay(BOPIT_RUN_DELAY_MS / portTICK_PERIOD_MS);
	}

	Ssd1306SInitGameScreen();
	while (bopItGameContext.GameState != BOPIT_GAMESTATE_END)
	{
		BopIt_Run(&bopItGameContext);

        /* TODO check if trigger was pressed and state is success, then wait for shot be received in remaining time to complete command */
        if (bopItGameContext.CurrentCommand == &BopItCommands_Trigger && bopItGameContext.GameState == BOPIT_GAMESTATE_SUCCESS)
        {
            bool shotReceived = false;
            while ((BopItTime() - bopItGameContext.WaitStart) < bopItGameContext.WaitTime && !shotReceived && BleCentral_IsConnected())
            {
                /* TODO check for shot received */
                shotReceived = true;

                if (xSemaphoreTake(ShotReceivedMutex, portMAX_DELAY) == pdTRUE)
                {
                    shotReceived = ShotReceived;
                    ShotReceived = false;
                    xSemaphoreGive(ShotReceivedMutex);
                }

                vTaskDelay(BOPIT_RUN_DELAY_MS / portTICK_PERIOD_MS);
            }

            if (!shotReceived)
            {
                bopItGameContext.GameState = BOPIT_GAMESTATE_FAIL;
            }
        }

		ssd1306_clear_screen(Ssd1306Handle, 0x00U);
		Ssd1306SetScore(bopItGameContext.Score);
		Ssd1306SetLives(bopItGameContext.Lives);

		vTaskDelay(BOPIT_RUN_DELAY_MS / portTICK_PERIOD_MS);
	}
	BopIt_Run(&bopItGameContext);
	Ssd1306SetScore(bopItGameContext.Score);
	Ssd1306SetLives(bopItGameContext.Lives);
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

/**
 * @brief Handle a joystick event.
 *
 * @param[in] gpioNum GPIO number of the joystick input that produced the event
 ******************************************************************************/
void JoystickEventHandler(const Gpio_GpioNum_t gpioNum)
{
	switch (gpioNum)
	{
	case GPIO_JOYSTICK_UP:
		break;
	case GPIO_JOYSTICK_DOWN:
		break;
	case GPIO_JOYSTICK_LEFT:
		break;
	case GPIO_JOYSTICK_RIGHT:
		break;
	case GPIO_JOYSTICK_CENTER:
		StartGame = true;
		break;
	default:
		break;
	}
}

static void Ssd1306Init(void)
{
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = I2C_MASTER_SDA_GPIO;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = I2C_MASTER_SCL_GPIO;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
	conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

	i2c_param_config(I2C_MASTER_NUM, &conf);
	i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0U, 0U, 0U);

	Ssd1306Handle = ssd1306_create(I2C_MASTER_NUM, SSD1306_I2C_ADDRESS);
	ssd1306_refresh_gram(Ssd1306Handle);
	ssd1306_clear_screen(Ssd1306Handle, 0x00U);
}

static void Ssd1306MainScreen(void)
{
	ssd1306_clear_screen(Ssd1306Handle, 0x00U);

	ssd1306_draw_bitmap(Ssd1306Handle, 0, 0, &mainscreen[0], 128, 64);
	ssd1306_refresh_gram(Ssd1306Handle);

	vTaskDelay(5000U / portTICK_PERIOD_MS);
}

static void Ssd1306WaitForBleConnectScreen(void)
{
	ssd1306_clear_screen(Ssd1306Handle, 0x00U);

	ssd1306_draw_bitmap(Ssd1306Handle, 0, 0, &waitforBTconnect[0], 128, 64);
	ssd1306_refresh_gram(Ssd1306Handle);
}

static void Ssd1306StartGameScreen(void)
{
	ssd1306_clear_screen(Ssd1306Handle, 0x00U);

	ssd1306_draw_bitmap(Ssd1306Handle, 0, 0, &StartGameScreen[0], 128, 64);
	ssd1306_refresh_gram(Ssd1306Handle);
}

static void Ssd1306SInitGameScreen(void)
{
	ssd1306_clear_screen(Ssd1306Handle, 0x00U);

	ssd1306_draw_bitmap(Ssd1306Handle, 108, 4, &fullheart[0], 16, 16);
	ssd1306_draw_bitmap(Ssd1306Handle, 108, 24, &fullheart[0], 16, 16);
	ssd1306_draw_bitmap(Ssd1306Handle, 108, 44, &fullheart[0], 16, 16);
	ssd1306_draw_bitmap(Ssd1306Handle, 30, 4, &zero[0], 32, 56);
	ssd1306_draw_bitmap(Ssd1306Handle, 66, 4, &zero[0], 32, 56);

	ssd1306_refresh_gram(Ssd1306Handle);
}

static void Ssd1306SetScore(const uint8_t score)
{
	uint8_t tens = score / 10;
	uint8_t ones = score % 10;

	switch (tens)
	{
	case 0U:
		ssd1306_draw_bitmap(Ssd1306Handle, 30, 4, &zero[0], 32, 56);
		break;
	case 1U:
		ssd1306_draw_bitmap(Ssd1306Handle, 30, 4, &one[0], 32, 56);
		break;
	case 2U:
		ssd1306_draw_bitmap(Ssd1306Handle, 30, 4, &two[0], 32, 56);
		break;
	case 3U:
		ssd1306_draw_bitmap(Ssd1306Handle, 30, 4, &three[0], 32, 56);
		break;
	case 4U:
		ssd1306_draw_bitmap(Ssd1306Handle, 30, 4, &four[0], 32, 56);
		break;
	case 5U:
		ssd1306_draw_bitmap(Ssd1306Handle, 30, 4, &five[0], 32, 56);
		break;
	case 6U:
		ssd1306_draw_bitmap(Ssd1306Handle, 30, 4, &six[0], 32, 56);
		break;
	case 7U:
		ssd1306_draw_bitmap(Ssd1306Handle, 30, 4, &seven[0], 32, 56);
		break;
	case 8U:
		ssd1306_draw_bitmap(Ssd1306Handle, 30, 4, &eight[0], 32, 56);
		break;
	case 9U:
		ssd1306_draw_bitmap(Ssd1306Handle, 30, 4, &nine[0], 32, 56);
		break;
	default:
		break;
	}

	switch (ones)
	{
	case 0U:
		ssd1306_draw_bitmap(Ssd1306Handle, 66, 4, &zero[0], 32, 56);
		break;
	case 1U:
		ssd1306_draw_bitmap(Ssd1306Handle, 66, 4, &one[0], 32, 56);
		break;
	case 2U:
		ssd1306_draw_bitmap(Ssd1306Handle, 66, 4, &two[0], 32, 56);
		break;
	case 3U:
		ssd1306_draw_bitmap(Ssd1306Handle, 66, 4, &three[0], 32, 56);
		break;
	case 4U:
		ssd1306_draw_bitmap(Ssd1306Handle, 66, 4, &four[0], 32, 56);
		break;
	case 5U:
		ssd1306_draw_bitmap(Ssd1306Handle, 66, 4, &five[0], 32, 56);
		break;
	case 6U:
		ssd1306_draw_bitmap(Ssd1306Handle, 66, 4, &six[0], 32, 56);
		break;
	case 7U:
		ssd1306_draw_bitmap(Ssd1306Handle, 66, 4, &seven[0], 32, 56);
		break;
	case 8U:
		ssd1306_draw_bitmap(Ssd1306Handle, 66, 4, &eight[0], 32, 56);
		break;
	case 9U:
		ssd1306_draw_bitmap(Ssd1306Handle, 66, 4, &nine[0], 32, 56);
		break;
	default:
		break;
	}

	ssd1306_refresh_gram(Ssd1306Handle);
}

static void Ssd1306SetLives(const uint8_t lives)
{
	switch (lives)
	{
	case 0U:
		ssd1306_draw_bitmap(Ssd1306Handle, 108, 4, &brokenheart[0], 16, 16);
		ssd1306_draw_bitmap(Ssd1306Handle, 108, 24, &brokenheart[0], 16, 16);
		ssd1306_draw_bitmap(Ssd1306Handle, 108, 44, &brokenheart[0], 16, 16);
		break;
	case 1U:
		ssd1306_draw_bitmap(Ssd1306Handle, 108, 4, &fullheart[0], 16, 16);
		ssd1306_draw_bitmap(Ssd1306Handle, 108, 24, &brokenheart[0], 16, 16);
		ssd1306_draw_bitmap(Ssd1306Handle, 108, 44, &brokenheart[0], 16, 16);
		break;
	case 2U:
		ssd1306_draw_bitmap(Ssd1306Handle, 108, 4, &fullheart[0], 16, 16);
		ssd1306_draw_bitmap(Ssd1306Handle, 108, 24, &fullheart[0], 16, 16);
		ssd1306_draw_bitmap(Ssd1306Handle, 108, 44, &brokenheart[0], 16, 16);
		break;
	case 3U:
		ssd1306_draw_bitmap(Ssd1306Handle, 108, 4, &fullheart[0], 16, 16);
		ssd1306_draw_bitmap(Ssd1306Handle, 108, 24, &fullheart[0], 16, 16);
		ssd1306_draw_bitmap(Ssd1306Handle, 108, 44, &fullheart[0], 16, 16);
		break;
	default:
		break;
	}

	ssd1306_refresh_gram(Ssd1306Handle);
}

static void NotifyCallback(uint8_t *const data, const size_t size)
{
    for (size_t i = 0U; i < size; i++)
    {
        printf("%02x ", data[i]);
    }
    printf("\n");

    if (xSemaphoreTake(ShotReceivedMutex, portMAX_DELAY) == pdTRUE)
    {
        ShotReceived = true;
        xSemaphoreGive(ShotReceivedMutex);
    }
}