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
/// \brief Function pointer used to retrieve data.
///
bool (*getCallback)(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t &value);

///
/// \brief Function pointer used to change data values.
///
bool (*setCallback)(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue);

///
/// \brief Function pointer used to handle user specified requests.
///
bool (*customRequestCallback)(uint8_t value);

///
/// \brief Function pointer used to send SysEx response.
///
void (*sysExWriteCallback)(uint8_t *sysExArray, uint8_t size);

///
/// \brief Flag indicating whether or not configuration is possible.
///
bool                sysExEnabled;

///
/// \brief Flag indicating whether or not silent mode is active.
/// When silent mode is active, protocol won't return any error or ACK messages.
///
bool                silentModeEnabled;

///
/// \brief Flag indicating whether or not message end has already been sent.
/// Message end (SysEx stop byte) is usually sent in handleMessage function,
/// however, for specific requests, response can be internally sent in other
/// functions which removes the need to send SysEx stop byte in handleMessage function.
///
bool                messageEndSent;

///
/// \brief Pointer to SysEx layout.
///
sysExBlock_t        *sysExMessage;

///
/// \brief Total number of blocks for a received SysEx layout.
///
uint8_t             sysExBlockCounter;

///
/// \brief Structure containing decoded data from SysEx request for easier access.
///
decodedMessage_t    decodedMessage;

///
/// \brief Pointer to SysEx array.
/// Same array is used for request and response.
/// Response modifies received request so that arrays aren't duplicated.
///
uint8_t             *sysExArray;

///
/// \brief Size of received SysEx array.
///
uint8_t             receivedArraySize;

///
/// \brief Size of SysEx response.
///
uint8_t             responseSize;

///
/// \brief User-set SysEx status.
/// Used when user sets custom status.
///
sysExStatus_t       userStatus;

///
/// \brief Array containing user-specifed custom requests.
///
uint8_t             customRequests[MAX_CUSTOM_REQUESTS];

///
/// \brief Holds amount of user-specified custom requests.
///
uint8_t             customRequestCounter;


///
/// \brief Default constructor.
///
SysEx::SysEx()
{
    sysExMessage = NULL;
    sysExBlockCounter = 0;
}

