/**
 * @file BleCentral.h
 *
 * @brief Initiates communication with Laser Target by scanning for nearby
 * peripherals and establishing connections with them if they support the
 * correct service.
 *
 ******************************************************************************/

#ifndef BLECENTRAL_H
#define BLECENTRAL_H

/* Includes
 ******************************************************************************/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Function Prototypes
 ******************************************************************************/

void BleCentral_Init(void);
void BleCentral_DeInit(void);
bool BleCentral_IsConnected(void);
void BleCentral_RegisterNotifyCallback(void (*notifyCallback)(uint8_t *const data, const size_t size));

#endif
