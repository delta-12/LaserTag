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

#define IRENCODER_CLOCK_SOURCE_FREQUENCY_HZ 1000000U                                                                        /* Frequency of clock source used to calculate RMT level durations */
#define IRENCODER_NEC_LEVEL_0 1U                                                                                            /* NEC IR signal level 0 is logic HIGH */
#define IRENCODER_NEC_LEVEL_1 0U                                                                                            /* NEC IR signal level 1 is logic LOW */
#define IRENCODER_NEC_LEADERCODE_SYMBOL_LENGTH 1U                                                                           /* NEC leader code is one RMT symbol in length */
#define IRENCODER_NEC_LEADERCODE_DURATION_0 9000ULL                                                                         /* NEC leader code leading 9ms pulse burst */
#define IRENCODER_NEC_LEADERCODE_DURATION_1 4500ULL                                                                         /* NEC leader code 4.5ms space following leading burst */
#define IRENCODER_NEC_ENDINGCODE_SYMBOL_LENGTH 1U                                                                           /* NEC ending code is one RMT symbol in length */
#define IRENCODER_NEC_ENDINGCODE_DURATION_0 560ULL                                                                          /* NEC ending code 560us pulse burst to end transmission */
#define IRENCODER_NEC_ENDINGCODE_DURATION_1 32767ULL                                                                        /* NEC ending code does not have a space after burst when duration ticks are 15 bits */
#define IRENCODER_NEC_CODES_SYMBOL_LENGTH (IRENCODER_NEC_LEADERCODE_SYMBOL_LENGTH + IRENCODER_NEC_ENDINGCODE_SYMBOL_LENGTH) /* Total RMT symbol length of NEC leading and ending codes */
#define IRENCODER_NEC_BIT0_DURATION_0 560ULL                                                                                /* NEC logic 0 has 560us pulse burst */
#define IRENCODER_NEC_BIT0_DURATION_1 560ULL                                                                                /* NEC logic 1 has 560us pulse burst */
#define IRENCODER_NEC_BIT1_DURATION_0 560ULL                                                                                /* NEC logic 0 has 560us space */
#define IRENCODER_NEC_BIT1_DURATION_1 1690ULL                                                                               /* NEC logic 0 has 1690us space */
#define IRENCODER_NEC_DECODE_MARGIN 200U                                                                                    /* Margin of error for NEC bit logic level durations */
#define IRENCODER_BITS_PER_BYTE 8U                                                                                          /* 8 bits per byte */
#define IRENCODER_NEC_BIT 1U                                                                                                /* Bit for masking */

/* Typedefs
 ******************************************************************************/

typedef uint32_t IrEncoder_Duration_t; /* Tick count used to measure a duration */

/* Function Prototypes
 ******************************************************************************/

static size_t IrEncoder_RmtBaseEncoder(rmt_encoder_t *rmtEncoder, rmt_channel_handle_t txChannelHandle, const void *primaryData, size_t size, rmt_encode_state_t *encoderState);
static esp_err_t IrEncoder_RmtDeleteEncoder(rmt_encoder_t *rmtEncoder);
static esp_err_t IrEncoder_RmtResetEncoder(rmt_encoder_t *rmtEncoder);
static inline bool IrEncoder_NecCheckInRange(const IrEncoder_Duration_t signalDuration, const uint32_t specifiedDuration);
static inline bool IrEncoder_NecParseLogic0(const rmt_symbol_word_t *const rmtSymbol);
static inline bool IrEncoder_NecParseLogic1(const rmt_symbol_word_t *const rmtSymbol);

/* Function Definitions
 ******************************************************************************/

/**
 * @brief Initialize RMT encoder used when transmitting data over IR.
 *
 * @param[in,out] rmtEncoderHandle RMT encoder handle to initialize
 * @param[in]     encoder          Encoder to convert raw data bytes to RMT
 *                                 format
 * @param[in]     resolution       Internal tick counter resolution used to
 *                                 calculate logic level durations
 *
 * @return esp_err_t Error code indicating success or reason for failure
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

    /* Delete copy and bytes encoder if an error is encountered during initialization */
    if (err != ESP_OK)
    {
        if (encoder->CopyEncoder != NULL)
        {
            ESP_ERROR_CHECK(rmt_del_encoder(encoder->CopyEncoder));
        }

        if (encoder->BytesEncoder != NULL)
        {
            ESP_ERROR_CHECK(rmt_del_encoder(encoder->BytesEncoder));
        }
    }

    return err;
}

