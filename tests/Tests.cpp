#include <gtest/gtest.h>
#include "../src/SysEx.h"

#define TEST_BLOCK_ID               3
#define TEST_SECTION_ID             2
#define TEST_INDEX_ID               5
#define TEST_NEW_VALUE_VALID        5
#define TEST_NEW_VALUE_INVALID      40
#define TEST_MSG_PART               0

#define TEST_VALUE_GET              3

#define CUSTOM_REQUEST_ID           54
#define CUSTOM_REQUEST_VALUE        1

#define TEST_BLOCK_USER_ERROR_ID    2
#define TEST_SECTION_USER_ERROR_ID  2

SysEx sysEx;
uint8_t sysExTestArray[200];
int responseCounter;

bool onCustom(uint8_t value)
{
    switch(value)
    {
        case CUSTOM_REQUEST_ID:
        sysEx.addToResponse(CUSTOM_REQUEST_VALUE);
        return true;
        break;

        default:
        return false;
        break;
    }
}

sysExParameter_t onGet(uint8_t block, uint8_t section, uint16_t index)
{
    if ((block == TEST_BLOCK_ID) && (section == TEST_SECTION_ID) && (index == TEST_INDEX_ID))
    {
        return TEST_VALUE_GET;
    }
    else if ((block == TEST_BLOCK_USER_ERROR_ID) && (section == TEST_SECTION_USER_ERROR_ID) && (index == TEST_INDEX_ID))
    {
        sysEx.setError(ERROR_NOT_SUPPORTED);
        return 0;
    }
    else
    {
        return 1;
    }
}

