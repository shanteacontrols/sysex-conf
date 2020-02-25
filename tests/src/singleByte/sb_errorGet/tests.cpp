#include "unity/src/unity.h"
#include "unity/Helpers.h"
#include "src/SysExTesting.h"

#define SYS_EX_CONF_M_ID_0 0x00
#define SYS_EX_CONF_M_ID_1 0x53
#define SYS_EX_CONF_M_ID_2 0x43

namespace
{
    SysExConf::manufacturerID_t mId = {
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2
    };

    SysExConf::section_t testSections[NUMBER_OF_SECTIONS] = {
        {
            .numberOfParameters = SECTION_0_PARAMETERS,
            .newValueMin        = SECTION_0_MIN,
            .newValueMax        = SECTION_0_MAX,
        },

        {
            .numberOfParameters = SECTION_1_PARAMETERS,
            .newValueMin        = SECTION_1_MIN,
            .newValueMax        = SECTION_1_MAX,
        },

        {
            .numberOfParameters = SECTION_2_PARAMETERS,
            .newValueMin        = SECTION_2_MIN,
            .newValueMax        = SECTION_2_MAX,
        }
    };

    SysExConf::block_t sysExLayout[NUMBER_OF_BLOCKS] = {
        { .numberOfSections = NUMBER_OF_SECTIONS,
          .section          = testSections }
    };

    SysExConf::customRequest_t customRequests[TOTAL_CUSTOM_REQUESTS] = {
        {
            .requestID     = CUSTOM_REQUEST_ID_VALID,
            .connOpenCheck = true,
        },

        {
            .requestID     = CUSTOM_REQUEST_ID_NO_CONN_CHECK,
            .connOpenCheck = false,
        },

        {
            .requestID     = CUSTOM_REQUEST_ID_ERROR_READ,
            .connOpenCheck = true,
        }
    };

    const uint8_t connOpen[8] = {
        //request used to enable sysex configuration
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::specialRequest_t::connOpen),
        0xF7
    };

    const uint8_t getSingleValid[12] = {
        //valid get single command
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::wish_t::get),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        TEST_BLOCK_ID,
        TEST_SECTION_SINGLE_PART_ID,
        TEST_INDEX_ID,
        0xF7
    };

    const uint8_t getAllValid_1part[11] = {
        //valid get all command
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::wish_t::get),
        static_cast<uint8_t>(SysExConf::amount_t::all),
        TEST_BLOCK_ID,
        TEST_SECTION_SINGLE_PART_ID,
        0xF7
    };

    SysExTestingErrorGet sysEx(mId, SysExConf::paramSize_t::_7bit, SysExConf::nrOfParam_t::_32);
}    // namespace

TEST_SETUP()
{
    sysEx.reset();
    sysEx.setLayout(sysExLayout, NUMBER_OF_BLOCKS);
    sysEx.setupCustomRequests(customRequests, TOTAL_CUSTOM_REQUESTS);

    uint8_t arraySize = sizeof(connOpen) / sizeof(uint8_t);
    memcpy(sysEx.testArray, connOpen, arraySize);

    //send open connection request and see if sysExTestArray is valid
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //sysex configuration should be enabled now
    TEST_ASSERT(1 == sysEx.isConfigurationEnabled());

    sysEx.responseCounter = 0;
}

TEST_CASE(ErrorRead)
{
    //send get single request
    //SysExConf::status_t::errorRead should be reported since onGet returns false
    uint8_t arraySize = sizeof(getSingleValid) / sizeof(uint8_t);
    memcpy(sysEx.testArray, getSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorRead) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //test get with all parameters
    //SysExConf::status_t::errorRead should be reported again
    arraySize = sizeof(getAllValid_1part) / sizeof(uint8_t);
    memcpy(sysEx.testArray, getAllValid_1part, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorRead) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}