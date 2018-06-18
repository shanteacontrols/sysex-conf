#include <gtest/gtest.h>
#include "../src/SysEx.h"

#define NUMBER_OF_BLOCKS                    1
#define NUMBER_OF_SECTIONS                  3

#define SECTION_0_PARAMETERS                10
#define SECTION_1_PARAMETERS                6
#define SECTION_2_PARAMETERS                33

#define SECTION_0_MIN                       0
#define SECTION_0_MAX                       50

#define SECTION_1_MIN                       0
#define SECTION_1_MAX                       0

#define SECTION_2_MIN                       0
#define SECTION_2_MAX                       0

#define TEST_BLOCK_ID                       0

#define TEST_SECTION_SINGLE_PART_ID         0
#define TEST_SECTION_MULTIPLE_PARTS_ID      2

#define TEST_SECTION_NOMINMAX               1
#define TEST_INDEX_ID                       5
#define TEST_MSG_PART_VALID                 0
#define TEST_MSG_PART_INVALID               10
#define TEST_INVALID_PARAMETER_B0S0         15
#define TEST_NEW_VALUE_VALID                25
#define TEST_NEW_VALUE_INVALID              100

#define TEST_VALUE_GET                      3

#define CUSTOM_REQUEST_ID_VALID             54
#define CUSTOM_REQUEST_ID_INVALID           55

#define CUSTOM_REQUEST_VALUE                1

uint8_t sysExTestArray[200];
int responseCounter;
sysExStatus_t userError;

static sysExSection_t testSections[NUMBER_OF_SECTIONS] =
{
    {
        .numberOfParameters = SECTION_0_PARAMETERS,
        .newValueMin = SECTION_0_MIN,
        .newValueMax = SECTION_0_MAX,
    },

    {
        .numberOfParameters = SECTION_1_PARAMETERS,
        .newValueMin = SECTION_1_MIN,
        .newValueMax = SECTION_1_MAX,
    },

    {
        .numberOfParameters = SECTION_2_PARAMETERS,
        .newValueMin = SECTION_2_MIN,
        .newValueMax = SECTION_2_MAX,
    }
};

sysExBlock_t sysExLayout[NUMBER_OF_BLOCKS] =
{
    {
        .numberOfSections = NUMBER_OF_SECTIONS,
        .section = testSections
    }
};

SysEx sysEx;

bool onCustom(uint8_t value)
{
    switch(value)
    {
        case CUSTOM_REQUEST_ID_VALID:
        sysEx.addToResponse(CUSTOM_REQUEST_VALUE);
        return true;
        break;

        default:
        return false;
        break;
    }
}

bool onGet(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t &value)
{
    if (userError)
    {
        sysEx.setError(userError);
        return false;
    }
    else
    {
        value = TEST_VALUE_GET;
        return true;
    }
}

bool onGetErrorRead(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t &value)
{
    return false;
}

bool onSet(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue)
{
    if (userError)
    {
        sysEx.setError(userError);
        return false;
    }
    else
    {
        return true;
    }
}

bool onSetInvalid(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue)
{
    return false;
}

void writeSysEx(uint8_t sysExTestArray[], uint8_t arraysize)
{
    for (int i=0; i<arraysize; i++)
        sysExTestArray[i] = sysExTestArray[i];

    responseCounter++;
}


class SysExTest : public ::testing::Test
{
    protected:
    virtual void SetUp()
    {
        userError = REQUEST;
        responseCounter = 0;
        sysEx.init(sysExLayout, NUMBER_OF_BLOCKS);
        sysEx.setHandleGet(onGet);
        sysEx.setHandleSet(onSet);
        sysEx.setHandleCustomRequest(onCustom);
        sysEx.setHandleSysExWrite(writeSysEx);

        uint8_t arraySize = sizeof(handshake)/sizeof(uint8_t);
        memcpy(sysExTestArray, handshake, arraySize);

        //send handshake message and see if sysExTestArray is valid
        sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

        //sysex configuration should be enabled after handshake
        EXPECT_EQ(1, sysEx.isConfigurationEnabled());
    }

    virtual void TearDown()
    {
        
    }

    const uint8_t handshake[8] =
    {
        //handshake request used to enable sysex configuration
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, 0x01, 0xF7
    };

    const uint8_t errorStatus[12] =
    {
        //get single message with invalid status byte for request message
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, ACK, TEST_MSG_PART_VALID, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, TEST_INDEX_ID, 0xF7
    };

