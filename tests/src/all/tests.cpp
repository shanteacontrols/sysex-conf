#include "framework/Framework.h"
#include "SysExConf.h"
#include "src/SysExTesting.h"

#define SYS_EX_CONF_M_ID_0 0x00
#define SYS_EX_CONF_M_ID_1 0x53
#define SYS_EX_CONF_M_ID_2 0x43

#define SYSEX_PARAM(value)                                                                                                         \
    (((value & 0xFF) >> 7) & 0x01) ? ((((value >> 8) & 0xFF) << 1) & 0x7F) | 0x01 : ((((value >> 8) & 0xFF) << 1) & 0x7F) & ~0x01, \
        (value & 0xFF) & 0x7F

namespace
{
    class SysExTest : public ::testing::Test
    {
        protected:
        void SetUp() override
        {
            sysEx.setLayout(sysExLayout);
            sysEx.setupCustomRequests(customRequests);
        }

        void TearDown() override
        {
        }

        const SysExConf::manufacturerID_t M_ID = {
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2
        };

        std::vector<SysExConf::Section> testSections = {
            {
                SECTION_0_PARAMETERS,
                SECTION_0_MIN,
                SECTION_0_MAX,
            },

            {
                SECTION_1_PARAMETERS,
                SECTION_1_MIN,
                SECTION_1_MAX,
            },

            {
                SECTION_2_PARAMETERS,
                SECTION_2_MIN,
                SECTION_2_MAX,
            }
        };

        std::vector<SysExConf::Block> sysExLayout = {
            {
                testSections,
            }
        };

        std::vector<SysExConf::customRequest_t> customRequests = {
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

        class SysExConfDataHandler : public SysExConf::DataHandler
        {
            public:
            SysExConfDataHandler() = default;

            uint8_t get(uint8_t block, uint8_t section, uint16_t index, uint16_t& value) override
            {
                value = TEST_VALUE_GET;

                if (getResults.empty())
                {
                    return static_cast<uint8_t>(SysExConf::status_t::ACK);
                }

                auto retVal = getResults.at(0);
                getResults.erase(getResults.begin());

                return retVal;
            }

            uint8_t set(uint8_t block, uint8_t section, uint16_t index, uint16_t newValue) override
            {
                if (setResults.empty())
                {
                    return static_cast<uint8_t>(SysExConf::status_t::ACK);
                }

                auto retVal = setResults.at(0);
                setResults.erase(setResults.begin());

                return retVal;
            }

            uint8_t customRequest(uint16_t request, CustomResponse& customResponse) override
            {
                switch (request)
                {
                case CUSTOM_REQUEST_ID_VALID:
                case CUSTOM_REQUEST_ID_NO_CONN_CHECK:
                {
                    for (int i = 0; i < _customReqArray.size(); i++)
                    {
                        customResponse.append(_customReqArray[i]);
                    }

                    return static_cast<uint8_t>(SysExConf::status_t::ACK);
                }
                break;

                case CUSTOM_REQUEST_ID_ERROR_READ:
                default:
                    return static_cast<uint8_t>(SysExConf::status_t::ERROR_READ);
                }
            }

            void reset()
            {
                _responseCounter = 0;
                _response.clear();
                getResults.clear();
                setResults.clear();
            }

            size_t responseCounter()
            {
                return _response.size();
            }

            std::vector<uint8_t> response(uint8_t index)
            {
                if (index >= _response.size())
                {
                    return {};
                }

                return _response.at(index);
            }

            void sendResponse(uint8_t* array, uint16_t size) override
            {
                std::vector<uint8_t> tempResponse;

                for (uint16_t i = 0; i < size; i++)
                {
                    tempResponse.push_back(array[i]);
                }

                _response.push_back(tempResponse);
            }

            std::vector<uint8_t> getResults = {};
            std::vector<uint8_t> setResults = {};

            private:
            std::vector<std::vector<uint8_t>> _response;
            std::vector<uint8_t>              _customReqArray = {
                1
            };

            size_t _responseCounter = 0;
        };

        template<typename T>
        void verifyMessage(const std::vector<uint8_t>& source, T status, const std::vector<uint8_t>* data = nullptr)
        {
            size_t size = source.size();

            if ((data != nullptr) && source.size())
            {
                size--;    // skip last byte (0xF7) - custom data starts here
            }

            for (int i = 0; i < size; i++)
            {
                if (i != 4)    // status byte
                {
                    ASSERT_EQ(source.at(i), dataHandler.response(dataHandler.responseCounter() - 1).at(i));
                }
                else
                {
                    ASSERT_EQ(static_cast<uint8_t>(status), dataHandler.response(dataHandler.responseCounter() - 1).at(i));
                }
            }

            // now verify data
            if ((data != nullptr) && source.size())
            {
                for (int i = size; i < (size + data->size()); i++)
                {
                    ASSERT_EQ(data->at(i - size), dataHandler.response(dataHandler.responseCounter() - 1).at(i));
                }

                ASSERT_EQ(0xF7, dataHandler.response(dataHandler.responseCounter() - 1).at(size + data->size()));
            }
        }

        void handleMessage(const std::vector<uint8_t>& source)
        {
            sysEx.handleMessage(&source[0], source.size());
        }

        void openConn()
        {
            // send open connection request
            handleMessage(CONN_OPEN);

            // sysex configuration should be enabled now
            ASSERT_TRUE(sysEx.isConfigurationEnabled());

            dataHandler.reset();
        }

        const std::vector<uint8_t> CONN_OPEN = {
            // request used to enable sysex configuration
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::specialRequest_t::CONN_OPEN),
            0xF7
        };

