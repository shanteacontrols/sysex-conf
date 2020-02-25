#include "unity/src/unity.h"
#include "unity/Helpers.h"
#include "SysExConf.h"
#include "src/SysExTesting.h"
#include <vector>

#define SYS_EX_CONF_M_ID_0 0x00
#define SYS_EX_CONF_M_ID_1 0x53
#define SYS_EX_CONF_M_ID_2 0x43

#define HANDLE_MESSAGE(source)                          \
    do                                                  \
    {                                                   \
        sysEx.handleMessage(&source[0], source.size()); \
    } while (0)

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
        {
            .numberOfSections = NUMBER_OF_SECTIONS,
            .section          = testSections,
        }
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

    const std::vector<uint8_t> connOpen = {
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

    const std::vector<uint8_t> connClose = {
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

    const std::vector<uint8_t> connOpenSilent = {
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

    const std::vector<uint8_t> silentModeDisable = {
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

    const std::vector<uint8_t> errorStatus = {
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
        0x00,
        TEST_INDEX_ID,
        0xF7
    };

    const std::vector<uint8_t> errorWish = {
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
        0x00,
        TEST_INDEX_ID,
        0xF7
    };

    const std::vector<uint8_t> errorLength = {
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
        0x00,
        TEST_INDEX_ID,
        0x00,
        TEST_INDEX_ID,
        0xF7
    };

    const std::vector<uint8_t> errorBlock = {
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
        0x00,
        TEST_INDEX_ID,
        0xF7
    };

    const std::vector<uint8_t> errorSection = {
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
        0x00,
        TEST_INDEX_ID,
        0xF7
    };

    const std::vector<uint8_t> errorIndex = {
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
        0x00,
        0x7F,
        0xF7
    };

    const std::vector<uint8_t> errorPart = {
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
        0x00,
        TEST_INDEX_ID,
        0xF7
    };

    const std::vector<uint8_t> errorAmount = {
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
        0x00,
        TEST_INDEX_ID,
        0x00,
        0xF7
    };

    const std::vector<uint8_t> customReq = {
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

    const std::vector<uint8_t> customReqErrorRead = {
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

    const std::vector<uint8_t> customReqInvalid = {
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

    const std::vector<uint8_t> customReqNoConnCheck = {
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

    const std::vector<uint8_t> shortMessage1 = {
        //short message which should be ignored by the protocol, variant 1
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        0xF7
    };

    const std::vector<uint8_t> shortMessage2 = {
        //short message which should be ignored by the protocol, variant 2
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        0xF7
    };

    const std::vector<uint8_t> shortMessage3 = {
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

    const std::vector<uint8_t> getSingleValid = {
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
        0x00,
        TEST_INDEX_ID,
        0xF7
    };

    const std::vector<uint8_t> getSingleInvalidSysExID = {
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
        0x00,
        TEST_INDEX_ID,
        0xF7
    };

    const std::vector<uint8_t> getAllValid_1part = {
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

    const std::vector<uint8_t> getAllValid_allParts_7F = {
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

    const std::vector<uint8_t> getAllValid_allParts_7E = {
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

    const std::vector<uint8_t> getSpecialReqBytesPerVal = {
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

    const std::vector<uint8_t> getSpecialReqParamPerMsg = {
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

    const std::vector<uint8_t> setSingleValid = {
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
        0x00,
        TEST_INDEX_ID,
        0x00,
        TEST_NEW_VALUE_VALID,
        0xF7
    };

    const std::vector<uint8_t> setSingleInvalidNewValue = {
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
        0x00,
        TEST_INDEX_ID,
        0x00,
        TEST_NEW_VALUE_INVALID,
        0xF7
    };

    const std::vector<uint8_t> setAllValid = {
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
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0xF7
    };

    const std::vector<uint8_t> setAllAllParts = {
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
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0xF7
    };

    const std::vector<uint8_t> setSingleNoMinMax1 = {
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
        0x00,
        TEST_INDEX_ID,
        0x00,
        0x00,
        0xF7
    };

    const std::vector<uint8_t> setSingleNoMinMax2 = {
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
        0x00,
        TEST_INDEX_ID,
        0x00,
        50,
        0xF7
    };

    const std::vector<uint8_t> setSingleNoMinMax3 = {
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
        0x00,
        TEST_INDEX_ID,
        0x00,
        127,
        0xF7
    };

    const std::vector<uint8_t> setSingleInvalidParam = {
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
        0x00,
        TEST_INVALID_PARAMETER_B0S0,
        0x00,
        0x00,
        0xF7
    };

    const std::vector<uint8_t> setAllnvalidNewVal = {
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
        0x00,
        TEST_NEW_VALUE_INVALID,
        0x00,
        TEST_NEW_VALUE_INVALID,
        0x00,
        TEST_NEW_VALUE_INVALID,
        0x00,
        TEST_NEW_VALUE_INVALID,
        0x00,
        TEST_NEW_VALUE_INVALID,
        0x00,
        TEST_NEW_VALUE_INVALID,
        0x00,
        TEST_NEW_VALUE_INVALID,
        0x00,
        TEST_NEW_VALUE_INVALID,
        0x00,
        TEST_NEW_VALUE_INVALID,
        0x00,
        TEST_NEW_VALUE_INVALID,
        0xF7
    };

    const std::vector<uint8_t> setAllMoreParts1 = {
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
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x01,
        0xF7
    };

    const std::vector<uint8_t> setAllMoreParts2 = {
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
        0x00,
        0x01,
        0xF7
    };

    const std::vector<uint8_t> backupAll = {
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

    const std::vector<uint8_t> backupSingleInvPart = {
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

    class SysExConfDataHandlerValid : public SysExConf::DataHandler
    {
        public:
        SysExConfDataHandlerValid()
        {}

        result_t get(uint8_t block, uint8_t section, size_t index, SysExConf::sysExParameter_t& value) override
        {
            value = TEST_VALUE_GET;
            return SysExConf::DataHandler::result_t::ok;
        }

        result_t set(uint8_t block, uint8_t section, size_t index, SysExConf::sysExParameter_t newValue) override
        {
            return SysExConf::DataHandler::result_t::ok;
        }

        result_t customRequest(size_t value, uint8_t*& array, size_t& size) override
        {
            switch (value)
            {
            case CUSTOM_REQUEST_ID_VALID:
            case CUSTOM_REQUEST_ID_NO_CONN_CHECK:
                array = (uint8_t*)&customReqArray[0];
                size  = customReqArray.size();
                return SysExConf::DataHandler::result_t::ok;
                break;

            case CUSTOM_REQUEST_ID_ERROR_READ:
            default:
                return SysExConf::DataHandler::result_t::error;
                break;
            }
        }

        void reset()
        {
            responseCounter = 0;
        }

        void sendResponse(uint8_t* array, size_t size) override
        {
            for (int i = 0; i < size; i++)
                sysExArray[i] = array[i];

            responseCounter++;
        }

        uint8_t sysExArray[200] = {};
        uint8_t responseCounter = 0;

        private:
        std::vector<uint8_t> customReqArray = {
            1
        };
    };

    SysExConfDataHandlerValid dataHandler;

    SysExConf sysEx(dataHandler,
                    mId,
                    dataHandler.sysExArray,
                    200,
                    SysExConf::paramSize_t::_14bit,
                    SysExConf::nrOfParam_t::_32);
}    // namespace

TEST_SETUP()
{
    sysEx.reset();
    sysEx.setLayout(sysExLayout, NUMBER_OF_BLOCKS);
    sysEx.setupCustomRequests(customRequests, TOTAL_CUSTOM_REQUESTS);

    //send open connection request and see if sysExTestArray is valid
    HANDLE_MESSAGE(connOpen);

    //sysex configuration should be enabled now
    TEST_ASSERT(1 == sysEx.isConfigurationEnabled());

    dataHandler.responseCounter = 0;
}

TEST_CASE(Init)
{
    //close connection
    HANDLE_MESSAGE(connClose);

    //sysex configuration should be disabled now
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset number of received messages
    dataHandler.responseCounter = 0;

    //test silent mode
    HANDLE_MESSAGE(connOpenSilent);

    //configuration and silent mode must be enabled
    TEST_ASSERT(true == sysEx.isSilentModeEnabled());
    TEST_ASSERT(true == sysEx.isConfigurationEnabled());

    // check that nothing was received as response
    TEST_ASSERT(dataHandler.responseCounter == 0);

    //now disable silent mode
    HANDLE_MESSAGE(silentModeDisable);

    //silent mode should be disabled, but connection should be still opened
    //response should be received
    TEST_ASSERT(false == sysEx.isSilentModeEnabled());
    TEST_ASSERT(true == sysEx.isConfigurationEnabled());

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset number of received messages
    dataHandler.responseCounter = 0;

    //open silent mode again
    HANDLE_MESSAGE(connOpenSilent);

    //verify no response was received
    TEST_ASSERT(dataHandler.responseCounter == 0);

    //now close connection
    HANDLE_MESSAGE(connClose);

    //verify that connection is closed
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //silent mode should be disabled as well as an result of closed connection
    TEST_ASSERT(false == sysEx.isSilentModeEnabled());

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

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
    HANDLE_MESSAGE(connOpenSilent);

    //configuration and silent mode must be enabled
    TEST_ASSERT(true == sysEx.isSilentModeEnabled());
    TEST_ASSERT(true == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 0);

    //send set sigle request
    HANDLE_MESSAGE(setSingleValid);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 0);

    //send set all request
    HANDLE_MESSAGE(setAllValid);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 0);

    //send request which causes status error
    HANDLE_MESSAGE(errorStatus);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 0);

    //send request which causes wish error
    HANDLE_MESSAGE(errorWish);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 0);

    //send request which causes amount error
    HANDLE_MESSAGE(errorAmount);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 0);

    //send request which causes block error
    HANDLE_MESSAGE(errorBlock);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 0);

    //send request which causes section error
    HANDLE_MESSAGE(errorSection);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 0);

    //send request which causes index error
    HANDLE_MESSAGE(errorIndex);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 0);

    //send request which causes part error
    HANDLE_MESSAGE(errorPart);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 0);

    //send request which causes length error
    HANDLE_MESSAGE(errorLength);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 0);

    //send custom request
    HANDLE_MESSAGE(customReq);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 0);
}

TEST_CASE(ErrorInit)
{
    //try to init sysex with null pointer
    TEST_ASSERT(false == sysEx.setLayout(nullptr, 1));

    //try to init sysex with zero blocks
    TEST_ASSERT(false == sysEx.setLayout(sysExLayout, 0));
}

TEST_CASE(ErrorConnClosed)
{
    //close connection first
    HANDLE_MESSAGE(connClose);

    //configuration should be closed now
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset number of received messages
    dataHandler.responseCounter = 0;

    //send valid get message
    //since connection is closed, SysExConf::status_t::errorConnection should be reported
    HANDLE_MESSAGE(getSingleValid);

    //test sysex array
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorConnection) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}

TEST_CASE(ErrorStatus)
{
    //send message with invalid status byte
    //SysExConf::status_t::errorStatus should be reported
    HANDLE_MESSAGE(errorStatus);

    //test sysex array
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorStatus) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}

TEST_CASE(ErrorWish)
{
    //send message with invalid wish byte
    HANDLE_MESSAGE(errorWish);

    //test sysex array
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorWish) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}

TEST_CASE(ErrorAmount)
{
    //send message with invalid amount byte
    //SysExConf::status_t::errorAmount should be reported
    HANDLE_MESSAGE(errorAmount);

    //test sysex array
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorAmount) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}

