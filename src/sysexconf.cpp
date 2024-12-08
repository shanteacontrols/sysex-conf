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

#include "lib/sysexconf/sysexconf.h"

#define LAYOUT_ACCESS (*_layout)

using namespace lib::sysexconf;

///
/// \brief Resets all variables to their default values.
///
void SysExConf::reset()
{
    _sysExEnabled               = false;
    _userErrorIgnoreModeEnabled = false;
    _decodedMessage             = {};
    _responseCounter            = 0;
    LAYOUT_ACCESS.clear();
    _sysExCustomRequest.clear();
}

///
/// Configures user specifed configuration layout and initializes data to their default values.
/// @param [in] sections     Vector containing all sections.
/// \returns True on success, false otherwise.
///
bool SysExConf::setLayout(std::vector<Block>& layout)
{
    _sysExEnabled = false;

    if (layout.size())
    {
        _layout = &layout;
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
bool SysExConf::setupCustomRequests(std::vector<CustomRequest>& customRequests)
{
    if (customRequests.size())
    {
        _sysExCustomRequest = std::move(customRequests);

        for (size_t i = 0; i < _sysExCustomRequest.size(); i++)
        {
            if (_sysExCustomRequest[i].requestId < static_cast<uint8_t>(specialRequest_t::AMOUNT))
            {
                _sysExCustomRequest = {};
                return false;    // id already used internally
            }
        }

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
    return _sysExEnabled;
}

///
/// \brief Enables or disables user error ignore mode.
/// When user error ignore mode is active, protocol will always return ACK
/// status for get/set messages ignoring the user status. For get requests,
/// retrieved value will be set to 0 and appended to response.
///
void SysExConf::setUserErrorIgnoreMode(bool state)
{
    _userErrorIgnoreModeEnabled = state;
}

///
/// \brief Handles incoming SysEx message.
/// @param [in] array   SysEx array.
/// @param [in] size    Array size.
///
void SysExConf::handleMessage(const uint8_t* array, uint16_t size)
{
    if (!LAYOUT_ACCESS.size())
    {
        return;
    }

    if (size < SPECIAL_REQ_MSG_SIZE)
    {
        return;    // ignore small messages
    }

    if (array[0] != 0xF0)
    {
        return;
    }

    if (array[size - 1] != 0xF7)
    {
        return;
    }

    if (size > MAX_MESSAGE_SIZE)
    {
        return;
    }

    resetDecodedMessage();

    // copy entire incoming message to internal buffer
    for (uint16_t i = 0; i < size; i++)
    {
        _responseArray[i] = array[i];
    }

    // for now, set the response counter to last position in request
    _responseCounter = size - 1;

    if (!checkManufacturerId())
    {
        return;    // don't send response to wrong ID
    }

    bool sendResponseVar = true;

    if (!checkStatus())
    {
        setStatus(status_t::ERROR_STATUS);
    }
    else
    {
        if (decode(array, size))
        {
            if (size == SPECIAL_REQ_MSG_SIZE)
            {
                processSpecialRequest();
            }
            else
            {
                if (processStandardRequest(size))
                {
                    // in this case, processStandardRequest will internally call
                    // sendResponse function, which means it's not necessary to call
                    // it again here
                    sendResponseVar = false;
                }
                else
                {
                    // function returned error
                    // send response manually and reset decoded message
                    resetDecodedMessage();
                }
            }
        }
        else
        {
            resetDecodedMessage();
        }
    }

    if (sendResponseVar)
    {
        sendResponse(false);
    }
}

///
/// \brief Resets all elements in decodedMessage structure to default values.
///
void SysExConf::resetDecodedMessage()
{
    _decodedMessage.status   = status_t::ACK;
    _decodedMessage.wish     = wish_t::INVALID;
    _decodedMessage.amount   = amount_t::INVALID;
    _decodedMessage.block    = 0;
    _decodedMessage.section  = 0;
    _decodedMessage.part     = 0;
    _decodedMessage.index    = 0;
    _decodedMessage.newValue = 0;
}

///
/// \brief Decodes received message.
/// \returns True on success, false otherwise (invalid values).
///
bool SysExConf::decode(const uint8_t* receivedArray, uint16_t receivedArraySize)
{
    if (receivedArraySize == SPECIAL_REQ_MSG_SIZE)
    {
        // special request
        return true;    // checked in processSpecialRequest
    }

    if (receivedArraySize < (STD_REQ_MIN_MSG_SIZE - BYTES_PER_VALUE))    // no index here
    {
        setStatus(status_t::ERROR_MESSAGE_LENGTH);
        return false;
    }

    if (!_sysExEnabled)
    {
        // connection open request hasn't been received
        setStatus(status_t::ERROR_CONNECTION);
        return false;
    }

    // don't try to request these parameters if the size is too small
    _decodedMessage.part    = receivedArray[static_cast<uint8_t>(byteOrder_t::PART_BYTE)];
    _decodedMessage.wish    = static_cast<wish_t>(receivedArray[static_cast<uint8_t>(byteOrder_t::WISH_BYTE)]);
    _decodedMessage.amount  = static_cast<amount_t>(receivedArray[static_cast<uint8_t>(byteOrder_t::AMOUNT_BYTE)]);
    _decodedMessage.block   = receivedArray[static_cast<uint8_t>(byteOrder_t::BLOCK_BYTE)];
    _decodedMessage.section = receivedArray[static_cast<uint8_t>(byteOrder_t::SECTION_BYTE)];

    if (!checkWish())
    {
        setStatus(status_t::ERROR_WISH);
        return false;
    }

    if (!checkBlock())
    {
        setStatus(status_t::ERROR_BLOCK);
        return false;
    }

    if (!checkSection())
    {
        setStatus(status_t::ERROR_SECTION);
        return false;
    }

    if (!checkAmount())
    {
        setStatus(status_t::ERROR_AMOUNT);
        return false;
    }

    if (!checkPart())
    {
        setStatus(status_t::ERROR_PART);
        return false;
    }

    if (receivedArraySize != generateMessageLenght())
    {
        setStatus(status_t::ERROR_MESSAGE_LENGTH);
        return false;
    }

    // start building response
    setStatus(status_t::ACK);

    if (_decodedMessage.amount == amount_t::SINGLE)
    {
        auto mergedIndex      = Merge14Bit(receivedArray[static_cast<uint8_t>(byteOrder_t::INDEX_BYTE)], receivedArray[static_cast<uint8_t>(byteOrder_t::INDEX_BYTE) + 1]);
        _decodedMessage.index = mergedIndex.value();

        if (_decodedMessage.wish == wish_t::SET)
        {
            auto mergedNewValue      = Merge14Bit(receivedArray[static_cast<uint8_t>(byteOrder_t::INDEX_BYTE) + BYTES_PER_VALUE], receivedArray[static_cast<uint8_t>(byteOrder_t::INDEX_BYTE) + BYTES_PER_VALUE + 1]);
            _decodedMessage.newValue = mergedNewValue.value();
        }
    }

    return true;
}

///
/// \brief Used to process standard SysEx request.
/// \returns True on success, false otherwise.
///
bool SysExConf::processStandardRequest(uint16_t receivedArraySize)
{
    uint16_t startIndex = 0, endIndex = 1;
    uint8_t  msgPartsLoop = 1, responseCounterLocal = _responseCounter;
    bool     allPartsAck  = false;
    bool     allPartsLoop = false;

    if ((_decodedMessage.wish == wish_t::BACKUP) || (_decodedMessage.wish == wish_t::GET))
    {
        if ((_decodedMessage.part == 127) || (_decodedMessage.part == 126))
        {
            // when parts 127 or 126 are specified, protocol will loop over all message parts and
            // deliver as many messages as there are parts as response
            msgPartsLoop = LAYOUT_ACCESS[_decodedMessage.block]._sections[_decodedMessage.section].parts();
            allPartsLoop = true;

            // when part is set to 126 (0x7E), status_t::ack message will be sent as the last message
            // indicating that all messages have been sent as response to specific request
            if (_decodedMessage.part == 126)
            {
                allPartsAck = true;
            }
        }

        if (_decodedMessage.wish == wish_t::BACKUP)
        {
            // convert response to request
            _responseArray[static_cast<uint8_t>(byteOrder_t::STATUS_BYTE)] = static_cast<uint8_t>(status_t::REQUEST);
            // now convert wish to set
            _responseArray[static_cast<uint8_t>(byteOrder_t::WISH_BYTE)] = (uint8_t)wish_t::SET;
            // decoded message wish needs to be set to get so that we can retrieve parameters
            _decodedMessage.wish = wish_t::GET;
            // when backup is request, erase received index/new value in response
            responseCounterLocal = receivedArraySize - 1 - (2 * BYTES_PER_VALUE);
        }
    }

    for (int j = 0; j < msgPartsLoop; j++)
    {
        _responseCounter = responseCounterLocal;

        if (allPartsLoop)
        {
            _decodedMessage.part                                         = j;
            _responseArray[static_cast<uint8_t>(byteOrder_t::PART_BYTE)] = j;
        }

        if (_decodedMessage.amount == amount_t::ALL)
        {
            startIndex = PARAMS_PER_MESSAGE * _decodedMessage.part;
            endIndex   = startIndex + PARAMS_PER_MESSAGE;

            if (endIndex > LAYOUT_ACCESS[_decodedMessage.block]._sections[_decodedMessage.section].numberOfParameters())
            {
                endIndex = LAYOUT_ACCESS[_decodedMessage.block]._sections[_decodedMessage.section].numberOfParameters();
            }
        }

        for (uint16_t i = startIndex; i < endIndex; i++)
        {
            switch (_decodedMessage.wish)
            {
            case wish_t::GET:
            {
                if (_decodedMessage.amount == amount_t::SINGLE)
                {
                    if (!checkParameterIndex())
                    {
                        setStatus(status_t::ERROR_INDEX);
                        return false;
                    }

                    uint16_t value  = 0;
                    uint8_t  result = _dataHandler.get(_decodedMessage.block, _decodedMessage.section, _decodedMessage.index, value);

                    switch (result)
                    {
                    case static_cast<uint8_t>(status_t::ACK):
                    {
                        addToResponse(value);
                    }
                    break;

                    default:
                    {
                        if (_userErrorIgnoreModeEnabled)
                        {
                            value = 0;
                            addToResponse(value);
                        }
                        else
                        {
                            setStatus(result);
                            return false;
                        }
                    }
                    break;
                    }
                }
                else
                {
                    // get all params - no index is specified
                    uint16_t value  = 0;
                    uint8_t  result = _dataHandler.get(_decodedMessage.block, _decodedMessage.section, i, value);

                    switch (result)
                    {
                    case static_cast<uint8_t>(status_t::ACK):
                    {
                        addToResponse(value);
                    }
                    break;

                    default:
                    {
                        if (_userErrorIgnoreModeEnabled)
                        {
                            value = 0;
                            addToResponse(value);
                        }
                        else
                        {
                            setStatus(result);
                            return false;
                        }
                    }
                    break;
                    }
                }
            }
            break;

            default:
            {
                // case wish_t::set:
                if (_decodedMessage.amount == amount_t::SINGLE)
                {
                    if (!checkParameterIndex())
                    {
                        setStatus(status_t::ERROR_INDEX);
                        return false;
                    }

                    if (!checkNewValue())
                    {
                        setStatus(status_t::ERROR_NEW_VALUE);
                        return false;
                    }

                    uint8_t result = _dataHandler.set(_decodedMessage.block, _decodedMessage.section, _decodedMessage.index, _decodedMessage.newValue);

                    switch (result)
                    {
                    case static_cast<uint8_t>(status_t::ACK):
                        break;

                    default:
                    {
                        if (!_userErrorIgnoreModeEnabled)
                        {
                            setStatus(result);
                            return false;
                        }
                    }
                    break;
                    }
                }
                else
                {
                    uint8_t arrayIndex = (i - startIndex);

                    arrayIndex *= BYTES_PER_VALUE;
                    arrayIndex += static_cast<uint8_t>(byteOrder_t::INDEX_BYTE);

                    auto merge               = Merge14Bit(_responseArray[arrayIndex], _responseArray[arrayIndex + 1]);
                    _decodedMessage.newValue = merge.value();

                    if (!checkNewValue())
                    {
                        setStatus(status_t::ERROR_NEW_VALUE);
                        return false;
                    }

                    uint8_t result = _dataHandler.set(_decodedMessage.block, _decodedMessage.section, i, _decodedMessage.newValue);

                    switch (result)
                    {
                    case static_cast<uint8_t>(status_t::ACK):
                        break;

                    default:
                    {
                        if (!_userErrorIgnoreModeEnabled)
                        {
                            setStatus(result);
                            return false;
                        }
                    }
                    break;
                    }
                }
            }
            break;
            }
        }

        sendResponse(false);
    }

    if (allPartsAck)
    {
        // send status_t::ack message at the end
        _responseCounter                   = 0;
        _responseArray[_responseCounter++] = 0xF0;
        _responseArray[_responseCounter++] = _manufacturerId.id1;
        _responseArray[_responseCounter++] = _manufacturerId.id2;
        _responseArray[_responseCounter++] = _manufacturerId.id3;
        _responseArray[_responseCounter++] = static_cast<uint8_t>(status_t::ACK);
        _responseArray[_responseCounter++] = 0x7E;
        _responseArray[_responseCounter++] = static_cast<uint8_t>(_decodedMessage.wish);
        _responseArray[_responseCounter++] = static_cast<uint8_t>(_decodedMessage.amount);
        _responseArray[_responseCounter++] = static_cast<uint8_t>(_decodedMessage.block);
        _responseArray[_responseCounter++] = static_cast<uint8_t>(_decodedMessage.section);
        _responseArray[_responseCounter++] = 0;
        _responseArray[_responseCounter++] = 0;
        _responseArray[_responseCounter++] = 0;
        _responseArray[_responseCounter++] = 0;

        sendResponse(false);
    }

    return true;
}

///
/// \brief Checks whether the manufacturer ID in message is correct.
/// @returns    True if valid, false otherwise.
///
bool SysExConf::checkManufacturerId()
{
    return (
        (_responseArray[static_cast<uint8_t>(byteOrder_t::ID_BYTE_1)] == _manufacturerId.id1) &&
        (_responseArray[static_cast<uint8_t>(byteOrder_t::ID_BYTE_2)] == _manufacturerId.id2) &&
        (_responseArray[static_cast<uint8_t>(byteOrder_t::ID_BYTE_3)] == _manufacturerId.id3));
}

///
/// \brief Checks whether the status byte in request is correct.
/// @returns    True if valid, false otherwise.
///
bool SysExConf::checkStatus()
{
    return (static_cast<status_t>(_responseArray[static_cast<uint8_t>(byteOrder_t::STATUS_BYTE)]) == status_t::REQUEST);
}

///
/// \brief Used to process special SysEx request.
/// \returns True on success, false otherwise.
///
bool SysExConf::processSpecialRequest()
{
    switch (_responseArray[static_cast<uint8_t>(byteOrder_t::WISH_BYTE)])
    {
    case static_cast<uint8_t>(specialRequest_t::CONN_CLOSE):
    {
        if (!_sysExEnabled)
        {
            // connection can't be closed if it isn't opened
            setStatus(status_t::ERROR_CONNECTION);
            return true;
        }

        // close sysex connection
        _sysExEnabled = false;
        setStatus(status_t::ACK);

        return true;
    }
    break;

    case static_cast<uint8_t>(specialRequest_t::CONN_OPEN):
    {
        // necessary to allow the configuration
        _sysExEnabled = true;
        setStatus(status_t::ACK);

        return true;
    }
    break;

    case static_cast<uint8_t>(specialRequest_t::BYTES_PER_VALUE):
    {
        if (_sysExEnabled)
        {
            setStatus(status_t::ACK);

            _responseArray[_responseCounter++] = 0;
            _responseArray[_responseCounter++] = BYTES_PER_VALUE;
        }
        else
        {
            setStatus(status_t::ERROR_CONNECTION);
        }
        return true;
    }
    break;

    case static_cast<uint8_t>(specialRequest_t::PARAMS_PER_MESSAGE):
    {
        if (_sysExEnabled)
        {
            setStatus(status_t::ACK);

            _responseArray[_responseCounter++] = 0;
            _responseArray[_responseCounter++] = PARAMS_PER_MESSAGE;
        }
        else
        {
            setStatus(status_t::ERROR_CONNECTION);
        }

        return true;
    }
    break;

    default:
    {
        // check for custom value
        for (size_t i = 0; i < _sysExCustomRequest.size(); i++)
        {
            // check only current wish/request
            if (_sysExCustomRequest[i].requestId != _responseArray[static_cast<uint8_t>(byteOrder_t::WISH_BYTE)])
            {
                continue;
            }

            if (_sysExEnabled || !_sysExCustomRequest[i].connOpenCheck)
            {
                setStatus(status_t::ACK);

                DataHandler::CustomResponse customResponse(_responseArray, _responseCounter);
                uint8_t                     result = _dataHandler.customRequest(_sysExCustomRequest[i].requestId, customResponse);

                switch (result)
                {
                case static_cast<uint8_t>(status_t::ACK):
                    break;

                case static_cast<uint8_t>(status_t::REQUEST):
                {
                    setStatus(status_t::ERROR_STATUS);
                }
                break;

                default:
                {
                    setStatus(result);
                    return false;
                }
                break;
                }
            }
            else
            {
                setStatus(status_t::ERROR_CONNECTION);
            }

            return true;
        }

        // custom string not found
        setStatus(status_t::ERROR_WISH);
        return true;
    }
    break;
    }
}

///
/// \brief Generates message length based on other parameters in message.
/// \returns    Message length in bytes.
///
uint16_t SysExConf::generateMessageLenght()
{
    uint16_t size = 0;

    switch (_decodedMessage.amount)
    {
    case amount_t::SINGLE:
        return STD_REQ_MIN_MSG_SIZE;

    default:
    {
        // case amount_t::all:
        switch (_decodedMessage.wish)
        {
        case wish_t::GET:
        case wish_t::BACKUP:
            return STD_REQ_MIN_MSG_SIZE;

        default:
        {
            // case wish_t::set:
            size = LAYOUT_ACCESS[_decodedMessage.block]._sections[_decodedMessage.section].numberOfParameters();

            if (size > PARAMS_PER_MESSAGE)
            {
                if ((_decodedMessage.part + 1) == LAYOUT_ACCESS[_decodedMessage.block]._sections[_decodedMessage.section].parts())
                {
                    size = size - ((LAYOUT_ACCESS[_decodedMessage.block]._sections[_decodedMessage.section].parts() - 1) * PARAMS_PER_MESSAGE);
                }
                else
                {
                    size = PARAMS_PER_MESSAGE;
                }
            }

            size *= BYTES_PER_VALUE;
            size += static_cast<uint8_t>(byteOrder_t::INDEX_BYTE) + 1;

            return size;
        }
        break;
        }
    }
    break;
    }

    return size;
}

///
/// \brief Checks whether the wish value is valid.
/// \returns    True if valid, false otherwise.
///
bool SysExConf::checkWish()
{
    return (_decodedMessage.wish <= wish_t::BACKUP);
}

///
/// \brief Checks whether the amount value is valid.
/// \returns    True if valid, false otherwise.
///
bool SysExConf::checkAmount()
{
    return (_decodedMessage.amount <= amount_t::ALL);
}

///
/// \brief Checks whether the block value is valid.
/// \returns    True if valid, false otherwise.
///
bool SysExConf::checkBlock()
{
    return _decodedMessage.block < LAYOUT_ACCESS.size();
}

///
/// \brief Checks whether the section value is valid.
/// \returns    True if valid, false otherwise.
///
bool SysExConf::checkSection()
{
    return (_decodedMessage.section < LAYOUT_ACCESS[_decodedMessage.block]._sections.size());
}

///
/// \brief Checks whether the message part is valid.
/// \returns    True if valid, false otherwise.
///
bool SysExConf::checkPart()
{
    if ((_decodedMessage.part == 127) || (_decodedMessage.part == 126))
    {
        if ((_decodedMessage.wish == wish_t::GET) || (_decodedMessage.wish == wish_t::BACKUP))
        {
            return true;
        }

        return false;
    }

    if (_decodedMessage.amount == amount_t::ALL)
    {
        if (_decodedMessage.part >= LAYOUT_ACCESS[_decodedMessage.block]._sections[_decodedMessage.section].parts())
        {
            return false;
        }

        return true;
    }

    // do not allow part other than 0 in single mode
    if (_decodedMessage.part)
    {
        return false;
    }

    return true;
}

///
/// \brief Checks whether the parameter index in message is valid.
/// \returns    True if valid, false otherwise.
///
bool SysExConf::checkParameterIndex()
{
    // block and section passed validation, check parameter index
    return (_decodedMessage.index < LAYOUT_ACCESS[_decodedMessage.block]._sections[_decodedMessage.section].numberOfParameters());
}

///
/// \brief Checks whether the new value in message is valid.
/// \returns    True if valid, false otherwise.
///
bool SysExConf::checkNewValue()
{
    uint16_t minValue = LAYOUT_ACCESS[_decodedMessage.block]._sections[_decodedMessage.section].newValueMin();
    uint16_t maxValue = LAYOUT_ACCESS[_decodedMessage.block]._sections[_decodedMessage.section].newValueMax();

    if (minValue != maxValue)
    {
        return ((_decodedMessage.newValue >= minValue) && (_decodedMessage.newValue <= maxValue));
    }

    return true;    // don't check new value if min and max are the same
}

///
/// \brief Used to send custom SysEx response.
/// @param [in] values          Array with values to send.
/// @param [in] size            Array size.
/// @param [in] ack             When set to true, status byte will be set to status_t::ack, otherwise status_t::request will be used.
///                             Set to true by default.
///
void SysExConf::sendCustomMessage(const uint16_t* values, uint16_t size, bool ack)
{
    _responseCounter = 0;

    _responseArray[_responseCounter++] = 0xF0;
    _responseArray[_responseCounter++] = _manufacturerId.id1;
    _responseArray[_responseCounter++] = _manufacturerId.id2;
    _responseArray[_responseCounter++] = _manufacturerId.id3;

    if (ack)
    {
        _responseArray[_responseCounter++] = static_cast<uint8_t>(status_t::ACK);
    }
    else
    {
        _responseArray[_responseCounter++] = static_cast<uint8_t>(status_t::REQUEST);
    }

    _responseArray[_responseCounter++] = 0;    // message part

    for (uint16_t i = 0; i < size; i++)
    {
        _responseArray[_responseCounter++] = values[i];
    }

    sendResponse(false, true);
}

///
/// \brief Used to send SysEx response.
/// @param [in] containsLastByte If set to true, last SysEx byte (0xF7) won't be appended.
/// @param [in] customMessage    If set to true, custom user-specified message is being sent.
///
void SysExConf::sendResponse(bool containsLastByte, bool customMessage)
{
    if (!containsLastByte)
    {
        _responseArray[_responseCounter++] = 0xF7;
    }

    _dataHandler.sendResponse(_responseArray, _responseCounter);
}

///
/// \brief Adds value to SysEx response.
/// This function append value to last specified SysEx array.
/// @param [in] value   New value.
/// \returns True on success, false otherwise.
///
bool SysExConf::addToResponse(uint16_t value)
{
    auto split = Split14Bit(value);

    if (_responseCounter >= (MAX_MESSAGE_SIZE - 1))
    {
        return false;
    }

    _responseArray[_responseCounter++] = split.high();
    _responseArray[_responseCounter++] = split.low();

    return true;
}

uint8_t SysExConf::blocks() const
{
    return LAYOUT_ACCESS.size();
}

uint8_t SysExConf::sections(uint8_t blockIndex) const
{
    return LAYOUT_ACCESS[blockIndex]._sections.size();
}