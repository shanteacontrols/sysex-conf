#include <gtest/gtest.h>
#include "SysEx.h"

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

static sysExBlock_t sysExLayout[NUMBER_OF_BLOCKS] =
{
    {
        .numberOfSections = NUMBER_OF_SECTIONS,
        .section = testSections
    }
};

static sysExCustomRequest_t customRequests[TOTAL_CUSTOM_REQUESTS] =
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

class GTSysExTestingValid : public ::testing::Test
{
    protected:
    SysExTestingValid sysEx;

    virtual void SetUp()
    {
        sysEx = SysExTestingValid();
        sysEx.setLayout(sysExLayout, NUMBER_OF_BLOCKS);
        sysEx.setupCustomRequests(customRequests, TOTAL_CUSTOM_REQUESTS);

        uint8_t arraySize = sizeof(connOpen)/sizeof(uint8_t);
        memcpy(sysEx.testArray, connOpen, arraySize);

        //send open connection request and see if sysExTestArray is valid
        sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

        //sysex configuration should be enabled now
        EXPECT_EQ(1, sysEx.isConfigurationEnabled());

        sysEx.responseCounter = 0;
    }

    virtual void TearDown()
    {
        
    }
};

class GTSysExTestingErrorGet : public ::testing::Test
{
    public:
    SysExTestingErrorGet sysEx;

    protected:
    virtual void SetUp()
    {
        sysEx = SysExTestingErrorGet();
        sysEx.setLayout(sysExLayout, NUMBER_OF_BLOCKS);
        sysEx.setupCustomRequests(customRequests, TOTAL_CUSTOM_REQUESTS);

        uint8_t arraySize = sizeof(connOpen)/sizeof(uint8_t);
        memcpy(sysEx.testArray, connOpen, arraySize);

        //send open connection request and see if sysExTestArray is valid
        sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

        //sysex configuration should be enabled now
        EXPECT_EQ(1, sysEx.isConfigurationEnabled());

        sysEx.responseCounter = 0;
    }

    virtual void TearDown()
    {
        
    }
};

class GTSysExTestingErrorSet : public ::testing::Test
{
    public:
    SysExTestingErrorSet sysEx;

    protected:
    virtual void SetUp()
    {
        sysEx = SysExTestingErrorSet();
        sysEx.setLayout(sysExLayout, NUMBER_OF_BLOCKS);
        sysEx.setupCustomRequests(customRequests, TOTAL_CUSTOM_REQUESTS);

        uint8_t arraySize = sizeof(connOpen)/sizeof(uint8_t);
        memcpy(sysEx.testArray, connOpen, arraySize);

        //send open connection request and see if sysExTestArray is valid
        sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

        //sysex configuration should be enabled now
        EXPECT_EQ(1, sysEx.isConfigurationEnabled());

        sysEx.responseCounter = 0;
    }

    virtual void TearDown()
    {
        
    }
};

