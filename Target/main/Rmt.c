/**
 * @file Rmt.c
 *
 * @brief Manage RMT peripherals.
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "driver/gpio.h"
#include "driver/rmt_rx.h"
#include "driver/rmt_tx.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "IrEncoder.h"
#include "Rmt.h"

/* Defines
 ******************************************************************************/

#define RMT_GPIO_IR_RECEIVER GPIO_NUM_4          /* GPIO pin with IR receiver */
#define RMT_RX_EVENT_QUEUE_SIZE 10U              /* Size of RX event queue */
#define RMT_MEM_BLOCK_SYMBOLS 128U               /* Size of RMT channel memory block */
#define RMT_RX_BUFFER_SIZE RMT_MEM_BLOCK_SYMBOLS /* Size of buffer to receive raw RMT symbols */
#define RMT_RX_DECODE_BUFFER_SIZE 1024U          /* Size of buffer for decoding received RMT messages */
#define RMT_ESP_INTR_FLAG_DEFAULT 0U             /* Default to allocating a non-shared interrupt of level 1, 2 or 3 */
#define RMT_TASK_STACK_DEPTH 2048U               /* Stack depth for Rmt RTOS tasks */
#define RMT_TASK_PRIORITY 10U                    /* Priority for Rmt RTOS tasks */
/* TODO Adjust signal ranges for Laser Tag signals */
#define RMT_RX_SIGNAL_RANGE_MIN_NS 1250U     /* Shortest duration for received signal valid not to be treated as noise */
#define RMT_RX_SIGNAL_RANGE_MAX_NS 12000000U /* Longest duration for the receive not be stopped early */
#define RMT_GPIO_IR_TRANSMITTER GPIO_NUM_15  /* GPIO pin with IR transmitter */
#define RMT_CLK_RESOLUTION_HZ 1000000U       /* 1MHz resolution, 1 tick = 1us */
#define RMT_TRANS_QUEUE_DEPTH 4U             /* Number of RMT transactions that can pend in the background */
#define RMT_CARRIER_DUTY_CYCLE 0.33F         /* 33% duty cycle */
#define RMT_FREQUENCY_HZ 40000               /* 40KHz carrier */
#define RMT_IR_ENCODER_RESOLUTION 1000000U   /* 1MHz IR encoder resolution, 1 tick = 1us*/
#define RMT_TRANSMIT_LOOP_COUNT 0U           /* No loop, loop transmit feature not supported */

/* Globals
 ******************************************************************************/

static rmt_channel_handle_t Rmt_IrRxChannelHandle = NULL;                                          /* Handle for RMT RX channel */
static StaticQueue_t Rmt_RxEventQueue;                                                             /* Queue for storing RMT RX events */
static uint8_t Rmt_RxEventQueueBuffer[RMT_RX_EVENT_QUEUE_SIZE * sizeof(rmt_rx_done_event_data_t)]; /* Statically allocated buffer for RMT RX done event queue's storage area */
static QueueHandle_t Rmt_RxEventQueueHandle = NULL;                                                /* Handle for queue storing RMT RX events */
static rmt_symbol_word_t Rmt_RxBuffer[RMT_RX_BUFFER_SIZE];                                         /* Buffer to receive RMT symbol words */
static uint8_t Rmt_RxDecodeBuffer[RMT_RX_DECODE_BUFFER_SIZE];                                      /* Buffer to store bytes from decoded RMT symbols */
static Rmt_RxEventHandler_t Rmt_RxEventHandler = NULL;                                             /* RMT RX done event handler registered by client, not be called directly */

/* Configuration for the mininum and maximum range for receiving IR signals */
static rmt_receive_config_t Rmt_RxConfig = {
    .signal_range_min_ns = RMT_RX_SIGNAL_RANGE_MIN_NS,
    .signal_range_max_ns = RMT_RX_SIGNAL_RANGE_MAX_NS,
};

