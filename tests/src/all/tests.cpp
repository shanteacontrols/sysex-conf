#include "unity/src/unity.h"
#include "unity/Helpers.h"
#include "SysExConf.h"
#include "src/SysExTesting.h"
#include <vector>

#ifdef SYSEX_2B
#define SYSEX_PARAMSIZE SysExConf::paramSize_t::_14bit
#else
#define SYSEX_PARAMSIZE SysExConf::paramSize_t::_7bit
#endif

#define SYS_EX_CONF_M_ID_0 0x00
#define SYS_EX_CONF_M_ID_1 0x53
#define SYS_EX_CONF_M_ID_2 0x43

#ifdef SYSEX_2B
#define SYSEX_PARAM(value)                                                                                                         \
    (((value & 0xFF) >> 7) & 0x01) ? ((((value >> 8) & 0xFF) << 1) & 0x7F) | 0x01 : ((((value >> 8) & 0xFF) << 1) & 0x7F) & ~0x01, \
        (value & 0xFF) & 0x7F
#else
#define SYSEX_PARAM(value) value
#endif

namespace
{
    const SysExConf::manufacturerID_t mId = {
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
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(0),
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
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(0),
        0xF7
    };

    const std::vector<uint8_t> errorMessageLength = {
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
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(0),
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
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(0),
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
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(0),
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
        SYSEX_PARAM(0x7F),
        SYSEX_PARAM(0),
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
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(0),
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
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(0),
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
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(0),
        0xF7
    };

    const std::vector<uint8_t> getSinglePart1 = {
        //get single command with part id set to 1
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        1,
        static_cast<uint8_t>(SysExConf::wish_t::get),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        TEST_BLOCK_ID,
        TEST_SECTION_MULTIPLE_PARTS_ID,
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(0),
        0xF7
    };