///
/// Configures user specifed configuration layout and initializes data to their default values.
/// @param [in] pointer     Pointer to configuration structure.
/// @param [in] numberOfBlocks  Total number of blocks in configuration structure.
/// \returns True on success, false otherwise.
///
bool SysEx::init(sysExBlock_t *pointer, uint8_t numberOfBlocks)
{
    getCallback             = NULL;
    setCallback             = NULL;
    customRequestCallback   = NULL;
    sysExWriteCallback      = NULL;
    sysExArray              = NULL;

    sysExEnabled = false;
    messageEndSent = false;

    for (int i=0; i<MAX_CUSTOM_REQUESTS; i++)
        customRequests[i] = 128;

    customRequestCounter = 0;
    userStatus = (sysExStatus_t)0;

    if ((pointer != NULL) && numberOfBlocks)
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
/// \brief Adds custom request.
/// If added byte is found in incoming message, and message is formatted as special request, custom message handler is called.
/// It is up to user to decide on action.
/// @param [in] value   Custom request value.
/// \returns            True on success, false otherwise.
///
bool SysEx::addCustomRequest(uint8_t value)
{
    if (customRequestCounter > MAX_CUSTOM_REQUESTS)
        return false;

    //don't add custom string if it's already defined as one of default strings
    if (value < SPECIAL_PARAMETERS)
        return false;

    //sanitize input
    value &= 0x7F;

    //check if custom request has already been added
    for (int i=0; i<customRequestCounter; i++)
    {
        if (customRequests[i] == value)
            return false;
    }

    customRequests[customRequestCounter] = value;
    customRequestCounter++;
    return true;
}

///
/// \brief Handles incoming SysEx message.
/// @param [in] array   SysEx array.
/// @param [in] size    Array size.
///
void SysEx::handleMessage(uint8_t *array, uint8_t size)
{
    if ((sysExMessage != NULL) && sysExBlockCounter)
    {
        //save pointer to received array so we can manipulate it directly
        sysExArray = array;
        receivedArraySize = size;
        responseSize = RESPONSE_SIZE;

        if (receivedArraySize < MIN_MESSAGE_LENGTH)
            return; //ignore small messages

        if (!checkID())
            return; //don't send response to wrong ID

        decode();

        if (!messageEndSent)
        {
            //if messageEndSent is set to true, response has already been sent
            sysExArray[responseSize] = 0xF7;
            responseSize++;

            if (sysExWriteCallback != NULL)
                sysExWriteCallback(sysExArray, responseSize);
        }

        messageEndSent = false;
        sysExArray = NULL;
    }
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

    if (checkSpecialRequests())
        return; //special request was handled by now

    //message appears to be fine for now
    if (!sysExEnabled)
    {
        //message is fine, but handshake hasn't been received
        setStatus(ERROR_HANDSHAKE);
        return;
    }

    if (receivedArraySize <= REQUEST_SIZE)
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

    if (receivedArraySize != length)
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
    if (receivedArraySize != MIN_MESSAGE_LENGTH)
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
        case SILENT_MODE_OPEN_REQUEST:
        //hello message, necessary to allow the configuration
        sysExEnabled = true;
        if (sysExArray[wishByte] == SILENT_MODE_OPEN_REQUEST)
            silentModeEnabled = true;
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
            if (customRequests[i] == 128)
                continue;

            if (customRequests[i] != sysExArray[wishByte])
                continue;

            if (sysExEnabled)
            {
                setStatus(ACK);

                if (customRequestCallback != NULL)
                {
                    customRequestCallback(customRequests[i]);
                }
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
            messageEndSent = true;

            if (decodedMessage.part == 126)
            {
                allPartsAck = true;
            }
        }

        if (decodedMessage.wish == sysExWish_backup)
        {
            //don't overwrite anything if backup is requested
            responseSize_ = receivedArraySize - 1;
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

        if (messageEndSent)
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
                        if (getCallback != NULL)
                        {
                            sysExParameter_t value = 0;
                            bool returnValue = getCallback(decodedMessage.block, decodedMessage.section, decodedMessage.index, value);

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
                }
                else
                {
                    if (getCallback != NULL)
                    {
                        //get all params - no index is specified
                        sysExParameter_t value = 0;
                        bool returnValue = getCallback(decodedMessage.block, decodedMessage.section, i, value);

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

                    if (setCallback != NULL)
                    {
                        if (setCallback(decodedMessage.block, decodedMessage.section, decodedMessage.index, decodedMessage.newValue))
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

                    if (setCallback != NULL)
                    {
                        if (!setCallback(decodedMessage.block, decodedMessage.section, i, decodedMessage.newValue))
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
                }
                break;
            }
        }

        if (messageEndSent)
        {
            sysExArray[responseSize] = 0xF7;
            responseSize++;

            if (sysExWriteCallback != NULL)
                sysExWriteCallback(sysExArray, responseSize);
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

        if (sysExWriteCallback != NULL)
            sysExWriteCallback(sysExArray, responseSize);
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

    sysExArray[responseSize] = 0xF7;
    responseSize++;

    if (sysExWriteCallback != NULL)
        sysExWriteCallback(sysExArray, responseSize);
}

///
/// \brief Adds value to SysEx response.
/// This function append value to last specified SysEx array.
/// @param [in] value   New value.
/// \returns True on success, false otherwise.
///
bool SysEx::addToResponse(sysExParameter_t value)
{
    if (sysExArray == NULL)
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
        userStatus = ERROR_WRITE;
        break;
    }
}

///
/// \brief Handler used to set callback function for get requests.
///
void SysEx::setHandleGet(bool(*fptr)(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t &value))
{
    getCallback = fptr;
}

///
/// \brief Handler used to set callback function for set requests.
///
void SysEx::setHandleSet(bool(*fptr)(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue))
{
    setCallback = fptr;
}

///
/// \brief Handler used to set callback function for custom requests.
///
void SysEx::setHandleCustomRequest(bool(*fptr)(uint8_t value)) 
{
    customRequestCallback = fptr;
}

///
/// \brief Handler used to set callback function for handling assembled SysEx array.
///
void SysEx::setHandleSysExWrite(void(*fptr)(uint8_t *sysExArray, uint8_t size))
{
    sysExWriteCallback = fptr;
}