TEST_CASE(ErrorBlock)
{
    //send message with invalid block byte
    //SysExConf::status_t::errorBlock should be reported
    HANDLE_MESSAGE(errorBlock);

    //test sysex array
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorBlock) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}

TEST_CASE(ErrorSection)
{
    //send message with invalid section byte
    //SysExConf::status_t::errorSection should be reported
    HANDLE_MESSAGE(errorSection);

    //test sysex array
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorSection) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}

TEST_CASE(ErrorPart)
{
    //send message with invalid index byte
    //SysExConf::status_t::errorPart should be reported
    HANDLE_MESSAGE(errorPart);

    //test sysex array
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorPart) == dataHandler.sysExArray[4]);
    TEST_ASSERT(TEST_MSG_PART_INVALID == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}

TEST_CASE(ErrorIndex)
{
    //send message with invalid index byte
    //SysExConf::status_t::errorIndex should be reported
    HANDLE_MESSAGE(errorIndex);

    //test sysex array
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorIndex) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}

TEST_CASE(ErrorLength)
{
    //send message with invalid index byte
    //SysExConf::status_t::errorMessageLength should be reported
    HANDLE_MESSAGE(errorLength);

    //test sysex array
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorMessageLength) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}

TEST_CASE(ErrorNewValue)
{
    //send invalid set message
    //SysExConf::status_t::errorNewValue should be reported
    HANDLE_MESSAGE(setSingleInvalidNewValue);

    //test sysex array
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorNewValue) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}

