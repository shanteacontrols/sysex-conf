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

sysExParameter_t (*sendGetCallback)(uint8_t block, uint8_t section, uint16_t index);
bool (*sendSetCallback)(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue);
bool (*sendCustomRequestCallback)(uint8_t value);
void (*sendSysExWriteCallback)(uint8_t *sysExArray, uint8_t size);

bool                sysExEnabled,
                    forcedSend;

sysExBlock          sysExMessage[SYSEX_MAX_BLOCKS];

decodedMessage_t    decodedMessage;

uint8_t             *sysExArray,
                    sysExArraySize,
                    customRequests[MAX_CUSTOM_REQUESTS],
                    customRequestCounter,
                    sysExBlockCounter,
                    responseSize;

sysExStatus_t       userStatus;

///
/// \brief Default constructor.
///
SysEx::SysEx()
{
    
}

void SysEx::init()
{
    sendGetCallback             = NULL;
    sendSetCallback             = NULL;
    sendCustomRequestCallback   = NULL;
    sendSysExWriteCallback      = NULL;

    sysExEnabled = false;
    forcedSend = false;

    for (int i=0; i<SYSEX_MAX_BLOCKS; i++)
    {
        sysExMessage[i].numberOfSections = 0;

        for (int j=0; j<SYSEX_MAX_SECTIONS; j++)
        {
            sysExMessage[i].section[j].numberOfParameters = INVALID_VALUE;
            sysExMessage[i].section[j].newValueMin = INVALID_VALUE;
            sysExMessage[i].section[j].newValueMax = INVALID_VALUE;
        }
    }

    for (int i=0; i<MAX_CUSTOM_REQUESTS; i++)
        customRequests[i] = INVALID_VALUE;

    customRequestCounter = 0;
    sysExBlockCounter = 0;
    userStatus = (sysExStatus_t)0;
}

///
/// \brief Checks whether the SysEx configuration is enabled or not.
/// \returns True if enabled, false otherwise.
///
bool SysEx::configurationEnabled()
{
    return sysExEnabled;
}

///
/// \brief Adds custom request.
/// If added byte is found in incoming message, and message is formatted as special request, custom message handler is called.
/// It is up to user to decide on action.
/// @param [in] value   Custom request value.
///
bool SysEx::addCustomRequest(uint8_t value)
{
    if (customRequestCounter > MAX_CUSTOM_REQUESTS)
        return false;

    //don't add custom string if it's already defined as one of default strings
    if (value < SPECIAL_PARAMETERS)
        return false;

    customRequests[customRequestCounter] = value;
    customRequestCounter++;
    return true;
}

///
/// \brief Adds specified number of SysEx blocks.
/// @param [in] numberOfBlocks  Number of blocks to add.
/// \returns True on success, false otherwise.
///
bool SysEx::addBlocks(uint8_t numberOfBlocks)
{
    if (sysExBlockCounter+numberOfBlocks > SYSEX_MAX_BLOCKS)
        return false;

    sysExBlockCounter += numberOfBlocks;
    return true;
}

///
/// \brief Adds section to specified block.
/// @param [in] blockID Block on which to add section.
/// @param [in] section Structure holding description of section.
/// \returns True on success, false otherwise.
/// 
bool SysEx::addSection(uint8_t blockID, sysExSection section)
{
    //make sure block exists
    if (blockID >= SYSEX_MAX_BLOCKS)
        return false;

    if (sysExMessage[blockID].numberOfSections > SYSEX_MAX_SECTIONS)
        return false;

    sysExMessage[blockID].section[sysExMessage[blockID].numberOfSections].numberOfParameters = section.numberOfParameters;
    sysExMessage[blockID].section[sysExMessage[blockID].numberOfSections].newValueMin = section.newValueMin;
    sysExMessage[blockID].section[sysExMessage[blockID].numberOfSections].newValueMax = section.newValueMax;

    //based on number of parameters, calculate how many parts message has in case of set/all request and get/all response
    sysExMessage[blockID].section[sysExMessage[blockID].numberOfSections].parts = sysExMessage[blockID].section[sysExMessage[blockID].numberOfSections].numberOfParameters / PARAMETERS_PER_MESSAGE;

    if (sysExMessage[blockID].section[sysExMessage[blockID].numberOfSections].numberOfParameters % PARAMETERS_PER_MESSAGE)
        sysExMessage[blockID].section[sysExMessage[blockID].numberOfSections].parts++;

    sysExMessage[blockID].numberOfSections++;
    return true;
}

