/**
 * @file DFPlayerMini_PlayerMini.cpp
 *
 * @brief Wrapper around DFRobotDFPlayerMini library
 *
 ******************************************************************************/

/* Includes
 ******************************************************************************/
#include "DFPlayerMini_PlayerMini.h"

/* Function Definitions
 ******************************************************************************/

DFPlayerMini_PlayerMini::DFPlayerMini_PlayerMini(const uart_port_t uart, const uint32_t rxPin, const uint32_t txPin)
{
    stream.setUart(uart);
    stream.setPins(rxPin, txPin);
}

bool DFPlayerMini_PlayerMini::begin(const bool isACK, const bool doReset)
{
    stream.begin();
    return DFRobotDFPlayerMini::begin(stream, isACK, doReset);
}