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

#include <inttypes.h>

///
/// \brief Maximum number of custom requests user can add.
///
#define MAX_CUSTOM_REQUESTS     16

///
/// \brief Maximum number of parameter indexes in single SysEx message.
///
#define PARAMETERS_PER_MESSAGE  32

///
/// \brief Size of single parameter value in SysEx message.
/// 1 - one byte size for parameter index and new value (uint8_t)
/// 2 - two byte size (uint16_t)
///
#define PARAM_SIZE              1

///
/// \brief Manufacturer SysEx bytes.
/// @{

#define SYS_EX_M_ID_0           0x00
#define SYS_EX_M_ID_1           0x53
#define SYS_EX_M_ID_2           0x43

/// @}