    const std::vector<uint8_t> setSinglePart1 = {
        //set single command with part id set to 1
        0xF0,
        SYS_EX_CONF_M_ID_0,
        SYS_EX_CONF_M_ID_1,
        SYS_EX_CONF_M_ID_2,
        static_cast<uint8_t>(SysExConf::status_t::request),
        1,
        static_cast<uint8_t>(SysExConf::wish_t::set),
        static_cast<uint8_t>(SysExConf::amount_t::single),
        TEST_BLOCK_ID,
        TEST_SECTION_MULTIPLE_PARTS_ID,
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(0),
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
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(0),
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
        SYSEX_PARAM(0),
        SYSEX_PARAM(0),
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
        SYSEX_PARAM(0),
        SYSEX_PARAM(0),
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
        SYSEX_PARAM(0),
        SYSEX_PARAM(0),
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
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(TEST_NEW_VALUE_VALID),
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
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(TEST_NEW_VALUE_INVALID),
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
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
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
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
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
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(0),
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
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(50),
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
        SYSEX_PARAM(TEST_INDEX_ID),
        SYSEX_PARAM(127),
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
        SYSEX_PARAM(TEST_INVALID_PARAMETER_B0S0),
        SYSEX_PARAM(0x00),
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
        SYSEX_PARAM(TEST_NEW_VALUE_INVALID),
        SYSEX_PARAM(TEST_NEW_VALUE_INVALID),
        SYSEX_PARAM(TEST_NEW_VALUE_INVALID),
        SYSEX_PARAM(TEST_NEW_VALUE_INVALID),
        SYSEX_PARAM(TEST_NEW_VALUE_INVALID),
        SYSEX_PARAM(TEST_NEW_VALUE_INVALID),
        SYSEX_PARAM(TEST_NEW_VALUE_INVALID),
        SYSEX_PARAM(TEST_NEW_VALUE_INVALID),
        SYSEX_PARAM(TEST_NEW_VALUE_INVALID),
        SYSEX_PARAM(TEST_NEW_VALUE_INVALID),
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
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x02),
        SYSEX_PARAM(0x03),
        SYSEX_PARAM(0x04),
        SYSEX_PARAM(0x05),
        SYSEX_PARAM(0x06),
        SYSEX_PARAM(0x07),
        SYSEX_PARAM(0x08),
        SYSEX_PARAM(0x09),
        SYSEX_PARAM(0x0A),
        SYSEX_PARAM(0x0B),
        SYSEX_PARAM(0x0C),
        SYSEX_PARAM(0x0D),
        SYSEX_PARAM(0x0E),
        SYSEX_PARAM(0x0F),
        SYSEX_PARAM(0x10),
        SYSEX_PARAM(0x11),
        SYSEX_PARAM(0x12),
        SYSEX_PARAM(0x13),
        SYSEX_PARAM(0x14),
        SYSEX_PARAM(0x15),
        SYSEX_PARAM(0x16),
        SYSEX_PARAM(0x17),
        SYSEX_PARAM(0x18),
        SYSEX_PARAM(0x19),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
        SYSEX_PARAM(0x01),
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
        SYSEX_PARAM(0x01),
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
        SYSEX_PARAM(0),
        SYSEX_PARAM(0),
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
        SYSEX_PARAM(0),
        SYSEX_PARAM(0),
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
            return getResult;
        }

        result_t set(uint8_t block, uint8_t section, size_t index, SysExConf::sysExParameter_t newValue) override
        {
            return setResult;
        }

        result_t customRequest(size_t request, CustomResponse& customResponse) override
        {
            switch (request)
            {
            case CUSTOM_REQUEST_ID_VALID:
            case CUSTOM_REQUEST_ID_NO_CONN_CHECK:
                for (int i = 0; i < customReqArray.size(); i++)
                    customResponse.append(customReqArray[i]);

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
            _responseCounter = 0;
            _response.clear();
        }

        size_t responseCounter()
        {
            return _response.size();
        }

        std::vector<uint8_t> response(uint8_t index)
        {
            if (index >= _response.size())
                return {};

            return _response.at(index);
        }

        void sendResponse(uint8_t* array, size_t size) override
        {
            std::vector<uint8_t> tempResponse;

            for (size_t i = 0; i < size; i++)
                tempResponse.push_back(array[i]);

            _response.push_back(tempResponse);
        }

        SysExConf::DataHandler::result_t setResult = SysExConf::DataHandler::result_t::ok;
        SysExConf::DataHandler::result_t getResult = SysExConf::DataHandler::result_t::ok;

        private:
        std::vector<std::vector<uint8_t>> _response;
        std::vector<uint8_t>              customReqArray = {
            1
        };

        size_t _responseCounter = 0;
    };

    SysExConfDataHandlerValid dataHandler;

    SysExConf sysEx(dataHandler,
                    mId,
                    SYSEX_PARAMSIZE,
                    SysExConf::nrOfParam_t::_32);

    void handleMessage(const std::vector<uint8_t>& source)
    {
        sysEx.handleMessage(&source[0], source.size());
    }

    void verifyMessage(const std::vector<uint8_t>& source, SysExConf::status_t status, const std::vector<uint8_t>* data = nullptr)
    {
        size_t size = source.size();

        if ((data != nullptr) && source.size())
            size--;

        for (int i = 0; i < size; i++)
        {
            if (i != 4)
                TEST_ASSERT_EQUAL_UINT8(source.at(i), dataHandler.response(dataHandler.responseCounter() - 1).at(i));
            else
                TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(status), dataHandler.response(dataHandler.responseCounter() - 1).at(i));
        }

        //now verify data
        if ((data != nullptr) && source.size())
        {
            for (int i = size; i < (size + data->size()); i++)
                TEST_ASSERT_EQUAL_UINT8(data->at(i - size), dataHandler.response(dataHandler.responseCounter() - 1).at(i));

            TEST_ASSERT_EQUAL_UINT8(0xF7, dataHandler.response(dataHandler.responseCounter() - 1).at(size + data->size()));
        }
    }
}    // namespace