///
/// \brief Handles incoming SysEx message.
/// @param [in] array   SysEx array.
/// @param [in] size    Array size.
///
void SysEx::handleMessage(uint8_t *array, uint8_t size)
{
    //save pointer to received array so we can manipulate it directly
    sysExArray = array;
    sysExArraySize = size;
    responseSize = RESPONSE_SIZE;

    if (sysExArraySize < MIN_MESSAGE_LENGTH)
        return; //ignore small messages

    decode();

    if (!forcedSend)
    {
        //if forcedSend is set to true, response has already been sent
        sysExArray[responseSize] = 0xF7;
        responseSize++;

        if (sendSysExWriteCallback != NULL)
        {
            sendSysExWriteCallback(sysExArray, responseSize);
        }
    }

    forcedSend = false;
}

///
/// \brief Decodes received message.
///
void SysEx::decode()
{
    decodedMessage.status = (sysExStatus_t)sysExArray[(uint8_t)statusByte];

    if (decodedMessage.status != REQUEST)
    {
        //don't let status be anything but request
        setStatus(ERROR_STATUS);
        return;
    }

    if (!checkID())
    {
        //set this variable to true to avoid incorrect sending of additional data
        forcedSend = true;
        return; //don't send response to wrong ID
    }

    if (checkSpecialRequests())
        return; //special request was handled by now

    //message appears to be fine for now
    if (!sysExEnabled)
    {
        //message is fine, but handshake hasn't been received
        setStatus(ERROR_HANDSHAKE);
        return;
    }

    if (sysExArraySize <= REQUEST_SIZE)
    {
        setStatus(ERROR_MESSAGE_LENGTH);
        return;
    }

    //don't try to request these parameters if the size is too small
    decodedMessage.part = sysExArray[(uint8_t)partByte];
    decodedMessage.wish = (sysExWish)sysExArray[(uint8_t)wishByte];
    decodedMessage.amount = (sysExAmount)sysExArray[(uint8_t)amountByte];
    decodedMessage.block = sysExArray[(uint8_t)blockByte];
    decodedMessage.section = sysExArray[(uint8_t)sectionByte];

    if (!checkRequest())
        return;

    uint16_t length = generateMessageLenght();

    if (sysExArraySize != length)
    {
        setStatus(ERROR_MESSAGE_LENGTH);
        return;
    }

    if (checkParameters())
    {
        //message is ok
    }
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
/// \brief Checks whether the request belong to special group of requests or not
/// \returns    True if request is special, false otherwise.
///
bool SysEx::checkSpecialRequests()
{
    if (sysExArraySize != MIN_MESSAGE_LENGTH)
        return false;

    switch(sysExArray[wishByte])
    {
        case SYSEX_CLOSE_REQUEST:
        if (!sysExEnabled)
        {
            //connection can't be closed if it isn't opened
            setStatus(ERROR_HANDSHAKE);
            return true;
        }
        else
        {
            //close sysex connection
            sysExEnabled = false;
            setStatus(ACK);
            return true;
        }
        break;

        case HANDSHAKE_REQUEST:
        //hello message, necessary for allowing configuration
        sysExEnabled = true;
        setStatus(ACK);
        return true;

        case BYTES_PER_VALUE_REQUEST:
        if (sysExEnabled)
        {
            setStatus(ACK);
            addToResponse(PARAM_SIZE);
        }
        else
        {
            setStatus(ERROR_HANDSHAKE);
        }
        return true;

        case PARAMS_PER_MESSAGE_REQUEST:
        if (sysExEnabled)
        {
            setStatus(ACK);
            addToResponse(PARAMETERS_PER_MESSAGE);
        }
        else
        {
            setStatus(ERROR_HANDSHAKE);
        }
        return true;

        default:
        //check for custom string
        for (int i=0; i<MAX_CUSTOM_REQUESTS; i++)
        {
            if (customRequests[i] == INVALID_VALUE)
                continue;

            if (customRequests[i] != sysExArray[wishByte])
                continue;

            if (sysExEnabled)
            {
                setStatus(ACK);
                sendCustomRequestCallback(customRequests[i]);
            }
            else
            {
                setStatus(ERROR_HANDSHAKE);
            }

            return true;
        }

        //custom string not found
        setStatus(ERROR_WISH);
        return true;
    }
}

///
/// \brief Checks whether request is valid by calling other checking functions.
/// @returns    True if valid, false otherwise.
///
bool SysEx::checkRequest()
{
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

    return true;
}

///
/// \brief Checks all parameters in message, builds and sends response.
/// \returns    True if entire request is valid, false otherwise.
///
bool SysEx::checkParameters()
{
    //sysex request is fine
    //start building response
    setStatus(ACK);

    uint8_t msgPartsLoop = 1, responseSize_ = responseSize;
    uint16_t startIndex = 0, endIndex = 1;
    bool allPartsAck = false;

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

    if ((decodedMessage.wish == sysExWish_backup) || (decodedMessage.wish == sysExWish_get))
    {
        if ((decodedMessage.part == 127) || (decodedMessage.part == 126))
        {
            msgPartsLoop = sysExMessage[decodedMessage.block].section[decodedMessage.section].parts;
            forcedSend = true;

            if (decodedMessage.part == 126)
            {
                allPartsAck = true;
            }
        }

        if (decodedMessage.wish == sysExWish_backup)
        {
            //don't overwrite anything if backup is requested
            responseSize_ = sysExArraySize - 1;
            //convert response to request
            sysExArray[(uint8_t)statusByte] = REQUEST;
            //now convert wish to set
            sysExArray[(uint8_t)wishByte] = (uint8_t)sysExWish_set;
            //decoded message wish needs to be set to get so that we can retrieve parameters
            decodedMessage.wish = sysExWish_get;
        }
    }

    for (int j=0; j<msgPartsLoop; j++)
    {
        responseSize = responseSize_;

        if (forcedSend)
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
                        sysExParameter_t value = sendGetCallback(decodedMessage.block, decodedMessage.section, decodedMessage.index);

                        //check for custom error
                        if (userStatus)
                        {
                            setStatus(userStatus);
                            //clear user status
                            userStatus = (sysExStatus_t)0;
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
                    sysExParameter_t value = sendGetCallback(decodedMessage.block, decodedMessage.section, i);

                    if (userStatus)
                    {
                        setStatus(userStatus);
                        userStatus = (sysExStatus_t)0;
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

                    if (sendSetCallback(decodedMessage.block, decodedMessage.section, decodedMessage.index, decodedMessage.newValue))
                    {
                        //no further checks are required
                        return true;
                    }
                    else
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

                    if (!sendSetCallback(decodedMessage.block, decodedMessage.section, i, decodedMessage.newValue))
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

        if (forcedSend)
        {
            sysExArray[responseSize] = 0xF7;
            responseSize++;

            sendSysExWriteCallback(sysExArray, responseSize);
        }
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
        addToResponse(decodedMessage.wish);
        addToResponse(decodedMessage.amount);
        addToResponse(decodedMessage.block);
        addToResponse(decodedMessage.section);
        addToResponse(0xF7);

        sendSysExWriteCallback(sysExArray, responseSize);
    }

    return true;
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
        if (decodedMessage.part == 127)
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
            if (decodedMessage.part == 127)
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
/// \brief Starts custom SysEx response.
///
void SysEx::startResponse()
{
    responseSize = 0;

    sysExArray[responseSize] = 0xF0;
    responseSize++;
    sysExArray[responseSize] = SYS_EX_M_ID_0;
    responseSize++;
    sysExArray[responseSize] = SYS_EX_M_ID_1;
    responseSize++;
    sysExArray[responseSize] = SYS_EX_M_ID_2;
    responseSize++;
    sysExArray[responseSize] = ACK;
    responseSize++;
    sysExArray[responseSize] = 0; //message part
    responseSize++;
}

///
/// \brief Adds value to custom SysEx response.
/// @param [in] value   New value.
///
void SysEx::addToResponse(sysExParameter_t value)
{
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
}

///
/// \brief Sends built SysEx response.
///
void SysEx::sendResponse()
{
    sysExArray[responseSize] = 0xF7;
    responseSize++;

    sendSysExWriteCallback(sysExArray, responseSize);
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
        case ERROR_HANDSHAKE:
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
        //no action
        break;
    }
}

///
/// \brief Handler used to set callback function for get requests.
///
void SysEx::setHandleGet(sysExParameter_t(*fptr)(uint8_t block, uint8_t section, uint16_t index))
{
    sendGetCallback = fptr;
}

///
/// \brief Handler used to set callback function for set requests.
///
void SysEx::setHandleSet(bool(*fptr)(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue))
{
    sendSetCallback = fptr;
}

///
/// \brief Handler used to set callback function for custom requests.
///
void SysEx::setHandleCustomRequest(bool(*fptr)(uint8_t value)) 
{
    sendCustomRequestCallback = fptr;
}

///
/// \brief Handler used to set callback function for handling assembled SysEx array.
///
void SysEx::setHandleSysExWrite(void(*fptr)(uint8_t *sysExArray, uint8_t size))
{
    sendSysExWriteCallback = fptr;
}
