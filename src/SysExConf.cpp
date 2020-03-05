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

#include "SysExConf.h"

///
/// \brief Resets all variables to their default values.
///
void SysExConf::reset()
{
    sysExEnabled           = false;
    silentModeEnabled      = false;
    sysExMessage           = nullptr;
    sysExBlockCounter      = 0;
    decodedMessage         = {};
    sysExCustomRequest     = nullptr;
    numberOfCustomRequests = 0;
    customRequestCounter   = 0;
    responseCounter        = 0;
}

///
/// Configures user specifed configuration layout and initializes data to their default values.
/// @param [in] pointer     Pointer to configuration structure.
/// @param [in] numberOfBlocks  Total number of blocks in configuration structure.
/// \returns True on success, false otherwise.
///
bool SysExConf::setLayout(block_t* pointer, uint8_t numberOfBlocks)
{
    sysExEnabled = false;

    customRequestCounter = 0;

    if ((pointer != nullptr) && numberOfBlocks)
    {
        sysExMessage      = pointer;
        sysExBlockCounter = numberOfBlocks;

        for (int i = 0; i < numberOfBlocks; i++)
        {
            for (int j = 0; j < sysExMessage[i].numberOfSections; j++)
            {
                //based on number of parameters, calculate how many parts message has in case of set/all request and get/all response
                sysExMessage[i].section[j].parts = sysExMessage[i].section[j].numberOfParameters / static_cast<uint8_t>(nrOfParam);

                if (sysExMessage[i].section[j].numberOfParameters % static_cast<uint8_t>(nrOfParam))
                    sysExMessage[i].section[j].parts++;
            }
        }

        return true;
    }

    return false;
}

///
/// \brief Configures custom requests stored in external structure.
/// @param [in] customRequests          Pointer to structure containing custom requests.
/// @param [in] numberOfCustomRequests  Total number of requests stored in specified structure.
/// \returns True on success, false otherwise.
///
bool SysExConf::setupCustomRequests(customRequest_t* pointer, size_t _numberOfCustomRequests)
{
    if ((pointer != nullptr) && _numberOfCustomRequests)
    {
        sysExCustomRequest = pointer;

        for (size_t i = 0; i < _numberOfCustomRequests; i++)
        {
            if (sysExCustomRequest[i].requestID < static_cast<uint8_t>(specialRequest_t::AMOUNT))
            {
                sysExCustomRequest     = nullptr;
                numberOfCustomRequests = 0;
                return false;    //id already used internally
            }
        }

        numberOfCustomRequests = _numberOfCustomRequests;
        return true;
    }

    return false;
}

///
/// \brief Checks whether the SysEx configuration is enabled or not.
/// \returns True if enabled, false otherwise.
///
bool SysExConf::isConfigurationEnabled()
{
    return sysExEnabled;
}

///
/// \brief Checks whether silent mode is enabled or not.
/// \returns True if enabled, false otherwise.
///
bool SysExConf::isSilentModeEnabled()
{
    return silentModeEnabled;
}

///
/// \brief Enables or disables silent protocol mode.
/// When enabled, only GET and BACKUP requests will return response.
///
void SysExConf::setSilentMode(bool state)
{
    silentModeEnabled = state;
}

///
/// \brief Handles incoming SysEx message.
/// @param [in] array   SysEx array.
/// @param [in] size    Array size.
///
void SysExConf::handleMessage(const uint8_t* array, size_t size)
{
    if ((sysExMessage == nullptr) || !sysExBlockCounter || !size)
        return;

    if (size < MIN_MESSAGE_LENGTH)
        return;    //ignore small messages

    if (array[0] != 0xF0)
        return;

    if (array[size - 1] != 0xF7)
        return;

    resetDecodedMessage();

    //copy entire incoming message to internal buffer
    for (size_t i = 0; i < size; i++)
        responseArray[i] = array[i];

    //response counter should at this point hold index of the array at which
    //response will differ from the original message, which is why we're not assigning
    //responseCounter to be total size of received array
    responseCounter = RESPONSE_SIZE;

    if (!checkID())
        return;    //don't send response to wrong ID

    bool sendResponse_var = true;

    if (!checkStatus())
    {
        setStatus(status_t::errorStatus);
    }
    else
    {
        if (decode(array, size))
        {
            if (size == MIN_MESSAGE_LENGTH)
            {
                processSpecialRequest();
            }
            else
            {
                if (processStandardRequest(size))
                {
                    //in this case, processStandardRequest will internally call
                    //sendResponse function, which means it's not necessary to call
                    //it again here
                    sendResponse_var = false;
                }
                else
                {
                    //function returned error
                    //send response manually and reset decoded message
                    resetDecodedMessage();
                }
            }
        }
        else
        {
            resetDecodedMessage();
        }
    }

    if (sendResponse_var)
        sendResponse(false);
}

