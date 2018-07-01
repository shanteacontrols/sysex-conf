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

#ifndef _SYSEX_
#define _SYSEX_

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
    static bool init(sysExBlock_t *pointer, uint8_t numberOfBlocks);
    static void handleMessage(uint8_t *sysExArray, uint8_t size);
    static bool isConfigurationEnabled();
    static bool isSilentModeEnabled();
    static bool addCustomRequest(uint8_t value, bool handshakeIgnore = false);
    static void sendCustomMessage(uint8_t *responseArray, sysExParameter_t *values, uint8_t size, bool ack = true);
    static void setError(sysExStatus_t status);
    static bool addToResponse(sysExParameter_t value);

    static void setHandleGet(bool(*fptr)(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t &value));
    static void setHandleSet(bool(*fptr)(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue));
    static void setHandleCustomRequest(bool(*fptr)(uint8_t value));
    static void setHandleSysExWrite(void(*fptr)(uint8_t *sysExArray, uint8_t size));

    private:
    static void decode();
    static bool checkID();
    static bool checkSpecialRequests();
    static bool checkWish();
    static bool checkAmount();
    static bool checkBlock();
    static bool checkSection();
    static bool checkPart();
    static bool checkParameterIndex();
    static bool checkNewValue();
    static bool checkRequest();
    static bool checkParameters();

    static uint8_t generateMessageLenght();
    static void setStatus(sysExStatus_t status);
};

/// @}
#endif