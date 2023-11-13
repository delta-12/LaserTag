/**
 * @file Rmt.h
 *
 * @brief Manage RMT peripherals.
 *
 ******************************************************************************/

#ifndef RMT_H
#define RMT_H

/* Includes
 ******************************************************************************/

/* Defines
 ******************************************************************************/

/* Typedefs
 ******************************************************************************/

typedef void (*Rmt_RxEventHandler_t)(uint8_t *const data, const size_t size);

/* Function Prototypes
 ******************************************************************************/

void Rmt_RxInit(void);
void Rmt_TxInit(void);
void Rmt_RegisterRxEventHandler(Rmt_RxEventHandler_t rxEventHandler);
esp_err_t Rmt_Transmit(const uint8_t *const data, const size_t size);

#endif