///
/// \brief Resets all elements in decodedMessage structure to default values.
///
void SysExConf::resetDecodedMessage()
{
    //reset decodedMessage
    decodedMessage.status   = status_t::ack;
    decodedMessage.wish     = wish_t::invalid;
    decodedMessage.amount   = amount_t::invalid;
    decodedMessage.block    = 0;
    decodedMessage.section  = 0;
    decodedMessage.part     = 0;
    decodedMessage.index    = 0;
    decodedMessage.newValue = 0;
}

///
/// \brief Decodes received message.
/// \returns True on success, false otherwise (invalid values).
///
bool SysExConf::decode(const uint8_t* receivedArray, size_t receivedArraySize)
{
    if (receivedArraySize == MIN_MESSAGE_LENGTH)
    {
        //special request
        return true;    //checked in processSpecialRequest
    }
    else if (receivedArraySize <= REQUEST_SIZE)
    {
        setStatus(status_t::errorMessageLength);
        return false;
    }
    else
    {
        if (!sysExEnabled)
        {
            //connection open request hasn't been received
            setStatus(status_t::errorConnection);
            return false;
        }

        //don't try to request these parameters if the size is too small
        decodedMessage.part    = receivedArray[(uint8_t)partByte];
        decodedMessage.wish    = static_cast<wish_t>(receivedArray[(uint8_t)wishByte]);
        decodedMessage.amount  = static_cast<amount_t>(receivedArray[(uint8_t)amountByte]);
        decodedMessage.block   = receivedArray[(uint8_t)blockByte];
        decodedMessage.section = receivedArray[(uint8_t)sectionByte];

        if (!checkWish())
        {
            setStatus(status_t::errorWish);
            return false;
        }

        if (!checkBlock())
        {
            setStatus(status_t::errorBlock);
            return false;
        }

        if (!checkSection())
        {
            setStatus(status_t::errorSection);
            return false;
        }

        if (!checkAmount())
        {
            setStatus(status_t::errorAmount);
            return false;
        }

        if (!checkPart())
        {
            setStatus(status_t::errorPart);
            return false;
        }

        if (receivedArraySize != generateMessageLenght())
        {
            setStatus(status_t::errorMessageLength);
            return false;
        }

        //start building response
        setStatus(status_t::ack);

        if (decodedMessage.amount == amount_t::single)
        {
            if (paramSize == paramSize_t::_14bit)
                mergeTo14bit(decodedMessage.index, receivedArray[indexByte], receivedArray[indexByte + 1]);
            else
                decodedMessage.index = receivedArray[indexByte];

            decodedMessage.index += (static_cast<uint8_t>(nrOfParam) * decodedMessage.part);

            if (decodedMessage.wish == wish_t::set)
            {
                //new value
                if (paramSize == paramSize_t::_14bit)
                    mergeTo14bit(decodedMessage.newValue, receivedArray[indexByte + static_cast<uint8_t>(paramSize)], receivedArray[indexByte + static_cast<uint8_t>(paramSize) + 1]);
                else
                    decodedMessage.newValue = receivedArray[indexByte + static_cast<uint8_t>(paramSize)];
            }
        }
    }

    return true;
}