TEST_SETUP()
{
    sysEx.reset();
    sysEx.setLayout(sysExLayout, NUMBER_OF_BLOCKS);
    sysEx.setupCustomRequests(customRequests, TOTAL_CUSTOM_REQUESTS);

    //send open connection request and see if sysExTestArray is valid
    handleMessage(connOpen);

    //sysex configuration should be enabled now
    TEST_ASSERT(1 == sysEx.isConfigurationEnabled());

    dataHandler.reset();
}

TEST_CASE(Init)
{
    //close connection
    handleMessage(connClose);

    //sysex configuration should be disabled now
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //check response
    verifyMessage(connClose, SysExConf::status_t::ack);

    //reset number of received messages
    dataHandler.reset();

    //test silent mode
    handleMessage(connOpenSilent);

    //configuration and silent mode must be enabled
    TEST_ASSERT(true == sysEx.isSilentModeEnabled());
    TEST_ASSERT(true == sysEx.isConfigurationEnabled());

    // check that nothing was received as response
    TEST_ASSERT(dataHandler.responseCounter() == 0);

    //now disable silent mode
    handleMessage(silentModeDisable);

    //silent mode should be disabled, but connection should be still opened
    //response should be received
    TEST_ASSERT(false == sysEx.isSilentModeEnabled());
    TEST_ASSERT(true == sysEx.isConfigurationEnabled());

    //check response
    verifyMessage(silentModeDisable, SysExConf::status_t::ack);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset number of received messages
    dataHandler.reset();

    //open silent mode again
    handleMessage(connOpenSilent);

    //verify no response was received
    TEST_ASSERT(dataHandler.responseCounter() == 0);

    //now close connection
    handleMessage(connClose);

    //verify that connection is closed
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //silent mode should be disabled as well as an result of closed connection
    TEST_ASSERT(false == sysEx.isSilentModeEnabled());

    //check response
    verifyMessage(connClose, SysExConf::status_t::ack);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

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
    handleMessage(connOpenSilent);

    //configuration and silent mode must be enabled
    TEST_ASSERT(true == sysEx.isSilentModeEnabled());
    TEST_ASSERT(true == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 0);

    //send set sigle request
    handleMessage(setSingleValid);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 0);

    //send set all request
    handleMessage(setAllValid);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 0);

    //send request which causes status error
    handleMessage(errorStatus);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 0);

    //send request which causes wish error
    handleMessage(errorWish);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 0);

    //send request which causes amount error
    handleMessage(errorAmount);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 0);

    //send request which causes block error
    handleMessage(errorBlock);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 0);

    //send request which causes section error
    handleMessage(errorSection);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 0);

    //send request which causes index error
    handleMessage(errorIndex);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 0);

    //send request which causes part error
    handleMessage(errorPart);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 0);

    //send request which causes length error
    handleMessage(errorMessageLength);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 0);

    //send custom request
    handleMessage(customReq);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 0);
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
    handleMessage(connClose);

    //configuration should be closed now
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check response
    verifyMessage(connClose, SysExConf::status_t::ack);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset number of received messages
    dataHandler.reset();

    //send valid get message
    //since connection is closed, SysExConf::status_t::errorConnection should be reported
    handleMessage(getSingleValid);

    //check response
    verifyMessage(getSingleValid, SysExConf::status_t::errorConnection);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);
}

TEST_CASE(ErrorStatus)
{
    //send message with invalid status byte
    //SysExConf::status_t::errorStatus should be reported
    handleMessage(errorStatus);

    //check response
    verifyMessage(errorStatus, SysExConf::status_t::errorStatus);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);
}

TEST_CASE(ErrorWish)
{
    //send message with invalid wish byte
    handleMessage(errorWish);

    //check response
    verifyMessage(errorWish, SysExConf::status_t::errorWish);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);
}

TEST_CASE(ErrorAmount)
{
    //send message with invalid amount byte
    //SysExConf::status_t::errorAmount should be reported
    handleMessage(errorAmount);

    //check response
    verifyMessage(errorAmount, SysExConf::status_t::errorAmount);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);
}

TEST_CASE(ErrorBlock)
{
    //send message with invalid block byte
    //SysExConf::status_t::errorBlock should be reported
    handleMessage(errorBlock);

    //check response
    verifyMessage(errorBlock, SysExConf::status_t::errorBlock);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);
}