TEST_CASE(SetSingle)
{
    //send valid set message
    HANDLE_MESSAGE(setSingleValid);

    // check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //send set single command with invalid param index
    HANDLE_MESSAGE(setSingleInvalidParam);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorIndex) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //test block which has same min and max value
    //in this case, SysExConf::status_t::errorNewValue should never be reported on any value
    HANDLE_MESSAGE(setSingleNoMinMax1);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    HANDLE_MESSAGE(setSingleNoMinMax2);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    HANDLE_MESSAGE(setSingleNoMinMax3);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;
}

TEST_CASE(SetAll)
{
    //send set all request
    HANDLE_MESSAGE(setAllValid);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //send set all message for section with more parts
    HANDLE_MESSAGE(setAllMoreParts1);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    // check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //send set all request with part byte being 0x01
    HANDLE_MESSAGE(setAllMoreParts2);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x01 == dataHandler.sysExArray[5]);    //second part
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //send set all requests for all parts and verify that status byte is set to SysExConf::status_t::errorPart
    HANDLE_MESSAGE(setAllAllParts);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorPart) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x7F == dataHandler.sysExArray[5]);    //same part as requested
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //send set all request with invalid value
    //status byte should be SysExConf::status_t::errorNewValue
    HANDLE_MESSAGE(setAllnvalidNewVal);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorNewValue) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;
}

