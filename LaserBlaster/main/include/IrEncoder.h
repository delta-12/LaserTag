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

typedef uint32_t IrEncoder_Resolution_t; /* Internal tick counter resolution */

typedef enum
{
    IRENCODER_STATE_LEADERCODER, /* Encode NEC leader code into RMT format */
    IRENCODER_STATE_DATA,        /* Encode data into RMT format */
    IRENCODER_STATE_ENDINGCODE   /* Encode NEC ending code into RMT format*/
} IrEncoder_State_t;             /* State of IR encoder */

typedef struct
{
    rmt_encoder_t Base;              /* Declare the standard encoder interface */
    rmt_encoder_t *CopyEncoder;      /* Used to encode the leading and ending pulse */
    rmt_encoder_t *BytesEncoder;     /* Used to encode the data */
    rmt_symbol_word_t NecLeaderCode; /* NEC leading code in RMT representation */
    rmt_symbol_word_t NecEndingCode; /* NEC ending code in RMT representation */
    IrEncoder_State_t State;         /* Store encoder state across multiple RMT encoding parts */
} IrEncoder_t;                       /* Encoder to convert raw data bytes to RMT format for IR transmission */

/* Function Prototypes
 ******************************************************************************/

esp_err_t IrEncoder_InitRmtEncoder(rmt_encoder_handle_t *const rmtEncoderHandle, IrEncoder_t *const encoder, const IrEncoder_Resolution_t resolution);
size_t IrEncoder_RmtDecode(rmt_symbol_word_t *const rmtSymbols, const size_t symbolCount, uint8_t *const decodeBuffer, const size_t bufferSize);

#endif
