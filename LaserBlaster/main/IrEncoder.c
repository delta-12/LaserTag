/**
 * @file IrEncoder.c
 *
 * @brief Modified NEC RMT encoder for raw bytes.
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "freertos/FreeRTOS.h"
#include "IrEncoder.h"

/* Defines
 ******************************************************************************/

#define IRENCODER_CLOCK_SOURCE_FREQUENCY_HZ 1000000U
#define IRENCODER_NEC_LEVEL_0 1U
#define IRENCODER_NEC_LEVEL_1 0U
#define IRENCODER_NEC_LEADERCODE_SYMBOL_LENGTH 1U
#define IRENCODER_NEC_LEADERCODE_DURATION_0 9000ULL
#define IRENCODER_NEC_LEADERCODE_DURATION_1 4500ULL
#define IRENCODER_NEC_ENDINGCODE_SYMBOL_LENGTH 1U
#define IRENCODER_NEC_ENDINGCODE_DURATION_0 560ULL
#define IRENCODER_NEC_ENDINGCODE_DURATION_1 32767ULL
#define IRENCODER_NEC_CODES_SYMBOL_LENGTH (IRENCODER_NEC_LEADERCODE_SYMBOL_LENGTH + IRENCODER_NEC_ENDINGCODE_SYMBOL_LENGTH)
#define IRENCODER_NEC_BIT0_DURATION_0 560ULL
#define IRENCODER_NEC_BIT0_DURATION_1 560ULL
#define IRENCODER_NEC_BIT1_DURATION_0 560ULL
#define IRENCODER_NEC_BIT1_DURATION_1 1690ULL
#define IRENCODER_NEC_DECODE_MARGIN 200U
#define IRENCODER_BITS_PER_BYTE 8U
#define IRENCODER_NEC_BIT 1U

/* Typedefs
 ******************************************************************************/

typedef uint32_t IrEncoder_Duration_t;

/* Function Prototypes
 ******************************************************************************/

static size_t IrEncoder_RmtBaseEncoder(rmt_encoder_t *rmtEncoder, rmt_channel_handle_t txChannelHandle, const void *primaryData, size_t size, rmt_encode_state_t *encoderState);
static esp_err_t IrEncoder_RmtDeleteEncoder(rmt_encoder_t *rmtEncoder);
static esp_err_t IrEncoder_RmtResetEncoder(rmt_encoder_t *rmtEncoder);
static inline bool IrEncoder_NecCheckInRange(const IrEncoder_Duration_t signalDuration, const uint32_t specifiedDuration);
static inline bool IrEncoder_NecParseLogic0(const rmt_symbol_word_t *const rmtNecSymbols);
static inline bool IrEncoder_NecParseLogic1(const rmt_symbol_word_t *const rmtNecSymbols);

/* Function Definitions
 ******************************************************************************/

