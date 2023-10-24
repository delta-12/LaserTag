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
    DFPlayerMini_PlayerMini(const uint32_t rxPin, const uint32_t txPin);
    bool begin(const bool isACK, const bool doReset);
};

#endif