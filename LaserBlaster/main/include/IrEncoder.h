/**
 * @file IrEncoder.h
 *
 * @brief Modified NEC RMT encoder for raw bytes.
 *
 ******************************************************************************/

#ifndef LTP_NEC_ENCODER_H
#define LTP_NEC_ENCODER_H

/* Includes
 ******************************************************************************/
#include <driver/rmt_encoder.h>
#include <stdint.h>

/* Typedefs
 ******************************************************************************/

typedef uint32_t IrEncoder_Resolution_t;

typedef enum
{
    IRENCODER_STATE_LEADERCODER,
    IRENCODER_STATE_DATA,
    IRENCODER_STATE_ENDINGCODE
} IrEncoder_State_t;

typedef struct
{
    rmt_encoder_t Base;
    rmt_encoder_t *CopyEncoder;
    rmt_encoder_t *BytesEncoder;
    rmt_symbol_word_t NecLeaderCode;
    rmt_symbol_word_t NecEndingCode;
    IrEncoder_State_t State;
} IrEncoder_t;

/* Function Prototypes
 ******************************************************************************/

esp_err_t IrEncoder_InitRmtEncoder(rmt_encoder_handle_t *const rmtEncoderHandle, IrEncoder_t *const encoder, const IrEncoder_Resolution_t resolution);
size_t IrEncoder_RmtDecode(rmt_symbol_word_t *const rmtSymbols, const size_t symbolCount, uint8_t *const decodeBuffer, const size_t bufferSize);

#endif