esp_err_t IrEncoder_InitRmtEncoder(rmt_encoder_handle_t *const rmtEncoderHandle, IrEncoder_t *const encoder, const IrEncoder_Resolution_t resolution)
{
    esp_err_t err = ESP_OK;

    /* Configure base encoder */
    encoder->Base.encode = IrEncoder_RmtBaseEncoder;
    encoder->Base.del = IrEncoder_RmtDeleteEncoder;
    encoder->Base.reset = IrEncoder_RmtResetEncoder;

    /* Configure copy encoder */
    rmt_copy_encoder_config_t copyEncoderConfig = {};
    err = rmt_new_copy_encoder(&copyEncoderConfig, &encoder->CopyEncoder);

    /* Configure bytes encoder */
    if (err == ESP_OK)
    {
        rmt_bytes_encoder_config_t bytesEncoderConfig = {
            .bit0 = {
                .level0 = IRENCODER_NEC_LEVEL_0,
                .duration0 = IRENCODER_NEC_BIT0_DURATION_0 * resolution / IRENCODER_CLOCK_SOURCE_FREQUENCY_HZ, // T0H=560us
                .level1 = IRENCODER_NEC_LEVEL_1,
                .duration1 = IRENCODER_NEC_BIT0_DURATION_1 * resolution / IRENCODER_CLOCK_SOURCE_FREQUENCY_HZ, // T0L=560us
            },
            .bit1 = {
                .level0 = IRENCODER_NEC_LEVEL_0,
                .duration0 = IRENCODER_NEC_BIT1_DURATION_0 * resolution / IRENCODER_CLOCK_SOURCE_FREQUENCY_HZ, // T1H=560us
                .level1 = IRENCODER_NEC_LEVEL_1,
                .duration1 = IRENCODER_NEC_BIT1_DURATION_1 * resolution / IRENCODER_CLOCK_SOURCE_FREQUENCY_HZ, // T1L=1690us
            },
        };
        err = rmt_new_bytes_encoder(&bytesEncoderConfig, &encoder->BytesEncoder);
    }

    /* Configure NEC leader coder */
    encoder->NecLeaderCode = (rmt_symbol_word_t){
        .level0 = IRENCODER_NEC_LEVEL_0,
        .duration0 = IRENCODER_NEC_LEADERCODE_DURATION_0 * resolution / IRENCODER_CLOCK_SOURCE_FREQUENCY_HZ,
        .level1 = IRENCODER_NEC_LEVEL_1,
        .duration1 = IRENCODER_NEC_LEADERCODE_DURATION_1 * resolution / IRENCODER_CLOCK_SOURCE_FREQUENCY_HZ,
    };

    /* Configure NEC ending code */
    encoder->NecEndingCode = (rmt_symbol_word_t){
        .level0 = IRENCODER_NEC_LEVEL_0,
        .duration0 = IRENCODER_NEC_ENDINGCODE_DURATION_0 * resolution / IRENCODER_CLOCK_SOURCE_FREQUENCY_HZ,
        .level1 = IRENCODER_NEC_LEVEL_1,
        .duration1 = IRENCODER_NEC_ENDINGCODE_DURATION_1,
    };

    *rmtEncoderHandle = &encoder->Base;

    if (err != ESP_OK)
    {
        if (encoder->BytesEncoder != NULL)
        {
            ESP_ERROR_CHECK(rmt_del_encoder(encoder->BytesEncoder));
        }

        if (encoder->CopyEncoder != NULL)
        {
            ESP_ERROR_CHECK(rmt_del_encoder(encoder->CopyEncoder));
        }
    }

    return err;
}

size_t IrEncoder_RmtDecode(rmt_symbol_word_t *const rmtSymbols, const size_t symbolCount, uint8_t *const decodeBuffer, const size_t bufferSize)
{
    size_t decodedDataBytes = 0U;
    bool validSymbols = true;

    if (symbolCount >= IRENCODER_NEC_CODES_SYMBOL_LENGTH)
    {
        rmt_symbol_word_t *currentRmtSymbol = rmtSymbols;

        /* Verify NEC leader code */
        if (!(IrEncoder_NecCheckInRange(currentRmtSymbol->duration0, IRENCODER_NEC_LEADERCODE_DURATION_0) && IrEncoder_NecCheckInRange(currentRmtSymbol->duration1, IRENCODER_NEC_LEADERCODE_DURATION_1)))
        {
            validSymbols = false;
        }
        currentRmtSymbol++;

        /* Decode data RMT symbols */
        size_t decodedDataSymbols = 0U;
        while (validSymbols && decodedDataBytes < bufferSize && decodedDataSymbols < (symbolCount - IRENCODER_NEC_CODES_SYMBOL_LENGTH))
        {
            *(decodeBuffer + decodedDataBytes) = 0U;

            for (size_t i = 0U; i < IRENCODER_BITS_PER_BYTE; i++)
            {
                if (IrEncoder_NecParseLogic1(currentRmtSymbol))
                {
                    *(decodeBuffer + decodedDataBytes) |= IRENCODER_NEC_BIT << i;
                }
                else if (IrEncoder_NecParseLogic0(currentRmtSymbol))
                {
                    *(decodeBuffer + decodedDataBytes) &= ~(IRENCODER_NEC_BIT << i);
                }
                else
                {
                    validSymbols = false;
                }

                currentRmtSymbol++;
                decodedDataSymbols++;
            }

            decodedDataBytes++;
        }

        /* Verify NEC ending code */
        if (!IrEncoder_NecCheckInRange(currentRmtSymbol->duration0, IRENCODER_NEC_ENDINGCODE_DURATION_0))
        {
            validSymbols = false;
        }
    }
    else
    {
        validSymbols = false;
    }

    if (!validSymbols)
    {
        decodedDataBytes = 0U;
    }

    return decodedDataBytes;
}

