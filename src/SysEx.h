/*
    OpenDeck MIDI platform firmware
    Copyright (C) 2015-2017 Igor Petrovic

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <stdlib.h>
#include "DataTypes.h"

///
/// \brief Configuration protocol created using custom SysEx MIDI messages.
/// @{
///

class SysEx
{
    public:
    SysEx();
    static void handleMessage(uint8_t *sysExArray, uint8_t size);
    static void decode();
    static bool configurationEnabled();
    static bool addCustomRequest(uint8_t value);
    static void startResponse();
    static void addToResponse(sysExParameter_t value);
    static void sendResponse();
    static void setError(sysExStatus_t status);

    static void setHandleGet(sysExParameter_t(*fptr)(uint8_t block, uint8_t section, uint16_t index));
    static void setHandleSet(bool(*fptr)(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue));
    static void setHandleCustomRequest(bool(*fptr)(uint8_t value));
    static void setHandleSysExWrite(void(*fptr)(uint8_t *sysExArray, uint8_t size));

    static bool addBlock();
    static bool addBlocks(uint8_t numberOfBlocks);
    static bool addSection(uint8_t blockID, sysExSection section);

    static bool checkRequest();
    static bool checkParameters();

    private:
    static bool checkID();
    static bool checkSpecialRequests();
    static bool checkWish();
    static bool checkAmount();
    static bool checkBlock();
    static bool checkSection();
    static bool checkPart();
    static bool checkParameterIndex();
    static bool checkNewValue();

    static uint8_t generateMinMessageLenght();
    static void setStatus(sysExStatus_t status);
};

/// @}