///
/// \brief Used to process standard SysEx request.
/// \returns True on success, false otherwise.
///
bool SysExConf::processStandardRequest(size_t receivedArraySize)
{
    size_t  startIndex = 0, endIndex = 1;
    uint8_t msgPartsLoop = 1, responseCounter_ = responseCounter;
    bool    allPartsAck  = false;
    bool    allPartsLoop = false;

    if ((decodedMessage.wish == wish_t::backup) || (decodedMessage.wish == wish_t::get))
    {
        if ((decodedMessage.part == 127) || (decodedMessage.part == 126))
        {
            //when parts 127 or 126 are specified, protocol will loop over all message parts and
            //deliver as many messages as there are parts as response
            msgPartsLoop = sysExMessage[decodedMessage.block].section[decodedMessage.section].parts;
            allPartsLoop = true;

            //when part is set to 126 (0x7E), status_t::ack message will be sent as the last message
            //indicating that all messages have been sent as response to specific request
            if (decodedMessage.part == 126)
                allPartsAck = true;
        }

        if (decodedMessage.wish == wish_t::backup)
        {
            //convert response to request
            responseArray[(uint8_t)statusByte] = static_cast<uint8_t>(status_t::request);
            //now convert wish to set
            responseArray[(uint8_t)wishByte] = (uint8_t)wish_t::set;
            //decoded message wish needs to be set to get so that we can retrieve parameters
            decodedMessage.wish = wish_t::get;
            //don't overwrite anything when backup is requested - just append
            responseCounter_ = receivedArraySize - 1;
        }
    }

    for (int j = 0; j < msgPartsLoop; j++)
    {
        responseCounter = responseCounter_;

        if (allPartsLoop)
        {
            decodedMessage.part     = j;
            responseArray[partByte] = j;
        }

        if (decodedMessage.amount == amount_t::all)
        {
            startIndex = static_cast<uint8_t>(nrOfParam) * decodedMessage.part;
            endIndex   = startIndex + static_cast<uint8_t>(nrOfParam);

            if (endIndex > sysExMessage[decodedMessage.block].section[decodedMessage.section].numberOfParameters)
                endIndex = sysExMessage[decodedMessage.block].section[decodedMessage.section].numberOfParameters;
        }

        for (uint16_t i = startIndex; i < endIndex; i++)
        {
            switch (decodedMessage.wish)
            {
            case wish_t::get:
                if (decodedMessage.amount == amount_t::single)
                {
                    if (!checkParameterIndex())
                    {
                        setStatus(status_t::errorIndex);
                        return false;
                    }
                    else
                    {
                        sysExParameter_t      value  = 0;
                        DataHandler::result_t result = dataHandler.get(decodedMessage.block, decodedMessage.section, decodedMessage.index, value);

                        switch (result)
                        {
                        case DataHandler::result_t::ok:
                            addToResponse(value);
                            break;

                        case DataHandler::result_t::error:
                            setStatus(status_t::errorRead);
                            return false;

                        case DataHandler::result_t::notSupported:
                            setStatus(status_t::errorNotSupported);
                            return false;
                        }
                    }
                }
                else
                {
                    //get all params - no index is specified
                    sysExParameter_t      value  = 0;
                    DataHandler::result_t result = dataHandler.get(decodedMessage.block, decodedMessage.section, i, value);

                    switch (result)
                    {
                    case DataHandler::result_t::ok:
                        addToResponse(value);
                        break;

                    case DataHandler::result_t::error:
                        setStatus(status_t::errorRead);
                        return false;

                    case DataHandler::result_t::notSupported:
                        setStatus(status_t::errorNotSupported);
                        return false;
                    }
                }
                break;

            default:
                // case wish_t::set:
                if (decodedMessage.amount == amount_t::single)
                {
                    if (!checkParameterIndex())
                    {
                        setStatus(status_t::errorIndex);
                        return false;
                    }

                    if (!checkNewValue())
                    {
                        setStatus(status_t::errorNewValue);
                        return false;
                    }

                    DataHandler::result_t result = dataHandler.set(decodedMessage.block, decodedMessage.section, decodedMessage.index, decodedMessage.newValue);

                    switch (result)
                    {
                    case DataHandler::result_t::ok:
                        break;

                    case DataHandler::result_t::error:
                        setStatus(status_t::errorWrite);
                        return false;

                    case DataHandler::result_t::notSupported:
                        setStatus(status_t::errorNotSupported);
                        return false;
                    }
                }
                else
                {
                    uint8_t arrayIndex = (i - startIndex);

                    if (paramSize == paramSize_t::_14bit)
                    {
                        arrayIndex *= static_cast<uint8_t>(paramSize);
                        arrayIndex += indexByte;

                        mergeTo14bit(decodedMessage.newValue, responseArray[arrayIndex], responseArray[arrayIndex + 1]);
                    }
                    else
                    {
                        decodedMessage.newValue = responseArray[arrayIndex + indexByte];
                    }

                    if (!checkNewValue())
                    {
                        setStatus(status_t::errorNewValue);
                        return false;
                    }

                    DataHandler::result_t result = dataHandler.set(decodedMessage.block, decodedMessage.section, i, decodedMessage.newValue);

                    switch (result)
                    {
                    case DataHandler::result_t::ok:
                        break;

                    case DataHandler::result_t::error:
                        setStatus(status_t::errorWrite);
                        return false;

                    case DataHandler::result_t::notSupported:
                        setStatus(status_t::errorNotSupported);
                        return false;
                    }
                }
                break;
            }
        }

        sendResponse(false);
    }

    if (allPartsAck)
    {
        //send status_t::ack message at the end
        responseCounter                  = 0;
        responseArray[responseCounter++] = 0xF0;
        responseArray[responseCounter++] = mID.id1;
        responseArray[responseCounter++] = mID.id2;
        responseArray[responseCounter++] = mID.id3;
        responseArray[responseCounter++] = static_cast<uint8_t>(status_t::ack);
        responseArray[responseCounter++] = 0x7E;
        sendResponse(false);
    }

    return true;
}