TEST_CASE(ErrorSection)
{
    //send message with invalid section byte
    //SysExConf::status_t::errorSection should be reported
    handleMessage(errorSection);

    //check response
    verifyMessage(errorSection, SysExConf::status_t::errorSection);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);
}

TEST_CASE(ErrorPart)
{
    //send message with invalid index byte
    //SysExConf::status_t::errorPart should be reported
    handleMessage(errorPart);

    //check response
    verifyMessage(errorPart, SysExConf::status_t::errorPart);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset number of received messages
    dataHandler.reset();

    //send get single request with section which normally contains more parts,
    //however, set message part byte to 1
    //error part should be thrown because message part must always be at value 0
    //when the amount is single

    handleMessage(getSinglePart1);

    //check response
    verifyMessage(getSinglePart1, SysExConf::status_t::errorPart);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset number of received messages
    dataHandler.reset();

    //same outcome is expected for set single message with part 1
    handleMessage(setSinglePart1);

    //check response
    verifyMessage(setSinglePart1, SysExConf::status_t::errorPart);
}

TEST_CASE(ErrorIndex)
{
    //send message with invalid index byte
    //SysExConf::status_t::errorIndex should be reported
    handleMessage(errorIndex);

    //check response
    verifyMessage(errorIndex, SysExConf::status_t::errorIndex);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);
}

TEST_CASE(ErrorMessageLength)
{
    //send message with invalid index byte
    //SysExConf::status_t::errorMessageLength should be reported
    handleMessage(errorMessageLength);

    //check response
    verifyMessage(errorMessageLength, SysExConf::status_t::errorMessageLength);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);
}

TEST_CASE(ErrorNewValue)
{
    //send invalid set message
    //SysExConf::status_t::errorNewValue should be reported
    handleMessage(setSingleInvalidNewValue);

    //check response
    verifyMessage(setSingleInvalidNewValue, SysExConf::status_t::errorNewValue);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);
}

TEST_CASE(ErrorWrite)
{
    //configure set function to always return error
    //check if status byte is SysExConf::status_t::errorWrite

    dataHandler.setResult = SysExConf::DataHandler::result_t::error;

    //send valid set message
    handleMessage(setSingleValid);

    //check response
    verifyMessage(setSingleValid, SysExConf::status_t::errorWrite);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset number of received messages
    dataHandler.reset();

    handleMessage(setAllValid);

    //check response
    verifyMessage(setAllValid, SysExConf::status_t::errorWrite);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //revert back
    dataHandler.setResult = SysExConf::DataHandler::result_t::ok;
}

TEST_CASE(ErrorRead)
{
    //configure get function to always return error
    //check if status byte is SysExConf::status_t::errorRead

    dataHandler.getResult = SysExConf::DataHandler::result_t::error;

    handleMessage(getSingleValid);

    //check response
    verifyMessage(getSingleValid, SysExConf::status_t::errorRead);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //test get with all parameters
    //SysExConf::status_t::errorRead should be reported again
    handleMessage(getAllValid_1part);

    //check response
    verifyMessage(getAllValid_1part, SysExConf::status_t::errorRead);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //revert back
    dataHandler.getResult = SysExConf::DataHandler::result_t::ok;
}

TEST_CASE(SetSingle)
{
    //send valid set message
    handleMessage(setSingleValid);

    //check response
    verifyMessage(setSingleValid, SysExConf::status_t::ack);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //send set single command with invalid param index
    handleMessage(setSingleInvalidParam);

    //check response
    verifyMessage(setSingleInvalidParam, SysExConf::status_t::errorIndex);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //test block which has same min and max value
    //in this case, SysExConf::status_t::errorNewValue should never be reported on any value
    handleMessage(setSingleNoMinMax1);

    //check response
    verifyMessage(setSingleNoMinMax1, SysExConf::status_t::ack);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    handleMessage(setSingleNoMinMax2);

    //check response
    verifyMessage(setSingleNoMinMax2, SysExConf::status_t::ack);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    handleMessage(setSingleNoMinMax3);

    //check response
    verifyMessage(setSingleNoMinMax3, SysExConf::status_t::ack);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();
}

