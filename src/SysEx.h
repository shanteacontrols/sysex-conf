/*
    Copyright 2017-2018 Igor Petrovic

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

    static uint8_t generateMessageLenght();
    static void setStatus(sysExStatus_t status);
};

/// @}