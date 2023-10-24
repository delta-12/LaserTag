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

DFPlayerMini_PlayerMini::DFPlayerMini_PlayerMini(const uint32_t rxPin, const uint32_t txPin)
{
    stream.setPins(rxPin, txPin);
    stream.begin();
}

bool DFPlayerMini_PlayerMini::begin(const bool isACK, const bool doReset)
{
    return DFRobotDFPlayerMini::begin(stream, isACK, doReset);
}