TEST_CASE(SetAll)
{
    //send set all request
    handleMessage(setAllValid);

    //check response
    verifyMessage(setAllValid, SysExConf::status_t::ack);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //send set all message for section with more parts
    handleMessage(setAllMoreParts1);

    //check response
    verifyMessage(setAllMoreParts1, SysExConf::status_t::ack);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //send set all request with part byte being 0x01
    handleMessage(setAllMoreParts2);

    //check response
    verifyMessage(setAllMoreParts2, SysExConf::status_t::ack);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //send set all requests for all parts and verify that status byte is set to SysExConf::status_t::errorPart
    handleMessage(setAllAllParts);

    //check response
    verifyMessage(setAllAllParts, SysExConf::status_t::errorPart);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //send set all request with invalid value
    //status byte should be SysExConf::status_t::errorNewValue
    handleMessage(setAllnvalidNewVal);

    //check response
    verifyMessage(setAllnvalidNewVal, SysExConf::status_t::errorNewValue);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();
}

TEST_CASE(GetSingle)
{
    //send get single request
    handleMessage(getSingleValid);

    const std::vector<uint8_t> data = {
        SYSEX_PARAM(TEST_VALUE_GET)
    };

    //check response
    verifyMessage(getSingleValid, SysExConf::status_t::ack, &data);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);
}

TEST_CASE(GetAll)
{
    //send get all request
    handleMessage(getAllValid_1part);

    const std::vector<uint8_t> data = {
        SYSEX_PARAM(TEST_VALUE_GET),
        SYSEX_PARAM(TEST_VALUE_GET),
        SYSEX_PARAM(TEST_VALUE_GET),
        SYSEX_PARAM(TEST_VALUE_GET),
        SYSEX_PARAM(TEST_VALUE_GET),
        SYSEX_PARAM(TEST_VALUE_GET),
        SYSEX_PARAM(TEST_VALUE_GET),
        SYSEX_PARAM(TEST_VALUE_GET),
        SYSEX_PARAM(TEST_VALUE_GET),
        SYSEX_PARAM(TEST_VALUE_GET)
    };

    //check response
    verifyMessage(getAllValid_1part, SysExConf::status_t::ack, &data);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //now send same request for all parts
    //we are expecting 2 messages now
    handleMessage(getAllValid_allParts_7F);

    //check number of received messages
    TEST_ASSERT_EQUAL_UINT8(2, dataHandler.responseCounter());

    //reset message count
    dataHandler.reset();

    //same message with part being 0x7E
    //in this case, last message should be SysExConf::status_t::ack message
    handleMessage(getAllValid_allParts_7E);

    //check last response
    TEST_ASSERT_EQUAL_UINT8(0xF0, dataHandler.response(dataHandler.responseCounter() - 1)[0]);
    TEST_ASSERT_EQUAL_UINT8(SYS_EX_CONF_M_ID_0, dataHandler.response(dataHandler.responseCounter() - 1)[1]);
    TEST_ASSERT_EQUAL_UINT8(SYS_EX_CONF_M_ID_1, dataHandler.response(dataHandler.responseCounter() - 1)[2]);
    TEST_ASSERT_EQUAL_UINT8(SYS_EX_CONF_M_ID_2, dataHandler.response(dataHandler.responseCounter() - 1)[3]);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(SysExConf::status_t::ack), dataHandler.response(dataHandler.responseCounter() - 1)[4]);
    TEST_ASSERT_EQUAL_UINT8(0x7E, dataHandler.response(dataHandler.responseCounter() - 1)[5]);
    TEST_ASSERT_EQUAL_UINT8(0x00, dataHandler.response(dataHandler.responseCounter() - 1)[6]);
    TEST_ASSERT_EQUAL_UINT8(0x01, dataHandler.response(dataHandler.responseCounter() - 1)[7]);
    TEST_ASSERT_EQUAL_UINT8(TEST_BLOCK_ID, dataHandler.response(dataHandler.responseCounter() - 1)[8]);
    TEST_ASSERT_EQUAL_UINT8(TEST_SECTION_MULTIPLE_PARTS_ID, dataHandler.response(dataHandler.responseCounter() - 1)[9]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 3);
}

