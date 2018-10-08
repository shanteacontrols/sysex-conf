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

#include "SysEx.h"

///
/// \brief Default constructor.
///
SysEx::SysEx()
{
    sysExMessage = nullptr;
    sysExCustomRequest = nullptr;
    sysExBlockCounter = 0;
    numberOfCustomRequests = 0;
}

///
/// Configures user specifed configuration layout and initializes data to their default values.
/// @param [in] pointer     Pointer to configuration structure.
/// @param [in] numberOfBlocks  Total number of blocks in configuration structure.
/// \returns True on success, false otherwise.
///
bool SysEx::setLayout(sysExBlock_t *pointer, uint8_t numberOfBlocks)
{
    sysExArray              = nullptr;

    sysExEnabled = false;

    customRequestCounter = 0;
    userStatus = (sysExStatus_t)0;

    if ((pointer != nullptr) && numberOfBlocks)
    {
        sysExMessage = pointer;
        sysExBlockCounter = numberOfBlocks;

        for (int i=0; i<numberOfBlocks; i++)
        {
            for (int j=0; j<sysExMessage[i].numberOfSections; j++)
            {
                //based on number of parameters, calculate how many parts message has in case of set/all request and get/all response
                sysExMessage[i].section[j].parts = sysExMessage[i].section[j].numberOfParameters / PARAMETERS_PER_MESSAGE;

                if (sysExMessage[i].section[j].numberOfParameters % PARAMETERS_PER_MESSAGE)
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
bool SysEx::setupCustomRequests(sysExCustomRequest_t *pointer, uint8_t _numberOfCustomRequests)
{
    if ((pointer != nullptr) && _numberOfCustomRequests)
    {
        sysExCustomRequest = pointer;

        for (int i=0; i<_numberOfCustomRequests; i++)
        {
            if (sysExCustomRequest[i].requestID < SYSEX_SR_TOTAL_NUMBER)
            {
                sysExCustomRequest = nullptr;
                numberOfCustomRequests = 0;
                return false; //id already used internally
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
bool SysEx::isConfigurationEnabled()
{
    return sysExEnabled;
}

///
/// \brief Checks whether silent mode is enabled or not.
/// \returns True if enabled, false otherwise.
///
bool SysEx::isSilentModeEnabled()
{
    return silentModeEnabled;
}

///
/// \brief Enables or disables silent protocol mode.
/// When enabled, only GET and BACKUP requests will return response.
///
void SysEx::setSilentMode(bool state)
{
    silentModeEnabled = state;
}

///
/// \brief Handles incoming SysEx message.
/// @param [in] array   SysEx array.
/// @param [in] size    Array size.
///
void SysEx::handleMessage(uint8_t *array, uint8_t size)
{
    if ((sysExMessage != nullptr) && sysExBlockCounter)
    {
        resetDecodedMessage();
        //save pointer to received array so we can manipulate it directly
        sysExArray = array;
        receivedArraySize = size;
        responseSize = RESPONSE_SIZE;

        if (receivedArraySize < MIN_MESSAGE_LENGTH)
            return; //ignore small messages

        if (!checkID())
            return; //don't send response to wrong ID

        bool sendResponse_var = true;

        if (!checkStatus())
        {
            setStatus(ERROR_STATUS);
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
void SysEx::resetDecodedMessage()
{
    //reset decodedMessage
    decodedMessage.status = SYSEX_STATUS_MAX;
    decodedMessage.wish = SYSEX_WISH_MAX;
    decodedMessage.amount = SYSEX_AMOUNT_MAX;
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
bool SysEx::decode()
{
    if (receivedArraySize == MIN_MESSAGE_LENGTH)
    {
        //special request
        return true; //checked in processSpecialRequest
    }
    else if (receivedArraySize <= REQUEST_SIZE)
    {
        setStatus(ERROR_MESSAGE_LENGTH);
        return false;
    }
    else
    {
        if (!sysExEnabled)
        {
            //connection open request hasn't been received
            setStatus(ERROR_CONNECTION);
            return false;
        }

        //don't try to request these parameters if the size is too small
        decodedMessage.part = sysExArray[(uint8_t)partByte];
        decodedMessage.wish = (sysExWish_t)sysExArray[(uint8_t)wishByte];
        decodedMessage.amount = (sysExAmount_t)sysExArray[(uint8_t)amountByte];
        decodedMessage.block = sysExArray[(uint8_t)blockByte];
        decodedMessage.section = sysExArray[(uint8_t)sectionByte];

        if (!checkWish())
        {
            setStatus(ERROR_WISH);
            return false;
        }

        if (!checkBlock())
        {
            setStatus(ERROR_BLOCK);
            return false;
        }

        if (!checkSection())
        {
            setStatus(ERROR_SECTION);
            return false;
        }

        if (!checkAmount())
        {
            setStatus(ERROR_AMOUNT);
            return false;
        }

        if (!checkPart())
        {
            setStatus(ERROR_PART);
            return false;
        }

        if (receivedArraySize != generateMessageLenght())
        {
            setStatus(ERROR_MESSAGE_LENGTH);
            return false;
        }

        //start building response
        setStatus(ACK);

        if (decodedMessage.amount == sysExAmount_single)
        {
            //param size should be handled here - relevant only on single param amount
            #if PARAM_SIZE == 2
            encDec_14bit decoded;
            //index
            decoded.high = sysExArray[indexByte];
            decoded.low = sysExArray[indexByte+1];
            decodedMessage.index = decoded.decode14bit();
            #elif PARAM_SIZE == 1
            decodedMessage.index = sysExArray[indexByte];
            #endif
            decodedMessage.index += (PARAMETERS_PER_MESSAGE*decodedMessage.part);

            if (decodedMessage.wish == sysExWish_set)
            {
                //new value
                #if PARAM_SIZE == 2
                decoded.high = sysExArray[newValueByte_single];
                decoded.low = sysExArray[newValueByte_single+1];
                decodedMessage.newValue = decoded.decode14bit();
                #elif PARAM_SIZE == 1
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
bool SysEx::processStandardRequest()
{
    uint16_t startIndex = 0, endIndex = 1;
    uint8_t msgPartsLoop = 1, responseSize_ = responseSize;
    bool allPartsAck = false;
    bool allPartsLoop = false;

    if ((decodedMessage.wish == sysExWish_backup) || (decodedMessage.wish == sysExWish_get))
    {
        if ((decodedMessage.part == 127) || (decodedMessage.part == 126))
        {
            //when parts 127 or 126 are specified, protocol will loop over all message parts and
            //deliver as many messages as there are parts as response
            msgPartsLoop = sysExMessage[decodedMessage.block].section[decodedMessage.section].parts;
            allPartsLoop = true;

            //when part is set to 126 (0x7E), ACK message will be sent as the last message
            //indicating that all messages have been sent as response to specific request
            if (decodedMessage.part == 126)
                allPartsAck = true;
        }

        if (decodedMessage.wish == sysExWish_backup)
        {
            //convert response to request
            sysExArray[(uint8_t)statusByte] = REQUEST;
            //now convert wish to set
            sysExArray[(uint8_t)wishByte] = (uint8_t)sysExWish_set;
            //decoded message wish needs to be set to get so that we can retrieve parameters
            decodedMessage.wish = sysExWish_get;
            //don't overwrite anything when backup is requested - just append
            responseSize_ = receivedArraySize - 1;
        }
    }

    for (int j=0; j<msgPartsLoop; j++)
    {
        responseSize = responseSize_;

        if (allPartsLoop)
        {
            decodedMessage.part = j;
            sysExArray[partByte] = j;
        }

        if (decodedMessage.amount == sysExAmount_all)
        {
            startIndex = PARAMETERS_PER_MESSAGE*decodedMessage.part;
            endIndex = startIndex + PARAMETERS_PER_MESSAGE;

            if ((sysExParameter_t)endIndex > sysExMessage[decodedMessage.block].section[decodedMessage.section].numberOfParameters)
                endIndex = sysExMessage[decodedMessage.block].section[decodedMessage.section].numberOfParameters;
        }

        for (uint16_t i=startIndex; i<endIndex; i++)
        {
            switch(decodedMessage.wish)
            {
                case sysExWish_get:
                if (decodedMessage.amount == sysExAmount_single)
                {
                    if (!checkParameterIndex())
                    {
                        setStatus(ERROR_INDEX);
                        return false;
                    }
                    else
                    {
                        sysExParameter_t value = 0;
                        bool returnValue = onGet(decodedMessage.block, decodedMessage.section, decodedMessage.index, value);

                        //check for custom error
                        if (userStatus)
                        {
                            setStatus(userStatus);
                            //clear user status
                            userStatus = (sysExStatus_t)0;
                            return false;
                        }
                        else if (!returnValue)
                        {
                            setStatus(ERROR_READ);
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
                    bool returnValue = onGet(decodedMessage.block, decodedMessage.section, i, value);

                    if (userStatus)
                    {
                        setStatus(userStatus);
                        userStatus = (sysExStatus_t)0;
                        return false;
                    }
                    else if (!returnValue)
                    {
                        setStatus(ERROR_READ);
                        return false;
                    }
                    else
                    {
                        addToResponse(value);
                    }
                }
                break;

                default:
                // case sysExWish_set:
                if (decodedMessage.amount == sysExAmount_single)
                {
                    if (!checkParameterIndex())
                    {
                        setStatus(ERROR_INDEX);
                        return false;
                    }

                    if (!checkNewValue())
                    {
                        setStatus(ERROR_NEW_VALUE);
                        return false;
                    }

                    if (!onSet(decodedMessage.block, decodedMessage.section, decodedMessage.index, decodedMessage.newValue))
                    {
                        if (userStatus)
                        {
                            setStatus(userStatus);
                            userStatus = (sysExStatus_t)0;
                        }
                        else
                        {
                            setStatus(ERROR_WRITE);
                        }

                        return false;
                    }
                }
                else
                {
                    uint8_t arrayIndex = (i-startIndex);

                    #if PARAM_SIZE == 2
                    arrayIndex *= sizeof(sysExParameter_t);
                    arrayIndex += newValueByte_all;
                    encDec_14bit decoded;
                    decoded.high = sysExArray[arrayIndex];
                    decoded.low = sysExArray[arrayIndex+1];
                    decodedMessage.newValue = decoded.decode14bit();
                    #elif PARAM_SIZE == 1
                    decodedMessage.newValue = sysExArray[arrayIndex+newValueByte_all];
                    #endif

                    if (!checkNewValue())
                    {
                        setStatus(ERROR_NEW_VALUE);
                        return false;
                    }

                    if (!onSet(decodedMessage.block, decodedMessage.section, i, decodedMessage.newValue))
                    {
                        //check for custom error
                        if (userStatus)
                        {
                            setStatus(userStatus);
                            //clear user status
                            userStatus = (sysExStatus_t)0;
                        }
                        else
                        {
                            setStatus(ERROR_WRITE);
                        }

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
        //send ACK message at the end
        responseSize = 0;
        addToResponse(0xF0);
        addToResponse(SYS_EX_M_ID_0);
        addToResponse(SYS_EX_M_ID_1);
        addToResponse(SYS_EX_M_ID_2);
        addToResponse(ACK);
        addToResponse(0x7E);
        sendResponse(false);
    }

    return true;
}

///
/// \brief Checks whether the manufacturer ID in message is correct.
/// @returns    True if valid, false otherwise.
///
bool SysEx::checkID()
{
    return
    (
        (sysExArray[idByte_1] == SYS_EX_M_ID_0)    &&
        (sysExArray[idByte_2] == SYS_EX_M_ID_1)    &&
        (sysExArray[idByte_3] == SYS_EX_M_ID_2)
    );
}

///
/// \brief Checks whether the status byte in request is correct.
/// @returns    True if valid, false otherwise.
///
bool SysEx::checkStatus()
{
    return ((sysExStatus_t)sysExArray[(uint8_t)statusByte] == REQUEST);
}

///
/// \brief Used to process special SysEx request.
/// \returns True on success, false otherwise.
///
bool SysEx::processSpecialRequest()
{
    switch(sysExArray[wishByte])
    {
        case SYSEX_SR_CONN_CLOSE:
        if (!sysExEnabled)
        {
            //connection can't be closed if it isn't opened
            setStatus(ERROR_CONNECTION);
            return true;
        }
        else
        {
            //close sysex connection
            sysExEnabled = false;
            setStatus(ACK);
            if (silentModeEnabled)
            {
                //also disable silent mode
                silentModeEnabled = false;
            }
            return true;
        }
        break;

        case SYSEX_SR_CONN_OPEN:
        case SYSEX_SR_CONN_OPEN_SILENT:
        //necessary to allow the configuration
        sysExEnabled = true;
        if (sysExArray[wishByte] == SYSEX_SR_CONN_OPEN_SILENT)
        {
            silentModeEnabled = true;
        }

        setStatus(ACK);
        return true;

        case SYSEX_SR_SILENT_DISABLE:
        silentModeEnabled = false;
        setStatus(ACK);
        return true;

        case SYSEX_SR_BYTES_PER_VALUE:
        if (sysExEnabled)
        {
            setStatus(ACK);
            addToResponse(PARAM_SIZE);
        }
        else
        {
            setStatus(ERROR_CONNECTION);
        }
        return true;

        case SYSEX_SR_PARAMS_PER_MESSAGE:
        if (sysExEnabled)
        {
            setStatus(ACK);
            addToResponse(PARAMETERS_PER_MESSAGE);
        }
        else
        {
            setStatus(ERROR_CONNECTION);
        }
        return true;

        default:
        //check for custom string
        for (int i=0; i<numberOfCustomRequests; i++)
        {
            //check only current wish/request
            if (sysExCustomRequest[i].requestID != sysExArray[wishByte])
                continue;

            if (sysExEnabled || !sysExCustomRequest[i].connOpenCheck)
            {
                setStatus(ACK);

                if (!onCustomRequest(sysExCustomRequest[i].requestID))
                    setStatus(ERROR_READ);
            }
            else
            {
                setStatus(ERROR_CONNECTION);
            }

            return true;
        }

        //custom string not found
        setStatus(ERROR_WISH);
        return true;
    }
}

///
/// \brief Generates message length based on other parameters in message.
/// \returns    Message length in bytes.
///
uint8_t SysEx::generateMessageLenght()
{
    uint16_t size = 0;

    switch(decodedMessage.amount)
    {
        case sysExAmount_single:
        switch(decodedMessage.wish)
        {
            case sysExWish_get:
            case sysExWish_backup:
            return ML_REQ_STANDARD + sizeof(sysExParameter_t); //add parameter length
            break;

            default:
            // case sysExWish_set:
            return ML_REQ_STANDARD + 2*sizeof(sysExParameter_t); //add parameter length and new value length
            break;
        }
        break;

        default:
        // case sysExAmount_all:
        switch(decodedMessage.wish)
        {
            case sysExWish_get:
            case sysExWish_backup:
            return ML_REQ_STANDARD;

            default:
            // case sysExWish_set:
            size = sysExMessage[decodedMessage.block].section[decodedMessage.section].numberOfParameters;

            if (size > PARAMETERS_PER_MESSAGE)
            {
                if ((decodedMessage.part+1) == sysExMessage[decodedMessage.block].section[decodedMessage.section].parts)
                    size = size - ((sysExMessage[decodedMessage.block].section[decodedMessage.section].parts-1)*PARAMETERS_PER_MESSAGE);
                else
                    size = PARAMETERS_PER_MESSAGE;
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
bool SysEx::checkWish()
{
    return (decodedMessage.wish < SYSEX_WISH_MAX);
}

///
/// \brief Checks whether the amount value is valid.
/// \returns    True if valid, false otherwise.
///
bool SysEx::checkAmount()
{
    return (decodedMessage.amount < SYSEX_AMOUNT_MAX);
}

///
/// \brief Checks whether the block value is valid.
/// \returns    True if valid, false otherwise.
///
bool SysEx::checkBlock()
{
    return decodedMessage.block < sysExBlockCounter;
}

///
/// \brief Checks whether the section value is valid.
/// \returns    True if valid, false otherwise.
///
bool SysEx::checkSection()
{
    return (decodedMessage.section < sysExMessage[decodedMessage.block].numberOfSections);
}

///
/// \brief Checks whether the message part is valid.
/// \returns    True if valid, false otherwise.
///
bool SysEx::checkPart()
{
    switch(decodedMessage.wish)
    {
        case sysExWish_get:
        if ((decodedMessage.part == 127) || (decodedMessage.part == 126))
            return true;

        if (decodedMessage.part >= sysExMessage[decodedMessage.block].section[decodedMessage.section].parts)
            return false;
        else
            return true;
        break;

        default:
        // case sysExWish_set:
        // case sysExWish_backup:
        if (decodedMessage.wish == sysExWish_backup)
        {
            if ((decodedMessage.part == 127) || (decodedMessage.part == 126))
                return true;
        }

        if (decodedMessage.amount == sysExAmount_all)
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
bool SysEx::checkParameterIndex()
{
    //block and section passed validation, check parameter index
    return (decodedMessage.index < sysExMessage[decodedMessage.block].section[decodedMessage.section].numberOfParameters);
}

///
/// \brief Checks whether the new value in message is valid.
/// \returns    True if valid, false otherwise.
///
bool SysEx::checkNewValue()
{
    sysExParameter_t minValue = sysExMessage[decodedMessage.block].section[decodedMessage.section].newValueMin;
    sysExParameter_t maxValue = sysExMessage[decodedMessage.block].section[decodedMessage.section].newValueMax;

    if (minValue != maxValue)
        return ((decodedMessage.newValue >= minValue) && (decodedMessage.newValue <= maxValue));
    else
        return true; //don't check new value if min and max are the same
}

///
/// \brief Used to send custom SysEx response.
/// @param [in, out] responseArray Array in which custom request will be stored.
/// @param [in] value           Array with values to send.
/// @param [in] size            Array size.
/// @param [in] ack             When set to true, status byte will be set to ACK, otherwise REQUEST will be used.
///                             Set to true by default.
///
void SysEx::sendCustomMessage(uint8_t *responseArray, sysExParameter_t *values, uint8_t size, bool ack)
{
    sysExArray = responseArray;
    responseSize = 0;

    sysExArray[responseSize] = 0xF0;
    responseSize++;
    sysExArray[responseSize] = SYS_EX_M_ID_0;
    responseSize++;
    sysExArray[responseSize] = SYS_EX_M_ID_1;
    responseSize++;
    sysExArray[responseSize] = SYS_EX_M_ID_2;
    responseSize++;
    if (ack)
        sysExArray[responseSize] = ACK;
    else
        sysExArray[responseSize] = REQUEST;
    responseSize++;
    sysExArray[responseSize] = 0; //message part
    responseSize++;

    for (int i=0; i<size; i++)
    {
        addToResponse(values[i]);
    }

    sendResponse(false);
}

///
/// \brief Used to send SysEx response.
/// @param [in] containsLastByte If set to true, last SysEx byte (0x07) won't be appended.
///
void SysEx::sendResponse(bool containsLastByte)
{
    if (!containsLastByte)
    {
        sysExArray[responseSize] = 0xF7;
        responseSize++;
    }

    if (silentModeEnabled && (decodedMessage.wish != sysExWish_get))
        return;

    onWrite(sysExArray, responseSize);
}

///
/// \brief Adds value to SysEx response.
/// This function append value to last specified SysEx array.
/// @param [in] value   New value.
/// \returns True on success, false otherwise.
///
bool SysEx::addToResponse(sysExParameter_t value)
{
    if (sysExArray == nullptr)
        return false;

    #if PARAM_SIZE == 2
    encDec_14bit encoded;
    encoded.value = value;
    encoded.encodeTo14bit();
    sysExArray[responseSize] = encoded.high;
    responseSize++;
    sysExArray[responseSize] = encoded.low;
    responseSize++;
    #elif PARAM_SIZE == 1
    sysExArray[responseSize] = (uint8_t)value;
    responseSize++;
    #endif

    return true;
}

///
/// \brief Sets status byte in SysEx response.
/// @param [in] status New status value. See sysExStatus_t enum.
///
void SysEx::setStatus(sysExStatus_t status)
{
    sysExArray[statusByte] = (uint8_t)status;
}

///
/// \brief Sets error in status byte in response specified by user.
/// @param [in] status New error value. See sysExStatus_t enum.
///
void SysEx::setError(sysExStatus_t status)
{
    switch(status)
    {
        case ERROR_STATUS:
        case ERROR_CONNECTION:
        case ERROR_WISH:
        case ERROR_AMOUNT:
        case ERROR_BLOCK:
        case ERROR_SECTION:
        case ERROR_PART:
        case ERROR_INDEX:
        case ERROR_NEW_VALUE:
        case ERROR_MESSAGE_LENGTH:
        case ERROR_WRITE:
        case ERROR_NOT_SUPPORTED:
        //any of these values are fine
        userStatus = status;
        break;

        default:
        userStatus = ERROR_WRITE;
        break;
    }
}