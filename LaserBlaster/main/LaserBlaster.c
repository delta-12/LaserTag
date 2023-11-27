#include "BopIt.h"
#include "BopItCommands.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "EventHandlers.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ssd1306.h"
#include "MenuScreens.h"

#define BOPIT_COMMAND_COUNT 3U
#define BOPIT_RUN_DELAY_MS 10U
#define US_PER_MS 1000ULL
#define I2C_MASTER_SCL_GPIO GPIO_NUM_22 /* Gpio number for I2C master clock */
#define I2C_MASTER_SDA_GPIO GPIO_NUM_21 /* Gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1        /* I2C port number for master */
#define I2C_MASTER_FREQ_HZ 100000U      /* I2C master clock frequency */

static const char *BopItTag = "BopIt";

static BopIt_Command_t *BopItCommands[BOPIT_COMMAND_COUNT] = {&BopItCommands_Trigger, &BopItCommands_Prime, &BopItCommands_Reload};

static ssd1306_handle_t ssd1306Handle = NULL;

static void Ssd1306Init(void);
static void Ssd1306StartGame(void);
static void Ssd1306SetScore(const uint8_t score);
static void Ssd1306SetLives(const uint8_t lives);
static void BopItLogger(const char *const message);
static BopIt_TimeMs_t BopItTime(void);

void app_main(void)
{
    Gpio_Init();
    Ssd1306Init();
    BopItCommands_Init();
    EventHandlers_Init();

    Gpio_RegisterEventHandler(GPIO_TYPE_BUTTON, EventHandlers_ButtonEventHandler);

    /* TODO Testing */
    Gpio_RegisterEventHandler(GPIO_TYPE_JOYSTICK, EventHandlers_JoystickEventHandler);


	//Main Menu Screen Held until Selected
	//while(!Joystick.Center){}
	//Ssd1306StartGame();
	Ssd1306StartGame();

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
		ssd1306_clear_screen(ssd1306Handle, 0x00U);
        Ssd1306SetScore(bopItGameContext.Score);
        Ssd1306SetLives(bopItGameContext.Lives);

        vTaskDelay(BOPIT_RUN_DELAY_MS / portTICK_PERIOD_MS);
    }
    BopIt_Run(&bopItGameContext);
    Ssd1306SetScore(bopItGameContext.Score);
    Ssd1306SetLives(bopItGameContext.Lives);
    vTaskDelay(BOPIT_RUN_DELAY_MS / portTICK_PERIOD_MS);

    BopItCommands_DeInit();
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

    ssd1306Handle = ssd1306_create(I2C_MASTER_NUM, SSD1306_I2C_ADDRESS);
    ssd1306_refresh_gram(ssd1306Handle);
    ssd1306_clear_screen(ssd1306Handle, 0x00U);
    
    //hold for 5 secs
    ssd1306_draw_bitmap(ssd1306Handle, 0, 0, &mainscreen[0], 128, 64);	
	ssd1306_refresh_gram(ssd1306Handle);
	
	vTaskDelay(5000/portTICK_PERIOD_MS);

    static char data_str[10] = {0};
    sprintf(data_str, "Score: ");
    //ssd1306_draw_string(ssd1306Handle, 0, 16, (const uint8_t *)data_str, 16, 1);
    sprintf(data_str, "Lives: ");
    //ssd1306_draw_string(ssd1306Handle, 0, 32, (const uint8_t *)data_str, 16, 1);
    ssd1306_refresh_gram(ssd1306Handle);
}

static void Ssd1306StartGame(void)
{
	ssd1306_clear_screen(ssd1306Handle, 0x00U);
	ssd1306_draw_bitmap(ssd1306Handle, 108, 4, &fullheart[0], 16, 16);
	ssd1306_draw_bitmap(ssd1306Handle, 108, 24, &fullheart[0], 16, 16);
	ssd1306_draw_bitmap(ssd1306Handle, 108, 44, &fullheart[0], 16, 16);
	
	ssd1306_draw_bitmap(ssd1306Handle, 30, 4, &zero[0], 32, 56);
	ssd1306_draw_bitmap(ssd1306Handle, 66, 4, &zero[0], 32, 56);
	
	ssd1306_refresh_gram(ssd1306Handle);
}

