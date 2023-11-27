/**
 * @file Adc.h
 *
 * @brief Manage ADC peripherals.
 *
 ******************************************************************************/

#ifndef ADC_H
#define ADC_H

/* Includes
 ******************************************************************************/
#include "esp_err.h"

/* Typedefs
 ******************************************************************************/

typedef int Adc_mV_t;

/* Function Prototypes
 ******************************************************************************/

void Adc_Init(void);
esp_err_t Adc_OneshotRead(Adc_mV_t *const voltage);
void Adc_DeInit(void);

#endif