static rmt_channel_handle_t Rmt_IrTxChannelHandle = NULL; /* Handle for RMT TX channel */
static rmt_encoder_handle_t Rmt_IrEncoderHandle = NULL;   /* RMT handle for IR encoder used when transmitting */
static IrEncoder_t Rmt_IrEncoder;                         /* IR encoder used when transmitting */

/* Function Prototypes
 ******************************************************************************/

static bool Rmt_RxDoneCallback(rmt_channel_handle_t rxChannelHandle, const rmt_rx_done_event_data_t *const rmtEventData, void *arg);
static void Rmt_RxEventHandlerTask(void *arg);

/* Function Definitions
 ******************************************************************************/

/**
 * @brief Initialize all required RMT peripherals for receiving IR data.
 ******************************************************************************/
void Rmt_RxInit(void)
{
    /* Configure RX channel */
    rmt_rx_channel_config_t rxChannelConfig = {
        .clk_src = RMT_CLK_SRC_DEFAULT,             /* Set source clock */
        .gpio_num = RMT_GPIO_IR_RECEIVER,           /* GPIO number of receiver */
        .mem_block_symbols = RMT_MEM_BLOCK_SYMBOLS, /* Memory block size */
        .resolution_hz = RMT_CLK_RESOLUTION_HZ,     /* Tick resolution */
        .flags.invert_in = false,                   /* Do not invert output signal */
        .flags.with_dma = false,                    /* DMA backend not supported */
    };
    ESP_ERROR_CHECK(rmt_new_rx_channel(&rxChannelConfig, &Rmt_IrRxChannelHandle));

    /* Register callback to handle RX done events */
    rmt_rx_event_callbacks_t rxEventCallbacks = {
        .on_recv_done = Rmt_RxDoneCallback, /* Set callback for “receive-done” event */
    };
    ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(Rmt_IrRxChannelHandle, &rxEventCallbacks, &Rmt_RxEventQueueHandle));

    /* Enable RMT RX channel */
    ESP_ERROR_CHECK(rmt_enable(Rmt_IrRxChannelHandle));

    /* Create a queue to store RX events from ISR */
    Rmt_RxEventQueueHandle = xQueueCreateStatic(RMT_RX_EVENT_QUEUE_SIZE, sizeof(rmt_rx_done_event_data_t), Rmt_RxEventQueueBuffer, &Rmt_RxEventQueue);

    /* Start task to handle events in the queue */
    xTaskCreate(Rmt_RxEventHandlerTask, "Rmt_RxEventHandlerTask", RMT_TASK_STACK_DEPTH, NULL, RMT_TASK_PRIORITY, NULL);
}

/**
 * @brief Initialize all required RMT peripherals for transmitting IR data.
 ******************************************************************************/
void Rmt_TxInit(void)
{
    /* Configure TX channel */
    rmt_tx_channel_config_t txChannelConfig = {
        .clk_src = RMT_CLK_SRC_DEFAULT,             /* Set source clock */
        .gpio_num = RMT_GPIO_IR_TRANSMITTER,        /* GPIO number of transmitter */
        .mem_block_symbols = RMT_MEM_BLOCK_SYMBOLS, /* Memory block size */
        .resolution_hz = RMT_CLK_RESOLUTION_HZ,     /* Tick resolution */
        .trans_queue_depth = RMT_TRANS_QUEUE_DEPTH, /* Number of transactions that can pend in the background */
        .flags.invert_out = false,                  /* Do not invert output signal */
        .flags.with_dma = false,                    /* DMA backend not supported */
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&txChannelConfig, &Rmt_IrTxChannelHandle));

    /* Modulate carrier to TX channel */
    rmt_carrier_config_t txCarrierConfig = {
        .duty_cycle = RMT_CARRIER_DUTY_CYCLE, /* Duty cycle */
        .frequency_hz = RMT_FREQUENCY_HZ,     /* Carrier frequency */
        .flags.polarity_active_low = false,   /* Carrier modulation level */
        .flags.always_on = false,             /* Set whether carrier is output after transmission finished */
    };
    ESP_ERROR_CHECK(rmt_apply_carrier(Rmt_IrTxChannelHandle, &txCarrierConfig));

    /* New RMT encoder */
    ESP_ERROR_CHECK(IrEncoder_InitRmtEncoder(&Rmt_IrEncoderHandle, &Rmt_IrEncoder, RMT_IR_ENCODER_RESOLUTION));

    /* Enable RMT TX channel */
    ESP_ERROR_CHECK(rmt_enable(Rmt_IrTxChannelHandle));
}