    const uint8_t errorWish[12] =
    {
        //wish byte set to invalid value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, 0x04, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, TEST_INDEX_ID, 0xF7
    };

    const uint8_t errorAmount[12] =
    {
        //amount byte set to invalid value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_get, 0x08, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, TEST_INDEX_ID, 0xF7
    };

    const uint8_t errorBlock[12] =
    {
        //block byte set to invalid value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_get, sysExAmount_single, 0x41, TEST_SECTION_SINGLE_PART_ID, TEST_INDEX_ID, 0xF7
    };

    const uint8_t errorSection[12] =
    {
        //section byte set to invalid value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, 0x61, TEST_INDEX_ID, 0xF7
    };

    const uint8_t errorIndex[12] =
    {
        //index byte set to invalid value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, 0x7F, 0xF7
    };

    const uint8_t errorPart[12] =
    {
        //part byte set to invalid value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_INVALID, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, TEST_INDEX_ID, 0xF7
    };

    const uint8_t errorLength[13] =
    {
        //message intentionally one byte too long
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, TEST_INDEX_ID, 0x00, 0xF7
    };

    const uint8_t customReq[8] =
    {
        //custom request with custom ID specified by user
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, CUSTOM_REQUEST_ID_VALID, 0xF7
    };

    const uint8_t customReqInvalid[8] =
    {
        //custom request with non-existing custom ID
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, CUSTOM_REQUEST_ID_INVALID, 0xF7
    };

    const uint8_t shortMessage1[6] =
    {
        //short message which should be ignored by the protocol, variant 1
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0xF7
    };

    const uint8_t shortMessage2[4] =
    {
        //short message which should be ignored by the protocol, variant 2
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, 0xF7
    };

    const uint8_t shortMessage3[10] =
    {
        //short message on which protocol should throw MESSAGE_LENGTH error
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, 0xF7
    };

    const uint8_t getSingleValid[12] =
    {
        //valid get single command
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, TEST_INDEX_ID, 0xF7
    };

    const uint8_t getSingleInvalidSysExID[12] =
    {
        //get single command with invalid sysex ids
        0xF0, SYS_EX_M_ID_2, SYS_EX_M_ID_1, SYS_EX_M_ID_0, REQUEST, TEST_MSG_PART_VALID, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, TEST_INDEX_ID, 0xF7
    };

    const uint8_t getAllValid_1part[11] =
    {
        //valid get all command
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_get, sysExAmount_all, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, 0xF7
    };

    const uint8_t getAllValid_allParts_7F[11] =
    {
        //valid get all command for all parts (7F variant)
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x7F, sysExWish_get, sysExAmount_all, TEST_BLOCK_ID, TEST_SECTION_MULTIPLE_PARTS_ID, 0xF7
    };

    const uint8_t getAllValid_allParts_7E[11] =
    {
        //valid get all command for all parts (7E variant)
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x7E, sysExWish_get, sysExAmount_all, TEST_BLOCK_ID, TEST_SECTION_MULTIPLE_PARTS_ID, 0xF7
    };

    const uint8_t getSpecialReqBytesPerVal[8] =
    {
        //built-in special request which returns number of bytes per value configured in protocol
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, 0x00, 0x00, 0x02, 0xF7
    };

    const uint8_t getSpecialReqParamPerMsg[8] =
    {
        //built-in special request which returns number of parameters per message configured in protocol
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, 0x00, 0x00, 0x03, 0xF7
    };

    const uint8_t getSpecialReqCloseConf[8] =
    {
        //built-in special request which disables further sysex configuration
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, 0x00, 0x00, 0x00, 0xF7
    };

    const uint8_t setSingleValid[13] =
    {
        //valid set singe command
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_set, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, TEST_INDEX_ID, TEST_NEW_VALUE_VALID, 0xF7
    };

    const uint8_t setSingleInvalidNewValue[13] =
    {
        //set single command - invalid new value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_set, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, TEST_INDEX_ID, TEST_NEW_VALUE_INVALID, 0xF7
    };

    const uint8_t setAllValid[21] =
    {
        //valid set all command
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x00, sysExWish_set, sysExAmount_all, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xF7
    };

    const uint8_t setAllAllParts[21] =
    {
        //set all command with all parts modifier (invalid request)
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x7F, sysExWish_set, sysExAmount_all, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xF7
    };

