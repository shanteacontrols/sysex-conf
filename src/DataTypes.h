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

#include "Config.h"

#if PARAM_SIZE == 2
typedef int16_t sysExParameter_t;
#elif PARAM_SIZE == 1
typedef int8_t sysExParameter_t;
#else
#error Incorrect parameter size for SysEx
#endif

///
/// \brief Structure holding data for single SysEx section within block.
///
typedef struct
{
    uint16_t numberOfParameters;
    sysExParameter_t newValueMin;
    sysExParameter_t newValueMax;
    uint8_t parts;
} sysExSection_t;

///
/// \brief Structure holding data for single SysEx block.
///
typedef struct
{
    uint8_t numberOfSections;
    sysExSection_t *section;
} sysExBlock_t;

///
/// \brief Structure containing data for single custom request.
///
typedef struct
{
    uint8_t requestID;  ///< ID byte representing specific request.
    bool connOpenCheck; ///< Flag indicating whether or not SysEx connection should be enabled before processing request.
} sysExCustomRequest_t;

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
    RESPONSE_SIZE = partByte + 1,
    MIN_MESSAGE_LENGTH = (wishByte + 1) + 1,    //special requests
    ML_REQ_STANDARD = REQUEST_SIZE + 1          //add end byte
} sysExRequestByteOrder;

///
/// \brief Byte order for parameters in SysEx message.
///
typedef enum
{
    indexByte = REQUEST_SIZE,
    newValueByte_single = indexByte+sizeof(sysExParameter_t),
    newValueByte_all = indexByte
} sysExParameterByteOrder;

///
/// \brief Descriptive list of SysEx wish bytes.
///
typedef enum
{
    sysExWish_get,
    sysExWish_set,
    sysExWish_backup,
    SYSEX_WISH_MAX
} sysExWish_t;

///
/// \brief Descriptive list of SysEx amount bytes.
///
typedef enum
{
    sysExAmount_single,
    sysExAmount_all,
    SYSEX_AMOUNT_MAX
} sysExAmount_t;

///
/// \brief Descriptive list of possible SysEx message statuses.
///
typedef enum
{
    REQUEST,                //0x00
    ACK,                    //0x01
    ERROR_STATUS,           //0x02
    ERROR_CONNECTION,       //0x03
    ERROR_WISH,             //0x04
    ERROR_AMOUNT,           //0x05
    ERROR_BLOCK,            //0x06
    ERROR_SECTION,          //0x07
    ERROR_PART,             //0x08
    ERROR_INDEX,            //0x09
    ERROR_NEW_VALUE,        //0x0A
    ERROR_MESSAGE_LENGTH,   //0x0B
    ERROR_WRITE,            //0x0C
    ERROR_NOT_SUPPORTED,    //0x0D
    ERROR_READ,             //0x0E
    SYSEX_STATUS_MAX
} sysExStatus_t;

///
/// \brief List of special SysEx IDs.
///
typedef enum
{
    SYSEX_SR_CONN_CLOSE,            //0x00
    SYSEX_SR_CONN_OPEN,             //0x01
    SYSEX_SR_BYTES_PER_VALUE,       //0x02
    SYSEX_SR_PARAMS_PER_MESSAGE,    //0x03
    SYSEX_SR_CONN_OPEN_SILENT,      //0x04
    SYSEX_SR_SILENT_DISABLE,        //0x05
    SYSEX_SR_TOTAL_NUMBER
} sysEx_specialRequest_t;

///
/// \brief Structure holding decoded request data.
///
typedef struct
{
    sysExStatus_t status;
    sysExWish_t wish;
    sysExAmount_t amount;
    uint8_t block;
    uint8_t section;
    uint8_t part;
    uint16_t index;
    sysExParameter_t newValue;
} decodedMessage_t;
