/*
    Copyright 2017-2020 Igor Petrovic

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

#include <vector>
#include <inttypes.h>
#include <stdlib.h>

///
/// \brief Configuration protocol created using custom SysEx MIDI messages.
/// @{
///

class SysExConf
{
    public:
    static constexpr uint16_t PARAMS_PER_MESSAGE = 32;
    static constexpr uint16_t BYTES_PER_VALUE    = 2;

    ///
    /// \brief Structure holding SysEx manufacturer ID bytes.
    ///
    struct manufacturerID_t
    {
        uint8_t id1;
        uint8_t id2;
        uint8_t id3;
    };

    ///
    /// \brief Structure holding data for single SysEx section within block.
    ///
    class Section
    {
        public:
        Section(
            uint16_t numberOfParameters,
            uint16_t newValueMin,
            uint16_t newValueMax)
            : _numberOfParameters(numberOfParameters)
            , _newValueMin(newValueMin)
            , _newValueMax(newValueMax)
        {
            // based on number of parameters, calculate how many parts message has in case of set/all request and get/all response
            _parts = numberOfParameters / SysExConf::PARAMS_PER_MESSAGE;

            if (numberOfParameters % SysExConf::PARAMS_PER_MESSAGE)
            {
                _parts++;
            }
        }

        uint16_t numberOfParameters() const
        {
            return _numberOfParameters;
        }

        uint16_t newValueMin() const
        {
            return _newValueMin;
        }

        uint16_t newValueMax() const
        {
            return _newValueMax;
        }

        uint8_t parts() const
        {
            return _parts;
        }

        private:
        const uint16_t _numberOfParameters;
        const uint16_t _newValueMin;
        const uint16_t _newValueMax;
        uint8_t        _parts;
    };

    ///
    /// \brief Structure holding data for single SysEx block.
    ///
    struct block_t
    {
        std::vector<Section> section;
    };

    ///
    /// \brief Structure containing data for single custom request.
    ///
    struct customRequest_t
    {
        uint16_t requestID;        ///< ID byte representing specific request.
        bool     connOpenCheck;    ///< Flag indicating whether or not SysEx connection should be enabled before processing request.
    };

    ///
    /// \brief Descriptive list of SysEx wish bytes.
    ///
    enum class wish_t : uint8_t
    {
        get,
        set,
        backup,
        invalid
    };

    ///
    /// \brief Descriptive list of SysEx amount bytes.
    ///
    enum class amount_t : uint8_t
    {
        single,
        all,
        invalid
    };

    ///
    /// \brief Descriptive list of possible SysEx message statuses.
    ///
    enum class status_t : uint8_t
    {
        request,               // 0x00
        ack,                   // 0x01
        errorStatus,           // 0x02
        errorConnection,       // 0x03
        errorWish,             // 0x04
        errorAmount,           // 0x05
        errorBlock,            // 0x06
        errorSection,          // 0x07
        errorPart,             // 0x08
        errorIndex,            // 0x09
        errorNewValue,         // 0x0A
        errorMessageLength,    // 0x0B
        errorWrite,            // 0x0C
        errorNotSupported,     // 0x0D
        errorRead,             // 0x0E
    };

    ///
    /// \brief List of special SysEx IDs.
    ///
    enum class specialRequest_t : uint8_t
    {
        connClose,            // 0x00
        connOpen,             // 0x01
        bytesPerValue,        // 0x02
        paramsPerMessage,     // 0x03
        connOpenSilent,       // 0x04
        connSilentDisable,    // 0x05
        AMOUNT
    };

    ///
    /// \brief Structure holding decoded request data.
    ///
    struct decodedMessage_t
    {
        status_t status;
        wish_t   wish;
        amount_t amount;
        uint8_t  block;
        uint8_t  section;
        uint8_t  part;
        uint16_t index;
        uint16_t newValue;
    };

    ///
    /// \brief Descriptive list of bytes in SysEx message.
    ///
    enum class byteOrder_t : uint8_t
    {
        startByte,      // 0
        idByte_1,       // 1
        idByte_2,       // 2
        idByte_3,       // 3
        statusByte,     // 4
        partByte,       // 5
        wishByte,       // 6
        amountByte,     // 7
        blockByte,      // 8
        sectionByte,    // 9
        indexByte,      // 10
    };

    class DataHandler
    {
        public:
        class CustomResponse
        {
            public:
            CustomResponse(uint8_t* responseArray, uint16_t& responseCounter)
                : _responseArray(responseArray)
                , _responseCounter(responseCounter)
            {}

            void append(uint16_t value)
            {
                value &= 0x3FFF;

                // make sure to leave space for 0xF7 byte
                if ((_responseCounter - 2) < MAX_MESSAGE_SIZE)
                {
                    // split into two 7-bit values

                    auto split                         = SysExConf::Split14bit(value);
                    _responseArray[_responseCounter++] = split.high();
                    _responseArray[_responseCounter++] = split.low();
                }
            }

            private:
            uint8_t*  _responseArray;
            uint16_t& _responseCounter;
        };

        DataHandler() = default;

        virtual uint8_t get(uint8_t block, uint8_t section, uint16_t index, uint16_t& value)   = 0;
        virtual uint8_t set(uint8_t block, uint8_t section, uint16_t index, uint16_t newValue) = 0;
        virtual uint8_t customRequest(uint16_t request, CustomResponse& customResponse)        = 0;
        virtual void    sendResponse(uint8_t* array, uint16_t size)                            = 0;
    };

    static constexpr uint8_t  SPECIAL_REQ_MSG_SIZE = (static_cast<uint8_t>(byteOrder_t::wishByte) + 1) + 1;    // extra byte for end
    static constexpr uint8_t  STD_REQ_MIN_MSG_SIZE = static_cast<uint8_t>(byteOrder_t::indexByte) + (BYTES_PER_VALUE * 2) + 1;
    static constexpr uint16_t MAX_MESSAGE_SIZE     = STD_REQ_MIN_MSG_SIZE + (PARAMS_PER_MESSAGE * BYTES_PER_VALUE);

    SysExConf(DataHandler&            dataHandler,
              const manufacturerID_t& mID)
        : _dataHandler(dataHandler)
        , _mID(mID)
    {}

    void    reset();
    bool    setLayout(std::vector<block_t>& layout);
    bool    setupCustomRequests(std::vector<customRequest_t>& customRequests);
    void    handleMessage(const uint8_t* array, uint16_t size);
    bool    isConfigurationEnabled();
    bool    isSilentModeEnabled();
    void    setSilentMode(bool state);
    void    sendCustomMessage(const uint16_t* values, uint16_t size, bool ack = true);
    uint8_t blocks() const;
    uint8_t sections(uint8_t blockID) const;

    // define these helper classes here so that they're usable without compiling the .cpp file

    ///
    /// \brief Helper class used to convert 7-bit high and low bytes to single 14-bit value.
    /// @param [in] high    Higher 7 bits.
    /// @param [in] low     Lower 7 bits.
    ///
    class Merge14bit
    {
        public:
        Merge14bit(uint8_t high, uint8_t low)
        {
            if (high & 0x01)
            {
                low |= (1 << 7);
            }
            else
            {
                low &= ~(1 << 7);
            }

            high >>= 1;

            uint16_t joined;
            joined = high;
            joined <<= 8;
            joined |= low;

            _value = joined;
        }

        uint16_t value() const
        {
            return _value;
        }

        private:
        uint16_t _value;
    };

    ///
    /// \brief Helper class used to convert single 14-bit value to high and low bytes (7-bit each).
    /// @param [in]     value   14-bit value to split.
    ///
    class Split14bit
    {
        public:
        Split14bit(uint16_t value)
        {
            uint8_t newHigh = (value >> 8) & 0xFF;
            uint8_t newLow  = value & 0xFF;
            newHigh         = (newHigh << 1) & 0x7F;

            if ((newLow >> 7) & 0x01)
            {
                newHigh |= 0x01;
            }
            else
            {
                newHigh &= ~0x01;
            }

            newLow &= 0x7F;
            _high = newHigh;
            _low  = newLow;
        }

        uint8_t high() const
        {
            return _high;
        }

        uint8_t low() const
        {
            return _low;
        }

        private:
        uint8_t _high;
        uint8_t _low;
    };

    private:
    bool     addToResponse(uint16_t value);
    bool     decode(const uint8_t* receivedArray, uint16_t receivedArraySize);
    void     resetDecodedMessage();
    bool     processStandardRequest(uint16_t receivedArraySize);
    bool     processSpecialRequest();
    bool     checkID();
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

        _responseArray[static_cast<uint8_t>(byteOrder_t::statusByte)] = status_uint8;
    }

    void sendResponse(bool containsLastByte, bool customMessage = false);

    ///
    /// \brief Reference to object performing reading and writing of actual data.
    ///
    DataHandler& _dataHandler;

    ///
    /// \brief Reference to structure containing manufacturer ID bytes.
    ///
    const manufacturerID_t& _mID;

    ///
    /// \brief Array in which response will be stored.
    ///
    uint8_t _responseArray[MAX_MESSAGE_SIZE];

    ///
    /// \brief Holds current size of response array.
    ///
    uint16_t _responseCounter = 0;

    ///
    /// \brief Flag indicating whether or not configuration is possible.
    ///
    bool _sysExEnabled = false;

    ///
    /// \brief Flag indicating whether or not silent mode is active.
    /// When silent mode is active, protocol won't return any error or status_t:ack messages.
    ///
    bool _silentModeEnabled = false;

    ///
    /// \brief SysEx layout.
    ///
    std::vector<block_t> _layout = {};

    ///
    /// \brief Structure containing decoded data from SysEx request for easier access.
    ///
    decodedMessage_t _decodedMessage;

    ///
    /// \brief Vector of structures containing data for custom requests.
    ///
    std::vector<customRequest_t> _sysExCustomRequest = {};
};

/// @}