TEST_CASE(CustomReq)
{
    //reset message count
    dataHandler.reset();

    //send valid custom request message
    handleMessage(customReq);

    std::vector<uint8_t> data = {
        SYSEX_PARAM(CUSTOM_REQUEST_VALUE)
    };

    //check response
    verifyMessage(customReq, SysExConf::status_t::ack, &data);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //send custom request message which should return false in custom request handler
    //in this case, SysExConf::status_t::errorRead should be reported
    handleMessage(customReqErrorRead);

    //check response
    verifyMessage(customReqErrorRead, SysExConf::status_t::errorRead);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //send non-existing custom request message
    //SysExConf::status_t::errorWish should be reported
    handleMessage(customReqInvalid);

    //check response
    verifyMessage(customReqInvalid, SysExConf::status_t::errorWish);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //disable configuration
    handleMessage(connClose);

    //verify that connection is closed
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //send valid custom request message
    //SysExConf::status_t::errorConnection should be reported
    handleMessage(customReq);

    //check response
    verifyMessage(customReq, SysExConf::status_t::errorConnection);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

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
    handleMessage(connClose);

    //sysex configuration should be disabled now
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //send custom request 0
    //SysExConf::status_t::errorConnection should be returned because connection is closed
    handleMessage(customReq);

    //check response
    verifyMessage(customReq, SysExConf::status_t::errorConnection);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //send another custom request which has flag set to ignore connection status
    //SysExConf::status_t::ack should be reported
    handleMessage(customReqNoConnCheck);

    data = {
        SYSEX_PARAM(CUSTOM_REQUEST_VALUE)
    };

    //check response
    verifyMessage(customReqNoConnCheck, SysExConf::status_t::ack, &data);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //open connection again
    handleMessage(connOpen);

    //check response
    verifyMessage(connOpen, SysExConf::status_t::ack);

    //verify that connection is opened
    TEST_ASSERT(true == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);
}

TEST_CASE(IgnoreMessage)
{
    //verify that no action takes place when sysex ids in message don't match
    //short message is any message without every required byte
    handleMessage(shortMessage1);

    //if no action took place, responseCounter should be 0
    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 0);

    //send another variant of short message
    handleMessage(shortMessage2);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 0);

    //send message with invalid SysEx ID
    handleMessage(getSingleInvalidSysExID);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 0);

    //short message where SysExConf::status_t::errorMessageLength should be returned
    handleMessage(shortMessage3);

    //check response
    verifyMessage(shortMessage3, SysExConf::status_t::errorMessageLength);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);
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
    TEST_ASSERT_EQUAL_UINT8(0xF0, dataHandler.response(dataHandler.responseCounter() - 1)[0]);
    TEST_ASSERT_EQUAL_UINT8(SYS_EX_CONF_M_ID_0, dataHandler.response(dataHandler.responseCounter() - 1)[1]);
    TEST_ASSERT_EQUAL_UINT8(SYS_EX_CONF_M_ID_1, dataHandler.response(dataHandler.responseCounter() - 1)[2]);
    TEST_ASSERT_EQUAL_UINT8(SYS_EX_CONF_M_ID_2, dataHandler.response(dataHandler.responseCounter() - 1)[3]);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(SysExConf::status_t::ack), dataHandler.response(dataHandler.responseCounter() - 1)[4]);
    TEST_ASSERT_EQUAL_UINT8(0x00, dataHandler.response(dataHandler.responseCounter() - 1)[5]);
    TEST_ASSERT_EQUAL_UINT8(0x05, dataHandler.response(dataHandler.responseCounter() - 1)[6]);
    TEST_ASSERT_EQUAL_UINT8(0x06, dataHandler.response(dataHandler.responseCounter() - 1)[7]);
    TEST_ASSERT_EQUAL_UINT8(0x07, dataHandler.response(dataHandler.responseCounter() - 1)[8]);
    TEST_ASSERT_EQUAL_UINT8(0xF7, dataHandler.response(dataHandler.responseCounter() - 1)[9]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //construct same message again with SysExConf::status_t::request as status byte
    sysEx.sendCustomMessage(&values[0], values.size(), false);

    //check response
    TEST_ASSERT_EQUAL_UINT8(0xF0, dataHandler.response(dataHandler.responseCounter() - 1)[0]);
    TEST_ASSERT_EQUAL_UINT8(SYS_EX_CONF_M_ID_0, dataHandler.response(dataHandler.responseCounter() - 1)[1]);
    TEST_ASSERT_EQUAL_UINT8(SYS_EX_CONF_M_ID_1, dataHandler.response(dataHandler.responseCounter() - 1)[2]);
    TEST_ASSERT_EQUAL_UINT8(SYS_EX_CONF_M_ID_2, dataHandler.response(dataHandler.responseCounter() - 1)[3]);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(SysExConf::status_t::request), dataHandler.response(dataHandler.responseCounter() - 1)[4]);
    TEST_ASSERT_EQUAL_UINT8(0x00, dataHandler.response(dataHandler.responseCounter() - 1)[5]);
    TEST_ASSERT_EQUAL_UINT8(0x05, dataHandler.response(dataHandler.responseCounter() - 1)[6]);
    TEST_ASSERT_EQUAL_UINT8(0x06, dataHandler.response(dataHandler.responseCounter() - 1)[7]);
    TEST_ASSERT_EQUAL_UINT8(0x07, dataHandler.response(dataHandler.responseCounter() - 1)[8]);
    TEST_ASSERT_EQUAL_UINT8(0xF7, dataHandler.response(dataHandler.responseCounter() - 1)[9]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);
}

