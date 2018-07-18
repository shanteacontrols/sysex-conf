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
#define CUSTOM_REQUEST_ID_ERROR_READ        56
#define CUSTOM_REQUEST_ID_NO_CONN_CHECK     57
#define CUSTOM_REQUEST_VALUE                1
#define TOTAL_CUSTOM_REQUESTS               3

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

sysExCustomRequest_t customRequests[TOTAL_CUSTOM_REQUESTS] =
{
    {
        .requestID = CUSTOM_REQUEST_ID_VALID,
        .connOpenCheck = true
    },

    {
        .requestID = CUSTOM_REQUEST_ID_NO_CONN_CHECK,
        .connOpenCheck = false
    },

    {
        .requestID = CUSTOM_REQUEST_ID_ERROR_READ,
        .connOpenCheck = true
    }
};

SysEx sysEx;

bool onCustom(uint8_t value)
{
    switch(value)
    {
        case CUSTOM_REQUEST_ID_VALID:
        case CUSTOM_REQUEST_ID_NO_CONN_CHECK:
        sysEx.addToResponse(CUSTOM_REQUEST_VALUE);
        return true;
        break;

        case CUSTOM_REQUEST_ID_ERROR_READ:
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
        sysEx.setupCustomRequests(customRequests, TOTAL_CUSTOM_REQUESTS);
        sysEx.setHandleGet(onGet);
        sysEx.setHandleSet(onSet);
        sysEx.setHandleCustomRequest(onCustom);
        sysEx.setHandleSysExWrite(writeSysEx);

        uint8_t arraySize = sizeof(connOpen)/sizeof(uint8_t);
        memcpy(sysExTestArray, connOpen, arraySize);

        //send open connection request and see if sysExTestArray is valid
        sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

        //sysex configuration should be enabled now
        EXPECT_EQ(1, sysEx.isConfigurationEnabled());
    }

    virtual void TearDown()
    {
        
    }

    const uint8_t connOpen[8] =
    {
        //request used to enable sysex configuration
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, SYSEX_SR_CONN_OPEN, 0xF7
    };

    const uint8_t connClose[8] =
    {
        //request used to disable sysex configuration
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, SYSEX_SR_CONN_CLOSE, 0xF7
    };

    const uint8_t connOpenSilent[8] =
    {
        //request used to enable sysex configuration in silent mode
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, SYSEX_SR_CONN_OPEN_SILENT, 0xF7
    };

    const uint8_t silentModeDisable[8] =
    {
        //request used to disable silent mode
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, SYSEX_SR_SILENT_DISABLE, 0xF7
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

    const uint8_t errorLength[13] =
    {
        //message intentionally one byte too long
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, TEST_INDEX_ID, TEST_INDEX_ID, 0xF7
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

    const uint8_t errorAmount[13] =
    {
        //amount byte set to invalid value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, sysExWish_get, 0x02, TEST_BLOCK_ID, TEST_SECTION_SINGLE_PART_ID, TEST_INDEX_ID, 0x00, 0xF7
    };

    const uint8_t customReq[8] =
    {
        //custom request with custom ID specified by user
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, CUSTOM_REQUEST_ID_VALID, 0xF7
    };

    const uint8_t customReqErrorRead[8] =
    {
        //custom request with custom ID specified by user
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, CUSTOM_REQUEST_ID_ERROR_READ, 0xF7
    };

    const uint8_t customReqInvalid[8] =
    {
        //custom request with non-existing custom ID
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, CUSTOM_REQUEST_ID_INVALID, 0xF7
    };

    const uint8_t customReqNoConnCheck[8] =
    {
        //custom request with non-existing custom ID
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART_VALID, CUSTOM_REQUEST_ID_NO_CONN_CHECK, 0xF7
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
    //reset message count
    responseCounter = 0;

    //send open request message
    uint8_t arraySize = sizeof(connOpen)/sizeof(uint8_t);
    memcpy(sysExTestArray, connOpen, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset number of received messages
    responseCounter = 0;

    //sysex configuration should be enabled now
    EXPECT_EQ(true, sysEx.isConfigurationEnabled());

    //close connection
    arraySize = sizeof(connClose)/sizeof(uint8_t);
    memcpy(sysExTestArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //sysex configuration should be disabled now
    EXPECT_EQ(false, sysEx.isConfigurationEnabled());

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset number of received messages
    responseCounter = 0;

    //test silent mode
    arraySize = sizeof(connOpenSilent)/sizeof(uint8_t);
    memcpy(sysExTestArray, connOpenSilent, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //configuration and silent mode must be enabled
    EXPECT_EQ(true, sysEx.isSilentModeEnabled());
    EXPECT_EQ(true, sysEx.isConfigurationEnabled());

    //check that nothing was received as response
    EXPECT_EQ(responseCounter, 0);

    //now disable silent mode
    arraySize = sizeof(silentModeDisable)/sizeof(uint8_t);
    memcpy(sysExTestArray, silentModeDisable, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //silent mode should be disabled, but connection should be still opened
    //response should be received
    EXPECT_EQ(false, sysEx.isSilentModeEnabled());
    EXPECT_EQ(true, sysEx.isConfigurationEnabled());

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset number of received messages
    responseCounter = 0;

    //open silent mode again
    arraySize = sizeof(connOpenSilent)/sizeof(uint8_t);
    memcpy(sysExTestArray, connOpenSilent, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //verify no response was received
    EXPECT_EQ(responseCounter, 0);

    //now close connection
    arraySize = sizeof(connClose)/sizeof(uint8_t);
    memcpy(sysExTestArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //verify that connection is closed
    EXPECT_EQ(false, sysEx.isConfigurationEnabled());

    //silent mode should be disabled as well as an result of closed connection
    EXPECT_EQ(false, sysEx.isSilentModeEnabled());

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //enable silent mode using direct function call
    sysEx.setSilentMode(true);

    //verify silent mode is enabled
    EXPECT_EQ(true, sysEx.isSilentModeEnabled());

    //disable silent mode using direct function call
    sysEx.setSilentMode(false);

    //verify silent mode is disabled
    EXPECT_EQ(false, sysEx.isSilentModeEnabled());
}

TEST_F(SysExTest, SilentMode)
{
    //reset message count
    responseCounter = 0;

    //open silent mode
    uint8_t arraySize = sizeof(connOpenSilent)/sizeof(uint8_t);
    memcpy(sysExTestArray, connOpenSilent, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //configuration and silent mode must be enabled
    EXPECT_EQ(true, sysEx.isSilentModeEnabled());
    EXPECT_EQ(true, sysEx.isConfigurationEnabled());

    //check number of received messages
    EXPECT_EQ(responseCounter, 0);

    //send set sigle request
    arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check number of received messages
    EXPECT_EQ(responseCounter, 0);

    //send set all request
    arraySize = sizeof(setAllValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check number of received messages
    EXPECT_EQ(responseCounter, 0);

    //send request which causes status error
    arraySize = sizeof(errorStatus)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorStatus, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check number of received messages
    EXPECT_EQ(responseCounter, 0);

    //send request which causes wish error
    arraySize = sizeof(errorWish)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorWish, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check number of received messages
    EXPECT_EQ(responseCounter, 0);

    //send request which causes amount error
    arraySize = sizeof(errorAmount)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorAmount, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check number of received messages
    EXPECT_EQ(responseCounter, 0);

    //send request which causes block error
    arraySize = sizeof(errorBlock)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorBlock, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check number of received messages
    EXPECT_EQ(responseCounter, 0);

    //send request which causes section error
    arraySize = sizeof(errorSection)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorSection, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check number of received messages
    EXPECT_EQ(responseCounter, 0);

    //send request which causes index error
    arraySize = sizeof(errorIndex)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorIndex, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check number of received messages
    EXPECT_EQ(responseCounter, 0);

    //send request which causes part error
    arraySize = sizeof(errorPart)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorPart, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check number of received messages
    EXPECT_EQ(responseCounter, 0);

    //send request which causes length error
    arraySize = sizeof(errorLength)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorLength, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check number of received messages
    EXPECT_EQ(responseCounter, 0);

    //send custom request
    arraySize = sizeof(customReq)/sizeof(uint8_t);
    memcpy(sysExTestArray, customReq, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check number of received messages
    EXPECT_EQ(responseCounter, 0);
}

TEST_F(SysExTest, ErrorInit)
{
    //try to init sysex with null pointer
    EXPECT_EQ(false, sysEx.init(NULL, 1));

    //try to init sysex with zero blocks
    EXPECT_EQ(false, sysEx.init(sysExLayout, 0));
}

TEST_F(SysExTest, ErrorConnClosed)
{
    //reset message count
    responseCounter = 0;

    uint8_t arraySize;

    if (sysEx.isConfigurationEnabled())
    {
        //close connection first
        arraySize = sizeof(connClose)/sizeof(uint8_t);
        memcpy(sysExTestArray, connClose, arraySize);
        sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

        //configuration should be closed now
        EXPECT_EQ(false, sysEx.isConfigurationEnabled());

        //check response
        EXPECT_EQ(0xF0, sysExTestArray[0]);
        EXPECT_EQ(0x00, sysExTestArray[1]);
        EXPECT_EQ(0x53, sysExTestArray[2]);
        EXPECT_EQ(0x43, sysExTestArray[3]);
        EXPECT_EQ(ACK, sysExTestArray[4]);
        EXPECT_EQ(0x00, sysExTestArray[5]);
        EXPECT_EQ(0xF7, sysExTestArray[6]);

        //check number of received messages
        EXPECT_EQ(responseCounter, 1);

        //reset number of received messages
        responseCounter = 0;
    }

    //send valid get message
    //since connection is closed, ERROR_CONNECTION should be reported
    arraySize = sizeof(getSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_CONNECTION, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, ErrorStatus)
{
    //reset message count
    responseCounter = 0;

    //send message with invalid status byte
    //ERROR_STATUS should be reported
    uint8_t arraySize = sizeof(errorStatus)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorStatus, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_STATUS, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, ErrorWish)
{
    //reset message count
    responseCounter = 0;

    //send message with invalid wish byte
    uint8_t arraySize = sizeof(errorWish)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorWish, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_WISH, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, ErrorAmount)
{
    //reset message count
    responseCounter = 0;

    //send message with invalid amount byte
    //ERROR_AMOUNT should be reported
    uint8_t arraySize = sizeof(errorAmount)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorAmount, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_AMOUNT, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, ErrorBlock)
{
    //reset message count
    responseCounter = 0;

    //send message with invalid block byte
    //ERROR_BLOCK should be reported
    uint8_t arraySize = sizeof(errorBlock)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorBlock, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_BLOCK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, ErrorSection)
{
    //reset message count
    responseCounter = 0;

    //send message with invalid section byte
    //ERROR_SECTION should be reported
    uint8_t arraySize = sizeof(errorSection)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorSection, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_SECTION, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, ErrorPart)
{
    //reset message count
    responseCounter = 0;

    //send message with invalid index byte
    //ERROR_PART should be reported
    uint8_t arraySize = sizeof(errorPart)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorPart, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_PART, sysExTestArray[4]);
    EXPECT_EQ(TEST_MSG_PART_INVALID, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, ErrorIndex)
{
    //reset message count
    responseCounter = 0;

    //send message with invalid index byte
    //ERROR_INDEX should be reported
    uint8_t arraySize = sizeof(errorIndex)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorIndex, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_INDEX, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, ErrorLength)
{
    //reset message count
    responseCounter = 0;

    //send message with invalid index byte
    //ERROR_MESSAGE_LENGTH should be reported
    uint8_t arraySize = sizeof(errorLength)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorLength, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_MESSAGE_LENGTH, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, ErrorWrite)
{
    //reset message count
    responseCounter = 0;

    //set userError to ERROR_WRITE and check if message returns the same error
    userError = ERROR_WRITE;

    //send valid set message
    //ERROR_WRITE should be reported since that error is assigned to userError
    uint8_t arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_WRITE, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, ErrorRead)
{
    //reset message count
    responseCounter = 0;

    //configure get callback to always return false
    sysEx.setHandleGet(onGetErrorRead);

    //send get single request
    //ERROR_READ should be reported since callback returns false
    uint8_t arraySize = sizeof(getSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_READ, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //test get with all parameters
    //ERROR_READ should be reported again
    arraySize = sizeof(getAllValid_1part)/sizeof(uint8_t);
    memcpy(sysExTestArray, getAllValid_1part, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_READ, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, ErrorNewValue)
{
    //reset message count
    responseCounter = 0;

    //send invalid set message
    //ERROR_NEW_VALUE should be reported
    uint8_t arraySize = sizeof(setSingleInvalidNewValue)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleInvalidNewValue, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_NEW_VALUE, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, ErrorUser)
{
    //reset message count
    responseCounter = 0;

    userError = ERROR_NOT_SUPPORTED;

    //send get request
    //ERROR_NOT_SUPPORTED should be returned since that value is assigned to userError
    uint8_t arraySize = sizeof(getSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_NOT_SUPPORTED, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //get all request
    arraySize = sizeof(getAllValid_1part)/sizeof(uint8_t);
    memcpy(sysExTestArray, getAllValid_1part, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_NOT_SUPPORTED, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //set single request
    arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_NOT_SUPPORTED, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //set all request
    arraySize = sizeof(setAllValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_NOT_SUPPORTED, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //try to set invalid user error
    //response should be just ERROR_WRITE in this case
    userError = SYSEX_STATUS_MAX;

    arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_WRITE, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, SetSingle)
{
    //reset message count
    responseCounter = 0;

    //send valid set message
    uint8_t arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //send set single command with invalid param index
    arraySize = sizeof(setSingleInvalidParam)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleInvalidParam, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_INDEX, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //test block which has same min and max value
    //in this case, ERROR_NEW_VALUE should never be reported on any value
    arraySize = sizeof(setSingleNoMinMax1)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleNoMinMax1, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    arraySize = sizeof(setSingleNoMinMax2)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleNoMinMax2, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    arraySize = sizeof(setSingleNoMinMax3)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleNoMinMax3, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //configure set callback to always return false
    //check if status byte is ERROR_WRITE
    sysEx.setHandleSet(onSetInvalid);

    //send valid set message
    arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_WRITE, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, SetAll)
{
    //reset message count
    responseCounter = 0;

    //send set all request
    uint8_t arraySize = sizeof(setAllValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //handle same message again, but configure set callback to always return false
    //status byte should then be ERROR_WRITE
    sysEx.setHandleSet(onSetInvalid);

    arraySize = sizeof(setAllValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_WRITE, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //restore callback
    sysEx.setHandleSet(onSet);

    //send set all message for section with more parts
    arraySize = sizeof(setAllMoreParts1)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllMoreParts1, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //send set all request with part byte being 0x01
    arraySize = sizeof(setAllMoreParts2)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllMoreParts2, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x01, sysExTestArray[5]); //second part
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //send set all requests for all parts and verify that status byte is set to ERROR_PART
    arraySize = sizeof(setAllAllParts)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllAllParts, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_PART, sysExTestArray[4]);
    EXPECT_EQ(0x7F, sysExTestArray[5]); //same part as requested
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //send set all request with invalid value
    //status byte should be ERROR_NEW_VALUE
    arraySize = sizeof(setAllnvalidNewVal)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllnvalidNewVal, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_NEW_VALUE, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;
}

TEST_F(SysExTest, GetSingle)
{
    //reset message count
    responseCounter = 0;

    //send get single request
    uint8_t arraySize = sizeof(getSingleValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(TEST_VALUE_GET, sysExTestArray[6]);
    EXPECT_EQ(0xF7, sysExTestArray[7]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, GetAll)
{
    //reset message count
    responseCounter = 0;

    //send get all request
    uint8_t arraySize = sizeof(getAllValid_1part)/sizeof(uint8_t);
    memcpy(sysExTestArray, getAllValid_1part, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    //10 values should be received
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(TEST_VALUE_GET, sysExTestArray[6]);
    EXPECT_EQ(TEST_VALUE_GET, sysExTestArray[7]);
    EXPECT_EQ(TEST_VALUE_GET, sysExTestArray[8]);
    EXPECT_EQ(TEST_VALUE_GET, sysExTestArray[9]);
    EXPECT_EQ(TEST_VALUE_GET, sysExTestArray[10]);
    EXPECT_EQ(TEST_VALUE_GET, sysExTestArray[11]);
    EXPECT_EQ(TEST_VALUE_GET, sysExTestArray[12]);
    EXPECT_EQ(TEST_VALUE_GET, sysExTestArray[13]);
    EXPECT_EQ(TEST_VALUE_GET, sysExTestArray[14]);
    EXPECT_EQ(TEST_VALUE_GET, sysExTestArray[15]);
    EXPECT_EQ(0xF7, sysExTestArray[16]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //now send same request for all parts
    //we are expecting 2 messages now
    arraySize = sizeof(getAllValid_allParts_7F)/sizeof(uint8_t);
    memcpy(sysExTestArray, getAllValid_allParts_7F, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check number of received messages
    EXPECT_EQ(responseCounter, 2);

    //reset message count
    responseCounter = 0;

    //same message with part being 0x7E
    //in this case, last message should be ACK message
    arraySize = sizeof(getAllValid_allParts_7E)/sizeof(uint8_t);
    memcpy(sysExTestArray, getAllValid_allParts_7E, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check last response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x7E, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 3);
}

TEST_F(SysExTest, CustomReq)
{
    //reset message count
    responseCounter = 0;

    //send valid custom request message
    uint8_t arraySize = sizeof(customReq)/sizeof(uint8_t);
    memcpy(sysExTestArray, customReq, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(CUSTOM_REQUEST_VALUE, sysExTestArray[6]);
    EXPECT_EQ(0xF7, sysExTestArray[7]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //send custom request message which should return false in custom request handler
    //in this case, ERROR_READ should be reported
    arraySize = sizeof(customReqErrorRead)/sizeof(uint8_t);
    memcpy(sysExTestArray, customReqErrorRead, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_READ, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //send non-existing custom request message
    //ERROR_WISH should be reported
    arraySize = sizeof(customReqInvalid)/sizeof(uint8_t);
    memcpy(sysExTestArray, customReqInvalid, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_WISH, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //disable configuration
    arraySize = sizeof(connClose)/sizeof(uint8_t);
    memcpy(sysExTestArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //verify that connection is closed
    EXPECT_EQ(false, sysEx.isConfigurationEnabled());

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //send valid custom request message
    //ERROR_CONNECTION should be reported
    arraySize = sizeof(customReq)/sizeof(uint8_t);
    memcpy(sysExTestArray, customReq, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_CONNECTION, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //try defining custom requests with invalid pointer
    EXPECT_EQ(sysEx.setupCustomRequests(NULL, 0), false);

    //try defining illegal custom requests
    sysExCustomRequest_t customRequests_invalid[SYSEX_SR_TOTAL_NUMBER] =
    {
        {
            .requestID = 0,
            .connOpenCheck = true
        },

        {
            .requestID = 1,
            .connOpenCheck = true
        },

        {
            .requestID = 2,
            .connOpenCheck = true
        },

        {
            .requestID = 3,
            .connOpenCheck = true
        },

        {
            .requestID = 4,
            .connOpenCheck = true
        },

        {
            .requestID = 5,
            .connOpenCheck = true
        }
    };

    //setupCustomRequests should return false because special requests which
    //are already used internally are defined in pointed structure
    EXPECT_EQ(sysEx.setupCustomRequests(customRequests_invalid, SYSEX_SR_TOTAL_NUMBER), false);

    //restore valid custom requests
    EXPECT_EQ(sysEx.setupCustomRequests(customRequests, TOTAL_CUSTOM_REQUESTS), true);

    //close sysex connection
    arraySize = sizeof(connClose)/sizeof(uint8_t);
    memcpy(sysExTestArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //sysex configuration should be disabled now
    EXPECT_EQ(false, sysEx.isConfigurationEnabled());

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //send custom request 0
    //ERROR_CONNECTION should be returned because connection is closed
    arraySize = sizeof(customReq)/sizeof(uint8_t);
    memcpy(sysExTestArray, customReq, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_CONNECTION, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //send another custom request which has flag set to ignore connection status
    //ACK should be reported
    arraySize = sizeof(customReqNoConnCheck)/sizeof(uint8_t);
    memcpy(sysExTestArray, customReqNoConnCheck, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(CUSTOM_REQUEST_VALUE, sysExTestArray[6]);
    EXPECT_EQ(0xF7, sysExTestArray[7]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //open connection again
    arraySize = sizeof(connOpen)/sizeof(uint8_t);
    memcpy(sysExTestArray, connOpen, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //verify that connection is opened
    EXPECT_EQ(true, sysEx.isConfigurationEnabled());

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, IgnoreMessage)
{
    //reset message count
    responseCounter = 0;

    //verify that no action takes place when sysex ids in message don't match
    //short message is any message without every required byte
    uint8_t arraySize = sizeof(shortMessage1)/sizeof(uint8_t);
    memcpy(sysExTestArray, shortMessage1, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //if no action took place, responseCounter should be 0
    //check number of received messages
    EXPECT_EQ(responseCounter, 0);

    //send another variant of short message
    arraySize = sizeof(shortMessage2)/sizeof(uint8_t);
    memcpy(sysExTestArray, shortMessage2, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check number of received messages
    EXPECT_EQ(responseCounter, 0);

    //send message with invalid SysEx ID
    arraySize = sizeof(getSingleInvalidSysExID)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSingleInvalidSysExID, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check number of received messages
    EXPECT_EQ(responseCounter, 0);

    //short message where ERROR_MESSAGE_LENGTH should be returned
    arraySize = sizeof(shortMessage3)/sizeof(uint8_t);
    memcpy(sysExTestArray, shortMessage3, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_MESSAGE_LENGTH, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, CustomMessage)
{
    //reset message count
    responseCounter = 0;

    //construct custom message and see if output matches
    sysExParameter_t values[] = 
    {
        0x05,
        0x06,
        0x07
    };

    sysEx.sendCustomMessage(sysExTestArray, values, 3);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0x05, sysExTestArray[6]);
    EXPECT_EQ(0x06, sysExTestArray[7]);
    EXPECT_EQ(0x07, sysExTestArray[8]);
    EXPECT_EQ(0xF7, sysExTestArray[9]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //construct same message again with REQUEST as status byte
    sysEx.sendCustomMessage(sysExTestArray, values, 3, false);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(REQUEST, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0x05, sysExTestArray[6]);
    EXPECT_EQ(0x06, sysExTestArray[7]);
    EXPECT_EQ(0x07, sysExTestArray[8]);
    EXPECT_EQ(0xF7, sysExTestArray[9]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, Backup)
{
    //reset message count
    responseCounter = 0;

    //send backup all request
    uint8_t arraySize = sizeof(backupAll)/sizeof(uint8_t);
    memcpy(sysExTestArray, backupAll, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte is set to REQUEST value
    EXPECT_EQ(REQUEST, sysExTestArray[(uint8_t)statusByte]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //send backup/single request with incorrect part
    arraySize = sizeof(backupSingleInvPart)/sizeof(uint8_t);
    memcpy(sysExTestArray, backupSingleInvPart, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_PART, sysExTestArray[4]);
    EXPECT_EQ(0x03, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);
}

TEST_F(SysExTest, SpecialRequest)
{
    //reset message count
    responseCounter = 0;

    //test all pre-configured special requests and see if they return correct value

    //bytes per value request
    uint8_t arraySize = sizeof(getSpecialReqBytesPerVal)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSpecialReqBytesPerVal, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(PARAM_SIZE, sysExTestArray[6]);
    EXPECT_EQ(0xF7, sysExTestArray[7]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //params per msg request
    arraySize = sizeof(getSpecialReqParamPerMsg)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSpecialReqParamPerMsg, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(PARAMETERS_PER_MESSAGE, sysExTestArray[6]);
    EXPECT_EQ(0xF7, sysExTestArray[7]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //now try those same requests, but without prior open connection request
    //status byte must equal ERROR_CONNECTION

    //close connection first
    arraySize = sizeof(connClose)/sizeof(uint8_t);
    memcpy(sysExTestArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //configuration should be closed now
    EXPECT_EQ(false, sysEx.isConfigurationEnabled());

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //bytes per value request
    arraySize = sizeof(getSpecialReqBytesPerVal)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSpecialReqBytesPerVal, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_CONNECTION, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //params per msg request
    arraySize = sizeof(getSpecialReqParamPerMsg)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSpecialReqParamPerMsg, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_CONNECTION, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //try to close configuration which is already closed
    arraySize = sizeof(connClose)/sizeof(uint8_t);
    memcpy(sysExTestArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ERROR_CONNECTION, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //send open connection request and check if sysExTestArray is valid
    arraySize = sizeof(connOpen)/sizeof(uint8_t);
    memcpy(sysExTestArray, connOpen, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //sysex configuration should be enabled now
    EXPECT_EQ(1, sysEx.isConfigurationEnabled());

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;

    //disable configuration again
    arraySize = sizeof(connClose)/sizeof(uint8_t);
    memcpy(sysExTestArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysExTestArray[0]);
    EXPECT_EQ(0x00, sysExTestArray[1]);
    EXPECT_EQ(0x53, sysExTestArray[2]);
    EXPECT_EQ(0x43, sysExTestArray[3]);
    EXPECT_EQ(ACK, sysExTestArray[4]);
    EXPECT_EQ(0x00, sysExTestArray[5]);
    EXPECT_EQ(0xF7, sysExTestArray[6]);

    //check number of received messages
    EXPECT_EQ(responseCounter, 1);

    //reset message count
    responseCounter = 0;
}

TEST_F(SysExTest, AddToReponseFail)
{
    //try to add bytes to sysex response without first specifying array source
    EXPECT_EQ(false, sysEx.addToResponse(0x50));
}