static void Ssd1306SetScore(const uint8_t score)
{
    static char data_str[10] = {0};
    itoa(score, data_str, 10);
    
    //Dummy var for testing screen score
    //uint8_t scores = 97;
    
    uint8_t tens = score/10;
    uint8_t ones = score % 10;
    
    switch(tens){
		case 0:
			ssd1306_draw_bitmap(ssd1306Handle, 30, 4, &zero[0], 32, 56);
			break;
		case 1:
			ssd1306_draw_bitmap(ssd1306Handle, 30, 4, &one[0], 32, 56);
			break;
		case 2:
			ssd1306_draw_bitmap(ssd1306Handle, 30, 4, &two[0], 32, 56);
			break;
		case 3:
			ssd1306_draw_bitmap(ssd1306Handle, 30, 4, &three[0], 32, 56);
			break;
		case 4:
			ssd1306_draw_bitmap(ssd1306Handle, 30, 4, &four[0], 32, 56);
			break;
		case 5:
			ssd1306_draw_bitmap(ssd1306Handle, 30, 4, &five[0], 32, 56);
			break;
		case 6:
			ssd1306_draw_bitmap(ssd1306Handle, 30, 4, &six[0], 32, 56);
			break;
		case 7:
			ssd1306_draw_bitmap(ssd1306Handle, 30, 4, &seven[0], 32, 56);
			break;
		case 8:
			ssd1306_draw_bitmap(ssd1306Handle, 30, 4, &eight[0], 32, 56);
			break;
		case 9:
			ssd1306_draw_bitmap(ssd1306Handle, 30, 4, &nine[0], 32, 56);
			break;
	}
	
	switch(ones){
		case 0:
			ssd1306_draw_bitmap(ssd1306Handle, 66, 4, &zero[0], 32, 56);
			break;
		case 1:
			ssd1306_draw_bitmap(ssd1306Handle, 66, 4, &one[0], 32, 56);
			break;
		case 2:
			ssd1306_draw_bitmap(ssd1306Handle, 66, 4, &two[0], 32, 56);
			break;
		case 3:
			ssd1306_draw_bitmap(ssd1306Handle, 66, 4, &three[0], 32, 56);
			break;
		case 4:
			ssd1306_draw_bitmap(ssd1306Handle, 66, 4, &four[0], 32, 56);
			break;
		case 5:
			ssd1306_draw_bitmap(ssd1306Handle, 66, 4, &five[0], 32, 56);
			break;
		case 6:
			ssd1306_draw_bitmap(ssd1306Handle, 66, 4, &six[0], 32, 56);
			break;
		case 7:
			ssd1306_draw_bitmap(ssd1306Handle, 66, 4, &seven[0], 32, 56);
			break;
		case 8:
			ssd1306_draw_bitmap(ssd1306Handle, 66, 4, &eight[0], 32, 56);
			break;
		case 9:
			ssd1306_draw_bitmap(ssd1306Handle, 66, 4, &nine[0], 32, 56);
			break;
	}

    //ssd1306_draw_string(ssd1306Handle, 56, 16, (const uint8_t *)data_str, 16, 1);
    ssd1306_refresh_gram(ssd1306Handle);
}

static void Ssd1306SetLives(const uint8_t lives)
{
    static char data_str[10] = {0};
    itoa(lives, data_str, 10);
    
    if(lives == 3){
		ssd1306_draw_bitmap(ssd1306Handle, 108, 4, &fullheart[0], 16, 16);
		ssd1306_draw_bitmap(ssd1306Handle, 108, 24, &fullheart[0], 16, 16);
		ssd1306_draw_bitmap(ssd1306Handle, 108, 44, &fullheart[0], 16, 16);
	}
    
    if(lives == 2){
		ssd1306_draw_bitmap(ssd1306Handle, 108, 4, &fullheart[0], 16, 16);
		ssd1306_draw_bitmap(ssd1306Handle, 108, 24, &fullheart[0], 16, 16);
		ssd1306_draw_bitmap(ssd1306Handle, 108, 44, &brokenheart[0], 16, 16);
	}
	
	if(lives == 1){
		ssd1306_draw_bitmap(ssd1306Handle, 108, 4, &fullheart[0], 16, 16);
		ssd1306_draw_bitmap(ssd1306Handle, 108, 24, &brokenheart[0], 16, 16);
		ssd1306_draw_bitmap(ssd1306Handle, 108, 44, &brokenheart[0], 16, 16);
	}
	
	if(lives == 0){
		ssd1306_draw_bitmap(ssd1306Handle, 108, 4, &brokenheart[0], 16, 16);
		ssd1306_draw_bitmap(ssd1306Handle, 108, 24, &brokenheart[0], 16, 16);
		ssd1306_draw_bitmap(ssd1306Handle, 108, 44, &brokenheart[0], 16, 16);
	}
		
    
    //ssd1306_draw_string(ssd1306Handle, 56, 32, (const uint8_t *)data_str, 16, 1);
    ssd1306_refresh_gram(ssd1306Handle);
}

static void BopItLogger(const char *const message)
{
    ESP_LOGI(BopItTag, "%s", message);
}

static BopIt_TimeMs_t BopItTime(void)
{
    return (BopIt_TimeMs_t)(esp_timer_get_time() / US_PER_MS);
}