bool onSet(uint8_t block, uint8_t section, uint16_t index, sysExParameter_t newValue)
{
    if ((block == 0x00) && (section == 0x00) && (index == 0x00))
    {
        sysEx.setError(ERROR_NOT_SUPPORTED);
        return false;
    }
    else if ((block == TEST_BLOCK_ID) && (section == TEST_SECTION_ID) && (index == (TEST_INDEX_ID+1)))
    {
        sysEx.setError(NUMBER_OF_ERRORS);
        return false;
    }
    else if ((block == 0x01) && (section == 0x03))
    {
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
        responseCounter = 0;
        sysEx.init();
        sysEx.setHandleGet(onGet);
        sysEx.setHandleSet(onSet);
        sysEx.setHandleCustomRequest(onCustom);
        sysEx.setHandleSysExWrite(writeSysEx);

        uint8_t arraySize = sizeof(handshake)/sizeof(uint8_t);
        memcpy(sysExTestArray, handshake, arraySize);

        //send handshake message and see if sysExTestArray is valid
        sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

        //sysex configuration should be enabled after handshake
        EXPECT_EQ(1, sysEx.configurationEnabled());

        sysEx.addBlocks(6);

        sysExSection section;

        {
            section.numberOfParameters = 4;
            section.newValueMin = 0;
            section.newValueMax = 1;
            sysEx.addSection(0, section);

            section.numberOfParameters = 3;
            section.newValueMin = 0;
            section.newValueMax = 0;
            sysEx.addSection(0, section);
        }

        {
            section.numberOfParameters = 96;
            section.newValueMin = 0;
            section.newValueMax = 0;
            sysEx.addSection(1, section);

            section.numberOfParameters = 96;
            section.newValueMin = 0;
            section.newValueMax = 14;
            sysEx.addSection(1, section);

            section.numberOfParameters = 96;
            section.newValueMin = 0;
            section.newValueMax = 127;
            sysEx.addSection(1, section);

            section.numberOfParameters = 2;
            section.newValueMin = 1;
            section.newValueMax = 127;
            sysEx.addSection(1, section);

            section.numberOfParameters = 33;
            section.newValueMin = 1;
            section.newValueMax = 16;
            sysEx.addSection(1, section);
        }

        {
            section.numberOfParameters = 32;
            section.newValueMin = 0;
            section.newValueMax = 1;
            sysEx.addSection(2, section);

            section.numberOfParameters = 32;
            section.newValueMin = 0;
            section.newValueMax = 1;
            sysEx.addSection(2, section);

            section.numberOfParameters = 32;
            section.newValueMin = 0;
            section.newValueMax = 1;
            sysEx.addSection(2, section);

            section.numberOfParameters = 32;
            section.newValueMin = 0;
            section.newValueMax = 127;
            sysEx.addSection(2, section);

            section.numberOfParameters = 32;
            section.newValueMin = 1;
            section.newValueMax = 16;
            sysEx.addSection(2, section);
        }

        {
            section.numberOfParameters = 32;
            section.newValueMin = 0;
            section.newValueMax = 1;
            sysEx.addSection(3, section);

            section.numberOfParameters = 32;
            section.newValueMin = 0;
            section.newValueMax = 1;
            sysEx.addSection(3, section);

            section.numberOfParameters = 32;
            section.newValueMin = 0;
            section.newValueMax = 7;
            sysEx.addSection(3, section);

            section.numberOfParameters = 32;
            section.newValueMin = 0;
            section.newValueMax = 127;
            sysEx.addSection(3, section);

            section.numberOfParameters = 32;
            section.newValueMin = 0;
            section.newValueMax = 127;
            sysEx.addSection(3, section);

            section.numberOfParameters = 32;
            section.newValueMin = 0;
            section.newValueMax = 127;
            sysEx.addSection(3, section);

            section.numberOfParameters = 32;
            section.newValueMin = 0;
            section.newValueMax = 127;
            sysEx.addSection(3, section);

            section.numberOfParameters = 32;
            section.newValueMin = 0;
            section.newValueMax = 127;
            sysEx.addSection(3, section);

            section.numberOfParameters = 32;
            section.newValueMin = 0;
            section.newValueMax = 127;
            sysEx.addSection(3, section);

            section.numberOfParameters = 32;
            section.newValueMin = 1;
            section.newValueMax = 16;
            sysEx.addSection(3, section);
        }

        {
            section.numberOfParameters = 48;
            section.newValueMin = 0;
            section.newValueMax = 7;
            sysEx.addSection(4, section);

            section.numberOfParameters = 48;
            section.newValueMin = 0;
            section.newValueMax = 1;
            sysEx.addSection(4, section);

            section.numberOfParameters = 3;
            section.newValueMin = 0;
            section.newValueMax = 0;
            sysEx.addSection(4, section);

            section.numberOfParameters = 48;
            section.newValueMin = 0;
            section.newValueMax = 127;
            sysEx.addSection(4, section);

            section.numberOfParameters = 48;
            section.newValueMin = 0;
            section.newValueMax = 1;
            sysEx.addSection(4, section);

            section.numberOfParameters = 48;
            section.newValueMin = 0;
            section.newValueMax = 1;
            sysEx.addSection(4, section);

            section.numberOfParameters = 48;
            section.newValueMin = 1;
            section.newValueMax = 127;
            sysEx.addSection(4, section);

            section.numberOfParameters = 48;
            section.newValueMin = 1;
            section.newValueMax = 16;
            sysEx.addSection(4, section);
        }

        {
            section.numberOfParameters = 7;
            section.newValueMin = 0;
            section.newValueMax = 0;
            sysEx.addSection(5, section);

            section.numberOfParameters = 2;
            section.newValueMin = 0;
            section.newValueMax = 0;
            sysEx.addSection(5, section);
        }
    }

    virtual void TearDown()
    {
        
    }

    const uint8_t handshake[8] =
    {
        //handshake request used to enable sysex configuration
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, 0x01, 0xF7
    };

    const uint8_t getValid[12] =
    {
        //get block 3, section 3, index 5
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_ID, TEST_INDEX_ID, 0xF7
    };

    const uint8_t getInvalidIDs[12] =
    {
        //get block 3, section 3, index 5
        0xF0, SYS_EX_M_ID_2, SYS_EX_M_ID_1, SYS_EX_M_ID_0, REQUEST, TEST_MSG_PART, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_ID, TEST_INDEX_ID, 0xF7
    };

    const uint8_t setValid[13] =
    {
        //set block 3, section 3, index 5, valid new value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_set, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_ID, TEST_INDEX_ID, TEST_NEW_VALUE_VALID, 0xF7
    };

    const uint8_t setValidUserErrorInvalid[13] =
    {
        //set block 3, section 3, index 5
        //valid new value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_set, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_ID, (TEST_INDEX_ID+1), TEST_NEW_VALUE_VALID, 0xF7
    };

    const uint8_t setInvalid[13] =
    {
        //set block 3, section 3, index 5
        //invalid new value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_set, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_ID, TEST_INDEX_ID, TEST_NEW_VALUE_INVALID, 0xF7
    };

    const uint8_t errorStatus[12] =
    {
        //get block 3, section 3, index 5
        //invalid status byte for request message
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, ACK, TEST_MSG_PART, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_ID, TEST_INDEX_ID, 0xF7
    };

    const uint8_t errorWish[12] =
    {
        //get block 3, section 3, index 5
        //wish byte is set to invalid value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, 0x04, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_ID, TEST_INDEX_ID, 0xF7
    };

    const uint8_t errorAmount[12] =
    {
        //get block 3, section 3, index 5
        //amount byte is set to invalid value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_get, 0x08, TEST_BLOCK_ID, TEST_SECTION_ID, TEST_INDEX_ID, 0xF7
    };

    const uint8_t errorBlock[12] =
    {
        //get block 3, section 3, index 5
        //block byte is set to invalid value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_get, sysExAmount_single, 0x41, TEST_SECTION_ID, TEST_INDEX_ID, 0xF7
    };

    const uint8_t errorSection[12] =
    {
        //get block 3, section 3, index 5
        //section byte is set to invalid value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, 0x61, TEST_INDEX_ID, 0xF7
    };

    const uint8_t errorIndex[12] =
    {
        //get block 3, section 3, index 5
        //index byte is set to invalid value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_ID, 0x7F, 0xF7
    };

    const uint8_t errorPart[12] =
    {
        //get block 3, section 3, index 5
        //part byte is set to invalid value
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x04, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_ID, TEST_INDEX_ID, 0xF7
    };

    const uint8_t errorLength[13] =
    {
        //get block 3, section 3, index 5
        //message is intentionally one byte too long
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, TEST_SECTION_ID, TEST_INDEX_ID, 0x00, 0xF7
    };

    const uint8_t customReq[8] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, CUSTOM_REQUEST_ID, 0xF7
    };

    const uint8_t customReqInvalid[8] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, 0x43, 0xF7
    };

    const uint8_t shortMessage1[6] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0xF7
    };

    const uint8_t shortMessage2[4] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, 0xF7
    };

    const uint8_t shortMessage3[10] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_get, sysExAmount_single, TEST_BLOCK_ID, 0xF7
    };

    const uint8_t getAllValid_1part[11] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_get, sysExAmount_all, TEST_BLOCK_ID, TEST_SECTION_ID, 0xF7
    };

    const uint8_t getAllValid_allParts_7F[11] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x7F, sysExWish_get, sysExAmount_all, 0x04, 0x01, 0xF7
    };

    const uint8_t getAllValid_allParts_7E[11] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x7E, sysExWish_get, sysExAmount_all, 0x04, 0x01, 0xF7
    };

    const uint8_t setAll[14] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x00, sysExWish_set, sysExAmount_all, 0x00, 0x01, 0x01, 0x01, 0x01, 0xF7
    };

    const uint8_t backupAll[11] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x7F, sysExWish_backup, sysExAmount_all, 0x00, 0x00, 0xF7
    };

    const uint8_t backupSingleInvPart[12] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x03, sysExWish_backup, sysExAmount_single, 0x00, 0x00, 0x00, 0xF7
    };

    const uint8_t getSingleUserError[12] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_get, sysExAmount_single, TEST_BLOCK_USER_ERROR_ID, TEST_SECTION_USER_ERROR_ID, TEST_INDEX_ID, 0xF7
    };

    const uint8_t getAllUserError[11] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_get, sysExAmount_all, TEST_BLOCK_USER_ERROR_ID, TEST_SECTION_USER_ERROR_ID, 0xF7
    };

    const uint8_t setAllParts[15] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x7F, sysExWish_set, sysExAmount_all, 0x00, 0x01, 0x01, 0x01, 0x01, 0xF7
    };

    const uint8_t getSpecialReqBytesPerVal[8] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, 0x00, 0x00, 0x02, 0xF7
    };

    const uint8_t getSpecialReqParamPerMsg[8] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, 0x00, 0x00, 0x03, 0xF7
    };

    const uint8_t getSpecialReqCloseConf[8] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, 0x00, 0x00, 0x00, 0xF7
    };

    const uint8_t setNoMinMax1[13] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_set, sysExAmount_single, 0x01, 0x00, TEST_INDEX_ID, 0, 0xF7
    };

    const uint8_t setNoMinMax2[13] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_set, sysExAmount_single, 0x01, 0x00, TEST_INDEX_ID, 50, 0xF7
    };

    const uint8_t setNoMinMax3[13] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_set, sysExAmount_single, 0x01, 0x00, TEST_INDEX_ID, 127, 0xF7
    };

    const uint8_t setSingleInvalidParam[13] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_set, sysExAmount_single, 0x00, 0x00, 35, 0x00, 0xF7
    };

    const uint8_t setAllnvalidNewVal[15] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_set, sysExAmount_all, 0x00, 0x00, 0x05, 0x05, 0x05, 0x05, 0xF7
    };

    const uint8_t setSingleUserError[13] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_set, sysExAmount_single, 0x00, 0x00, 0x0, 0x00, 0xF7
    };

    const uint8_t setAllUserError[15] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_set, sysExAmount_all, 0x00, 0x00, 0x0, 0x00, 0x00, 0x00, 0xF7
    };

    const uint8_t setAllWriteError[13] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, TEST_MSG_PART, sysExWish_set, sysExAmount_all, 0x01, 0x03, 0x1, 0x01, 0xF7
    };

    const uint8_t setAllMoreParts1[43] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x00, sysExWish_set, sysExAmount_all, 0x01, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xF7
    };

    const uint8_t setAllMoreParts2[12] =
    {
        0xF0, SYS_EX_M_ID_0, SYS_EX_M_ID_1, SYS_EX_M_ID_2, REQUEST, 0x01, sysExWish_set, sysExAmount_all, 0x01, 0x04, 0x01, 0xF7
    };
};

