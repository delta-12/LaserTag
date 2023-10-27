/**
 * @file DFPlayerMini_PlayerMini.h
 *
 * @brief Wrapper around DFRobotDFPlayerMini library
 *
 ******************************************************************************/

#ifndef DFPLAYERMINI_PLAYERMINI_H
#define DFPLAYERMINI_PLAYERMINI_H

/* Includes
 ******************************************************************************/
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

/* Class Declarations
 ******************************************************************************/

class DFPlayerMini_PlayerMini : public DFRobotDFPlayerMini
{
private:
    Stream stream;

public:
    DFPlayerMini_PlayerMini(const uart_port_t uart = DFPLAYERMINI_ARDUINO_DEFAULT_UART_NUM, const uint32_t rxPin = DFPLAYERMINI_ARDUINO_DEFAULT_UART_RX_PIN, const uint32_t txPin = DFPLAYERMINI_ARDUINO_DEFAULT_UART_TX_PIN);
    bool begin(const bool isACK, const bool doReset);
};

#endif