///
/// \brief Checks whether the manufacturer ID in message is correct.
/// @returns    True if valid, false otherwise.
///
bool SysExConf::checkID()
{
    return (
        (responseArray[idByte_1] == mID.id1) &&
        (responseArray[idByte_2] == mID.id2) &&
        (responseArray[idByte_3] == mID.id3));
}

///
/// \brief Checks whether the status byte in request is correct.
/// @returns    True if valid, false otherwise.
///
bool SysExConf::checkStatus()
{
    return (static_cast<status_t>(responseArray[static_cast<uint8_t>(statusByte)]) == status_t::request);
}

///
/// \brief Used to process special SysEx request.
/// \returns True on success, false otherwise.
///
bool SysExConf::processSpecialRequest()
{
    switch (responseArray[wishByte])
    {
    case static_cast<uint8_t>(specialRequest_t::connClose):
        if (!sysExEnabled)
        {
            //connection can't be closed if it isn't opened
            setStatus(status_t::errorConnection);
            return true;
        }
        else
        {
            //close sysex connection
            sysExEnabled = false;
            setStatus(status_t::ack);

            if (silentModeEnabled)
            {
                //also disable silent mode
                silentModeEnabled = false;
            }
            return true;
        }
        break;

    case static_cast<uint8_t>(specialRequest_t::connOpen):
    case static_cast<uint8_t>(specialRequest_t::connOpenSilent):
        //necessary to allow the configuration
        sysExEnabled = true;

        if (responseArray[wishByte] == static_cast<uint8_t>(specialRequest_t::connOpenSilent))
            silentModeEnabled = true;

        setStatus(status_t::ack);
        return true;

    case static_cast<uint8_t>(specialRequest_t::connSilentDisable):
        silentModeEnabled = false;
        setStatus(status_t::ack);
        return true;

    case static_cast<uint8_t>(specialRequest_t::bytesPerValue):
        if (sysExEnabled)
        {
            setStatus(status_t::ack);
            responseArray[responseCounter++] = static_cast<uint8_t>(paramSize);
        }
        else
        {
            setStatus(status_t::errorConnection);
        }
        return true;

    case static_cast<uint8_t>(specialRequest_t::paramsPerMessage):
        if (sysExEnabled)
        {
            setStatus(status_t::ack);
            responseArray[responseCounter++] = static_cast<uint8_t>(nrOfParam);
        }
        else
        {
            setStatus(status_t::errorConnection);
        }
        return true;

    default:
        //check for custom value
        for (size_t i = 0; i < numberOfCustomRequests; i++)
        {
            //check only current wish/request
            if (sysExCustomRequest[i].requestID != responseArray[wishByte])
                continue;

            if (sysExEnabled || !sysExCustomRequest[i].connOpenCheck)
            {
                setStatus(status_t::ack);

                DataHandler::CustomResponse customResponse(responseArray, responseCounter);
                DataHandler::result_t       result = dataHandler.customRequest(sysExCustomRequest[i].requestID, customResponse);

                switch (result)
                {
                case DataHandler::result_t::error:
                    setStatus(status_t::errorRead);
                    return false;

                case DataHandler::result_t::notSupported:
                    setStatus(status_t::errorNotSupported);
                    return false;

                default:
                    break;
                }
            }
            else
            {
                setStatus(status_t::errorConnection);
            }

            return true;
        }

        //custom string not found
        setStatus(status_t::errorWish);
        return true;
    }
}

