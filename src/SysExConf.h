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

#include <inttypes.h>
#include <stdlib.h>

///
/// \brief Configuration protocol created using custom SysEx MIDI messages.
/// @{
///

class SysExConf
{
    public:
    using sysExParameter_t = uint16_t;

    ///
    /// \brief Structure holding SysEx manufacturer ID bytes.
    ///
    typedef struct
    {
        uint8_t id1;
        uint8_t id2;
        uint8_t id3;
    } manufacturerID_t;

    ///
    /// \brief Structure holding data for single SysEx section within block.
    ///
    typedef struct
    {
        size_t           numberOfParameters;
        sysExParameter_t newValueMin;
        sysExParameter_t newValueMax;
        uint8_t          parts;
    } section_t;

    ///
    /// \brief Structure holding data for single SysEx block.
    ///
    typedef struct
    {
        uint8_t    numberOfSections;
        section_t* section;
    } block_t;

    ///
    /// \brief Structure containing data for single custom request.
    ///
    typedef struct
    {
        size_t requestID;        ///< ID byte representing specific request.
        bool   connOpenCheck;    ///< Flag indicating whether or not SysEx connection should be enabled before processing request.
    } customRequest_t;

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
        request,               //0x00
        ack,                   //0x01
        errorStatus,           //0x02
        errorConnection,       //0x03
        errorWish,             //0x04
        errorAmount,           //0x05
        errorBlock,            //0x06
        errorSection,          //0x07
        errorPart,             //0x08
        errorIndex,            //0x09
        errorNewValue,         //0x0A
        errorMessageLength,    //0x0B
        errorWrite,            //0x0C
        errorNotSupported,     //0x0D
        errorRead,             //0x0E
    };

    ///
    /// \brief List of special SysEx IDs.
    ///
    enum class specialRequest_t : uint8_t
    {
        connClose,            //0x00
        connOpen,             //0x01
        bytesPerValue,        //0x02
        paramsPerMessage,     //0x03
        connOpenSilent,       //0x04
        connSilentDisable,    //0x05
        AMOUNT
    };

    ///
    /// \brief Structure holding decoded request data.
    ///
    typedef struct
    {
        status_t         status;
        wish_t           wish;
        amount_t         amount;
        uint8_t          block;
        uint8_t          section;
        uint8_t          part;
        sysExParameter_t index;
        sysExParameter_t newValue;
    } decodedMessage_t;

    ///
    /// \brief List of all possible sizes for parameter indexes and values used during configuration.
    ///
    enum class paramSize_t : uint8_t
    {
        _7bit  = 1,
        _14bit = 2
    };

    ///
    /// \brief List of all possible sizes for number of parameters specified per single SysEx message.
    ///
    enum class nrOfParam_t : uint8_t
    {
        _32 = 32,
        _64 = 64
    };

    class DataHandler
    {
        public:
        DataHandler() {}

        enum class result_t : uint8_t
        {
            ok,
            error,
            notSupported,
        };

        virtual result_t get(uint8_t block, uint8_t section, size_t index, SysExConf::sysExParameter_t& value)   = 0;
        virtual result_t set(uint8_t block, uint8_t section, size_t index, SysExConf::sysExParameter_t newValue) = 0;
        virtual result_t customRequest(size_t value, uint8_t*& array, size_t& size)                              = 0;
        virtual void     sendResponse(uint8_t* array, size_t size)                                               = 0;
    };

    SysExConf(DataHandler&      dataHandler,
              manufacturerID_t& mID,
              uint8_t*          responseArray,
              size_t            responseArraySize,
              paramSize_t       paramSize,
              nrOfParam_t       nrOfParam)
        : dataHandler(dataHandler)
        , mID(mID)
        , responseArray(responseArray)
        , responseArraySize(responseArraySize)
        , paramSize(paramSize)
        , nrOfParam(nrOfParam)
    {}

    void reset();
    bool setLayout(block_t* pointer, uint8_t numberOfBlocks);
    bool setupCustomRequests(customRequest_t* customRequests, size_t numberOfCustomRequests);
    void handleMessage(const uint8_t* sysExArray, size_t size);
    bool isConfigurationEnabled();
    bool isSilentModeEnabled();
    void setSilentMode(bool state);
    void sendCustomMessage(const sysExParameter_t* values, size_t size, bool ack = true);

    static void split14bit(uint16_t value, uint8_t& high, uint8_t& low);
    static void mergeTo14bit(uint16_t& value, uint8_t high, uint8_t low);

    private:
    bool   addToResponse(sysExParameter_t value);
    bool   decode(const uint8_t* receivedArray, size_t receivedArraySize);
    void   resetDecodedMessage();
    bool   processStandardRequest(size_t receivedArraySize);
    bool   processSpecialRequest();
    bool   checkID();
    bool   checkStatus();
    bool   checkWish();
    bool   checkAmount();
    bool   checkBlock();
    bool   checkSection();
    bool   checkPart();
    bool   checkParameterIndex();
    bool   checkNewValue();
    bool   checkParameters();
    size_t generateMessageLenght();
    void   setStatus(status_t status);
    void   sendResponse(bool containsLastByte);

    ///
    /// \brief Descriptive list of bytes in SysEx message.
    ///
    typedef enum
    {
        startByte,      //0
        idByte_1,       //1
        idByte_2,       //2
        idByte_3,       //3
        statusByte,     //4
        partByte,       //5
        wishByte,       //6
        amountByte,     //7
        blockByte,      //8
        sectionByte,    //9
        REQUEST_SIZE,
        RESPONSE_SIZE      = partByte + 1,
        MIN_MESSAGE_LENGTH = (wishByte + 1) + 1,    //special requests
        ML_REQ_STANDARD    = REQUEST_SIZE + 1,      //add end byte
        indexByte          = REQUEST_SIZE,
    } sysExByteOrder;

    ///
    /// \brief Reference to object performing reading and writing of actual data.
    ///
    DataHandler& dataHandler;

    ///
    /// \brief Reference to structure containing manufacturer ID bytes.
    ///
    manufacturerID_t& mID;

    ///
    /// \brief Pointer to array in which response will be stored.
    ///
    uint8_t* const responseArray;

    ///
    /// \brief Holds maximum size of response array.
    ///
    const size_t responseArraySize;

    ///
    /// \brief Holds current size of response array.
    ///
    size_t responseCounter = 0;

    ///
    /// \brief Holds size of SysEx parameter indexes and values.
    ///
    const paramSize_t paramSize;

    ///
    /// \brief Holds total number of parameters per single SysEx message.
    ///
    const nrOfParam_t nrOfParam;

    ///
    /// \brief Flag indicating whether or not configuration is possible.
    ///
    bool sysExEnabled = false;

    ///
    /// \brief Flag indicating whether or not silent mode is active.
    /// When silent mode is active, protocol won't return any error or status_t:ack messages.
    ///
    bool silentModeEnabled = false;

    ///
    /// \brief Pointer to SysEx layout.
    ///
    block_t* sysExMessage = nullptr;

    ///
    /// \brief Total number of blocks for a received SysEx layout.
    ///
    uint8_t sysExBlockCounter = 0;

    ///
    /// \brief Structure containing decoded data from SysEx request for easier access.
    ///
    decodedMessage_t decodedMessage;

    ///
    /// \brief Pointer to structure containing data for custom requests.
    ///
    customRequest_t* sysExCustomRequest = nullptr;

    ///
    /// \brief Total number of custom SysEx requests stored in pointed structure.
    ///
    size_t numberOfCustomRequests = 0;

    ///
    /// \brief Holds amount of user-specified custom requests.
    ///
    size_t customRequestCounter = 0;
};

/// @}