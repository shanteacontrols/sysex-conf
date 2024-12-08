/*
    Copyright 2017-2022 Igor Petrovic

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

#include "common.h"

///
/// \brief Configuration protocol created using custom SysEx MIDI messages.
/// @{
///

namespace lib::sysexconf
{
    class SysExConf
    {
        public:
        SysExConf(DataHandler&          dataHandler,
                  const ManufacturerId& manufacturerId)
            : _dataHandler(dataHandler)
            , _manufacturerId(manufacturerId)
        {}

        void    reset();
        bool    setLayout(std::vector<Block>& layout);
        bool    setupCustomRequests(std::vector<CustomRequest>& customRequests);
        void    handleMessage(const uint8_t* array, uint16_t size);
        bool    isConfigurationEnabled();
        void    setUserErrorIgnoreMode(bool state);
        void    sendCustomMessage(const uint16_t* values, uint16_t size, bool ack = true);
        uint8_t blocks() const;
        uint8_t sections(uint8_t blockIndex) const;

        private:
        ///
        /// \brief Reference to object performing reading and writing of actual data.
        ///
        DataHandler& _dataHandler;

        ///
        /// \brief Reference to structure containing manufacturer ID bytes.
        ///
        const ManufacturerId& _manufacturerId;

        ///
        /// \brief Array in which response will be stored.
        ///
        uint8_t _responseArray[MAX_MESSAGE_SIZE] = {};

        ///
        /// \brief Holds current size of response array.
        ///
        uint16_t _responseCounter = 0;

        ///
        /// \brief Flag indicating whether or not configuration is possible.
        ///
        bool _sysExEnabled = false;

        ///
        /// \brief Flag indicating whether or not user error ignore mode is active.
        /// When user error ignore mode is active, protocol will always return ACK
        /// status for get/set messages ignoring the user status. For get requests,
        /// retrieved value will be set to 0 and appended to response.
        ///
        bool _userErrorIgnoreModeEnabled = false;

        ///
        /// \brief SysEx layout.
        ///
        std::vector<Block>* _layout = {};

        ///
        /// \brief Structure containing decoded data from SysEx request for easier access.
        ///
        DecodedMessage _decodedMessage;

        ///
        /// \brief Vector of structures containing data for custom requests.
        ///
        std::vector<CustomRequest> _sysExCustomRequest = {};

        bool     addToResponse(uint16_t value);
        bool     decode(const uint8_t* receivedArray, uint16_t receivedArraySize);
        void     resetDecodedMessage();
        bool     processStandardRequest(uint16_t receivedArraySize);
        bool     processSpecialRequest();
        bool     checkManufacturerId();
        bool     checkStatus();
        bool     checkWish();
        bool     checkAmount();
        bool     checkBlock();
        bool     checkSection();
        bool     checkPart();
        bool     checkParameterIndex();
        bool     checkNewValue();
        bool     checkParameters();
        uint16_t generateMessageLenght();

        template<typename T>
        void setStatus(T status)
        {
            uint8_t status_uint8 = static_cast<uint8_t>(status);
            status_uint8 &= 0x7F;

            _responseArray[static_cast<uint8_t>(byteOrder_t::STATUS_BYTE)] = status_uint8;
        }

        void sendResponse(bool containsLastByte, bool customMessage = false);
    };
}    // namespace lib::sysexconf

/// @}