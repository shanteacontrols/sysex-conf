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
    SysEx() {}
    bool setLayout(sysExBlock_t *pointer, uint8_t numberOfBlocks);
    bool setupCustomRequests(sysExCustomRequest_t *customRequests, uint8_t numberOfCustomRequests);
    void handleMessage(uint8_t *sysExArray, uint8_t size);
    bool isConfigurationEnabled();
    bool isSilentModeEnabled();
    void setSilentMode(bool state);
    void sendCustomMessage(uint8_t *responseArray, sysExParameter_t *values, uint8_t size, bool ack = true);
    void setError(sysExStatus_t status);
    bool addToResponse(sysExParameter_t value);

    virtual bool onGet(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t &value) = 0;
    virtual bool onSet(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue) = 0;
    virtual bool onCustomRequest(uint8_t value) = 0;
    virtual void onWrite(uint8_t *sysExArray, uint8_t size) = 0;

    private:
    bool decode();
    void resetDecodedMessage();
    bool processStandardRequest();
    bool processSpecialRequest();
    bool checkID();
    bool checkStatus();
    bool checkWish();
    bool checkAmount();
    bool checkBlock();
    bool checkSection();
    bool checkPart();
    bool checkParameterIndex();
    bool checkNewValue();
    bool checkParameters();

    uint8_t generateMessageLenght();
    void setStatus(sysExStatus_t status);
    void sendResponse(bool containsLastByte);

    private:
    ///
    /// \brief Flag indicating whether or not configuration is possible.
    ///
    bool                    sysExEnabled = false;

    ///
    /// \brief Flag indicating whether or not silent mode is active.
    /// When silent mode is active, protocol won't return any error or ACK messages.
    ///
    bool                    silentModeEnabled = false;

    ///
    /// \brief Pointer to SysEx layout.
    ///
    sysExBlock_t            *sysExMessage = nullptr;

    ///
    /// \brief Total number of blocks for a received SysEx layout.
    ///
    uint8_t                 sysExBlockCounter = 0;

    ///
    /// \brief Structure containing decoded data from SysEx request for easier access.
    ///
    decodedMessage_t        decodedMessage;

    ///
    /// \brief Pointer to SysEx array.
    /// Same array is used for request and response.
    /// Response modifies received request so that arrays aren't duplicated.
    ///
    uint8_t                 *sysExArray = nullptr;

    ///
    /// \brief Pointer to structure containing data for custom requests.
    ///
    sysExCustomRequest_t    *sysExCustomRequest = nullptr;

    ///
    /// \brief Total number of custom SysEx requests stored in pointed structure.
    ///
    uint8_t                 numberOfCustomRequests = 0;

    ///
    /// \brief Size of received SysEx array.
    ///
    uint8_t                 receivedArraySize = 0;

    ///
    /// \brief Size of SysEx response.
    ///
    uint8_t                 responseSize = 0;

    ///
    /// \brief User-set SysEx status.
    /// Used when user sets custom status.
    ///
    sysExStatus_t           userStatus = REQUEST;

    ///
    /// \brief Holds amount of user-specified custom requests.
    ///
    uint8_t                 customRequestCounter = 0;

    ///
    /// \brief Variable holding info on whether custom requests need connection open request before they're processed.
    /// \warning This variable assumes no more than 16 custom requests can be specified.
    ///
    uint16_t                customReqConnIgnore = 0;
};

/// @}