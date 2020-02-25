#include "unity/src/unity.h"
#include "unity/Helpers.h"
#include "src/SysExTesting.h"
#include <vector>

#define SYS_EX_CONF_M_ID_0 0x00
#define SYS_EX_CONF_M_ID_1 0x53
#define SYS_EX_CONF_M_ID_2 0x43

#define HANDLE_MESSAGE(source, dest)                                     \
    do                                                                   \
    {                                                                    \
        std::copy(source.begin(), source.end(), dest);                   \
        sysEx.handleMessage(static_cast<uint8_t*>(dest), source.size()); \
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
        TEST_INDEX_ID,
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
        TEST_INDEX_ID,
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
        TEST_INDEX_ID,
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
        TEST_INDEX_ID,
        0,
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
        TEST_INDEX_ID,
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
        TEST_INDEX_ID,
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
        TEST_INVALID_PARAMETER_B0S0,
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

    SysExTestingValid sysEx(mId, SysExConf::paramSize_t::_7bit, SysExConf::nrOfParam_t::_32);
}    // namespace

TEST_SETUP()
{
    sysEx.reset();
    sysEx.setLayout(sysExLayout, NUMBER_OF_BLOCKS);
    sysEx.setupCustomRequests(customRequests, TOTAL_CUSTOM_REQUESTS);

    //send open connection request and see if sysExTestArray is valid
    HANDLE_MESSAGE(connOpen, sysEx.testArray);

    //sysex configuration should be enabled now
    TEST_ASSERT(1 == sysEx.isConfigurationEnabled());

    sysEx.responseCounter = 0;
}

TEST_CASE(Init)
{
    //close connection
    HANDLE_MESSAGE(connClose, sysEx.testArray);

    //sysex configuration should be disabled now
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset number of received messages
    sysEx.responseCounter = 0;

    //test silent mode
    HANDLE_MESSAGE(connOpenSilent, sysEx.testArray);

    //configuration and silent mode must be enabled
    TEST_ASSERT(true == sysEx.isSilentModeEnabled());
    TEST_ASSERT(true == sysEx.isConfigurationEnabled());

    // check that nothing was received as response
    TEST_ASSERT(sysEx.responseCounter == 0);

    //now disable silent mode
    HANDLE_MESSAGE(silentModeDisable, sysEx.testArray);

    //silent mode should be disabled, but connection should be still opened
    //response should be received
    TEST_ASSERT(false == sysEx.isSilentModeEnabled());
    TEST_ASSERT(true == sysEx.isConfigurationEnabled());

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset number of received messages
    sysEx.responseCounter = 0;

    //open silent mode again
    HANDLE_MESSAGE(connOpenSilent, sysEx.testArray);

    //verify no response was received
    TEST_ASSERT(sysEx.responseCounter == 0);

    //now close connection
    HANDLE_MESSAGE(connClose, sysEx.testArray);

    //verify that connection is closed
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //silent mode should be disabled as well as an result of closed connection
    TEST_ASSERT(false == sysEx.isSilentModeEnabled());

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(connOpenSilent, sysEx.testArray);

    //configuration and silent mode must be enabled
    TEST_ASSERT(true == sysEx.isSilentModeEnabled());
    TEST_ASSERT(true == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send set sigle request
    HANDLE_MESSAGE(setSingleValid, sysEx.testArray);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send set all request
    HANDLE_MESSAGE(setAllValid, sysEx.testArray);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send request which causes status error
    HANDLE_MESSAGE(errorStatus, sysEx.testArray);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send request which causes wish error
    HANDLE_MESSAGE(errorWish, sysEx.testArray);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send request which causes amount error
    HANDLE_MESSAGE(errorAmount, sysEx.testArray);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send request which causes block error
    HANDLE_MESSAGE(errorBlock, sysEx.testArray);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send request which causes section error
    HANDLE_MESSAGE(errorSection, sysEx.testArray);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send request which causes index error
    HANDLE_MESSAGE(errorIndex, sysEx.testArray);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send request which causes part error
    HANDLE_MESSAGE(errorPart, sysEx.testArray);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send request which causes length error
    HANDLE_MESSAGE(errorLength, sysEx.testArray);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send custom request
    HANDLE_MESSAGE(customReq, sysEx.testArray);

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
    //close connection first
    HANDLE_MESSAGE(connClose, sysEx.testArray);

    //configuration should be closed now
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset number of received messages
    sysEx.responseCounter = 0;

    //send valid get message
    //since connection is closed, SysExConf::status_t::errorConnection should be reported
    HANDLE_MESSAGE(getSingleValid, sysEx.testArray);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(errorStatus, sysEx.testArray);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorStatus) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(ErrorWish)
{
    //send message with invalid wish byte
    HANDLE_MESSAGE(errorWish, sysEx.testArray);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(errorAmount, sysEx.testArray);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(errorBlock, sysEx.testArray);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(errorSection, sysEx.testArray);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(errorPart, sysEx.testArray);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(errorIndex, sysEx.testArray);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(errorLength, sysEx.testArray);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(setSingleInvalidNewValue, sysEx.testArray);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(getSingleValid, sysEx.testArray);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorNotSupported) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //get all request
    HANDLE_MESSAGE(getAllValid_1part, sysEx.testArray);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorNotSupported) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //set single request
    HANDLE_MESSAGE(setSingleValid, sysEx.testArray);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorNotSupported) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //set all request
    HANDLE_MESSAGE(setAllValid, sysEx.testArray);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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

    HANDLE_MESSAGE(setSingleValid, sysEx.testArray);

    //test sysex array
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorWrite) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);
}

