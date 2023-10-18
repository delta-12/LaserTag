#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <inttypes.h>
#include <stdio.h>

#define BUTTON_0_PIN GPIO_NUM_18
#define BUTTON_1_PIN GPIO_NUM_19
#define BUTTON_2_PIN GPIO_NUM_21
#define BUTTON_PIN_SEL ((1UL << BUTTON_0_PIN) | (1UL << BUTTON_1_PIN) | (1UL << BUTTON_2_PIN))

#define ESP_INTR_FLAG_DEFAULT 0

static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void *arg)
{
    uint32_t io_num;
    for (;;)
    {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
        {
            printf("GPIO[%" PRIu32 "] intr, val: %d\n", io_num, gpio_get_level(io_num));
        }
    }
}

void app_main(void)
{
    gpio_config_t buttons = {
        .pin_bit_mask = BUTTON_PIN_SEL,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&buttons);

    // create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    // start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    // install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(BUTTON_0_PIN, gpio_isr_handler, (void *)BUTTON_0_PIN);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(BUTTON_1_PIN, gpio_isr_handler, (void *)BUTTON_1_PIN);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(BUTTON_2_PIN, gpio_isr_handler, (void *)BUTTON_2_PIN);
}
