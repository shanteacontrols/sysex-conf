/*
    Copyright 2017-2019 Igor Petrovic

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
/// Configures user specifed configuration layout and initializes data to their default values.
/// @param [in] pointer     Pointer to configuration structure.
/// @param [in] numberOfBlocks  Total number of blocks in configuration structure.
/// \returns True on success, false otherwise.
///
bool SysExConf::setLayout(block_t* pointer, uint8_t numberOfBlocks)
{
    sysExArray = nullptr;

    sysExEnabled = false;

    customRequestCounter = 0;

    if ((pointer != nullptr) && numberOfBlocks)
    {
        sysExMessage = pointer;
        sysExBlockCounter = numberOfBlocks;

        for (int i = 0; i < numberOfBlocks; i++)
        {
            for (int j = 0; j < sysExMessage[i].numberOfSections; j++)
            {
                //based on number of parameters, calculate how many parts message has in case of set/all request and get/all response
                sysExMessage[i].section[j].parts = sysExMessage[i].section[j].numberOfParameters / SYS_EX_CONF_PARAMETERS_PER_MESSAGE;

                if (sysExMessage[i].section[j].numberOfParameters % SYS_EX_CONF_PARAMETERS_PER_MESSAGE)
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
bool SysExConf::setupCustomRequests(customRequest_t* pointer, uint8_t _numberOfCustomRequests)
{
    if ((pointer != nullptr) && _numberOfCustomRequests)
    {
        sysExCustomRequest = pointer;

        for (int i = 0; i < _numberOfCustomRequests; i++)
        {
            if (sysExCustomRequest[i].requestID < static_cast<uint8_t>(specialRequest_t::AMOUNT))
            {
                sysExCustomRequest = nullptr;
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
void SysExConf::handleMessage(uint8_t* array, uint8_t size)
{
    userStatus = status_t::request;

    if ((sysExMessage != nullptr) && sysExBlockCounter)
    {
        resetDecodedMessage();
        //save pointer to received array so we can manipulate it directly
        sysExArray = array;
        receivedArraySize = size;
        responseSize = RESPONSE_SIZE;

        if (receivedArraySize < MIN_MESSAGE_LENGTH)
            return;    //ignore small messages

        if (!checkID())
            return;    //don't send response to wrong ID

        bool sendResponse_var = true;

        if (!checkStatus())
        {
            setStatus(status_t::errorStatus);
        }
        else
        {
            if (decode())
            {
                if (receivedArraySize == MIN_MESSAGE_LENGTH)
                {
                    processSpecialRequest();
                }
                else
                {
                    if (processStandardRequest())
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

        sysExArray = nullptr;
    }
}

///
/// \brief Resets all elements in decodedMessage structure to default values.
///
void SysExConf::resetDecodedMessage()
{
    //reset decodedMessage
    decodedMessage.status = status_t::invalid;
    decodedMessage.wish = wish_t::invalid;
    decodedMessage.amount = amount_t::invalid;
    decodedMessage.block = 0;
    decodedMessage.section = 0;
    decodedMessage.part = 0;
    decodedMessage.index = 0;
    decodedMessage.newValue = 0;
}

///
/// \brief Decodes received message.
/// \returns True on success, false otherwise (invalid values).
///
bool SysExConf::decode()
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
        decodedMessage.part = sysExArray[(uint8_t)partByte];
        decodedMessage.wish = static_cast<wish_t>(sysExArray[(uint8_t)wishByte]);
        decodedMessage.amount = static_cast<amount_t>(sysExArray[(uint8_t)amountByte]);
        decodedMessage.block = sysExArray[(uint8_t)blockByte];
        decodedMessage.section = sysExArray[(uint8_t)sectionByte];

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
//param size should be handled here - relevant only on single param amount
#if SYS_EX_CONF_PARAM_SIZE == 2
            encDec_14bit decoded;
            //index
            decoded.high = sysExArray[indexByte];
            decoded.low = sysExArray[indexByte + 1];
            decodedMessage.index = decoded.decode14bit();
#elif SYS_EX_CONF_PARAM_SIZE == 1
            decodedMessage.index = sysExArray[indexByte];
#endif
            decodedMessage.index += (SYS_EX_CONF_PARAMETERS_PER_MESSAGE * decodedMessage.part);

            if (decodedMessage.wish == wish_t::set)
            {
//new value
#if SYS_EX_CONF_PARAM_SIZE == 2
                decoded.high = sysExArray[newValueByte_single];
                decoded.low = sysExArray[newValueByte_single + 1];
                decodedMessage.newValue = decoded.decode14bit();
#elif SYS_EX_CONF_PARAM_SIZE == 1
                decodedMessage.newValue = sysExArray[newValueByte_single];
#endif
            }
        }
    }

    return true;
}

///
/// \brief Used to process standard SysEx request.
/// \returns True on success, false otherwise.
///
bool SysExConf::processStandardRequest()
{
    uint16_t startIndex = 0, endIndex = 1;
    uint8_t  msgPartsLoop = 1, responseSize_ = responseSize;
    bool     allPartsAck = false;
    bool     allPartsLoop = false;

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
            sysExArray[(uint8_t)statusByte] = static_cast<uint8_t>(status_t::request);
            //now convert wish to set
            sysExArray[(uint8_t)wishByte] = (uint8_t)wish_t::set;
            //decoded message wish needs to be set to get so that we can retrieve parameters
            decodedMessage.wish = wish_t::get;
            //don't overwrite anything when backup is requested - just append
            responseSize_ = receivedArraySize - 1;
        }
    }

    for (int j = 0; j < msgPartsLoop; j++)
    {
        responseSize = responseSize_;

        if (allPartsLoop)
        {
            decodedMessage.part = j;
            sysExArray[partByte] = j;
        }

        if (decodedMessage.amount == amount_t::all)
        {
            startIndex = SYS_EX_CONF_PARAMETERS_PER_MESSAGE * decodedMessage.part;
            endIndex = startIndex + SYS_EX_CONF_PARAMETERS_PER_MESSAGE;

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
                        sysExParameter_t value = 0;
                        bool             returnValue = onGet(decodedMessage.block, decodedMessage.section, decodedMessage.index, value);

                        //check for custom status
                        if (userStatus != status_t::request)
                        {
                            setStatus(userStatus);
                            return false;
                        }
                        else if (!returnValue)
                        {
                            setStatus(status_t::errorRead);
                            return false;
                        }
                        else
                        {
                            addToResponse(value);
                        }
                    }
                }
                else
                {
                    //get all params - no index is specified
                    sysExParameter_t value = 0;
                    bool             returnValue = onGet(decodedMessage.block, decodedMessage.section, i, value);

                    if (userStatus != status_t::request)
                    {
                        setStatus(userStatus);
                        return false;
                    }
                    else if (!returnValue)
                    {
                        setStatus(status_t::errorRead);
                        return false;
                    }
                    else
                    {
                        addToResponse(value);
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

                    if (!onSet(decodedMessage.block, decodedMessage.section, decodedMessage.index, decodedMessage.newValue))
                    {
                        if (userStatus != status_t::request)
                            setStatus(userStatus);
                        else
                            setStatus(status_t::errorWrite);

                        return false;
                    }
                }
                else
                {
                    uint8_t arrayIndex = (i - startIndex);

#if SYS_EX_CONF_PARAM_SIZE == 2
                    arrayIndex *= sizeof(sysExParameter_t);
                    arrayIndex += newValueByte_all;
                    encDec_14bit decoded;
                    decoded.high = sysExArray[arrayIndex];
                    decoded.low = sysExArray[arrayIndex + 1];
                    decodedMessage.newValue = decoded.decode14bit();
#elif SYS_EX_CONF_PARAM_SIZE == 1
                    decodedMessage.newValue = sysExArray[arrayIndex + newValueByte_all];
#endif

                    if (!checkNewValue())
                    {
                        setStatus(status_t::errorNewValue);
                        return false;
                    }

                    if (!onSet(decodedMessage.block, decodedMessage.section, i, decodedMessage.newValue))
                    {
                        //check for custom status
                        if (userStatus != status_t::request)
                            setStatus(userStatus);
                        else
                            setStatus(status_t::errorWrite);

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
        responseSize = 0;
        addToResponse(0xF0);
        addToResponse(SYS_EX_CONF_M_ID_0);
        addToResponse(SYS_EX_CONF_M_ID_1);
        addToResponse(SYS_EX_CONF_M_ID_2);
        addToResponse(static_cast<uint8_t>(status_t::ack));
        addToResponse(0x7E);
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
        (sysExArray[idByte_1] == SYS_EX_CONF_M_ID_0) &&
        (sysExArray[idByte_2] == SYS_EX_CONF_M_ID_1) &&
        (sysExArray[idByte_3] == SYS_EX_CONF_M_ID_2));
}

///
/// \brief Checks whether the status byte in request is correct.
/// @returns    True if valid, false otherwise.
///
bool SysExConf::checkStatus()
{
    return ((status_t)sysExArray[(uint8_t)statusByte] == status_t::request);
}

///
/// \brief Used to process special SysEx request.
/// \returns True on success, false otherwise.
///
bool SysExConf::processSpecialRequest()
{
    switch (sysExArray[wishByte])
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

        if (sysExArray[wishByte] == static_cast<uint8_t>(specialRequest_t::connOpenSilent))
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
            addToResponse(SYS_EX_CONF_PARAM_SIZE);
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
            addToResponse(SYS_EX_CONF_PARAMETERS_PER_MESSAGE);
        }
        else
        {
            setStatus(status_t::errorConnection);
        }
        return true;

    default:
        //check for custom value
        for (int i = 0; i < numberOfCustomRequests; i++)
        {
            //check only current wish/request
            if (sysExCustomRequest[i].requestID != sysExArray[wishByte])
                continue;

            if (sysExEnabled || !sysExCustomRequest[i].connOpenCheck)
            {
                setStatus(status_t::ack);

                if (!onCustomRequest(sysExCustomRequest[i].requestID))
                    setStatus(status_t::errorRead);
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
uint8_t SysExConf::generateMessageLenght()
{
    uint16_t size = 0;

    switch (decodedMessage.amount)
    {
    case amount_t::single:
        switch (decodedMessage.wish)
        {
        case wish_t::get:
        case wish_t::backup:
            return ML_REQ_STANDARD + sizeof(sysExParameter_t);    //add parameter length
            break;

        default:
            // case wish_t::set:
            return ML_REQ_STANDARD + 2 * sizeof(sysExParameter_t);    //add parameter length and new value length
            break;
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

            if (size > SYS_EX_CONF_PARAMETERS_PER_MESSAGE)
            {
                if ((decodedMessage.part + 1) == sysExMessage[decodedMessage.block].section[decodedMessage.section].parts)
                    size = size - ((sysExMessage[decodedMessage.block].section[decodedMessage.section].parts - 1) * SYS_EX_CONF_PARAMETERS_PER_MESSAGE);
                else
                    size = SYS_EX_CONF_PARAMETERS_PER_MESSAGE;
            }

            size *= sizeof(sysExParameter_t);
            size += ML_REQ_STANDARD;
            return size;
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
/// @param [in, out] responseArray Array in which custom request will be stored.
/// @param [in] value           Array with values to send.
/// @param [in] size            Array size.
/// @param [in] ack             When set to true, status byte will be set to status_t::ack, otherwise status_t::request will be used.
///                             Set to true by default.
///
void SysExConf::sendCustomMessage(uint8_t* responseArray, sysExParameter_t* values, uint8_t size, bool ack)
{
    sysExArray = responseArray;
    responseSize = 0;

    sysExArray[responseSize] = 0xF0;
    responseSize++;
    sysExArray[responseSize] = SYS_EX_CONF_M_ID_0;
    responseSize++;
    sysExArray[responseSize] = SYS_EX_CONF_M_ID_1;
    responseSize++;
    sysExArray[responseSize] = SYS_EX_CONF_M_ID_2;
    responseSize++;
    if (ack)
        sysExArray[responseSize] = static_cast<uint8_t>(status_t::ack);
    else
        sysExArray[responseSize] = static_cast<uint8_t>(status_t::request);
    responseSize++;
    sysExArray[responseSize] = 0;    //message part
    responseSize++;

    for (int i = 0; i < size; i++)
    {
        addToResponse(values[i]);
    }

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
        sysExArray[responseSize] = 0xF7;
        responseSize++;
    }

    if (silentModeEnabled && (decodedMessage.wish != wish_t::get))
        return;

    onWrite(sysExArray, responseSize);
}

///
/// \brief Adds value to SysEx response.
/// This function append value to last specified SysEx array.
/// @param [in] value   New value.
/// \returns True on success, false otherwise.
///
bool SysExConf::addToResponse(sysExParameter_t value)
{
    if (sysExArray == nullptr)
        return false;

#if SYS_EX_CONF_PARAM_SIZE == 2
    encDec_14bit encoded;
    encoded.value = value;
    encoded.encodeTo14bit();
    sysExArray[responseSize] = encoded.high;
    responseSize++;
    sysExArray[responseSize] = encoded.low;
    responseSize++;
#elif SYS_EX_CONF_PARAM_SIZE == 1
    sysExArray[responseSize] = (uint8_t)value;
    responseSize++;
#endif

    return true;
}

///
/// \brief Sets status byte in SysEx response.
/// @param [in] status New status value. See status_t enum.
///
void SysExConf::setStatus(status_t status)
{
    sysExArray[statusByte] = (uint8_t)status;
}

///
/// \brief Sets error in status byte in response specified by user.
/// @param [in] status New error value. See status_t enum.
///
void SysExConf::setError(status_t status)
{
    switch (status)
    {
    case status_t::errorStatus:
    case status_t::errorConnection:
    case status_t::errorWish:
    case status_t::errorAmount:
    case status_t::errorBlock:
    case status_t::errorSection:
    case status_t::errorPart:
    case status_t::errorIndex:
    case status_t::errorNewValue:
    case status_t::errorMessageLength:
    case status_t::errorWrite:
    case status_t::errorNotSupported:
        //any of these values are fine
        userStatus = status;
        break;

    default:
        userStatus = status_t::errorWrite;
        break;
    }
}