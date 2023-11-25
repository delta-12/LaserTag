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

/* Function Prototypes
 ******************************************************************************/

void BleCentral_Init(void);
void BleCentral_DeInit(void);

#endif