/**
 * @brief Decode RMT symbols to raw bytes.
 *
 * @param[in]     rmtSymbols   Pointer to RMT symbols to decode
 * @param[in]     symbolCount  Number of RMT symbols to decode
 * @param[in,out] decodeBuffer Pointer to buffer to store raw bytes from
 *                             decoded RMT symbols
 * @param[in]     bufferSize   Size of buffer to store decoded data
 *
 * @return size_t Number of bytes successfully decoded from RMT symbols
 ******************************************************************************/
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

    /* Return zero bytes if at least one symbol is invalid */
    if (!validSymbols)
    {
        decodedDataBytes = 0U;
    }

    return decodedDataBytes;
}

/**
 * @brief Encode raw bytes to RMT symbols for IR transmission.
 *
 * @param[in]     rmtEncoder       RMT encoder used to encode raw bytes to RMT
 *                                 symbols
 * @param[in]     txChannelHandle  RMT TX channel to write encoded data to for
 *                                 transmission
 * @param[in]     primaryData      Pointer to raw bytes to encode to RMT
 *                                 symbols
 * @param[in]     size             Number of raw bytes to encode to RMT symbols
 * @param[in,out] encoderState     Pointer to RMT encoding state of RMT encoder
 *
 * @return size_t Number of bytes successfully encoded to RMT symbols
 ******************************************************************************/
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

/**
 * @brief Delete RMT copy and bytes encoders.
 *
 * @param[in,out] rmtEncoder Pointer to RMT encoder with encoders to delete
 *
 * @return esp_err_t Error code indicating success or reason for failure
 ******************************************************************************/
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

/**
 * @brief Reset RMT copy and bytes encoders.
 *
 * @param[in,out] rmtEncoder Pointer to RMT encoder with encoders to reset
 *
 * @return esp_err_t Error code indicating success or reason for failure
 ******************************************************************************/
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

/**
 * @brief Check if a duration is within an expected range.
 *
 * @param[in] signalDuration    Duration to check
 * @param[in] specifiedDuration Duration to check against, specified by a
 *                              given protocol, center of range
 *
 * @return bool Whether a duration is within the expected range
 *
 * @retval true  Duration is within the expected range
 * @retval false Duration is not within the expected range
 ******************************************************************************/
static inline bool IrEncoder_NecCheckInRange(const IrEncoder_Duration_t signalDuration, const IrEncoder_Duration_t specifiedDuration)
{
    return (signalDuration < (specifiedDuration + IRENCODER_NEC_DECODE_MARGIN)) && (signalDuration > (specifiedDuration - IRENCODER_NEC_DECODE_MARGIN));
}

/**
 * @brief Check if a RMT symbol represents NEC logic zero.
 *
 * @param[in] rmtSymbol Pointer to RMT symbol to check
 *
 * @return bool Whether a RMT symbol represents a NEC logic zero
 *
 * @retval true  RMT symbol represents a NEC logic zero
 * @retval false RMT symbol does not represent a NEC logic zero
 ******************************************************************************/
static inline bool IrEncoder_NecParseLogic0(const rmt_symbol_word_t *const rmtSymbol)
{
    return IrEncoder_NecCheckInRange(rmtSymbol->duration0, IRENCODER_NEC_BIT0_DURATION_0) && IrEncoder_NecCheckInRange(rmtSymbol->duration1, IRENCODER_NEC_BIT0_DURATION_1);
}

/**
 * @brief Check if a RMT symbol represents NEC logic one.
 *
 * @param[in] rmtSymbol Pointer to RMT symbol to check
 *
 * @return bool Whether a RMT symbol represents a NEC logic one
 *
 * @retval true  RMT symbol represents a NEC logic one
 * @retval false RMT symbol does not represent a NEC logic one
 ******************************************************************************/
static inline bool IrEncoder_NecParseLogic1(const rmt_symbol_word_t *const rmtSymbol)
{
    return IrEncoder_NecCheckInRange(rmtSymbol->duration0, IRENCODER_NEC_BIT1_DURATION_0) && IrEncoder_NecCheckInRange(rmtSymbol->duration1, IRENCODER_NEC_BIT1_DURATION_1);
}