    const uint8_t setSingleNoMinMax1[13] =
    {
        //valid set single command for section without min/max checking, variant 1
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_set, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_NOMINMAX, TEST_INDEX_ID, 0, 0xF7
    };

    const uint8_t setSingleNoMinMax2[13] =
    {
        //valid set single command for section without min/max checking, variant 2
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_set, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_NOMINMAX, TEST_INDEX_ID, 50, 0xF7
    };

    const uint8_t setSingleNoMinMax3[13] =
    {
        //valid set single command for section without min/max checking, variant 3
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_set, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_NOMINMAX, TEST_INDEX_ID, 127, 0xF7
    };

    const uint8_t setSingleInvalidParam[13] =
    {
        //set single command with invalid parameter index
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_set, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, TEST_INVALID_PARAMETER_B0S0, 0x00, 0xF7
    };

    const uint8_t setAllnvalidNewVal[21] =
    {
        //set all command with invalid new value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_set, sysExAmount_all, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, TEST_NEW_VALUE_INVALID, TEST_NEW_VALUE_INVALID, TEST_NEW_VALUE_INVALID, TEST_NEW_VALUE_INVALID, TEST_NEW_VALUE_INVALID, TEST_NEW_VALUE_INVALID, TEST_NEW_VALUE_INVALID, TEST_NEW_VALUE_INVALID, TEST_NEW_VALUE_INVALID, TEST_NEW_VALUE_INVALID, 0xF7
    };

    const uint8_t setAllMoreParts1[43] =
    {
        //set all command for section with more parts, part 0
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x00, sysExWish_set, sysExAmount_all, TEST_BLOCK_ID, TEST_SECTION_MULTIPLE_PARTS_ID, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xF7
    };

    const uint8_t setAllMoreParts2[12] =
    {
        //set all command for section with more parts, part 1
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x01, sysExWish_set, sysExAmount_all, TEST_BLOCK_ID, TEST_SECTION_MULTIPLE_PARTS_ID, 0x01, 0xF7
    };

    const uint8_t backupAll[11] =
    {
        //backup all command
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x7F, sysExWish_backup, sysExAmount_all, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, 0xF7
    };

    const uint8_t backupSingleInvPart[12] =
    {
        //backup single command with invalid part set
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x03, sysExWish_backup, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, 0x00, 0xF7
    };
};