TEST_CASE(GetSingle)
{
    //send get single request
    HANDLE_MESSAGE(getSingleValid);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[6]);
    TEST_ASSERT(TEST_VALUE_GET == dataHandler.sysExArray[7]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[8]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}

TEST_CASE(GetAll)
{
    //send get all request
    HANDLE_MESSAGE(getAllValid_1part);

    //check response
    //10 values should be received
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);

    TEST_ASSERT(0x00 == dataHandler.sysExArray[6]);
    TEST_ASSERT(TEST_VALUE_GET == dataHandler.sysExArray[7]);

    TEST_ASSERT(0x00 == dataHandler.sysExArray[8]);
    TEST_ASSERT(TEST_VALUE_GET == dataHandler.sysExArray[9]);

    TEST_ASSERT(0x00 == dataHandler.sysExArray[10]);
    TEST_ASSERT(TEST_VALUE_GET == dataHandler.sysExArray[11]);

    TEST_ASSERT(0x00 == dataHandler.sysExArray[12]);
    TEST_ASSERT(TEST_VALUE_GET == dataHandler.sysExArray[13]);

    TEST_ASSERT(0x00 == dataHandler.sysExArray[14]);
    TEST_ASSERT(TEST_VALUE_GET == dataHandler.sysExArray[15]);

    TEST_ASSERT(0x00 == dataHandler.sysExArray[16]);
    TEST_ASSERT(TEST_VALUE_GET == dataHandler.sysExArray[17]);

    TEST_ASSERT(0x00 == dataHandler.sysExArray[18]);
    TEST_ASSERT(TEST_VALUE_GET == dataHandler.sysExArray[19]);

    TEST_ASSERT(0x00 == dataHandler.sysExArray[20]);
    TEST_ASSERT(TEST_VALUE_GET == dataHandler.sysExArray[21]);

    TEST_ASSERT(0x00 == dataHandler.sysExArray[22]);
    TEST_ASSERT(TEST_VALUE_GET == dataHandler.sysExArray[23]);

    TEST_ASSERT(0x00 == dataHandler.sysExArray[24]);
    TEST_ASSERT(TEST_VALUE_GET == dataHandler.sysExArray[25]);

    TEST_ASSERT(0xF7 == dataHandler.sysExArray[26]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //now send same request for all parts
    //we are expecting 2 messages now
    HANDLE_MESSAGE(getAllValid_allParts_7F);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 2);

    //reset message count
    dataHandler.responseCounter = 0;

    //same message with part being 0x7E
    //in this case, last message should be SysExConf::status_t::ack message
    HANDLE_MESSAGE(getAllValid_allParts_7E);

    // check last response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x7E == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 3);
}

