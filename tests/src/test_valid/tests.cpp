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

    SysExConf::block_t sysExLayout[NUMBER_OF_BLOCKS] = {
        { .numberOfSections = NUMBER_OF_SECTIONS,
          .section = testSections }
    };

    SysExConf::customRequest_t customRequests[TOTAL_CUSTOM_REQUESTS] = {
        { .requestID = CUSTOM_REQUEST_ID_VALID,
          .connOpenCheck = true },

        { .requestID = CUSTOM_REQUEST_ID_NO_CONN_CHECK,
          .connOpenCheck = false },

        { .requestID = CUSTOM_REQUEST_ID_ERROR_READ,
          .connOpenCheck = true }
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

    const uint8_t connClose[8] = {
        //request used to disable sysex configuration
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::specialRequest_t::connClose),
        0xF7
    };

    const uint8_t connOpenSilent[8] = {
        //request used to enable sysex configuration in silent mode
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::specialRequest_t::connOpenSilent),
        0xF7
    };

    const uint8_t silentModeDisable[8] = {
        //request used to disable silent mode
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::specialRequest_t::connSilentDisable),
        0xF7
    };

    const uint8_t errorStatus[12] = {
        //get single message with invalid status byte for request message
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::ack),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::wish_t::get),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        TEST_BLOCK_ID,
        TEST_SECTION_SINGLE_PART_ID,
        TEST_INDEX_ID,
        0xF7
    };

    const uint8_t errorWish[12] = {
        //wish byte set to invalid value
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        0x04,
        static_cast<uint8_t>(SysExConf::amount_t::single),
        TEST_BLOCK_ID,
        TEST_SECTION_SINGLE_PART_ID,
        TEST_INDEX_ID,
        0xF7
    };

    const uint8_t errorLength[13] = {
        //message intentionally one byte too long
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
        TEST_INDEX_ID,
        0xF7
    };

    const uint8_t errorBlock[12] = {
        //block byte set to invalid value
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::wish_t::get),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        0x41,
        TEST_SECTION_SINGLE_PART_ID,
        TEST_INDEX_ID,
        0xF7
    };

    const uint8_t errorSection[12] = {
        //section byte set to invalid value
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::wish_t::get),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        TEST_BLOCK_ID,
        0x61,
        TEST_INDEX_ID,
        0xF7
    };

    const uint8_t errorIndex[12] = {
        //index byte set to invalid value
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
        0x7F,
        0xF7
    };

    const uint8_t errorPart[12] = {
        //part byte set to invalid value
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_INVALID,
        static_cast<uint8_t>(SysExConf::wish_t::get),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        TEST_BLOCK_ID,
        TEST_SECTION_SINGLE_PART_ID,
        TEST_INDEX_ID,
        0xF7
    };

    const uint8_t errorAmount[13] = {
        //amount byte set to invalid value
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::wish_t::get),
        0x02,
        TEST_BLOCK_ID,
        TEST_SECTION_SINGLE_PART_ID,
        TEST_INDEX_ID,
        0x00,
        0xF7
    };

    const uint8_t customReq[8] = {
        //custom request with custom ID specified by user
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        CUSTOM_REQUEST_ID_VALID,
        0xF7
    };

    const uint8_t customReqErrorRead[8] = {
        //custom request with custom ID specified by user
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        CUSTOM_REQUEST_ID_ERROR_READ,
        0xF7
    };

    const uint8_t customReqInvalid[8] = {
        //custom request with non-existing custom ID
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        CUSTOM_REQUEST_ID_INVALID,
        0xF7
    };

    const uint8_t customReqNoConnCheck[8] = {
        //custom request with non-existing custom ID
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        CUSTOM_REQUEST_ID_NO_CONN_CHECK,
        0xF7
    };

    const uint8_t shortMessage1[6] = {
        //short message which should be ignored by the protocol, variant 1
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        0xF7
    };

    const uint8_t shortMessage2[4] = {
        //short message which should be ignored by the protocol, variant 2
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        0xF7
    };

    const uint8_t shortMessage3[10] = {
        //short message on which protocol should throw MESSAGE_LENGTH error
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::wish_t::get),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        TEST_BLOCK_ID,
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

    const uint8_t getSingleInvalidSysExID[12] = {
        //get single command with invalid sysex ids
        0xF0,
        SYS_EX_CONF_M_ID_2,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_0,
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

    const uint8_t getAllValid_allParts_7F[11] = {
        //valid get all command for all parts (7F variant)
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        0x7F,
        static_cast<uint8_t>(SysExConf::wish_t::get),
        static_cast<uint8_t>(SysExConf::amount_t::all),
        TEST_BLOCK_ID,
        TEST_SECTION_MULTIPLE_PARTS_ID,
        0xF7
    };

    const uint8_t getAllValid_allParts_7E[11] = {
        //valid get all command for all parts (7E variant)
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        0x7E,
        static_cast<uint8_t>(SysExConf::wish_t::get),
        static_cast<uint8_t>(SysExConf::amount_t::all),
        TEST_BLOCK_ID,
        TEST_SECTION_MULTIPLE_PARTS_ID,
        0xF7
    };

    const uint8_t getSpecialReqBytesPerVal[8] = {
        //built-in special request which returns number of bytes per value configured in protocol
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        0x00,
        0x00,
        0x02,
        0xF7
    };

    const uint8_t getSpecialReqParamPerMsg[8] = {
        //built-in special request which returns number of parameters per message configured in protocol
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        0x00,
        0x00,
        0x03,
        0xF7
    };

    const uint8_t setSingleValid[13] = {
        //valid set singe command
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::wish_t::set),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        TEST_BLOCK_ID,
        TEST_SECTION_SINGLE_PART_ID,
        TEST_INDEX_ID,
        TEST_NEW_VALUE_VALID,
        0xF7
    };

    const uint8_t setSingleInvalidNewValue[13] = {
        //set single command - invalid new value
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::wish_t::set),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        TEST_BLOCK_ID,
        TEST_SECTION_SINGLE_PART_ID,
        TEST_INDEX_ID,
        TEST_NEW_VALUE_INVALID,
        0xF7
    };

    const uint8_t setAllValid[21] = {
        //valid set all command
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        0x00,
        static_cast<uint8_t>(SysExConf::wish_t::set),
        static_cast<uint8_t>(SysExConf::amount_t::all),
        TEST_BLOCK_ID,
        TEST_SECTION_SINGLE_PART_ID,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0xF7
    };

    const uint8_t setAllAllParts[21] = {
        //set all command with all parts modifier (invalid request)
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        0x7F,
        static_cast<uint8_t>(SysExConf::wish_t::set),
        static_cast<uint8_t>(SysExConf::amount_t::all),
        TEST_BLOCK_ID,
        TEST_SECTION_SINGLE_PART_ID,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0xF7
    };

    const uint8_t setSingleNoMinMax1[13] = {
        //valid set single command for section without min/max checking, variant 1
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::wish_t::set),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        TEST_BLOCK_ID,
        TEST_SECTION_NOMINMAX,
        TEST_INDEX_ID,
        0,
        0xF7
    };

    const uint8_t setSingleNoMinMax2[13] = {
        //valid set single command for section without min/max checking, variant 2
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::wish_t::set),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        TEST_BLOCK_ID,
        TEST_SECTION_NOMINMAX,
        TEST_INDEX_ID,
        50,
        0xF7
    };

    const uint8_t setSingleNoMinMax3[13] = {
        //valid set single command for section without min/max checking, variant 3
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::wish_t::set),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        TEST_BLOCK_ID,
        TEST_SECTION_NOMINMAX,
        TEST_INDEX_ID,
        127,
        0xF7
    };

    const uint8_t setSingleInvalidParam[13] = {
        //set single command with invalid parameter index
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::wish_t::set),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        TEST_BLOCK_ID,
        TEST_SECTION_SINGLE_PART_ID,
        TEST_INVALID_PARAMETER_B0S0,
        0x00,
        0xF7
    };

    const uint8_t setAllnvalidNewVal[21] = {
        //set all command with invalid new value
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        TEST_MSG_PART_VALID,
        static_cast<uint8_t>(SysExConf::wish_t::set),
        static_cast<uint8_t>(SysExConf::amount_t::all),
        TEST_BLOCK_ID,
        TEST_SECTION_SINGLE_PART_ID,
        TEST_NEW_VALUE_INVALID,
        TEST_NEW_VALUE_INVALID,
        TEST_NEW_VALUE_INVALID,
        TEST_NEW_VALUE_INVALID,
        TEST_NEW_VALUE_INVALID,
        TEST_NEW_VALUE_INVALID,
        TEST_NEW_VALUE_INVALID,
        TEST_NEW_VALUE_INVALID,
        TEST_NEW_VALUE_INVALID,
        TEST_NEW_VALUE_INVALID,
        0xF7
    };

    const uint8_t setAllMoreParts1[43] = {
        //set all command for section with more parts, part 0
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        0x00,
        static_cast<uint8_t>(SysExConf::wish_t::set),
        static_cast<uint8_t>(SysExConf::amount_t::all),
        TEST_BLOCK_ID,
        TEST_SECTION_MULTIPLE_PARTS_ID,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0xF7
    };

    const uint8_t setAllMoreParts2[12] = {
        //set all command for section with more parts, part 1
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        0x01,
        static_cast<uint8_t>(SysExConf::wish_t::set),
        static_cast<uint8_t>(SysExConf::amount_t::all),
        TEST_BLOCK_ID,
        TEST_SECTION_MULTIPLE_PARTS_ID,
        0x01,
        0xF7
    };

    const uint8_t backupAll[11] = {
        //backup all command
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        0x7F,
        static_cast<uint8_t>(SysExConf::wish_t::backup),
        static_cast<uint8_t>(SysExConf::amount_t::all),
        TEST_BLOCK_ID,
        TEST_SECTION_SINGLE_PART_ID,
        0xF7
    };

    const uint8_t backupSingleInvPart[12] = {
        //backup single command with invalid part set
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        0x03,
        static_cast<uint8_t>(SysExConf::wish_t::backup),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        TEST_BLOCK_ID,
        TEST_SECTION_SINGLE_PART_ID,
        0x00,
        0xF7
    };

    SysExTestingValid sysEx(mId);
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