static size_t IRAM_ATTR IrEncoder_RmtBaseEncoder(rmt_encoder_t *rmtEncoder, rmt_channel_handle_t txChannelHandle, const void *primaryData, size_t size, rmt_encode_state_t *encoderState)
{
    IrEncoder_t *encoder = __containerof(rmtEncoder, IrEncoder_t, Base);
    rmt_encoder_handle_t copyEncoder = encoder->CopyEncoder;
    rmt_encoder_handle_t bytesEncoder = encoder->BytesEncoder;
    rmt_encode_state_t sessionState = RMT_ENCODING_RESET;
    rmt_encode_state_t state = RMT_ENCODING_RESET;
    size_t encodedSymbols = 0U;

    /* Send NEC leader coder */
    if (encoder->State == IRENCODER_STATE_LEADERCODER && !(sessionState & RMT_ENCODING_MEM_FULL))
    {
        encodedSymbols += copyEncoder->encode(copyEncoder, txChannelHandle, &encoder->NecLeaderCode, sizeof(rmt_symbol_word_t), &sessionState);
        if (sessionState & RMT_ENCODING_COMPLETE)
        {
            encoder->State = IRENCODER_STATE_DATA;
        }
        if (sessionState & RMT_ENCODING_MEM_FULL)
        {
            state |= RMT_ENCODING_MEM_FULL;
        }
    }

    /* Send data */
    if (encoder->State == IRENCODER_STATE_DATA && !(sessionState & RMT_ENCODING_MEM_FULL))
    {
        encodedSymbols += bytesEncoder->encode(bytesEncoder, txChannelHandle, (uint8_t *)primaryData, size, &sessionState);
        if (sessionState & RMT_ENCODING_COMPLETE)
        {
            encoder->State = IRENCODER_STATE_ENDINGCODE;
        }
        if (sessionState & RMT_ENCODING_MEM_FULL)
        {
            state |= RMT_ENCODING_MEM_FULL;
        }
    }

    /* Send NEC ending code */
    if (encoder->State == IRENCODER_STATE_ENDINGCODE && !(sessionState & RMT_ENCODING_MEM_FULL))
    {
        encodedSymbols += copyEncoder->encode(copyEncoder, txChannelHandle, &encoder->NecEndingCode, sizeof(rmt_symbol_word_t), &sessionState);
        if (sessionState & RMT_ENCODING_COMPLETE)
        {
            encoder->State = IRENCODER_STATE_LEADERCODER;
            state |= RMT_ENCODING_COMPLETE;
        }
        if (sessionState & RMT_ENCODING_MEM_FULL)
        {
            state |= RMT_ENCODING_MEM_FULL;
        }
    }

    *encoderState = state;

    return encodedSymbols;
}

static esp_err_t IrEncoder_RmtDeleteEncoder(rmt_encoder_t *rmtEncoder)
{
    esp_err_t err = ESP_OK;
    IrEncoder_t *encoder = __containerof(rmtEncoder, IrEncoder_t, Base);

    if (rmt_del_encoder(encoder->CopyEncoder) != ESP_OK || rmt_del_encoder(encoder->BytesEncoder) != ESP_OK)
    {
        err = ESP_FAIL;
    }

    return err;
}

static esp_err_t IrEncoder_RmtResetEncoder(rmt_encoder_t *rmtEncoder)
{
    esp_err_t err = ESP_OK;
    IrEncoder_t *encoder = __containerof(rmtEncoder, IrEncoder_t, Base);

    if (rmt_encoder_reset(encoder->CopyEncoder) != ESP_OK || rmt_encoder_reset(encoder->BytesEncoder) != ESP_OK)
    {
        err = ESP_FAIL;
    }

    encoder->State = IRENCODER_STATE_LEADERCODER;

    return err;
}

static inline bool IrEncoder_NecCheckInRange(const IrEncoder_Duration_t signalDuration, const IrEncoder_Duration_t specifiedDuration)
{
    return (signalDuration < (specifiedDuration + IRENCODER_NEC_DECODE_MARGIN)) && (signalDuration > (specifiedDuration - IRENCODER_NEC_DECODE_MARGIN));
}

static inline bool IrEncoder_NecParseLogic0(const rmt_symbol_word_t *const rmtNecSymbols)
{
    return IrEncoder_NecCheckInRange(rmtNecSymbols->duration0, IRENCODER_NEC_BIT0_DURATION_0) && IrEncoder_NecCheckInRange(rmtNecSymbols->duration1, IRENCODER_NEC_BIT0_DURATION_1);
}

static inline bool IrEncoder_NecParseLogic1(const rmt_symbol_word_t *const rmtNecSymbols)
{
    return IrEncoder_NecCheckInRange(rmtNecSymbols->duration0, IRENCODER_NEC_BIT1_DURATION_0) && IrEncoder_NecCheckInRange(rmtNecSymbols->duration1, IRENCODER_NEC_BIT1_DURATION_1);
}