TEST_CASE(CustomReq)
{
    //reset message count
    dataHandler.responseCounter = 0;

    //send valid custom request message
    HANDLE_MESSAGE(customReq);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);

    TEST_ASSERT(0x00 == dataHandler.sysExArray[6]);
    TEST_ASSERT(CUSTOM_REQUEST_VALUE == dataHandler.sysExArray[7]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[8]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //send custom request message which should return false in custom request handler
    //in this case, SysExConf::status_t::errorRead should be reported
    HANDLE_MESSAGE(customReqErrorRead);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorRead) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //send non-existing custom request message
    //SysExConf::status_t::errorWish should be reported
    HANDLE_MESSAGE(customReqInvalid);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorWish) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //disable configuration
    HANDLE_MESSAGE(connClose);

    //verify that connection is closed
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //send valid custom request message
    //SysExConf::status_t::errorConnection should be reported
    HANDLE_MESSAGE(customReq);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorConnection) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //try defining custom requests with invalid pointer
    TEST_ASSERT(sysEx.setupCustomRequests(nullptr, 0) == false);

    //try defining illegal custom requests
    SysExConf::customRequest_t customRequests_invalid[static_cast<uint8_t>(SysExConf::specialRequest_t::AMOUNT)] = {
        {
            .requestID     = 0,
            .connOpenCheck = true,
        },

        {
            .requestID     = 1,
            .connOpenCheck = true,
        },

        {
            .requestID     = 2,
            .connOpenCheck = true,
        },

        {
            .requestID     = 3,
            .connOpenCheck = true,
        },

        {
            .requestID     = 4,
            .connOpenCheck = true,
        },

        {
            .requestID     = 5,
            .connOpenCheck = true,
        }
    };

    //setupCustomRequests should return false because special requests which
    //are already used internally are defined in pointed structure
    TEST_ASSERT(sysEx.setupCustomRequests(customRequests_invalid, static_cast<uint8_t>(SysExConf::specialRequest_t::AMOUNT)) == false);

    //restore valid custom requests
    TEST_ASSERT(sysEx.setupCustomRequests(customRequests, TOTAL_CUSTOM_REQUESTS) == true);

    //close sysex connection
    HANDLE_MESSAGE(connClose);

    //sysex configuration should be disabled now
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //send custom request 0
    //SysExConf::status_t::errorConnection should be returned because connection is closed
    HANDLE_MESSAGE(customReq);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorConnection) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //send another custom request which has flag set to ignore connection status
    //SysExConf::status_t::ack should be reported
    HANDLE_MESSAGE(customReqNoConnCheck);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);

    TEST_ASSERT(0x00 == dataHandler.sysExArray[6]);
    TEST_ASSERT(CUSTOM_REQUEST_VALUE == dataHandler.sysExArray[7]);

    TEST_ASSERT(0xF7 == dataHandler.sysExArray[8]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //open connection again
    HANDLE_MESSAGE(connOpen);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //verify that connection is opened
    TEST_ASSERT(true == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}

TEST_CASE(IgnoreMessage)
{
    //verify that no action takes place when sysex ids in message don't match
    //short message is any message without every required byte
    HANDLE_MESSAGE(shortMessage1);

    //if no action took place, responseCounter should be 0
    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 0);

    //send another variant of short message
    HANDLE_MESSAGE(shortMessage2);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 0);

    //send message with invalid SysEx ID
    HANDLE_MESSAGE(getSingleInvalidSysExID);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 0);

    //short message where SysExConf::status_t::errorMessageLength should be returned
    HANDLE_MESSAGE(shortMessage3);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorMessageLength) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}

