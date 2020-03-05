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

    class SysExConfDataHandlerErrorSet : public SysExConf::DataHandler
    {
        public:
        SysExConfDataHandlerErrorSet()
        {}

        result_t get(uint8_t block, uint8_t section, size_t index, SysExConf::sysExParameter_t& value) override
        {
            value = TEST_VALUE_GET;
            return SysExConf::DataHandler::result_t::ok;
        }

        result_t set(uint8_t block, uint8_t section, size_t index, SysExConf::sysExParameter_t newValue) override
        {
            return SysExConf::DataHandler::result_t::error;
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
            responseCounter = 0;
        }

        void sendResponse(uint8_t* array, size_t size) override
        {
            for (size_t i = 0; i < size; i++)
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

    SysExConfDataHandlerErrorSet dataHandler;

    SysExConf sysEx(dataHandler,
                    mId,
                    SysExConf::paramSize_t::_7bit,
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

TEST_CASE(ErrorWrite)
{
    //onSet is configure to always return false
    //check if status byte is SysExConf::status_t::errorWrite

    //send valid set message
    HANDLE_MESSAGE(setSingleValid);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorWrite) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}

TEST_CASE(ErrorSet)
{
    //verify that status is set to status_t::errorWrite if onSet returns false
    HANDLE_MESSAGE(setAllValid);

    //check response
    TEST_ASSERT(0xF0 == dataHandler.sysExArray[0]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_0 == dataHandler.sysExArray[1]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_1 == dataHandler.sysExArray[2]);
    TEST_ASSERT(SYS_EX_CONF_M_ID_2 == dataHandler.sysExArray[3]);
    TEST_ASSERT(static_cast<uint8_t>(SysExConf::status_t::errorWrite) == dataHandler.sysExArray[4]);
    TEST_ASSERT(0x00 == dataHandler.sysExArray[5]);
    TEST_ASSERT(0xF7 == dataHandler.sysExArray[6]);

    //check number of received messages
    TEST_ASSERT(dataHandler.responseCounter == 1);
}