TEST_CASE(Backup)
{
    //send backup all request
    handleMessage(backupAll);

    //check if status byte is set to SysExConf::status_t::request value
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::request) == dataHandler.response(dataHandler.responseCounter() - 1)[4]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //send backup/single request with incorrect part
    handleMessage(backupSingleInvPart);

    //check response
    verifyMessage(backupSingleInvPart, SysExConf::status_t::errorPart);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);
}

TEST_CASE(SpecialRequest)
{
    std::vector<uint8_t> data;

    //test all pre-configured special requests and see if they return correct value

    //bytes per value request
    handleMessage(getSpecialReqBytesPerVal);

    data = {
        static_cast<uint8_t>(SYSEX_PARAMSIZE)
    };

    //check response
    verifyMessage(getSpecialReqBytesPerVal, SysExConf::status_t::ack, &data);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //params per msg request
    handleMessage(getSpecialReqParamPerMsg);

    data = {
        static_cast<uint8_t>(SysExConf::nrOfParam_t::_32)
    };

    //check response
    verifyMessage(getSpecialReqParamPerMsg, SysExConf::status_t::ack, &data);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //now try those same requests, but without prior open connection request
    //status byte must equal SysExConf::status_t::errorConnection

    //close connection first
    handleMessage(connClose);

    //configuration should be closed now
    TEST_ASSERT(false == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //bytes per value request
    handleMessage(getSpecialReqBytesPerVal);

    //check response
    verifyMessage(getSpecialReqBytesPerVal, SysExConf::status_t::errorConnection);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //params per msg request
    handleMessage(getSpecialReqParamPerMsg);

    //check response
    verifyMessage(getSpecialReqParamPerMsg, SysExConf::status_t::errorConnection);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //try to close configuration which is already closed
    handleMessage(connClose);

    //check response
    verifyMessage(connClose, SysExConf::status_t::errorConnection);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //send open connection request
    handleMessage(connOpen);

    //sysex configuration should be enabled now
    TEST_ASSERT(1 == sysEx.isConfigurationEnabled());

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();

    //disable configuration again
    handleMessage(connClose);

    //check response
    verifyMessage(connClose, SysExConf::status_t::ack);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter() == 1);

    //reset message count
    dataHandler.reset();
}