TEST_F(GTSysExTestingValid, Init)
{
    //close connection
    uint8_t arraySize = sizeof(connClose)/sizeof(uint8_t);
    memcpy(sysEx.testArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //sysex configuration should be disabled now
    EXPECT_EQ(false, sysEx.isConfigurationEnabled());

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset number of received messages
    sysEx.responseCounter = 0;

    //test silent mode
    arraySize = sizeof(connOpenSilent)/sizeof(uint8_t);
    memcpy(sysEx.testArray, connOpenSilent, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //configuration and silent mode must be enabled
    EXPECT_EQ(true, sysEx.isSilentModeEnabled());
    EXPECT_EQ(true, sysEx.isConfigurationEnabled());

    //check that nothing was received as response
    EXPECT_EQ(sysEx.responseCounter, 0);

    //now disable silent mode
    arraySize = sizeof(silentModeDisable)/sizeof(uint8_t);
    memcpy(sysEx.testArray, silentModeDisable, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //silent mode should be disabled, but connection should be still opened
    //response should be received
    EXPECT_EQ(false, sysEx.isSilentModeEnabled());
    EXPECT_EQ(true, sysEx.isConfigurationEnabled());

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset number of received messages
    sysEx.responseCounter = 0;

    //open silent mode again
    arraySize = sizeof(connOpenSilent)/sizeof(uint8_t);
    memcpy(sysEx.testArray, connOpenSilent, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //verify no response was received
    EXPECT_EQ(sysEx.responseCounter, 0);

    //now close connection
    arraySize = sizeof(connClose)/sizeof(uint8_t);
    memcpy(sysEx.testArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //verify that connection is closed
    EXPECT_EQ(false, sysEx.isConfigurationEnabled());

    //silent mode should be disabled as well as an result of closed connection
    EXPECT_EQ(false, sysEx.isSilentModeEnabled());

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //enable silent mode using direct function call
    sysEx.setSilentMode(true);

    //verify silent mode is enabled
    EXPECT_EQ(true, sysEx.isSilentModeEnabled());

    //disable silent mode using direct function call
    sysEx.setSilentMode(false);

    //verify silent mode is disabled
    EXPECT_EQ(false, sysEx.isSilentModeEnabled());
}

TEST_F(GTSysExTestingValid, SilentMode)
{
    //open silent mode
    uint8_t arraySize = sizeof(connOpenSilent)/sizeof(uint8_t);
    memcpy(sysEx.testArray, connOpenSilent, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //configuration and silent mode must be enabled
    EXPECT_EQ(true, sysEx.isSilentModeEnabled());
    EXPECT_EQ(true, sysEx.isConfigurationEnabled());

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 0);

    //send set sigle request
    arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 0);

    //send set all request
    arraySize = sizeof(setAllValid)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setAllValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 0);

    //send request which causes status error
    arraySize = sizeof(errorStatus)/sizeof(uint8_t);
    memcpy(sysEx.testArray, errorStatus, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 0);

    //send request which causes wish error
    arraySize = sizeof(errorWish)/sizeof(uint8_t);
    memcpy(sysEx.testArray, errorWish, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 0);

    //send request which causes amount error
    arraySize = sizeof(errorAmount)/sizeof(uint8_t);
    memcpy(sysEx.testArray, errorAmount, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 0);

    //send request which causes block error
    arraySize = sizeof(errorBlock)/sizeof(uint8_t);
    memcpy(sysEx.testArray, errorBlock, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 0);

    //send request which causes section error
    arraySize = sizeof(errorSection)/sizeof(uint8_t);
    memcpy(sysEx.testArray, errorSection, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 0);

    //send request which causes index error
    arraySize = sizeof(errorIndex)/sizeof(uint8_t);
    memcpy(sysEx.testArray, errorIndex, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 0);

    //send request which causes part error
    arraySize = sizeof(errorPart)/sizeof(uint8_t);
    memcpy(sysEx.testArray, errorPart, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 0);

    //send request which causes length error
    arraySize = sizeof(errorLength)/sizeof(uint8_t);
    memcpy(sysEx.testArray, errorLength, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 0);

    //send custom request
    arraySize = sizeof(customReq)/sizeof(uint8_t);
    memcpy(sysEx.testArray, customReq, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 0);
}

TEST_F(GTSysExTestingValid, ErrorInit)
{
    //try to init sysex with null pointer
    EXPECT_EQ(false, sysEx.setLayout(NULL, 1));

    //try to init sysex with zero blocks
    EXPECT_EQ(false, sysEx.setLayout(sysExLayout, 0));
}

TEST_F(GTSysExTestingValid, ErrorConnClosed)
{
    uint8_t arraySize;

    //close connection first
    arraySize = sizeof(connClose)/sizeof(uint8_t);
    memcpy(sysEx.testArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //configuration should be closed now
    EXPECT_EQ(false, sysEx.isConfigurationEnabled());

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset number of received messages
    sysEx.responseCounter = 0;

    //send valid get message
    //since connection is closed, ERROR_CONNECTION should be reported
    arraySize = sizeof(getSingleValid)/sizeof(uint8_t);
    memcpy(sysEx.testArray, getSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_CONNECTION, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, ErrorStatus)
{
    //send message with invalid status byte
    //ERROR_STATUS should be reported
    uint8_t arraySize = sizeof(errorStatus)/sizeof(uint8_t);
    memcpy(sysEx.testArray, errorStatus, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_STATUS, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, ErrorWish)
{
    //send message with invalid wish byte
    uint8_t arraySize = sizeof(errorWish)/sizeof(uint8_t);
    memcpy(sysEx.testArray, errorWish, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_WISH, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, ErrorAmount)
{
    //send message with invalid amount byte
    //ERROR_AMOUNT should be reported
    uint8_t arraySize = sizeof(errorAmount)/sizeof(uint8_t);
    memcpy(sysEx.testArray, errorAmount, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_AMOUNT, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, ErrorBlock)
{
    //send message with invalid block byte
    //ERROR_BLOCK should be reported
    uint8_t arraySize = sizeof(errorBlock)/sizeof(uint8_t);
    memcpy(sysEx.testArray, errorBlock, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_BLOCK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, ErrorSection)
{
    //send message with invalid section byte
    //ERROR_SECTION should be reported
    uint8_t arraySize = sizeof(errorSection)/sizeof(uint8_t);
    memcpy(sysEx.testArray, errorSection, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_SECTION, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, ErrorPart)
{
    //send message with invalid index byte
    //ERROR_PART should be reported
    uint8_t arraySize = sizeof(errorPart)/sizeof(uint8_t);
    memcpy(sysEx.testArray, errorPart, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_PART, sysEx.testArray[4]);
    EXPECT_EQ(TEST_MSG_PART_INVALID, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, ErrorIndex)
{
    //send message with invalid index byte
    //ERROR_INDEX should be reported
    uint8_t arraySize = sizeof(errorIndex)/sizeof(uint8_t);
    memcpy(sysEx.testArray, errorIndex, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_INDEX, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, ErrorLength)
{
    //send message with invalid index byte
    //ERROR_MESSAGE_LENGTH should be reported
    uint8_t arraySize = sizeof(errorLength)/sizeof(uint8_t);
    memcpy(sysEx.testArray, errorLength, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_MESSAGE_LENGTH, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingErrorSet, ErrorWrite)
{
    //onSet is configure to always return false
    //check if status byte is ERROR_WRITE

    //send valid set message
    uint8_t arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_WRITE, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //set userError to ERROR_WRITE and check if message returns the same error
    sysEx.userError = ERROR_WRITE;

    //send valid set message
    //ERROR_WRITE should be reported since that error is assigned to userError
    arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_WRITE, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingErrorGet, ErrorRead)
{
    //send get single request
    //ERROR_READ should be reported since onGet returns false
    uint8_t arraySize = sizeof(getSingleValid)/sizeof(uint8_t);
    memcpy(sysEx.testArray, getSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_READ, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //test get with all parameters
    //ERROR_READ should be reported again
    arraySize = sizeof(getAllValid_1part)/sizeof(uint8_t);
    memcpy(sysEx.testArray, getAllValid_1part, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_READ, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, ErrorNewValue)
{
    //send invalid set message
    //ERROR_NEW_VALUE should be reported
    uint8_t arraySize = sizeof(setSingleInvalidNewValue)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleInvalidNewValue, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_NEW_VALUE, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, ErrorUser)
{
    sysEx.userError = ERROR_NOT_SUPPORTED;

    //send get request
    //ERROR_NOT_SUPPORTED should be returned since that value is assigned to userError
    uint8_t arraySize = sizeof(getSingleValid)/sizeof(uint8_t);
    memcpy(sysEx.testArray, getSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_NOT_SUPPORTED, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //get all request
    arraySize = sizeof(getAllValid_1part)/sizeof(uint8_t);
    memcpy(sysEx.testArray, getAllValid_1part, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_NOT_SUPPORTED, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //set single request
    arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_NOT_SUPPORTED, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //set all request
    arraySize = sizeof(setAllValid)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setAllValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_NOT_SUPPORTED, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //try to set invalid user error
    //response should be just ERROR_WRITE in this case
    sysEx.userError = SYSEX_STATUS_MAX;

    arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_WRITE, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, SetSingle)
{
    //send valid set message
    uint8_t arraySize = sizeof(setSingleValid)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send set single command with invalid param index
    arraySize = sizeof(setSingleInvalidParam)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleInvalidParam, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_INDEX, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //test block which has same min and max value
    //in this case, ERROR_NEW_VALUE should never be reported on any value
    arraySize = sizeof(setSingleNoMinMax1)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleNoMinMax1, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    arraySize = sizeof(setSingleNoMinMax2)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleNoMinMax2, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    arraySize = sizeof(setSingleNoMinMax3)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleNoMinMax3, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;
}

TEST_F(GTSysExTestingValid, SetAll)
{
    //send set all request
    uint8_t arraySize = sizeof(setAllValid)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setAllValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send set all message for section with more parts
    arraySize = sizeof(setAllMoreParts1)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setAllMoreParts1, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send set all request with part byte being 0x01
    arraySize = sizeof(setAllMoreParts2)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setAllMoreParts2, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x01, sysEx.testArray[5]); //second part
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send set all requests for all parts and verify that status byte is set to ERROR_PART
    arraySize = sizeof(setAllAllParts)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setAllAllParts, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_PART, sysEx.testArray[4]);
    EXPECT_EQ(0x7F, sysEx.testArray[5]); //same part as requested
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send set all request with invalid value
    //status byte should be ERROR_NEW_VALUE
    arraySize = sizeof(setAllnvalidNewVal)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setAllnvalidNewVal, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_NEW_VALUE, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;
}

TEST_F(GTSysExTestingErrorSet, ErrorSet)
{
    //verify that status is set to ERROR_WRITE if onSet returns false
    uint8_t arraySize = sizeof(setAllValid)/sizeof(uint8_t);
    memcpy(sysEx.testArray, setAllValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_WRITE, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, GetSingle)
{
    //send get single request
    uint8_t arraySize = sizeof(getSingleValid)/sizeof(uint8_t);
    memcpy(sysEx.testArray, getSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(TEST_VALUE_GET, sysEx.testArray[6]);
    EXPECT_EQ(0xF7, sysEx.testArray[7]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, GetAll)
{
    //send get all request
    uint8_t arraySize = sizeof(getAllValid_1part)/sizeof(uint8_t);
    memcpy(sysEx.testArray, getAllValid_1part, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    //10 values should be received
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(TEST_VALUE_GET, sysEx.testArray[6]);
    EXPECT_EQ(TEST_VALUE_GET, sysEx.testArray[7]);
    EXPECT_EQ(TEST_VALUE_GET, sysEx.testArray[8]);
    EXPECT_EQ(TEST_VALUE_GET, sysEx.testArray[9]);
    EXPECT_EQ(TEST_VALUE_GET, sysEx.testArray[10]);
    EXPECT_EQ(TEST_VALUE_GET, sysEx.testArray[11]);
    EXPECT_EQ(TEST_VALUE_GET, sysEx.testArray[12]);
    EXPECT_EQ(TEST_VALUE_GET, sysEx.testArray[13]);
    EXPECT_EQ(TEST_VALUE_GET, sysEx.testArray[14]);
    EXPECT_EQ(TEST_VALUE_GET, sysEx.testArray[15]);
    EXPECT_EQ(0xF7, sysEx.testArray[16]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //now send same request for all parts
    //we are expecting 2 messages now
    arraySize = sizeof(getAllValid_allParts_7F)/sizeof(uint8_t);
    memcpy(sysEx.testArray, getAllValid_allParts_7F, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 2);

    //reset message count
    sysEx.responseCounter = 0;

    //same message with part being 0x7E
    //in this case, last message should be ACK message
    arraySize = sizeof(getAllValid_allParts_7E)/sizeof(uint8_t);
    memcpy(sysEx.testArray, getAllValid_allParts_7E, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check last response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x7E, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 3);
}

TEST_F(GTSysExTestingValid, CustomReq)
{
    //reset message count
    sysEx.responseCounter = 0;

    //send valid custom request message
    uint8_t arraySize = sizeof(customReq)/sizeof(uint8_t);
    memcpy(sysEx.testArray, customReq, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(CUSTOM_REQUEST_VALUE, sysEx.testArray[6]);
    EXPECT_EQ(0xF7, sysEx.testArray[7]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send custom request message which should return false in custom request handler
    //in this case, ERROR_READ should be reported
    arraySize = sizeof(customReqErrorRead)/sizeof(uint8_t);
    memcpy(sysEx.testArray, customReqErrorRead, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_READ, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send non-existing custom request message
    //ERROR_WISH should be reported
    arraySize = sizeof(customReqInvalid)/sizeof(uint8_t);
    memcpy(sysEx.testArray, customReqInvalid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_WISH, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //disable configuration
    arraySize = sizeof(connClose)/sizeof(uint8_t);
    memcpy(sysEx.testArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //verify that connection is closed
    EXPECT_EQ(false, sysEx.isConfigurationEnabled());

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send valid custom request message
    //ERROR_CONNECTION should be reported
    arraySize = sizeof(customReq)/sizeof(uint8_t);
    memcpy(sysEx.testArray, customReq, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_CONNECTION, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

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
    memcpy(sysEx.testArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //sysex configuration should be disabled now
    EXPECT_EQ(false, sysEx.isConfigurationEnabled());

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send custom request 0
    //ERROR_CONNECTION should be returned because connection is closed
    arraySize = sizeof(customReq)/sizeof(uint8_t);
    memcpy(sysEx.testArray, customReq, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_CONNECTION, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send another custom request which has flag set to ignore connection status
    //ACK should be reported
    arraySize = sizeof(customReqNoConnCheck)/sizeof(uint8_t);
    memcpy(sysEx.testArray, customReqNoConnCheck, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(CUSTOM_REQUEST_VALUE, sysEx.testArray[6]);
    EXPECT_EQ(0xF7, sysEx.testArray[7]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //open connection again
    arraySize = sizeof(connOpen)/sizeof(uint8_t);
    memcpy(sysEx.testArray, connOpen, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //verify that connection is opened
    EXPECT_EQ(true, sysEx.isConfigurationEnabled());

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, IgnoreMessage)
{
    //verify that no action takes place when sysex ids in message don't match
    //short message is any message without every required byte
    uint8_t arraySize = sizeof(shortMessage1)/sizeof(uint8_t);
    memcpy(sysEx.testArray, shortMessage1, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //if no action took place, responseCounter should be 0
    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 0);

    //send another variant of short message
    arraySize = sizeof(shortMessage2)/sizeof(uint8_t);
    memcpy(sysEx.testArray, shortMessage2, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 0);

    //send message with invalid SysEx ID
    arraySize = sizeof(getSingleInvalidSysExID)/sizeof(uint8_t);
    memcpy(sysEx.testArray, getSingleInvalidSysExID, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 0);

    //short message where ERROR_MESSAGE_LENGTH should be returned
    arraySize = sizeof(shortMessage3)/sizeof(uint8_t);
    memcpy(sysEx.testArray, shortMessage3, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_MESSAGE_LENGTH, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, CustomMessage)
{
    //construct custom message and see if output matches
    sysExParameter_t values[] = 
    {
        0x05,
        0x06,
        0x07
    };

    sysEx.sendCustomMessage(sysEx.testArray, values, 3);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0x05, sysEx.testArray[6]);
    EXPECT_EQ(0x06, sysEx.testArray[7]);
    EXPECT_EQ(0x07, sysEx.testArray[8]);
    EXPECT_EQ(0xF7, sysEx.testArray[9]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //construct same message again with REQUEST as status byte
    sysEx.sendCustomMessage(sysEx.testArray, values, 3, false);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(REQUEST, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0x05, sysEx.testArray[6]);
    EXPECT_EQ(0x06, sysEx.testArray[7]);
    EXPECT_EQ(0x07, sysEx.testArray[8]);
    EXPECT_EQ(0xF7, sysEx.testArray[9]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, Backup)
{
    //send backup all request
    uint8_t arraySize = sizeof(backupAll)/sizeof(uint8_t);
    memcpy(sysEx.testArray, backupAll, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check if status byte is set to REQUEST value
    EXPECT_EQ(REQUEST, sysEx.testArray[(uint8_t)statusByte]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send backup/single request with incorrect part
    arraySize = sizeof(backupSingleInvPart)/sizeof(uint8_t);
    memcpy(sysEx.testArray, backupSingleInvPart, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_PART, sysEx.testArray[4]);
    EXPECT_EQ(0x03, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);
}

TEST_F(GTSysExTestingValid, SpecialRequest)
{
    //test all pre-configured special requests and see if they return correct value

    //bytes per value request
    uint8_t arraySize = sizeof(getSpecialReqBytesPerVal)/sizeof(uint8_t);
    memcpy(sysEx.testArray, getSpecialReqBytesPerVal, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(PARAM_SIZE, sysEx.testArray[6]);
    EXPECT_EQ(0xF7, sysEx.testArray[7]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //params per msg request
    arraySize = sizeof(getSpecialReqParamPerMsg)/sizeof(uint8_t);
    memcpy(sysEx.testArray, getSpecialReqParamPerMsg, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(PARAMETERS_PER_MESSAGE, sysEx.testArray[6]);
    EXPECT_EQ(0xF7, sysEx.testArray[7]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //now try those same requests, but without prior open connection request
    //status byte must equal ERROR_CONNECTION

    //close connection first
    arraySize = sizeof(connClose)/sizeof(uint8_t);
    memcpy(sysEx.testArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //configuration should be closed now
    EXPECT_EQ(false, sysEx.isConfigurationEnabled());

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //bytes per value request
    arraySize = sizeof(getSpecialReqBytesPerVal)/sizeof(uint8_t);
    memcpy(sysEx.testArray, getSpecialReqBytesPerVal, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_CONNECTION, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //params per msg request
    arraySize = sizeof(getSpecialReqParamPerMsg)/sizeof(uint8_t);
    memcpy(sysEx.testArray, getSpecialReqParamPerMsg, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_CONNECTION, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //try to close configuration which is already closed
    arraySize = sizeof(connClose)/sizeof(uint8_t);
    memcpy(sysEx.testArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ERROR_CONNECTION, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send open connection request and check if sysEx.testArray is valid
    arraySize = sizeof(connOpen)/sizeof(uint8_t);
    memcpy(sysEx.testArray, connOpen, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //sysex configuration should be enabled now
    EXPECT_EQ(1, sysEx.isConfigurationEnabled());

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;

    //disable configuration again
    arraySize = sizeof(connClose)/sizeof(uint8_t);
    memcpy(sysEx.testArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    EXPECT_EQ(0xF0, sysEx.testArray[0]);
    EXPECT_EQ(0x00, sysEx.testArray[1]);
    EXPECT_EQ(0x53, sysEx.testArray[2]);
    EXPECT_EQ(0x43, sysEx.testArray[3]);
    EXPECT_EQ(ACK, sysEx.testArray[4]);
    EXPECT_EQ(0x00, sysEx.testArray[5]);
    EXPECT_EQ(0xF7, sysEx.testArray[6]);

    //check number of received messages
    EXPECT_EQ(sysEx.responseCounter, 1);

    //reset message count
    sysEx.responseCounter = 0;
}

TEST_F(GTSysExTestingValid, AddToReponseFail)
{
    // try to add bytes to sysex response without first specifying array source
    EXPECT_EQ(false, sysEx.addToResponse(0x50));
}