TEST_CASE(Init)
{
    //close connection
    uint8_t arraySize = sizeof(connClose) / sizeof(uint8_t);
    memcpy(sysEx.testArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //sysex configuration should be disabled now
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset number of received messages
    sysEx.responseCounter = 0;

    //test silent mode
    arraySize = sizeof(connOpenSilent) / sizeof(uint8_t);
    memcpy(sysEx.testArray, connOpenSilent, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //configuration and silent mode must be enabled
    TEST_ASSERT(true == sysEx.isSilentModeEnabled());
    TEST_ASSERT(true == sysEx.isConfigurationEnabled());

    // check that nothing was received as response
    TEST_ASSERT(sysEx.responseCounter == 0);

    //now disable silent mode
    arraySize = sizeof(silentModeDisable) / sizeof(uint8_t);
    memcpy(sysEx.testArray, silentModeDisable, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //silent mode should be disabled, but connection should be still opened
    //response should be received
    TEST_ASSERT(false == sysEx.isSilentModeEnabled());
    TEST_ASSERT(true == sysEx.isConfigurationEnabled());

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset number of received messages
    sysEx.responseCounter = 0;

    //open silent mode again
    arraySize = sizeof(connOpenSilent) / sizeof(uint8_t);
    memcpy(sysEx.testArray, connOpenSilent, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //verify no response was received
    TEST_ASSERT(sysEx.responseCounter == 0);

    //now close connection
    arraySize = sizeof(connClose) / sizeof(uint8_t);
    memcpy(sysEx.testArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //verify that connection is closed
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //silent mode should be disabled as well as an result of closed connection
    TEST_ASSERT(false == sysEx.isSilentModeEnabled());

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //enable silent mode using direct function call
    sysEx.setSilentMode(true);

    //verify silent mode is enabled
    TEST_ASSERT(true == sysEx.isSilentModeEnabled());

    //disable silent mode using direct function call
    sysEx.setSilentMode(false);

    //verify silent mode is disabled
    TEST_ASSERT(false == sysEx.isSilentModeEnabled());
}

TEST_CASE(SilentMode)
{
    //open silent mode
    uint8_t arraySize = sizeof(connOpenSilent) / sizeof(uint8_t);
    memcpy(sysEx.testArray, connOpenSilent, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //configuration and silent mode must be enabled
    TEST_ASSERT(true == sysEx.isSilentModeEnabled());
    TEST_ASSERT(true == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send set sigle request
    arraySize = sizeof(setSingleValid) / sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send set all request
    arraySize = sizeof(setAllValid) / sizeof(uint8_t);
    memcpy(sysEx.testArray, setAllValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send request which causes status error
    arraySize = sizeof(errorStatus) / sizeof(uint8_t);
    memcpy(sysEx.testArray, errorStatus, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send request which causes wish error
    arraySize = sizeof(errorWish) / sizeof(uint8_t);
    memcpy(sysEx.testArray, errorWish, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send request which causes amount error
    arraySize = sizeof(errorAmount) / sizeof(uint8_t);
    memcpy(sysEx.testArray, errorAmount, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send request which causes block error
    arraySize = sizeof(errorBlock) / sizeof(uint8_t);
    memcpy(sysEx.testArray, errorBlock, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send request which causes section error
    arraySize = sizeof(errorSection) / sizeof(uint8_t);
    memcpy(sysEx.testArray, errorSection, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send request which causes index error
    arraySize = sizeof(errorIndex) / sizeof(uint8_t);
    memcpy(sysEx.testArray, errorIndex, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send request which causes part error
    arraySize = sizeof(errorPart) / sizeof(uint8_t);
    memcpy(sysEx.testArray, errorPart, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send request which causes length error
    arraySize = sizeof(errorLength) / sizeof(uint8_t);
    memcpy(sysEx.testArray, errorLength, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send custom request
    arraySize = sizeof(customReq) / sizeof(uint8_t);
    memcpy(sysEx.testArray, customReq, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);
}

TEST_CASE(ErrorInit)
{
    //try to init sysex with null pointer
    TEST_ASSERT(false == sysEx.setLayout(NULL, 1));

    //try to init sysex with zero blocks
    TEST_ASSERT(false == sysEx.setLayout(sysExLayout, 0));
}

TEST_CASE(ErrorConnClosed)
{
    uint8_t arraySize;

    //close connection first
    arraySize = sizeof(connClose) / sizeof(uint8_t);
    memcpy(sysEx.testArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //configuration should be closed now
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset number of received messages
    sysEx.responseCounter = 0;

    //send valid get message
    //since connection is closed, SysExConf::status_t::errorConnection should be reported
    arraySize = sizeof(getSingleValid) / sizeof(uint8_t);
    memcpy(sysEx.testArray, getSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorConnection) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(ErrorStatus)
{
    //send message with invalid status byte
    //SysExConf::status_t::errorStatus should be reported
    uint8_t arraySize = sizeof(errorStatus) / sizeof(uint8_t);
    memcpy(sysEx.testArray, errorStatus, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorStatus) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(ErrorWish)
{
    //send message with invalid wish byte
    uint8_t arraySize = sizeof(errorWish) / sizeof(uint8_t);
    memcpy(sysEx.testArray, errorWish, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorWish) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(ErrorAmount)
{
    //send message with invalid amount byte
    //SysExConf::status_t::errorAmount should be reported
    uint8_t arraySize = sizeof(errorAmount) / sizeof(uint8_t);
    memcpy(sysEx.testArray, errorAmount, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorAmount) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(ErrorBlock)
{
    //send message with invalid block byte
    //SysExConf::status_t::errorBlock should be reported
    uint8_t arraySize = sizeof(errorBlock) / sizeof(uint8_t);
    memcpy(sysEx.testArray, errorBlock, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorBlock) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(ErrorSection)
{
    //send message with invalid section byte
    //SysExConf::status_t::errorSection should be reported
    uint8_t arraySize = sizeof(errorSection) / sizeof(uint8_t);
    memcpy(sysEx.testArray, errorSection, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorSection) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(ErrorPart)
{
    //send message with invalid index byte
    //SysExConf::status_t::errorPart should be reported
    uint8_t arraySize = sizeof(errorPart) / sizeof(uint8_t);
    memcpy(sysEx.testArray, errorPart, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorPart) == sysEx.testArray[4]);
    TEST_ASSERT(TEST_MSG_PART_INVALID == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(ErrorIndex)
{
    //send message with invalid index byte
    //SysExConf::status_t::errorIndex should be reported
    uint8_t arraySize = sizeof(errorIndex) / sizeof(uint8_t);
    memcpy(sysEx.testArray, errorIndex, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorIndex) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(ErrorLength)
{
    //send message with invalid index byte
    //SysExConf::status_t::errorMessageLength should be reported
    uint8_t arraySize = sizeof(errorLength) / sizeof(uint8_t);
    memcpy(sysEx.testArray, errorLength, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorMessageLength) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(ErrorNewValue)
{
    //send invalid set message
    //SysExConf::status_t::errorNewValue should be reported
    uint8_t arraySize = sizeof(setSingleInvalidNewValue) / sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleInvalidNewValue, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorNewValue) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(ErrorUser)
{
    sysEx.userError = SysExConf::status_t::errorNotSupported;

    //send get request
    //SysExConf::status_t::errorNotSupported should be returned since that value is assigned to userError
    uint8_t arraySize = sizeof(getSingleValid) / sizeof(uint8_t);
    memcpy(sysEx.testArray, getSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorNotSupported) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //get all request
    arraySize = sizeof(getAllValid_1part) / sizeof(uint8_t);
    memcpy(sysEx.testArray, getAllValid_1part, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorNotSupported) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //set single request
    arraySize = sizeof(setSingleValid) / sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorNotSupported) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //set all request
    arraySize = sizeof(setAllValid) / sizeof(uint8_t);
    memcpy(sysEx.testArray, setAllValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorNotSupported) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //try to set invalid user error
    //response should be just SysExConf::status_t::errorWrite in this case
    sysEx.userError = SysExConf::status_t::invalid;

    arraySize = sizeof(setSingleValid) / sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorWrite) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(SetSingle)
{
    //send valid set message
    uint8_t arraySize = sizeof(setSingleValid) / sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    // check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send set single command with invalid param index
    arraySize = sizeof(setSingleInvalidParam) / sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleInvalidParam, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorIndex) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //test block which has same min and max value
    //in this case, SysExConf::status_t::errorNewValue should never be reported on any value
    arraySize = sizeof(setSingleNoMinMax1) / sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleNoMinMax1, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    arraySize = sizeof(setSingleNoMinMax2) / sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleNoMinMax2, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    arraySize = sizeof(setSingleNoMinMax3) / sizeof(uint8_t);
    memcpy(sysEx.testArray, setSingleNoMinMax3, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;
}

TEST_CASE(SetAll)
{
    //send set all request
    uint8_t arraySize = sizeof(setAllValid) / sizeof(uint8_t);
    memcpy(sysEx.testArray, setAllValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send set all message for section with more parts
    arraySize = sizeof(setAllMoreParts1) / sizeof(uint8_t);
    memcpy(sysEx.testArray, setAllMoreParts1, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send set all request with part byte being 0x01
    arraySize = sizeof(setAllMoreParts2) / sizeof(uint8_t);
    memcpy(sysEx.testArray, setAllMoreParts2, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x01 == sysEx.testArray[5]);    //second part
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send set all requests for all parts and verify that status byte is set to SysExConf::status_t::errorPart
    arraySize = sizeof(setAllAllParts) / sizeof(uint8_t);
    memcpy(sysEx.testArray, setAllAllParts, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorPart) == sysEx.testArray[4]);
    TEST_ASSERT(0x7F == sysEx.testArray[5]);    //same part as requested
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send set all request with invalid value
    //status byte should be SysExConf::status_t::errorNewValue
    arraySize = sizeof(setAllnvalidNewVal) / sizeof(uint8_t);
    memcpy(sysEx.testArray, setAllnvalidNewVal, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorNewValue) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;
}

TEST_CASE(GetSingle)
{
    //send get single request
    uint8_t arraySize = sizeof(getSingleValid) / sizeof(uint8_t);
    memcpy(sysEx.testArray, getSingleValid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(TEST_VALUE_GET == sysEx.testArray[6]);
    TEST_ASSERT(0xF7 == sysEx.testArray[7]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(GetAll)
{
    //send get all request
    uint8_t arraySize = sizeof(getAllValid_1part) / sizeof(uint8_t);
    memcpy(sysEx.testArray, getAllValid_1part, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    //10 values should be received
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(TEST_VALUE_GET == sysEx.testArray[6]);
    TEST_ASSERT(TEST_VALUE_GET == sysEx.testArray[7]);
    TEST_ASSERT(TEST_VALUE_GET == sysEx.testArray[8]);
    TEST_ASSERT(TEST_VALUE_GET == sysEx.testArray[9]);
    TEST_ASSERT(TEST_VALUE_GET == sysEx.testArray[10]);
    TEST_ASSERT(TEST_VALUE_GET == sysEx.testArray[11]);
    TEST_ASSERT(TEST_VALUE_GET == sysEx.testArray[12]);
    TEST_ASSERT(TEST_VALUE_GET == sysEx.testArray[13]);
    TEST_ASSERT(TEST_VALUE_GET == sysEx.testArray[14]);
    TEST_ASSERT(TEST_VALUE_GET == sysEx.testArray[15]);
    TEST_ASSERT(0xF7 == sysEx.testArray[16]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //now send same request for all parts
    //we are expecting 2 messages now
    arraySize = sizeof(getAllValid_allParts_7F) / sizeof(uint8_t);
    memcpy(sysEx.testArray, getAllValid_allParts_7F, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 2);

    //reset message count
    sysEx.responseCounter = 0;

    //same message with part being 0x7E
    //in this case, last message should be SysExConf::status_t::ack message
    arraySize = sizeof(getAllValid_allParts_7E) / sizeof(uint8_t);
    memcpy(sysEx.testArray, getAllValid_allParts_7E, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check last response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x7E == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 3);
}

TEST_CASE(CustomReq)
{
    //reset message count
    sysEx.responseCounter = 0;

    //send valid custom request message
    uint8_t arraySize = sizeof(customReq) / sizeof(uint8_t);
    memcpy(sysEx.testArray, customReq, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(CUSTOM_REQUEST_VALUE == sysEx.testArray[6]);
    TEST_ASSERT(0xF7 == sysEx.testArray[7]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send custom request message which should return false in custom request handler
    //in this case, SysExConf::status_t::errorRead should be reported
    arraySize = sizeof(customReqErrorRead) / sizeof(uint8_t);
    memcpy(sysEx.testArray, customReqErrorRead, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorRead) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send non-existing custom request message
    //SysExConf::status_t::errorWish should be reported
    arraySize = sizeof(customReqInvalid) / sizeof(uint8_t);
    memcpy(sysEx.testArray, customReqInvalid, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorWish) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //disable configuration
    arraySize = sizeof(connClose) / sizeof(uint8_t);
    memcpy(sysEx.testArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //verify that connection is closed
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send valid custom request message
    //SysExConf::status_t::errorConnection should be reported
    arraySize = sizeof(customReq) / sizeof(uint8_t);
    memcpy(sysEx.testArray, customReq, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorConnection) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //try defining custom requests with invalid pointer
    TEST_ASSERT(sysEx.setupCustomRequests(NULL, 0) == false);

    //try defining illegal custom requests
    SysExConf::customRequest_t customRequests_invalid[static_cast<uint8_t>(SysExConf::specialRequest_t::AMOUNT)] = {
        { .requestID = 0,
          .connOpenCheck = true },

        { .requestID = 1,
          .connOpenCheck = true },

        { .requestID = 2,
          .connOpenCheck = true },

        { .requestID = 3,
          .connOpenCheck = true },

        { .requestID = 4,
          .connOpenCheck = true },

        { .requestID = 5,
          .connOpenCheck = true }
    };

    //setupCustomRequests should return false because special requests which
    //are already used internally are defined in pointed structure
    TEST_ASSERT(sysEx.setupCustomRequests(customRequests_invalid, static_cast<uint8_t>(SysExConf::specialRequest_t::AMOUNT)) == false);

    //restore valid custom requests
    TEST_ASSERT(sysEx.setupCustomRequests(customRequests, TOTAL_CUSTOM_REQUESTS) == true);

    //close sysex connection
    arraySize = sizeof(connClose) / sizeof(uint8_t);
    memcpy(sysEx.testArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //sysex configuration should be disabled now
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send custom request 0
    //SysExConf::status_t::errorConnection should be returned because connection is closed
    arraySize = sizeof(customReq) / sizeof(uint8_t);
    memcpy(sysEx.testArray, customReq, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorConnection) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send another custom request which has flag set to ignore connection status
    //SysExConf::status_t::ack should be reported
    arraySize = sizeof(customReqNoConnCheck) / sizeof(uint8_t);
    memcpy(sysEx.testArray, customReqNoConnCheck, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(CUSTOM_REQUEST_VALUE == sysEx.testArray[6]);
    TEST_ASSERT(0xF7 == sysEx.testArray[7]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //open connection again
    arraySize = sizeof(connOpen) / sizeof(uint8_t);
    memcpy(sysEx.testArray, connOpen, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //verify that connection is opened
    TEST_ASSERT(true == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(IgnoreMessage)
{
    //verify that no action takes place when sysex ids in message don't match
    //short message is any message without every required byte
    uint8_t arraySize = sizeof(shortMessage1) / sizeof(uint8_t);
    memcpy(sysEx.testArray, shortMessage1, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //if no action took place, responseCounter should be 0
    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send another variant of short message
    arraySize = sizeof(shortMessage2) / sizeof(uint8_t);
    memcpy(sysEx.testArray, shortMessage2, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send message with invalid SysEx ID
    arraySize = sizeof(getSingleInvalidSysExID) / sizeof(uint8_t);
    memcpy(sysEx.testArray, getSingleInvalidSysExID, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //short message where SysExConf::status_t::errorMessageLength should be returned
    arraySize = sizeof(shortMessage3) / sizeof(uint8_t);
    memcpy(sysEx.testArray, shortMessage3, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorMessageLength) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(CustomMessage)
{
    //construct custom message and see if output matches
    SysExConf::sysExParameter_t values[] = {
        0x05,
        0x06,
        0x07
    };

    sysEx.sendCustomMessage(sysEx.testArray, values, 3);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0x05 == sysEx.testArray[6]);
    TEST_ASSERT(0x06 == sysEx.testArray[7]);
    TEST_ASSERT(0x07 == sysEx.testArray[8]);
    TEST_ASSERT(0xF7 == sysEx.testArray[9]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //construct same message again with SysExConf::status_t::request as status byte
    sysEx.sendCustomMessage(sysEx.testArray, values, 3, false);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::request) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0x05 == sysEx.testArray[6]);
    TEST_ASSERT(0x06 == sysEx.testArray[7]);
    TEST_ASSERT(0x07 == sysEx.testArray[8]);
    TEST_ASSERT(0xF7 == sysEx.testArray[9]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(Backup)
{
    //send backup all request
    uint8_t arraySize = sizeof(backupAll) / sizeof(uint8_t);
    memcpy(sysEx.testArray, backupAll, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check if status byte is set to SysExConf::status_t::request value
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::request) == sysEx.testArray[4]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send backup/single request with incorrect part
    arraySize = sizeof(backupSingleInvPart) / sizeof(uint8_t);
    memcpy(sysEx.testArray, backupSingleInvPart, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorPart) == sysEx.testArray[4]);
    TEST_ASSERT(0x03 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(SpecialRequest)
{
    //test all pre-configured special requests and see if they return correct value

    //bytes per value request
    uint8_t arraySize = sizeof(getSpecialReqBytesPerVal) / sizeof(uint8_t);
    memcpy(sysEx.testArray, getSpecialReqBytesPerVal, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(SYS_EX_CONF_PARAM_SIZE == sysEx.testArray[6]);
    TEST_ASSERT(0xF7 == sysEx.testArray[7]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //params per msg request
    arraySize = sizeof(getSpecialReqParamPerMsg) / sizeof(uint8_t);
    memcpy(sysEx.testArray, getSpecialReqParamPerMsg, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(SYS_EX_CONF_PARAMETERS_PER_MESSAGE == sysEx.testArray[6]);
    TEST_ASSERT(0xF7 == sysEx.testArray[7]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //now try those same requests, but without prior open connection request
    //status byte must equal SysExConf::status_t::errorConnection

    //close connection first
    arraySize = sizeof(connClose) / sizeof(uint8_t);
    memcpy(sysEx.testArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //configuration should be closed now
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //bytes per value request
    arraySize = sizeof(getSpecialReqBytesPerVal) / sizeof(uint8_t);
    memcpy(sysEx.testArray, getSpecialReqBytesPerVal, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorConnection) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //params per msg request
    arraySize = sizeof(getSpecialReqParamPerMsg) / sizeof(uint8_t);
    memcpy(sysEx.testArray, getSpecialReqParamPerMsg, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorConnection) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //try to close configuration which is already closed
    arraySize = sizeof(connClose) / sizeof(uint8_t);
    memcpy(sysEx.testArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorConnection) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send open connection request and check if sysEx.testArray is valid
    arraySize = sizeof(connOpen) / sizeof(uint8_t);
    memcpy(sysEx.testArray, connOpen, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //sysex configuration should be enabled now
    TEST_ASSERT(1 == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //disable configuration again
    arraySize = sizeof(connClose) / sizeof(uint8_t);
    memcpy(sysEx.testArray, connClose, arraySize);
    sysEx.handleMessage((uint8_t*)sysEx.testArray, arraySize);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(0x00 == sysEx.testArray[1]);
    TEST_ASSERT(0x53 == sysEx.testArray[2]);
    TEST_ASSERT(0x43 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;
}

TEST_CASE(AddToReponseFail)
{
    // try to add bytes to sysex response without first specifying array source
    TEST_ASSERT(false == sysEx.addToResponse(0x50));
}