TEST_CASE(SetSingle)
{
    //send valid set message
    HANDLE_MESSAGE(setSingleValid, sysEx.testArray);

    // check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send set single command with invalid param index
    HANDLE_MESSAGE(setSingleInvalidParam, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorIndex) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //test block which has same min and max value
    //in this case, SysExConf::status_t::errorNewValue should never be reported on any value
    HANDLE_MESSAGE(setSingleNoMinMax1, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    HANDLE_MESSAGE(setSingleNoMinMax2, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    HANDLE_MESSAGE(setSingleNoMinMax3, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(setAllValid, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send set all message for section with more parts
    HANDLE_MESSAGE(setAllMoreParts1, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send set all request with part byte being 0x01
    HANDLE_MESSAGE(setAllMoreParts2, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x01 == sysEx.testArray[5]);    //second part
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send set all requests for all parts and verify that status byte is set to SysExConf::status_t::errorPart
    HANDLE_MESSAGE(setAllAllParts, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorPart) == sysEx.testArray[4]);
    TEST_ASSERT(0x7F == sysEx.testArray[5]);    //same part as requested
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send set all request with invalid value
    //status byte should be SysExConf::status_t::errorNewValue
    HANDLE_MESSAGE(setAllnvalidNewVal, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(getSingleValid, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(getAllValid_1part, sysEx.testArray);

    //check response
    //10 values should be received
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(getAllValid_allParts_7F, sysEx.testArray);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 2);

    //reset message count
    sysEx.responseCounter = 0;

    //same message with part being 0x7E
    //in this case, last message should be SysExConf::status_t::ack message
    HANDLE_MESSAGE(getAllValid_allParts_7E, sysEx.testArray);

    //check last response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(customReq, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(customReqErrorRead, sysEx.testArray);

    //check response
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

    //send non-existing custom request message
    //SysExConf::status_t::errorWish should be reported
    HANDLE_MESSAGE(customReqInvalid, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorWish) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //disable configuration
    HANDLE_MESSAGE(connClose, sysEx.testArray);

    //verify that connection is closed
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send valid custom request message
    //SysExConf::status_t::errorConnection should be reported
    HANDLE_MESSAGE(customReq, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(connClose, sysEx.testArray);

    //sysex configuration should be disabled now
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send custom request 0
    //SysExConf::status_t::errorConnection should be returned because connection is closed
    HANDLE_MESSAGE(customReq, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorConnection) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send another custom request which has flag set to ignore connection status
    //SysExConf::status_t::ack should be reported
    HANDLE_MESSAGE(customReqNoConnCheck, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(CUSTOM_REQUEST_VALUE == sysEx.testArray[6]);
    TEST_ASSERT(0xF7 == sysEx.testArray[7]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //open connection again
    HANDLE_MESSAGE(connOpen, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(shortMessage1, sysEx.testArray);

    //if no action took place, responseCounter should be 0
    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send another variant of short message
    HANDLE_MESSAGE(shortMessage2, sysEx.testArray);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //send message with invalid SysEx ID
    HANDLE_MESSAGE(getSingleInvalidSysExID, sysEx.testArray);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 0);

    //short message where SysExConf::status_t::errorMessageLength should be returned
    HANDLE_MESSAGE(shortMessage3, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(backupAll, sysEx.testArray);

    //check if status byte is set to SysExConf::status_t::request value
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::request) == sysEx.testArray[4]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send backup/single request with incorrect part
    HANDLE_MESSAGE(backupSingleInvPart, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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
    HANDLE_MESSAGE(getSpecialReqBytesPerVal, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::paramSize_t::_7bit) == sysEx.testArray[6]);
    TEST_ASSERT(0xF7 == sysEx.testArray[7]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //params per msg request
    HANDLE_MESSAGE(getSpecialReqParamPerMsg, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::ack) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::nrOfParam_t::_32) == sysEx.testArray[6]);
    TEST_ASSERT(0xF7 == sysEx.testArray[7]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //now try those same requests, but without prior open connection request
    //status byte must equal SysExConf::status_t::errorConnection

    //close connection first
    HANDLE_MESSAGE(connClose, sysEx.testArray);

    //configuration should be closed now
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //bytes per value request
    HANDLE_MESSAGE(getSpecialReqBytesPerVal, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorConnection) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //params per msg request
    HANDLE_MESSAGE(getSpecialReqParamPerMsg, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorConnection) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //try to close configuration which is already closed
    HANDLE_MESSAGE(connClose, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorConnection) == sysEx.testArray[4]);
    TEST_ASSERT(0x00 == sysEx.testArray[5]);
    TEST_ASSERT(0xF7 == sysEx.testArray[6]);

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //send open connection request and check if sysEx.testArray is valid
    HANDLE_MESSAGE(connOpen, sysEx.testArray);

    //sysex configuration should be enabled now
    TEST_ASSERT(1 == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(sysEx.responseCounter == 1);

    //reset message count
    sysEx.responseCounter = 0;

    //disable configuration again
    HANDLE_MESSAGE(connClose, sysEx.testArray);

    //check response
    TEST_ASSERT(0xF0 == sysEx.testArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == sysEx.testArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == sysEx.testArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == sysEx.testArray[3]);
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