///
/// \brief Generates message length based on other parameters in message.
/// \returns    Message length in bytes.
///
size_t SysExConf::generateMessageLenght()
{
    size_t size = 0;

    switch (decodedMessage.amount)
    {
    case amount_t::single:
        switch (decodedMessage.wish)
        {
        case wish_t::get:
        case wish_t::backup:
            return ML_REQ_STANDARD + static_cast<uint8_t>(paramSize);    //add parameter length

        default:
            // case wish_t::set:
            return ML_REQ_STANDARD + 2 * static_cast<uint8_t>(paramSize);    //add parameter length and new value length
        }
        break;

    default:
        // case amount_t::all:
        switch (decodedMessage.wish)
        {
        case wish_t::get:
        case wish_t::backup:
            return ML_REQ_STANDARD;

        default:
            // case wish_t::set:
            size = sysExMessage[decodedMessage.block].section[decodedMessage.section].numberOfParameters;

            if (size > static_cast<uint8_t>(nrOfParam))
            {
                if ((decodedMessage.part + 1) == sysExMessage[decodedMessage.block].section[decodedMessage.section].parts)
                    size = size - ((sysExMessage[decodedMessage.block].section[decodedMessage.section].parts - 1) * static_cast<uint8_t>(nrOfParam));
                else
                    size = static_cast<uint8_t>(nrOfParam);
            }

            size *= static_cast<uint8_t>(paramSize);
            size += ML_REQ_STANDARD;
            return size;
        }
    }

    return size;
}

///
/// \brief Checks whether the wish value is valid.
/// \returns    True if valid, false otherwise.
///
bool SysExConf::checkWish()
{
    return (decodedMessage.wish <= wish_t::backup);
}

///
/// \brief Checks whether the amount value is valid.
/// \returns    True if valid, false otherwise.
///
bool SysExConf::checkAmount()
{
    return (decodedMessage.amount <= amount_t::all);
}

///
/// \brief Checks whether the block value is valid.
/// \returns    True if valid, false otherwise.
///
bool SysExConf::checkBlock()
{
    return decodedMessage.block < sysExBlockCounter;
}

///
/// \brief Checks whether the section value is valid.
/// \returns    True if valid, false otherwise.
///
bool SysExConf::checkSection()
{
    return (decodedMessage.section < sysExMessage[decodedMessage.block].numberOfSections);
}

///
/// \brief Checks whether the message part is valid.
/// \returns    True if valid, false otherwise.
///
bool SysExConf::checkPart()
{
    switch (decodedMessage.wish)
    {
    case wish_t::get:
        if ((decodedMessage.part == 127) || (decodedMessage.part == 126))
            return true;

        if (decodedMessage.part >= sysExMessage[decodedMessage.block].section[decodedMessage.section].parts)
            return false;
        else
            return true;
        break;

    default:
        // case wish_t::set:
        // case wish_t::backup:
        if (decodedMessage.wish == wish_t::backup)
        {
            if ((decodedMessage.part == 127) || (decodedMessage.part == 126))
                return true;
        }

        if (decodedMessage.amount == amount_t::all)
        {
            if (decodedMessage.part < sysExMessage[decodedMessage.block].section[decodedMessage.section].parts)
                return true;
            else
                return false;
        }
        else
        {
            if (decodedMessage.part >= sysExMessage[decodedMessage.block].section[decodedMessage.section].parts)
                return false;
            else
                return true;
        }
        break;
    }
}

///
/// \brief Checks whether the parameter index in message is valid.
/// \returns    True if valid, false otherwise.
///
bool SysExConf::checkParameterIndex()
{
    //block and section passed validation, check parameter index
    return (decodedMessage.index < sysExMessage[decodedMessage.block].section[decodedMessage.section].numberOfParameters);
}