TEST_F(SysExTest, Init)
{
    sysEx.init();
    sysEx.setHandleGet(onGet);
    sysEx.setHandleSet(onSet);
    sysEx.setHandleCustomRequest(onCustom);
    sysEx.setHandleSysExWrite(writeSysEx);

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
    EXPECT_EQ(true, sysEx.configurationEnabled());
}

TEST_F(SysExTest, ErrorCheckStatus)
{
    uint8_t arraySize = sizeof(errorStatus)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorStatus, arraySize);

    //send message with invalid status byte
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status error is set
    EXPECT_EQ(ERROR_STATUS, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorCheckWish)
{
    uint8_t arraySize = sizeof(errorWish)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorWish, arraySize);

    //send message with invalid wish byte
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if wish error is set
    EXPECT_EQ(ERROR_WISH, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorCheckAmount)
{
    uint8_t arraySize = sizeof(errorAmount)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorAmount, arraySize);

    //send message with invalid amount byte
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if amount error is set
    EXPECT_EQ(ERROR_AMOUNT, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorCheckBlock)
{
    uint8_t arraySize = sizeof(errorBlock)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorBlock, arraySize);

    //send message with invalid block byte
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if block error is set
    EXPECT_EQ(ERROR_BLOCK, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorCheckSection)
{
    uint8_t arraySize = sizeof(errorSection)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorSection, arraySize);

    //send message with invalid section byte
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if section error is set
    EXPECT_EQ(ERROR_SECTION, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorCheckPart)
{
    uint8_t arraySize = sizeof(errorPart)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorPart, arraySize);

    //send message with invalid index byte
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if part error is set
    EXPECT_EQ(ERROR_PART, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorCheckIndex)
{
    uint8_t arraySize = sizeof(errorIndex)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorIndex, arraySize);

    //send message with invalid index byte
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if index error is set
    EXPECT_EQ(ERROR_INDEX, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorCheckLength)
{
    uint8_t arraySize = sizeof(errorLength)/sizeof(uint8_t);
    memcpy(sysExTestArray, errorLength, arraySize);

    //send message with invalid index byte
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if message length error is set
    EXPECT_EQ(ERROR_MESSAGE_LENGTH, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorHandshake)
{
    sysEx.init();
    sysEx.setHandleGet(onGet);
    sysEx.setHandleSet(onSet);
    sysEx.setHandleCustomRequest(onCustom);
    sysEx.setHandleSysExWrite(writeSysEx);

    uint8_t arraySize = sizeof(getValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, getValid, arraySize);

    //send valid get message
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if handshake error error is set
    EXPECT_EQ(ERROR_HANDSHAKE, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorCheckWrite)
{
    uint8_t arraySize = sizeof(setValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setValid, arraySize);

    //configure callback which always returns false on setting value
    sysEx.setHandleSet(onSetInvalid);

    //send valid set message
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if write error is set
    EXPECT_EQ(ERROR_WRITE, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ErrorNewValue)
{
    uint8_t arraySize = sizeof(setInvalid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setInvalid, arraySize);

    //send invalid set message
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if write error is set
    EXPECT_EQ(ERROR_NEW_VALUE, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ValidSet)
{
    uint8_t arraySize = sizeof(setValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setValid, arraySize);

    //send valid set message
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte has ACK value
    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, ValidGet)
{
    uint8_t arraySize = sizeof(getValid)/sizeof(uint8_t);
    memcpy(sysExTestArray, getValid, arraySize);

    //send valid get message
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte has ACK value
    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    //check if sysExTestArray equals value we're expecting
    EXPECT_EQ(TEST_VALUE_GET, sysExTestArray[6]);
}

TEST_F(SysExTest, CustomReq)
{
    //define custom request
    sysEx.addCustomRequest(CUSTOM_REQUEST_ID);

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

    //send invalid custom request message
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte has error wish value
    EXPECT_EQ(ERROR_WISH, sysExTestArray[(uint8_t)statusByte]);

    //init sysex instance so that configuration is disabled
    sysEx.init();

    //define custom request
    sysEx.addCustomRequest(CUSTOM_REQUEST_ID);

    //define callbacks
    sysEx.setHandleGet(onGet);
    sysEx.setHandleSet(onSet);
    sysEx.setHandleCustomRequest(onCustom);
    sysEx.setHandleSysExWrite(writeSysEx);

    arraySize = sizeof(customReq)/sizeof(uint8_t);
    memcpy(sysExTestArray, customReq, arraySize);

    //send valid custom request message
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte has error handshake value
    EXPECT_EQ(ERROR_HANDSHAKE, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, TooManyCustomReqsValid)
{
    bool value;

    for (int i=0; i<=MAX_CUSTOM_REQUESTS; i++)
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

TEST_F(SysExTest, TooManyCustomReqsInvalid)
{
    bool value;

    for (int i=0; i<SPECIAL_PARAMETERS; i++)
    {
        value = sysEx.addCustomRequest(i);
        //function must return false every time because
        //invalid values are being assigned as custom requests
        EXPECT_EQ(false, value);
    }
}

TEST_F(SysExTest, TooManyBlocks)
{
    bool value = sysEx.addBlocks(SYSEX_MAX_BLOCKS+1);
    //function must return false
    EXPECT_EQ(false, value);

    //try to add section to non-existing block
    sysExSection testSection;
    value = sysEx.addSection(SYSEX_MAX_BLOCKS, testSection);
    EXPECT_EQ(false, value);
}

TEST_F(SysExTest, TooManySections)
{
    sysExSection testSection;

    for (int i=0; i<SYSEX_MAX_SECTIONS; i++)
    {
        sysEx.addSection(0, testSection);
    }

    bool value = sysEx.addSection(0, testSection);
    //function must return false
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

    arraySize = sizeof(getInvalidIDs)/sizeof(uint8_t);
    memcpy(sysExTestArray, getInvalidIDs, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);
    //if no action took place, sysExTestArray counter should be unchanged
    EXPECT_EQ(tempResponseCounter, responseCounter);

    arraySize = sizeof(shortMessage3)/sizeof(uint8_t);
    memcpy(sysExTestArray, shortMessage3, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);
    EXPECT_EQ(ERROR_MESSAGE_LENGTH, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, GetAllValid)
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

TEST_F(SysExTest, CustomMessage)
{
    //construct custom message and see if output matches
    sysEx.startResponse();
    sysEx.addToResponse(0x05);
    sysEx.addToResponse(0x06);
    sysEx.addToResponse(0x07);
    sysEx.sendResponse();

    EXPECT_EQ(0x05, sysExTestArray[6]);
    EXPECT_EQ(0x06, sysExTestArray[7]);
    EXPECT_EQ(0x07, sysExTestArray[8]);
}

TEST_F(SysExTest, SetAll)
{
    uint8_t arraySize = sizeof(setAll)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAll, arraySize);

    //send set all request and check if result is ACK
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte has ACK value
    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    arraySize = sizeof(setAllWriteError)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllWriteError, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ERROR_WRITE, sysExTestArray[(uint8_t)statusByte]);

    //send set all message for section with more parts
    arraySize = sizeof(setAllMoreParts1)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllMoreParts1, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    arraySize = sizeof(setAllMoreParts2)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllMoreParts2, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);
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

TEST_F(SysExTest, UserError)
{
    uint8_t arraySize = sizeof(getSingleUserError)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSingleUserError, arraySize);

    //send get request
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte is set to NOT_SUPPORTTED value
    EXPECT_EQ(ERROR_NOT_SUPPORTED, sysExTestArray[(uint8_t)statusByte]);

    //get all request
    arraySize = sizeof(getAllUserError)/sizeof(uint8_t);
    memcpy(sysExTestArray, getAllUserError, arraySize);

    //send get request
    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte is set to NOT_SUPPORTTED value
    EXPECT_EQ(ERROR_NOT_SUPPORTED, sysExTestArray[(uint8_t)statusByte]);

    //set single request
    arraySize = sizeof(setSingleUserError)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleUserError, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte is set to NOT_SUPPORTTED value
    EXPECT_EQ(ERROR_NOT_SUPPORTED, sysExTestArray[(uint8_t)statusByte]);

    //set all request
    arraySize = sizeof(setAllUserError)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllUserError, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //check if status byte is set to NOT_SUPPORTTED value
    EXPECT_EQ(ERROR_NOT_SUPPORTED, sysExTestArray[(uint8_t)statusByte]);

    //try to set invalid user error
    //response should be just ERROR_WRITE in this case

    arraySize = sizeof(setValidUserErrorInvalid)/sizeof(uint8_t);
    memcpy(sysExTestArray, setValidUserErrorInvalid, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ERROR_WRITE, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, SetAllParts)
{
    //send set all requests for all parts and verify that status byte is set to ERROR_PART
    uint8_t arraySize = sizeof(setAllParts)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllParts, arraySize);

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
    sysEx.init();
    sysEx.setHandleGet(onGet);
    sysEx.setHandleSet(onSet);
    sysEx.setHandleCustomRequest(onCustom);
    sysEx.setHandleSysExWrite(writeSysEx);

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
    EXPECT_EQ(1, sysEx.configurationEnabled());

    //close conf again
    arraySize = sizeof(getSpecialReqCloseConf)/sizeof(uint8_t);
    memcpy(sysExTestArray, getSpecialReqCloseConf, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    //status must now be ACK
    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, SetNoMinMax)
{
    //test block which has same min and max value
    //in this case, error_newvalue should never be reported on any value
    uint8_t arraySize = sizeof(setNoMinMax1)/sizeof(uint8_t);
    memcpy(sysExTestArray, setNoMinMax1, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    arraySize = sizeof(setNoMinMax2)/sizeof(uint8_t);
    memcpy(sysExTestArray, setNoMinMax2, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);

    arraySize = sizeof(setNoMinMax3)/sizeof(uint8_t);
    memcpy(sysExTestArray, setNoMinMax3, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ACK, sysExTestArray[(uint8_t)statusByte]);
}

TEST_F(SysExTest, SetInvalid)
{
    //send set single command with invalid param index
    uint8_t arraySize = sizeof(setSingleInvalidParam)/sizeof(uint8_t);
    memcpy(sysExTestArray, setSingleInvalidParam, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ERROR_INDEX, sysExTestArray[(uint8_t)statusByte]);

    //now try set all with invalid value
    arraySize = sizeof(setAllnvalidNewVal)/sizeof(uint8_t);
    memcpy(sysExTestArray, setAllnvalidNewVal, arraySize);

    sysEx.handleMessage((uint8_t*)sysExTestArray, arraySize);

    EXPECT_EQ(ERROR_NEW_VALUE, sysExTestArray[(uint8_t)statusByte]);
}