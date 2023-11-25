/**
 * @file BlePeripheral.h
 *
 * @brief Simple BLE peripheral using Bluetooth controller and NimBLE stack.
 * Creates a GATT server, advertises, and waits to be connected to a GATT
 * client.
 *
 ******************************************************************************/

#ifndef BLE_PRPH_H
#define BLE_PRPH_H

/* Includes
 ******************************************************************************/
#include <stdbool.h>

/* Function Prototypes
 ******************************************************************************/

void BlePeripheral_Init(void);
bool BlePeripheral_IsConnected(void);

#endif