///
/// \brief Checks whether the new value in message is valid.
/// \returns    True if valid, false otherwise.
///
bool SysExConf::checkNewValue()
{
    sysExParameter_t minValue = sysExMessage[decodedMessage.block].section[decodedMessage.section].newValueMin;
    sysExParameter_t maxValue = sysExMessage[decodedMessage.block].section[decodedMessage.section].newValueMax;

    if (minValue != maxValue)
        return ((decodedMessage.newValue >= minValue) && (decodedMessage.newValue <= maxValue));
    else
        return true;    //don't check new value if min and max are the same
}

///
/// \brief Used to send custom SysEx response.
/// @param [in] values          Array with values to send.
/// @param [in] size            Array size.
/// @param [in] ack             When set to true, status byte will be set to status_t::ack, otherwise status_t::request will be used.
///                             Set to true by default.
///
void SysExConf::sendCustomMessage(const sysExParameter_t* values, size_t size, bool ack)
{
    responseCounter = 0;

    responseArray[responseCounter++] = 0xF0;
    responseArray[responseCounter++] = mID.id1;
    responseArray[responseCounter++] = mID.id2;
    responseArray[responseCounter++] = mID.id3;

    if (ack)
        responseArray[responseCounter++] = static_cast<uint8_t>(status_t::ack);
    else
        responseArray[responseCounter++] = static_cast<uint8_t>(status_t::request);

    responseArray[responseCounter++] = 0;    //message part

    for (size_t i = 0; i < size; i++)
        responseArray[responseCounter++] = values[i];

    sendResponse(false);
}

///
/// \brief Used to send SysEx response.
/// @param [in] containsLastByte If set to true, last SysEx byte (0x07) won't be appended.
///
void SysExConf::sendResponse(bool containsLastByte)
{
    if (!containsLastByte)
    {
        responseArray[responseCounter++] = 0xF7;
    }

    if (silentModeEnabled && (decodedMessage.wish != wish_t::get))
        return;

    dataHandler.sendResponse(responseArray, responseCounter);
}

///
/// \brief Adds value to SysEx response.
/// This function append value to last specified SysEx array.
/// @param [in] value   New value.
/// \returns True on success, false otherwise.
///
bool SysExConf::addToResponse(sysExParameter_t value)
{
    if (paramSize == paramSize_t::_14bit)
    {
        uint8_t high;
        uint8_t low;

        split14bit(value, high, low);

        responseArray[responseCounter++] = high;
        responseArray[responseCounter++] = low;
    }
    else
    {
        if ((value != 0xF0) && (value != 0xF7))
        {
            if (value > 127)
                value = 127;
        }

        responseArray[responseCounter++] = (uint8_t)value;
    }

    return true;
}

///
/// \brief Sets status byte in SysEx response.
/// @param [in] status New status value. See status_t enum.
///
void SysExConf::setStatus(status_t status)
{
    responseArray[statusByte] = static_cast<uint8_t>(status);
}

///
/// \brief Convert single 14-bit value to high and low bytes (7-bit each).
/// @param [in]     value   14-bit value to split.
/// @param [in,out] high    Higher 7 bits of original 14-bit value.
/// @param [in,out] low     Lower 7 bits of original 14-bit value.
///
void SysExConf::split14bit(uint16_t value, uint8_t& high, uint8_t& low)
{
    uint8_t newHigh = (value >> 8) & 0xFF;
    uint8_t newLow  = value & 0xFF;
    newHigh         = (newHigh << 1) & 0x7F;

    if ((newLow >> 7) & 0x01)
        newHigh |= 0x01;
    else
        newHigh &= ~0x01;

    newLow &= 0x7F;
    high = newHigh;
    low  = newLow;
}

///
/// \brief Convert 7-bit high and low bytes to single 14-bit value.
/// @param [in,out] value Resulting 14-bit value.
/// @param [in,out] high    Higher 7 bits.
/// @param [in,out] low     Lower 7 bits.
///
void SysExConf::mergeTo14bit(uint16_t& value, uint8_t high, uint8_t low)
{
    if (high & 0x01)
        low |= (1 << 7);
    else
        low &= ~(1 << 7);

    high >>= 1;

    uint16_t joined;

    joined = high;
    joined <<= 8;
    joined |= low;

    value = joined;
}