TEST_CASE(CustomMessage)
{
    //construct custom message and see if output matches
    std::vector<SysExConf::sysExParameter_t> values = {
        0x05,
        0x06,
        0x07
    };

    sysEx.sendCustomMessage(&values[0], values.size());

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0x05 == dataHandler.sysExArray[6]);
    TEST_ASSERT(0x06 == dataHandler.sysExArray[7]);
    TEST_ASSERT(0x07 == dataHandler.sysExArray[8]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[9]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //construct same message again with SysExConf::status_t::request as status byte
    sysEx.sendCustomMessage(&values[0], values.size(), false);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::request) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0x05 == dataHandler.sysExArray[6]);
    TEST_ASSERT(0x06 == dataHandler.sysExArray[7]);
    TEST_ASSERT(0x07 == dataHandler.sysExArray[8]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[9]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}

TEST_CASE(Backup)
{
    //send backup all request
    HANDLE_MESSAGE(backupAll);

    //check if status byte is set to SysExConf::status_t::request value
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::request) == dataHandler.sysExArray[4]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //send backup/single request with incorrect part
    HANDLE_MESSAGE(backupSingleInvPart);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorPart) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x03 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}

TEST_CASE(SpecialRequest)
{
    //test all pre-configured special requests and see if they return correct value

    //bytes per value request
    HANDLE_MESSAGE(getSpecialReqBytesPerVal);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::paramSize_t::_14bit) == dataHandler.sysExArray[6]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[7]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //params per msg request
    HANDLE_MESSAGE(getSpecialReqParamPerMsg);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::nrOfParam_t::_32) == dataHandler.sysExArray[6]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[7]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //now try those same requests, but without prior open connection request
    //status byte must equal SysExConf::status_t::errorConnection

    //close connection first
    HANDLE_MESSAGE(connClose);

    //configuration should be closed now
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //bytes per value request
    HANDLE_MESSAGE(getSpecialReqBytesPerVal);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorConnection) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //params per msg request
    HANDLE_MESSAGE(getSpecialReqParamPerMsg);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorConnection) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //try to close configuration which is already closed
    HANDLE_MESSAGE(connClose);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorConnection) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //send open connection request and check if dataHandler.sysExArray is valid
    HANDLE_MESSAGE(connOpen);

    //sysex configuration should be enabled now
    TEST_ASSERT(1 == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;

    //disable configuration again
    HANDLE_MESSAGE(connClose);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);

    //reset message count
    dataHandler.responseCounter = 0;
}