TEST_F(SysExTest, Init)
{
    uint8_t arraySize = sizeof(handshake)/sizeof(uint8_t);
    memcpy(sysExTestArray, handshake, arraySize);

    //send handshake message and see if sysExTestArray is valid
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(0x01, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //sysex configuration should be enabled after handshake
    EXPECT_EQ(true, sysEx.isConfigurationEnabled());
}

TEST_F(SysExTest, ErrorInit)
{
    bool returnValue;

    //try to init sysex with null pointer
    returnValue = sysEx.init(NULL, 1);
    EXPECT_EQ(false, returnValue);

    //try to init sysex with zero blocks
    returnValue = sysEx.init(sysExLayout, 0);
    EXPECT_EQ(false, returnValue);
}

TEST_F(SysExTest, ErrorHandshake)
{
    uint8_t arraySize;

    if (sysEx.isConfigurationEnabled())
    {
        //close connection first
        arraySize = sizeof(getSpecialReqCloseConf)/sizeof(uint8_t);
        memcpy(sysExTestArray, getSpecialReqCloseConf, arraySize);

        sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

        //configuration should be closed now
        EXPECT_EQ(false, sysEx.isConfigurationEnabled());
    }

    arraySize = sizeof(getSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSingleValid, arraySize);

    //send valid get message
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if handshake error is set
    EXPECT_EQ(ERROR_HANDSHAKE, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorStatus)
{
    uint8_t arraySize = sizeof(errorStatus)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorStatus, arraySize);

    //send message with invalid status byte
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status error is set
    EXPECT_EQ(ERROR_STATUS, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorWish)
{
    uint8_t arraySize = sizeof(errorWish)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorWish, arraySize);

    //send message with invalid wish byte
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if wish error is set
    EXPECT_EQ(ERROR_WISH, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorAmount)
{
    uint8_t arraySize = sizeof(errorAmount)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorAmount, arraySize);

    //send message with invalid amount byte
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if amount error is set
    EXPECT_EQ(ERROR_AMOUNT, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorBlock)
{
    uint8_t arraySize = sizeof(errorBlock)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorBlock, arraySize);

    //send message with invalid block byte
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if block error is set
    EXPECT_EQ(ERROR_BLOCK, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorSection)
{
    uint8_t arraySize = sizeof(errorSection)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorSection, arraySize);

    //send message with invalid section byte
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if section error is set
    EXPECT_EQ(ERROR_SECTION, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorPart)
{
    uint8_t arraySize = sizeof(errorPart)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorPart, arraySize);

    //send message with invalid index byte
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if part error is set
    EXPECT_EQ(ERROR_PART, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorIndex)
{
    uint8_t arraySize = sizeof(errorIndex)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorIndex, arraySize);

    //send message with invalid index byte
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if index error is set
    EXPECT_EQ(ERROR_INDEX, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorLength)
{
    uint8_t arraySize = sizeof(errorLength)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorLength, arraySize);

    //send message with invalid index byte
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if message length error is set
    EXPECT_EQ(ERROR_MESSAGE_LENGTH, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorWrite)
{
    //set userError to ERROR_WRITE and check if message returns the same error
    userError = ERROR_WRITE;

    uint8_t arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleValid, arraySize);

    //send valid set message
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if write error is set
    EXPECT_EQ(ERROR_WRITE, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorRead)
{
    //test if ERROR_READ is set when get callback returns false
    sysEx.setHandleGet(onGetErrorRead);

    //test get with single parameter
    uint8_t arraySize = sizeof(getSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSingleValid, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if read error is set
    EXPECT_EQ(ERROR_READ, sysExTestArray[(uint8_t)statusByte]);

    //test get with all parameters
    arraySize = sizeof(getAllValid_1part)/sizeof(uint8_t);
    memcpy(sysExTestArray, getAllValid_1part, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if read error is set
    EXPECT_EQ(ERROR_READ, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorNewValue)
{
    uint8_t arraySize = sizeof(setSingleInvalidNewValue)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleInvalidNewValue, arraySize);

    //send invalid set message
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if write error is set
    EXPECT_EQ(ERROR_NEW_VALUE, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorUser)
{
    uint8_t arraySize = sizeof(getSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSingleValid, arraySize);

    userError = ERROR_NOT_SUPPORTED;

    //send get request
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte is set to NOT_SUPPORTTED value
    EXPECT_EQ(ERROR_NOT_SUPPORTED, sysExTestArray[(uint8_t)statusByte]);

    //get all request
    arraySize = sizeof(getAllValid_1part)/sizeof(uint8_t);
    memcpy(sysExTestArray, getAllValid_1part, arraySize);

    //send get request
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte is set to NOT_SUPPORTTED value
    EXPECT_EQ(ERROR_NOT_SUPPORTED, sysExTestArray[(uint8_t)statusByte]);

    //set single request
    arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleValid, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte is set to NOT_SUPPORTTED value
    EXPECT_EQ(ERROR_NOT_SUPPORTED, sysExTestArray[(uint8_t)statusByte]);

    //set all request
    arraySize = sizeof(setAllValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllValid, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte is set to NOT_SUPPORTTED value
    EXPECT_EQ(ERROR_NOT_SUPPORTED, sysExTestArray[(uint8_t)statusByte]);

    //try to set invalid user error
    //response should be just ERROR_WRITE in this case
    userError = NUMBER_OF_ERRORS;

    arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleValid, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ERROR_WRITE, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, SetSingle)
{
    uint8_t arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleValid, arraySize);

    //send valid set message
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte has ACK value
    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    //send set single command with invalid param index
    arraySize = sizeof(setSingleInvalidParam)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleInvalidParam, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ERROR_INDEX, sysExTestArray[(uint8_t)statusByte]);

    //test block which has same min and max value
    //in this case, error_newvalue should never be reported on any value
    arraySize = sizeof(setSingleNoMinMax1)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleNoMinMax1, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    arraySize = sizeof(setSingleNoMinMax2)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleNoMinMax2, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    arraySize = sizeof(setSingleNoMinMax3)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleNoMinMax3, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    //try calling valid set single command
    //configure set callback to always return false
    //check if status byte is ERROR_WRITE
    sysEx.setHandleSet(onSetInvalid);

    arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleValid, arraySize);

    //send valid set message
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ERROR_WRITE, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, SetAll)
{
    uint8_t arraySize = sizeof(setAllValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllValid, arraySize);

    //send set all request and check if result is ACK
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte has ACK value
    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    //handle same message again, but configure set callback to always return false
    sysEx.setHandleSet(onSetInvalid);

    arraySize = sizeof(setAllValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllValid, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ERROR_WRITE, sysExTestArray[(uint8_t)statusByte]);

    //restore callback
    sysEx.setHandleSet(onSet);

    //send set all message for section with more parts
    arraySize = sizeof(setAllMoreParts1)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllMoreParts1, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    arraySize = sizeof(setAllMoreParts2)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllMoreParts2, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    //send set all requests for all parts and verify that status byte is set to ERROR_PART
    arraySize = sizeof(setAllAllParts)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllAllParts, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ERROR_PART, sysExTestArray[(uint8_t)statusByte]);

    //set all with invalid value
    arraySize = sizeof(setAllnvalidNewVal)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllnvalidNewVal, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ERROR_NEW_VALUE, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, GetSingle)
{
    uint8_t arraySize = sizeof(getSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSingleValid, arraySize);

    //send valid get message
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte has ACK value
    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    //check if sysExTestArray equals value we're expecting
    EXPECT_EQ(TEST_VALUE_GET, sysExTestArray[6]);
}

TEST_F(SysExTest, GetAll)
{
    uint8_t arraySize = sizeof(getAllValid_1part)/sizeof(uint8_t);
    memcpy(sysExTestArray, getAllValid_1part, arraySize);

    //we are expecting 1 message as an sysExTestArray
    int tempResponseCounter = responseCounter;

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    int expectedCounter = tempResponseCounter+1;

    EXPECT_EQ(responseCounter, expectedCounter);

    //check if status byte has ACK value
    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    //now send same request for all parts
    //we are expecting 2 messages now

    arraySize = sizeof(getAllValid_allParts_7F)/sizeof(uint8_t);
    memcpy(sysExTestArray, getAllValid_allParts_7F, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(responseCounter, expectedCounter+2);

    expectedCounter = responseCounter;

    arraySize = sizeof(getAllValid_allParts_7E)/sizeof(uint8_t);
    memcpy(sysExTestArray, getAllValid_allParts_7E, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //last returned message should have status byte set to ACK
    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    //also, three messages should be returned
    EXPECT_EQ(responseCounter, expectedCounter+3);
}

TEST_F(SysExTest, CustomReq)
{
    //define custom request
    sysEx.addCustomRequest(CUSTOM_REQUEST_ID_VALID);

    uint8_t arraySize = sizeof(customReq)/sizeof(uint8_t);
    memcpy(sysExTestArray, customReq, arraySize);

    //send valid custom request message
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte has ACK value
    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    //check if sysExTestArray equals value we're expecting
    EXPECT_EQ(CUSTOM_REQUEST_VALUE, sysExTestArray[6]);

    arraySize = sizeof(customReqInvalid)/sizeof(uint8_t);
    memcpy(sysExTestArray, customReqInvalid, arraySize);

    //send non-existing custom request message
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte has error wish value
    EXPECT_EQ(ERROR_WISH, sysExTestArray[(uint8_t)statusByte]);

    //disable configuration
    arraySize = sizeof(getSpecialReqCloseConf)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSpecialReqCloseConf, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);
    EXPECT_EQ(false, sysEx.isConfigurationEnabled());

    arraySize = sizeof(customReq)/sizeof(uint8_t);
    memcpy(sysExTestArray, customReq, arraySize);

    //send valid custom request message
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte has error handshake value
    EXPECT_EQ(ERROR_HANDSHAKE, sysExTestArray[(uint8_t)statusByte]);

    bool value;

    //try defining illegal custom requests
    for (int i=0; i<SPECIAL_PARAMETERS; i++)
    {
        value = sysEx.addCustomRequest(i);
        //function must return false every time because
        //invalid values are being assigned as custom requests
        EXPECT_EQ(false, value);
    }

    //add maximum number of custom requests
    //start from 1 since one requests is already added
    for (int i=1; i<=MAX_CUSTOM_REQUESTS; i++)
    {
        value = sysEx.addCustomRequest(SPECIAL_PARAMETERS+i);
        //function must return true every time
        EXPECT_EQ(true, value);
    }

    //add another request
    value = sysEx.addCustomRequest(SPECIAL_PARAMETERS+MAX_CUSTOM_REQUESTS+1);

    //check if function returned false on too many custom requests
    EXPECT_EQ(false, value);
}

TEST_F(SysExTest, IgnoreMessage)
{
    //verify that no action takes place when sysex ids in message don't match
    //short message is any message without every required byte
    //store current sysExTestArray counter
    int tempResponseCounter = responseCounter;

    uint8_t arraySize = sizeof(shortMessage1)/sizeof(uint8_t);
    memcpy(sysExTestArray, shortMessage1, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);
    //if no action took place, sysExTestArray counter should be unchanged
    EXPECT_EQ(tempResponseCounter, responseCounter);

    arraySize = sizeof(shortMessage2)/sizeof(uint8_t);
    memcpy(sysExTestArray, shortMessage2, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);
    //if no action took place, sysExTestArray counter should be unchanged
    EXPECT_EQ(tempResponseCounter, responseCounter);

    arraySize = sizeof(getSingleInvalidSysExID)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSingleInvalidSysExID, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);
    //if no action took place, sysExTestArray counter should be unchanged
    EXPECT_EQ(tempResponseCounter, responseCounter);

    arraySize = sizeof(shortMessage3)/sizeof(uint8_t);
    memcpy(sysExTestArray, shortMessage3, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);
    EXPECT_EQ(ERROR_MESSAGE_LENGTH, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, CustomMessage)
{
    //construct custom message and see if output matches
    sysExParameter_t values[] = 
    {
        0x05,
        0x06,
        0x07
    };

    sysEx.sendCustomMessage(sysExTestArray, values, 3);

    EXPECT_EQ(0x05, sysExTestArray[6]);
    EXPECT_EQ(0x06, sysExTestArray[7]);
    EXPECT_EQ(0x07, sysExTestArray[8]);
}

TEST_F(SysExTest, Backup)
{
    uint8_t arraySize = sizeof(backupAll)/sizeof(uint8_t);
    memcpy(sysExTestArray, backupAll, arraySize);

    //send backup all request
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte is set to REQUEST value
    EXPECT_EQ(REQUEST, sysExTestArray[(uint8_t)statusByte]);

    //send backup/single request with incorrect part
    arraySize = sizeof(backupSingleInvPart)/sizeof(uint8_t);
    memcpy(sysExTestArray, backupSingleInvPart, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ERROR_PART, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, SpecialRequest)
{
    //test all pre-configured special requests and see if they return correct value
    //bytes per value request
    uint8_t arraySize = sizeof(getSpecialReqBytesPerVal)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSpecialReqBytesPerVal, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //status byte must be ACK
    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    //returned value must be PARAM_SIZE
    EXPECT_EQ(PARAM_SIZE, sysExTestArray[6]);

    //params per msg request
    arraySize = sizeof(getSpecialReqParamPerMsg)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSpecialReqParamPerMsg, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //status byte must be ACK
    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    //returned value must be PARAMETERS_PER_MESSAGE
    EXPECT_EQ(PARAMETERS_PER_MESSAGE, sysExTestArray[6]);

    //now try those same requests, but without prior sending of handshake
    //status byte must equal ERROR_HANDSHAKE
    //close connection first
    arraySize = sizeof(getSpecialReqCloseConf)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSpecialReqCloseConf, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //configuration should be closed now
    EXPECT_EQ(false, sysEx.isConfigurationEnabled());

    //bytes per value request
    arraySize = sizeof(getSpecialReqBytesPerVal)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSpecialReqBytesPerVal, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ERROR_HANDSHAKE, sysExTestArray[(uint8_t)statusByte]);

    //params per msg request
    arraySize = sizeof(getSpecialReqParamPerMsg)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSpecialReqParamPerMsg, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ERROR_HANDSHAKE, sysExTestArray[(uint8_t)statusByte]);

    //try to close configuration which is already closed
    arraySize = sizeof(getSpecialReqCloseConf)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSpecialReqCloseConf, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ERROR_HANDSHAKE, sysExTestArray[(uint8_t)statusByte]);

    //now re-enable conf
    arraySize = sizeof(handshake)/sizeof(uint8_t);
    memcpy(sysExTestArray, handshake, arraySize);

    //send handshake message and see if sysExTestArray is valid
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //sysex configuration should be enabled after handshake
    EXPECT_EQ(1, sysEx.isConfigurationEnabled());

    //close conf again
    arraySize = sizeof(getSpecialReqCloseConf)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSpecialReqCloseConf, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //status must now be ACK
    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, AddToReponseFail)
{
    //try to add bytes to sysex response without first specifying array source
    bool returnValue = sysEx.addToResponse(0x50);
    EXPECT_EQ(false, returnValue);
}