        const std::vector<uint8_t> CONN_CLOSE = {
            // request used to disable sysex configuration
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::specialRequest_t::CONN_CLOSE),
            0xF7
        };

        const std::vector<uint8_t> CONN_OPEN_SILENT = {
            // request used to enable sysex configuration in silent mode
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::specialRequest_t::CONN_OPEN_SILENT),
            0xF7
        };

        const std::vector<uint8_t> SILENT_MODE_DISABLE = {
            // request used to disable silent mode
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::specialRequest_t::CONN_SILENT_DISABLE),
            0xF7
        };

        const std::vector<uint8_t> ERROR_STATUS = {
            // get single message with invalid status byte for request message
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::ACK),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::GET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            TEST_SECTION_SINGLE_PART_ID,
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> ERROR_WISH = {
            // wish byte set to invalid value
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            0x04,
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            TEST_SECTION_SINGLE_PART_ID,
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> ERROR_MESSAGE_LENGTH = {
            // message intentionally one byte too long
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::GET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            TEST_SECTION_SINGLE_PART_ID,
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> ERROR_BLOCK = {
            // block byte set to invalid value
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::GET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            0x41,
            TEST_SECTION_SINGLE_PART_ID,
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> ERROR_SECTION = {
            // section byte set to invalid value
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::GET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            0x61,
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> ERROR_INDEX = {
            // index byte set to invalid value
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::GET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            TEST_SECTION_SINGLE_PART_ID,
            SYSEX_PARAM(0x7F),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> ERROR_PART = {
            // part byte set to invalid value
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_INVALID,
            static_cast<uint8_t>(SysExConf::wish_t::GET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            TEST_SECTION_SINGLE_PART_ID,
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> ERROR_AMOUNT = {
            // amount byte set to invalid value
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::GET),
            0x02,
            TEST_BLOCK_ID,
            TEST_SECTION_SINGLE_PART_ID,
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> CUSTOM_REQ = {
            // custom request with custom ID specified by user
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            CUSTOM_REQUEST_ID_VALID,
            0xF7
        };

        const std::vector<uint8_t> CUSTOM_REQ_ERROR_READ = {
            // custom request with custom ID specified by user
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            CUSTOM_REQUEST_ID_ERROR_READ,
            0xF7
        };

        const std::vector<uint8_t> CUSTOM_REQ_INVALID = {
            // custom request with non-existing custom ID
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            CUSTOM_REQUEST_ID_INVALID,
            0xF7
        };

        const std::vector<uint8_t> CUSTOM_REQ_NO_CONN_CHECK = {
            // custom request with non-existing custom ID
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            CUSTOM_REQUEST_ID_NO_CONN_CHECK,
            0xF7
        };

        const std::vector<uint8_t> SHORT_MESSAGE1 = {
            // short message which should be ignored by the protocol, variant 1
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            0xF7
        };

        const std::vector<uint8_t> SHORT_MESSAGE2 = {
            // short message which should be ignored by the protocol, variant 2
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            0xF7
        };

        const std::vector<uint8_t> SHORT_MESSAGE3 = {
            // short message on which protocol should throw MESSAGE_LENGTH error
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::GET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            0xF7
        };

        const std::vector<uint8_t> GET_SINGLE_VALID = {
            // valid get single command
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::GET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            TEST_SECTION_SINGLE_PART_ID,
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> GET_SINGLE_PART1 = {
            // get single command with part id set to 1
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            1,
            static_cast<uint8_t>(SysExConf::wish_t::GET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            TEST_SECTION_MULTIPLE_PARTS_ID,
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> SET_SINGLE_PART1 = {
            // set single command with part id set to 1
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            1,
            static_cast<uint8_t>(SysExConf::wish_t::SET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            TEST_SECTION_MULTIPLE_PARTS_ID,
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> GET_SINGLE_INVALID_SYS_EX_ID = {
            // get single command with invalid sysex ids
            0xF0,
            SYS_EX_CONF_M_ID_2,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_0,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::GET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            TEST_SECTION_SINGLE_PART_ID,
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> GET_ALL_VALID_1PART = {
            // valid get all command
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::GET),
            static_cast<uint8_t>(SysExConf::amount_t::ALL),
            TEST_BLOCK_ID,
            TEST_SECTION_SINGLE_PART_ID,
            SYSEX_PARAM(0),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> GET_ALL_VALID_ALL_PARTS_7_F = {
            // valid get all command for all parts (7F variant)
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            0x7F,
            static_cast<uint8_t>(SysExConf::wish_t::GET),
            static_cast<uint8_t>(SysExConf::amount_t::ALL),
            TEST_BLOCK_ID,
            TEST_SECTION_MULTIPLE_PARTS_ID,
            SYSEX_PARAM(0),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> GET_ALL_VALID_ALL_PARTS_7_E = {
            // valid get all command for all parts (7E variant)
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            0x7E,
            static_cast<uint8_t>(SysExConf::wish_t::GET),
            static_cast<uint8_t>(SysExConf::amount_t::ALL),
            TEST_BLOCK_ID,
            TEST_SECTION_MULTIPLE_PARTS_ID,
            SYSEX_PARAM(0),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> GET_SPECIAL_REQ_BYTES_PER_VAL = {
            // built-in special request which returns number of bytes per value configured in protocol
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            0x00,
            0x00,
            0x02,
            0xF7
        };

        const std::vector<uint8_t> GET_SPECIAL_REQ_PARAM_PER_MSG = {
            // built-in special request which returns number of parameters per message configured in protocol
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            0x00,
            0x00,
            0x03,
            0xF7
        };

        const std::vector<uint8_t> SET_SINGLE_VALID = {
            // valid set singe command
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::SET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            TEST_SECTION_SINGLE_PART_ID,
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(TEST_NEW_VALUE_VALID),
            0xF7
        };

        const std::vector<uint8_t> SET_SINGLE_INVALID_NEW_VALUE = {
            // set single command - invalid new value
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::SET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            TEST_SECTION_SINGLE_PART_ID,
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(TEST_NEW_VALUE_INVALID),
            0xF7
        };

        const std::vector<uint8_t> SET_ALL_VALID = {
            // valid set all command
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            0x00,
            static_cast<uint8_t>(SysExConf::wish_t::SET),
            static_cast<uint8_t>(SysExConf::amount_t::ALL),
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

        const std::vector<uint8_t> SET_ALL_ALL_PARTS = {
            // set all command with all parts modifier (invalid request)
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            0x7F,
            static_cast<uint8_t>(SysExConf::wish_t::SET),
            static_cast<uint8_t>(SysExConf::amount_t::ALL),
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

        const std::vector<uint8_t> SET_SINGLE_NO_MIN_MAX1 = {
            // valid set single command for section without min/max checking, variant 1
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::SET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            TEST_SECTION_NOMINMAX,
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> SET_SINGLE_NO_MIN_MAX2 = {
            // valid set single command for section without min/max checking, variant 2
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::SET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            TEST_SECTION_NOMINMAX,
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(50),
            0xF7
        };

        const std::vector<uint8_t> SET_SINGLE_NO_MIN_MAX3 = {
            // valid set single command for section without min/max checking, variant 3
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::SET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            TEST_SECTION_NOMINMAX,
            SYSEX_PARAM(TEST_INDEX_ID),
            SYSEX_PARAM(127),
            0xF7
        };

        const std::vector<uint8_t> SET_SINGLE_INVALID_PARAM = {
            // set single command with invalid parameter index
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::SET),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            TEST_SECTION_SINGLE_PART_ID,
            SYSEX_PARAM(TEST_INVALID_PARAMETER_B0S0),
            SYSEX_PARAM(0x00),
            0xF7
        };

        const std::vector<uint8_t> SET_ALLNVALID_NEW_VAL = {
            // set all command with invalid new value
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            TEST_MSG_PART_VALID,
            static_cast<uint8_t>(SysExConf::wish_t::SET),
            static_cast<uint8_t>(SysExConf::amount_t::ALL),
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

        const std::vector<uint8_t> SET_ALL_MORE_PARTS1 = {
            // set all command for section with more parts, part 0
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            0x00,
            static_cast<uint8_t>(SysExConf::wish_t::SET),
            static_cast<uint8_t>(SysExConf::amount_t::ALL),
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

        const std::vector<uint8_t> SET_ALL_MORE_PARTS2 = {
            // set all command for section with more parts, part 1
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            0x01,
            static_cast<uint8_t>(SysExConf::wish_t::SET),
            static_cast<uint8_t>(SysExConf::amount_t::ALL),
            TEST_BLOCK_ID,
            TEST_SECTION_MULTIPLE_PARTS_ID,
            SYSEX_PARAM(0x01),
            0xF7
        };

        const std::vector<uint8_t> BACKUP_ALL = {
            // backup all command
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            0x7F,
            static_cast<uint8_t>(SysExConf::wish_t::BACKUP),
            static_cast<uint8_t>(SysExConf::amount_t::ALL),
            TEST_BLOCK_ID,
            TEST_SECTION_SINGLE_PART_ID,
            SYSEX_PARAM(0),
            SYSEX_PARAM(0),
            0xF7
        };

        const std::vector<uint8_t> BACKUP_SINGLE_INV_PART = {
            // backup single command with invalid part set
            0xF0,
            SYS_EX_CONF_M_ID_0,
            SYS_EX_CONF_M_ID_1,
            SYS_EX_CONF_M_ID_2,
            static_cast<uint8_t>(SysExConf::status_t::REQUEST),
            0x03,
            static_cast<uint8_t>(SysExConf::wish_t::BACKUP),
            static_cast<uint8_t>(SysExConf::amount_t::SINGLE),
            TEST_BLOCK_ID,
            TEST_SECTION_SINGLE_PART_ID,
            SYSEX_PARAM(0),
            SYSEX_PARAM(0),
            0xF7
        };

        SysExConfDataHandler dataHandler;
        SysExConf            sysEx = SysExConf(dataHandler, M_ID);
    };

}    // namespace

TEST_F(SysExTest, Init)
{
    openConn();

    // close connection
    handleMessage(CONN_CLOSE);

    // sysex configuration should be disabled now
    ASSERT_FALSE(sysEx.isConfigurationEnabled());

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // check response
    verifyMessage(CONN_CLOSE, SysExConf::status_t::ACK);

    // reset number of received messages
    dataHandler.reset();

    // test silent mode
    handleMessage(CONN_OPEN_SILENT);

    // configuration and silent mode must be enabled
    ASSERT_TRUE(sysEx.isSilentModeEnabled());
    ASSERT_TRUE(sysEx.isConfigurationEnabled());

    // check that nothing was received as response
    ASSERT_EQ(0, dataHandler.responseCounter());

    // now disable silent mode
    handleMessage(SILENT_MODE_DISABLE);

    // silent mode should be disabled, but connection should be still opened
    // response should be received
    ASSERT_FALSE(sysEx.isSilentModeEnabled());
    ASSERT_TRUE(sysEx.isConfigurationEnabled());

    // check response
    verifyMessage(SILENT_MODE_DISABLE, SysExConf::status_t::ACK);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset number of received messages
    dataHandler.reset();

    // open silent mode again
    handleMessage(CONN_OPEN_SILENT);

    // verify no response was received
    ASSERT_EQ(0, dataHandler.responseCounter());

    // now close connection
    handleMessage(CONN_CLOSE);

    // verify that connection is closed
    ASSERT_FALSE(sysEx.isConfigurationEnabled());

    // silent mode should be disabled as well as an result of closed connection
    ASSERT_FALSE(sysEx.isSilentModeEnabled());

    // check response
    verifyMessage(CONN_CLOSE, SysExConf::status_t::ACK);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // enable silent mode using direct function call
    sysEx.setSilentMode(true);

    // verify silent mode is enabled
    ASSERT_TRUE(sysEx.isSilentModeEnabled());

    // disable silent mode using direct function call
    sysEx.setSilentMode(false);

    // verify silent mode is disabled
    ASSERT_FALSE(sysEx.isSilentModeEnabled());

    // initialized = true;
}

TEST_F(SysExTest, SilentMode)
{
    // open silent mode
    handleMessage(CONN_OPEN_SILENT);

    // configuration and silent mode must be enabled
    ASSERT_TRUE(sysEx.isSilentModeEnabled());
    ASSERT_TRUE(sysEx.isConfigurationEnabled());

    // check number of received messages
    ASSERT_EQ(0, dataHandler.responseCounter());

    // send set sigle request
    handleMessage(SET_SINGLE_VALID);

    // check number of received messages
    ASSERT_EQ(0, dataHandler.responseCounter());

    // send set all request
    handleMessage(SET_ALL_VALID);

    // check number of received messages
    ASSERT_EQ(0, dataHandler.responseCounter());

    // send request which causes status error
    handleMessage(ERROR_STATUS);

    // check number of received messages
    ASSERT_EQ(0, dataHandler.responseCounter());

    // send request which causes wish error
    handleMessage(ERROR_WISH);

    // check number of received messages
    ASSERT_EQ(0, dataHandler.responseCounter());

    // send request which causes amount error
    handleMessage(ERROR_AMOUNT);

    // check number of received messages
    ASSERT_EQ(0, dataHandler.responseCounter());

    // send request which causes block error
    handleMessage(ERROR_BLOCK);

    // check number of received messages
    ASSERT_EQ(0, dataHandler.responseCounter());

    // send request which causes section error
    handleMessage(ERROR_SECTION);

    // check number of received messages
    ASSERT_EQ(0, dataHandler.responseCounter());

    // send request which causes index error
    handleMessage(ERROR_INDEX);

    // check number of received messages
    ASSERT_EQ(0, dataHandler.responseCounter());

    // send request which causes part error
    handleMessage(ERROR_PART);

    // check number of received messages
    ASSERT_EQ(0, dataHandler.responseCounter());

    // send request which causes length error
    handleMessage(ERROR_MESSAGE_LENGTH);

    // check number of received messages
    ASSERT_EQ(0, dataHandler.responseCounter());

    // send custom request
    handleMessage(CUSTOM_REQ);

    // check number of received messages
    ASSERT_EQ(0, dataHandler.responseCounter());
}

TEST_F(SysExTest, ErrorConnClosed)
{
    // configuration should be closed initially
    ASSERT_FALSE(sysEx.isConfigurationEnabled());

    // send valid get message
    // since connection is closed, SysExConf::status_t::ERROR_CONNECTION should be reported
    handleMessage(GET_SINGLE_VALID);

    // check response
    verifyMessage(GET_SINGLE_VALID, SysExConf::status_t::ERROR_CONNECTION);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());
}

TEST_F(SysExTest, ErrorStatus)
{
    openConn();

    // send message with invalid status byte
    // SysExConf::status_t::ERROR_STATUS should be reported
    handleMessage(ERROR_STATUS);

    // check response
    verifyMessage(ERROR_STATUS, SysExConf::status_t::ERROR_STATUS);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());
}

TEST_F(SysExTest, ErrorWish)
{
    openConn();

    // send message with invalid wish byte
    handleMessage(ERROR_WISH);

    // check response
    verifyMessage(ERROR_WISH, SysExConf::status_t::ERROR_WISH);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());
}

TEST_F(SysExTest, ERROR_AMOUNT)
{
    openConn();

    // send message with invalid amount byte
    // SysExConf::status_t::ERROR_AMOUNT should be reported
    handleMessage(ERROR_AMOUNT);

    // check response
    verifyMessage(ERROR_AMOUNT, SysExConf::status_t::ERROR_AMOUNT);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());
}

TEST_F(SysExTest, ErrorBlock)
{
    openConn();

    // send message with invalid block byte
    // SysExConf::status_t::ERROR_BLOCK should be reported
    handleMessage(ERROR_BLOCK);

    // check response
    verifyMessage(ERROR_BLOCK, SysExConf::status_t::ERROR_BLOCK);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());
}

TEST_F(SysExTest, ErrorSection)
{
    openConn();

    // send message with invalid section byte
    // SysExConf::status_t::ERROR_SECTION should be reported
    handleMessage(ERROR_SECTION);

    // check response
    verifyMessage(ERROR_SECTION, SysExConf::status_t::ERROR_SECTION);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());
}

TEST_F(SysExTest, ErrorPart)
{
    openConn();

    // send message with invalid index byte
    // SysExConf::status_t::ERROR_PART should be reported
    handleMessage(ERROR_PART);

    // check response
    verifyMessage(ERROR_PART, SysExConf::status_t::ERROR_PART);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset number of received messages
    dataHandler.reset();

    // send get single request with section which normally contains more parts,
    // however, set message part byte to 1
    // error part should be thrown because message part must always be at value 0
    // when the amount is single

    handleMessage(GET_SINGLE_PART1);

    // check response
    verifyMessage(GET_SINGLE_PART1, SysExConf::status_t::ERROR_PART);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset number of received messages
    dataHandler.reset();

    // same outcome is expected for set single message with part 1
    handleMessage(SET_SINGLE_PART1);

    // check response
    verifyMessage(SET_SINGLE_PART1, SysExConf::status_t::ERROR_PART);
}

TEST_F(SysExTest, ErrorIndex)
{
    openConn();

    // send message with invalid index byte
    // SysExConf::status_t::ERROR_INDEX should be reported
    handleMessage(ERROR_INDEX);

    // check response
    verifyMessage(ERROR_INDEX, SysExConf::status_t::ERROR_INDEX);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());
}

TEST_F(SysExTest, ErrorMessageLength)
{
    openConn();

    // send message with invalid index byte
    // SysExConf::status_t::ERROR_MESSAGE_LENGTH should be reported
    handleMessage(ERROR_MESSAGE_LENGTH);

    // check response
    verifyMessage(ERROR_MESSAGE_LENGTH, SysExConf::status_t::ERROR_MESSAGE_LENGTH);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());
}

TEST_F(SysExTest, ErrorNewValue)
{
    openConn();

    // send invalid set message
    // SysExConf::status_t::ERROR_NEW_VALUE should be reported
    handleMessage(SET_SINGLE_INVALID_NEW_VALUE);

    // check response
    verifyMessage(SET_SINGLE_INVALID_NEW_VALUE, SysExConf::status_t::ERROR_NEW_VALUE);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());
}

TEST_F(SysExTest, ErrorWrite)
{
    openConn();

    // configure set function to return error
    // check if status byte is SysExConf::status_t::ERROR_WRITE

    dataHandler.setResults.push_back(static_cast<uint8_t>(SysExConf::status_t::ERROR_WRITE));

    // send valid set message
    handleMessage(SET_SINGLE_VALID);

    // check response
    verifyMessage(SET_SINGLE_VALID, SysExConf::status_t::ERROR_WRITE);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset number of received messages
    dataHandler.reset();
    dataHandler.setResults.push_back(static_cast<uint8_t>(SysExConf::status_t::ERROR_WRITE));

    handleMessage(SET_ALL_VALID);

    // check response
    verifyMessage(SET_ALL_VALID, SysExConf::status_t::ERROR_WRITE);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());
}

TEST_F(SysExTest, ErrorRead)
{
    openConn();

    // configure get function to return error
    // check if status byte is SysExConf::status_t::ERROR_READ

    dataHandler.getResults.push_back(static_cast<uint8_t>(SysExConf::status_t::ERROR_READ));

    handleMessage(GET_SINGLE_VALID);

    // check response
    verifyMessage(GET_SINGLE_VALID, SysExConf::status_t::ERROR_READ);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();
    dataHandler.getResults.push_back(static_cast<uint8_t>(SysExConf::status_t::ERROR_READ));

    // test get with all parameters
    // SysExConf::status_t::ERROR_READ should be reported again
    handleMessage(GET_ALL_VALID_1PART);

    // check response
    verifyMessage(GET_ALL_VALID_1PART, SysExConf::status_t::ERROR_READ);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());
}

TEST_F(SysExTest, ErrorCustom)
{
    openConn();

    // configure set function to return custom error
    uint8_t error = 63;
    dataHandler.setResults.push_back(error);

    // send valid set message
    handleMessage(SET_SINGLE_VALID);

    // check response
    verifyMessage(SET_SINGLE_VALID, error);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset number of received messages
    dataHandler.reset();
    dataHandler.setResults.push_back(error);

    handleMessage(SET_ALL_VALID);

    // check response
    verifyMessage(SET_ALL_VALID, error);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());
}

TEST_F(SysExTest, SetSingle)
{
    openConn();

    // send valid set message
    handleMessage(SET_SINGLE_VALID);

    // check response
    verifyMessage(SET_SINGLE_VALID, SysExConf::status_t::ACK);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // send set single command with invalid param index
    handleMessage(SET_SINGLE_INVALID_PARAM);

    // check response
    verifyMessage(SET_SINGLE_INVALID_PARAM, SysExConf::status_t::ERROR_INDEX);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // test block which has same min and max value
    // in this case, SysExConf::status_t::ERROR_NEW_VALUE should never be reported on any value
    handleMessage(SET_SINGLE_NO_MIN_MAX1);

    // check response
    verifyMessage(SET_SINGLE_NO_MIN_MAX1, SysExConf::status_t::ACK);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    handleMessage(SET_SINGLE_NO_MIN_MAX2);

    // check response
    verifyMessage(SET_SINGLE_NO_MIN_MAX2, SysExConf::status_t::ACK);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    handleMessage(SET_SINGLE_NO_MIN_MAX3);

    // check response
    verifyMessage(SET_SINGLE_NO_MIN_MAX3, SysExConf::status_t::ACK);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();
}

TEST_F(SysExTest, SetAll)
{
    openConn();

    // send set all request
    handleMessage(SET_ALL_VALID);

    // check response
    verifyMessage(SET_ALL_VALID, SysExConf::status_t::ACK);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // send set all message for section with more parts
    handleMessage(SET_ALL_MORE_PARTS1);

    // check response
    verifyMessage(SET_ALL_MORE_PARTS1, SysExConf::status_t::ACK);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // send set all request with part byte being 0x01
    handleMessage(SET_ALL_MORE_PARTS2);

    // check response
    verifyMessage(SET_ALL_MORE_PARTS2, SysExConf::status_t::ACK);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // send set all requests for all parts and verify that status byte is set to SysExConf::status_t::ERROR_PART
    handleMessage(SET_ALL_ALL_PARTS);

    // check response
    verifyMessage(SET_ALL_ALL_PARTS, SysExConf::status_t::ERROR_PART);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // send set all request with invalid value
    // status byte should be SysExConf::status_t::ERROR_NEW_VALUE
    handleMessage(SET_ALLNVALID_NEW_VAL);

    // check response
    verifyMessage(SET_ALLNVALID_NEW_VAL, SysExConf::status_t::ERROR_NEW_VALUE);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();
}

TEST_F(SysExTest, GetSingle)
{
    openConn();

    // send get single request
    handleMessage(GET_SINGLE_VALID);

    const std::vector<uint8_t> DATA = {
        SYSEX_PARAM(TEST_VALUE_GET)
    };

    // check response
    verifyMessage(GET_SINGLE_VALID, SysExConf::status_t::ACK, &DATA);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());
}

TEST_F(SysExTest, GetAll)
{
    openConn();

    // send get all request
    handleMessage(GET_ALL_VALID_1PART);

    const std::vector<uint8_t> DATA = {
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

    // check response
    verifyMessage(GET_ALL_VALID_1PART, SysExConf::status_t::ACK, &DATA);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // now send same request for all parts
    // we are expecting 2 messages now
    handleMessage(GET_ALL_VALID_ALL_PARTS_7_F);

    // check number of received messages
    ASSERT_EQ(2, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // same message with part being 0x7E
    // in this case, last message should be SysExConf::status_t::ACK message
    handleMessage(GET_ALL_VALID_ALL_PARTS_7_E);

    // check last response
    ASSERT_EQ(0xF0, dataHandler.response(dataHandler.responseCounter() - 1)[0]);
    ASSERT_EQ(SYS_EX_CONF_M_ID_0, dataHandler.response(dataHandler.responseCounter() - 1)[1]);
    ASSERT_EQ(SYS_EX_CONF_M_ID_1, dataHandler.response(dataHandler.responseCounter() - 1)[2]);
    ASSERT_EQ(SYS_EX_CONF_M_ID_2, dataHandler.response(dataHandler.responseCounter() - 1)[3]);
    ASSERT_EQ(static_cast<uint8_t>(SysExConf::status_t::ACK), dataHandler.response(dataHandler.responseCounter() - 1)[4]);
    ASSERT_EQ(0x7E, dataHandler.response(dataHandler.responseCounter() - 1)[5]);
    ASSERT_EQ(0x00, dataHandler.response(dataHandler.responseCounter() - 1)[6]);
    ASSERT_EQ(0x01, dataHandler.response(dataHandler.responseCounter() - 1)[7]);
    ASSERT_EQ(TEST_BLOCK_ID, dataHandler.response(dataHandler.responseCounter() - 1)[8]);
    ASSERT_EQ(TEST_SECTION_MULTIPLE_PARTS_ID, dataHandler.response(dataHandler.responseCounter() - 1)[9]);

    // check number of received messages
    ASSERT_EQ(3, dataHandler.responseCounter());
}

TEST_F(SysExTest, CustomReq)
{
    openConn();

    // send valid custom request message
    handleMessage(CUSTOM_REQ);

    std::vector<uint8_t> data = {
        SYSEX_PARAM(CUSTOM_REQUEST_VALUE)
    };

    // check response
    verifyMessage(CUSTOM_REQ, SysExConf::status_t::ACK, &data);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // send custom request message which should return false in custom request handler
    // in this case, SysExConf::status_t::ERROR_READ should be reported
    handleMessage(CUSTOM_REQ_ERROR_READ);

    // check response
    verifyMessage(CUSTOM_REQ_ERROR_READ, SysExConf::status_t::ERROR_READ);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // send non-existing custom request message
    // SysExConf::status_t::ERROR_WISH should be reported
    handleMessage(CUSTOM_REQ_INVALID);

    // check response
    verifyMessage(CUSTOM_REQ_INVALID, SysExConf::status_t::ERROR_WISH);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // disable configuration
    handleMessage(CONN_CLOSE);

    // verify that connection is closed
    ASSERT_FALSE(sysEx.isConfigurationEnabled());

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // send valid custom request message
    // SysExConf::status_t::ERROR_CONNECTION should be reported
    handleMessage(CUSTOM_REQ);

    // check response
    verifyMessage(CUSTOM_REQ, SysExConf::status_t::ERROR_CONNECTION);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // close sysex connection
    handleMessage(CONN_CLOSE);

    // sysex configuration should be disabled now
    ASSERT_FALSE(sysEx.isConfigurationEnabled());

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // send custom request 0
    // SysExConf::status_t::ERROR_CONNECTION should be returned because connection is closed
    handleMessage(CUSTOM_REQ);

    // check response
    verifyMessage(CUSTOM_REQ, SysExConf::status_t::ERROR_CONNECTION);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // send another custom request which has flag set to ignore connection status
    // SysExConf::status_t::ACK should be reported
    handleMessage(CUSTOM_REQ_NO_CONN_CHECK);

    data = {
        SYSEX_PARAM(CUSTOM_REQUEST_VALUE)
    };

    // check response
    verifyMessage(CUSTOM_REQ_NO_CONN_CHECK, SysExConf::status_t::ACK, &data);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // open connection again
    handleMessage(CONN_OPEN);

    // check response
    verifyMessage(CONN_OPEN, SysExConf::status_t::ACK);

    // verify that connection is opened
    ASSERT_TRUE(sysEx.isConfigurationEnabled());

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // try defining illegal custom requests
    std::vector<SysExConf::customRequest_t> customRequests_invalid = {
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

    // setupCustomRequests should return false because special requests which
    // are already used internally are defined in pointed structure
    ASSERT_FALSE(sysEx.setupCustomRequests(customRequests_invalid));
}

TEST_F(SysExTest, IgnoreMessage)
{
    openConn();

    // verify that no action takes place when sysex ids in message don't match
    // short message is any message without every required byte
    handleMessage(SHORT_MESSAGE1);

    // if no action took place, responseCounter should be 0
    // check number of received messages
    ASSERT_EQ(0, dataHandler.responseCounter());

    // send another variant of short message
    handleMessage(SHORT_MESSAGE2);

    // check number of received messages
    ASSERT_EQ(0, dataHandler.responseCounter());

    // send message with invalid SysEx ID
    handleMessage(GET_SINGLE_INVALID_SYS_EX_ID);

    // check number of received messages
    ASSERT_EQ(0, dataHandler.responseCounter());

    // short message where SysExConf::status_t::ERROR_MESSAGE_LENGTH should be returned
    handleMessage(SHORT_MESSAGE3);

    // check response
    verifyMessage(SHORT_MESSAGE3, SysExConf::status_t::ERROR_MESSAGE_LENGTH);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());
}

TEST_F(SysExTest, CustomMessage)
{
    openConn();

    // construct custom message and see if output matches
    std::vector<uint16_t> values = {
        0x05,
        0x06,
        0x07
    };

    sysEx.sendCustomMessage(&values[0], values.size());

    // check response
    ASSERT_EQ(0xF0, dataHandler.response(dataHandler.responseCounter() - 1)[0]);
    ASSERT_EQ(SYS_EX_CONF_M_ID_0, dataHandler.response(dataHandler.responseCounter() - 1)[1]);
    ASSERT_EQ(SYS_EX_CONF_M_ID_1, dataHandler.response(dataHandler.responseCounter() - 1)[2]);
    ASSERT_EQ(SYS_EX_CONF_M_ID_2, dataHandler.response(dataHandler.responseCounter() - 1)[3]);
    ASSERT_EQ(static_cast<uint8_t>(SysExConf::status_t::ACK), dataHandler.response(dataHandler.responseCounter() - 1)[4]);
    ASSERT_EQ(0x00, dataHandler.response(dataHandler.responseCounter() - 1)[5]);
    ASSERT_EQ(0x05, dataHandler.response(dataHandler.responseCounter() - 1)[6]);
    ASSERT_EQ(0x06, dataHandler.response(dataHandler.responseCounter() - 1)[7]);
    ASSERT_EQ(0x07, dataHandler.response(dataHandler.responseCounter() - 1)[8]);
    ASSERT_EQ(0xF7, dataHandler.response(dataHandler.responseCounter() - 1)[9]);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // construct same message again with SysExConf::status_t::REQUEST as status byte
    sysEx.sendCustomMessage(&values[0], values.size(), false);

    // check response
    ASSERT_EQ(0xF0, dataHandler.response(dataHandler.responseCounter() - 1)[0]);
    ASSERT_EQ(SYS_EX_CONF_M_ID_0, dataHandler.response(dataHandler.responseCounter() - 1)[1]);
    ASSERT_EQ(SYS_EX_CONF_M_ID_1, dataHandler.response(dataHandler.responseCounter() - 1)[2]);
    ASSERT_EQ(SYS_EX_CONF_M_ID_2, dataHandler.response(dataHandler.responseCounter() - 1)[3]);
    ASSERT_EQ(static_cast<uint8_t>(SysExConf::status_t::REQUEST), dataHandler.response(dataHandler.responseCounter() - 1)[4]);
    ASSERT_EQ(0x00, dataHandler.response(dataHandler.responseCounter() - 1)[5]);
    ASSERT_EQ(0x05, dataHandler.response(dataHandler.responseCounter() - 1)[6]);
    ASSERT_EQ(0x06, dataHandler.response(dataHandler.responseCounter() - 1)[7]);
    ASSERT_EQ(0x07, dataHandler.response(dataHandler.responseCounter() - 1)[8]);
    ASSERT_EQ(0xF7, dataHandler.response(dataHandler.responseCounter() - 1)[9]);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());
}

TEST_F(SysExTest, Backup)
{
    openConn();

    // send backup all request
    handleMessage(BACKUP_ALL);

    // check if status byte is set to SysExConf::status_t::REQUEST value
    ASSERT_EQ(static_cast<uint8_t>(SysExConf::status_t::REQUEST), dataHandler.response(dataHandler.responseCounter() - 1)[4]);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // now, try to send received response back
    handleMessage(dataHandler.response(0));

    // check if status byte is set to SysExConf::status_t::ACK value
    ASSERT_EQ(static_cast<uint8_t>(SysExConf::status_t::ACK), dataHandler.response(dataHandler.responseCounter() - 1)[4]);

    // reset message count
    dataHandler.reset();

    // send backup/single request with incorrect part
    handleMessage(BACKUP_SINGLE_INV_PART);

    // check response
    verifyMessage(BACKUP_SINGLE_INV_PART, SysExConf::status_t::ERROR_PART);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());
}

TEST_F(SysExTest, SpecialRequest)
{
    openConn();

    std::vector<uint8_t> data;

    // test all pre-configured special requests and see if they return correct value

    // bytes per value request
    handleMessage(GET_SPECIAL_REQ_BYTES_PER_VAL);

    data = {
        SYSEX_PARAM(2)
    };

    // check response
    verifyMessage(GET_SPECIAL_REQ_BYTES_PER_VAL, SysExConf::status_t::ACK, &data);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // params per msg request
    handleMessage(GET_SPECIAL_REQ_PARAM_PER_MSG);

    data = {
        SYSEX_PARAM(SysExConf::PARAMS_PER_MESSAGE)
    };

    // check response
    verifyMessage(GET_SPECIAL_REQ_PARAM_PER_MSG, SysExConf::status_t::ACK, &data);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // now try those same requests, but without prior open connection request
    // status byte must equal SysExConf::status_t::ERROR_CONNECTION

    // close connection first
    handleMessage(CONN_CLOSE);

    // configuration should be closed now
    ASSERT_FALSE(sysEx.isConfigurationEnabled());

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // bytes per value request
    handleMessage(GET_SPECIAL_REQ_BYTES_PER_VAL);

    // check response
    verifyMessage(GET_SPECIAL_REQ_BYTES_PER_VAL, SysExConf::status_t::ERROR_CONNECTION);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // params per msg request
    handleMessage(GET_SPECIAL_REQ_PARAM_PER_MSG);

    // check response
    verifyMessage(GET_SPECIAL_REQ_PARAM_PER_MSG, SysExConf::status_t::ERROR_CONNECTION);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // try to close configuration which is already closed
    handleMessage(CONN_CLOSE);

    // check response
    verifyMessage(CONN_CLOSE, SysExConf::status_t::ERROR_CONNECTION);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // send open connection request
    handleMessage(CONN_OPEN);

    // sysex configuration should be enabled now
    ASSERT_TRUE(sysEx.isConfigurationEnabled());

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();

    // disable configuration again
    handleMessage(CONN_CLOSE);

    // check response
    verifyMessage(CONN_CLOSE, SysExConf::status_t::ACK);

    // check number of received messages
    ASSERT_EQ(1, dataHandler.responseCounter());

    // reset message count
    dataHandler.reset();
}