/**
 * @brief Register handler for data from RMT receive events.
 *
 * @param[in] rxEventHandler Handler for RMT receive event data
 ******************************************************************************/
void Rmt_RegisterRxEventHandler(Rmt_RxEventHandler_t rxEventHandler)
{
    if (rxEventHandler != NULL)
    {
        Rmt_RxEventHandler = rxEventHandler;
    }
}

/**
 * @brief Transmit IR data.
 *
 * @param[in] data Pointer to bytes to transmit
 * @param[in] size Number of bytes to transmit
 *
 * @return esp_err_t Result of RMT transmit
 ******************************************************************************/
esp_err_t Rmt_Transmit(const uint8_t *const data, const size_t size)
{
    rmt_transmit_config_t transmitConfig = {
        .loop_count = RMT_TRANSMIT_LOOP_COUNT,
    };

    return rmt_transmit(Rmt_IrTxChannelHandle, Rmt_IrEncoderHandle, data, size, &transmitConfig);
}

/**
 * @brief Callback for RMT RX done event.
 *
 * @param[in] data Pointer to bytes to transmit
 * @param[in] size Number of bytes to transmit
 *
 * @return bool Whether a high priority task has been woken up by this
 *              function
 *
 * @retval true  A high priority task has been woken up by this function
 * @retval false A high priority task has not been woken up by this function
 ******************************************************************************/
static bool IRAM_ATTR Rmt_RxDoneCallback(rmt_channel_handle_t rxChannelHandle, const rmt_rx_done_event_data_t *const rmtEventData, void *arg)
{
    BaseType_t highTaskWakeup = pdFALSE;
    QueueHandle_t rxEventQueueHandle = *(QueueHandle_t *)arg;

    xQueueSendFromISR(rxEventQueueHandle, rmtEventData, &highTaskWakeup);

    return highTaskWakeup == pdTRUE;
}

/**
 * @brief Task to handle RMT RX done events in the RX event queue.
 *
 * @param[in] arg UNUSED
 ******************************************************************************/
static void Rmt_RxEventHandlerTask(void *arg)
{
    rmt_rx_done_event_data_t rxData;
    size_t decodedDataSize;

    /* Start receive */
    ESP_ERROR_CHECK(rmt_receive(Rmt_IrRxChannelHandle, Rmt_RxBuffer, sizeof(Rmt_RxBuffer), &Rmt_RxConfig));

    for (;;)
    {
        if (xQueueReceive(Rmt_RxEventQueueHandle, &rxData, portMAX_DELAY))
        {
            /* Decode RMT symbols */
            decodedDataSize = IrEncoder_RmtDecode(rxData.received_symbols, rxData.num_symbols, Rmt_RxDecodeBuffer, sizeof(Rmt_RxDecodeBuffer));
            if (decodedDataSize > 0U && Rmt_RxEventHandler != NULL)
            {
                (*Rmt_RxEventHandler)(Rmt_RxDecodeBuffer, decodedDataSize);
            }

            /* Start receive again */
            ESP_ERROR_CHECK(rmt_receive(Rmt_IrRxChannelHandle, Rmt_RxBuffer, sizeof(Rmt_RxBuffer), &Rmt_RxConfig));
        }
    }

    vTaskDelete(NULL);
}
