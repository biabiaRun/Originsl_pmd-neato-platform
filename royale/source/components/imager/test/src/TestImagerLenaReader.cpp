/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <gtest/gtest.h>

#include <imager/ImagerLenaReader.hpp>

#include <stdexcept>
#include <fstream>

using namespace royale::imager;

namespace
{
    //a file that contains valid Lena data, but no real-world meaning (created at run-time)
    const std::string testFileName = IMAGER_TEST_FILE_PATH "/TestImagerLenaReader.lena";

    //a file that contains invalid Lena data, but no real-world meaning (created at run-time)
    const std::string testInvFileName = IMAGER_TEST_FILE_PATH "/TestInvImagerLenaReader.lena";

    //some real-world Lena data (tested once before with a real camera)
    const std::string testM2452_B1xFileName = IMAGER_TEST_FILE_PATH "/C2_M2452_B1x.lena";

    //some valid Lena data that can be used to be stored to the file located at testFileName
    const std::string testFileData =
        R"(
ROYALE-IMAGER-LENA-FILE
VERSION;1003
INIT-MAP // comment
0x0123;0xAAAA;0 // comment 2
0x0124;0xAAAA;1000 # python comment
0x0125;0xAAAA;0 # python comments
INIT-MAP-END
FW-PAGE-1
0x0123;0xAAAA;0
FW-PAGE-1-END
FW-PAGE-2
0x0123;0xAAAA;0
FW-PAGE-2-END
FW-START-MAP
0x0123;0xAAAA;0 #set some fw-param
FW-START-MAP-END
START-MAP
0x0123;0xAAAA;0
START-MAP-END
STOP-MAP
0x0123;0xAAAA;0
STOP-MAP-END
USECASE-START
GUID;{69F0E9EA-0DFF-47C9-8311-E236F35AB6FF}
NAME;MyUseCase1
BLOCKS;9
MODFREQS;60240000;80320000;80320000;80320000;80320000;60240000;60240000;60240000;60240000
0x0123;0xAAAA;0
0x0124;0xAAAA;0
0x0125;0xAAAA;0
0x0126;0xAAAA;0
USECASE-END
USECASE-START
GUID;{020B6301-30B5-4E39-9F5F-92F7B4DF4A7D}
NAME;MyUseCase2
BLOCKS;4;4;1
MODFREQS;80320000;80320000;80320000;80320000;60240000;60240000;60240000;60240000;60240000
0x0123;0xAAAA;0
0x0124;0xAAAA;0
USECASE-END
)";

    //some invalid Lena data that can be used to be stored to the file located at testInvFileName
    const std::string testInvFileData =
        R"(
NOT-THE-MAGIC-WORD
)";
}

class TestImagerLenaReader : public ::testing::Test
{
protected:
    TestImagerLenaReader()
    {
        std::ofstream outputFile(testFileName);
        outputFile << testFileData;

        std::ofstream outputInvFile(testInvFileName);
        outputInvFile << testInvFileData;
    }

    virtual ~TestImagerLenaReader()
    {

    }

    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {

    }
};

TEST_F (TestImagerLenaReader, CreateImagerLenaReaderInvalidFile)
{
    ASSERT_THROW(ImagerLenaReader::fromFile(""), std::invalid_argument);
    ASSERT_THROW(ImagerLenaReader::fromFile(testInvFileName), std::logic_error);
}

TEST_F(TestImagerLenaReader, CreateImagerLenaReaderInvalidString)
{
    ASSERT_THROW(ImagerLenaReader::fromString(""), std::invalid_argument);
    ASSERT_THROW(ImagerLenaReader::fromString("no-magic-word"), std::logic_error);
}

TEST_F(TestImagerLenaReader, SupportsTheExtCfgInterface)
{
    auto reader = ImagerLenaReader::fromFile(testFileName);

    const auto extCfg = dynamic_cast<IImagerExternalConfig*> (reader.get());
    ASSERT_NE(nullptr, extCfg);
}

/**
 * ImagerLenaReader is now a factory that returns unique_ptr<IImagerExternalConfig>, for other tests
 * in this file it is cast back to a ILR*, so this test checks that the cast will work, and provides
 * a place for the next paragraph's comment.
 *
 * This is an implementation detail that could change during a refactor, in which case the other
 * tests that rely on this should change - the tests shouldn't block the refactor.
 */
TEST_F(TestImagerLenaReader, SupportsTheImagerLenaReaderInterface)
{
    auto reader = ImagerLenaReader::fromFile(testFileName);

    const auto ilr = dynamic_cast<ImagerLenaReader*> (reader.get());
    ASSERT_NE(nullptr, ilr);
}

TEST_F(TestImagerLenaReader, ReadImagerTestString)
{
    ASSERT_NO_THROW(ImagerLenaReader::fromString(testFileData));
}

TEST_F(TestImagerLenaReader, ReadImagerTestFile)
{
    ASSERT_NO_THROW(ImagerLenaReader::fromFile(testFileName));
    const auto reader = ImagerLenaReader::fromFile(testFileName);

    // scope for the ilr temporary
    {
        const auto ilr = dynamic_cast<ImagerLenaReader*> (reader.get());
        ASSERT_NE(nullptr, ilr);
        ASSERT_EQ(uint32_t{ 1003 }, ilr->getVersion());
    }
    
    const auto initMap = reader->getInitializationMap();
    const auto fwPage1 = reader->getFirmwarePage1();
    const auto fwPage2 = reader->getFirmwarePage2();
    const auto fwStartMap = reader->getFirmwareStartMap();
    const auto startMap = reader->getStartMap();
    const auto stopMap = reader->getStopMap();
    const auto useCaseList = reader->getUseCaseList();

    ASSERT_EQ(size_t{ 3 }, initMap.size());
    ASSERT_EQ(size_t{ 1 }, fwPage1.size());
    ASSERT_EQ(size_t{ 1 }, fwPage2.size());
    ASSERT_EQ(size_t{ 1 }, fwStartMap.size());
    ASSERT_EQ(size_t{ 1 }, startMap.size());
    ASSERT_EQ(size_t{ 1 }, stopMap.size());
    ASSERT_EQ(size_t{ 2 }, useCaseList.size());

    ASSERT_EQ(0x0123, initMap.at(0).address);
    ASSERT_EQ(0x0124, initMap.at(1).address);
    ASSERT_EQ(1000u, initMap.at(1).sleepTime);
    ASSERT_EQ(0x0125, initMap.at(2).address);
    ASSERT_EQ(0xAAAA, initMap.at(0).value);
    ASSERT_EQ(0xAAAA, initMap.at(1).value);
    ASSERT_EQ(0xAAAA, initMap.at(2).value);

    ASSERT_EQ(0x0123, startMap.at(0).address);
    ASSERT_EQ(0xAAAA, startMap.at(0).value);

    ASSERT_EQ(0x0123, stopMap.at(0).address);
    ASSERT_EQ(0xAAAA, stopMap.at(0).value);

    ASSERT_EQ(parseImagerUseCaseIdentifierGuidString ("{69F0E9EA-0DFF-47C9-8311-E236F35AB6FF}"), useCaseList.at(0).guid);
    ASSERT_EQ("MyUseCase1", useCaseList.at(0).name);
    ASSERT_EQ(size_t{ 1 }, useCaseList.at(0).imageStreamBlockSizes.size());
    ASSERT_EQ(size_t{ 9 }, useCaseList.at(0).imageStreamBlockSizes.at(0));
    ASSERT_EQ(size_t{ 9 }, useCaseList.at(0).modulationFrequencies.size());
    ASSERT_EQ(60240000u, useCaseList.at(0).modulationFrequencies.at(0));
    ASSERT_EQ(80320000u, useCaseList.at(0).modulationFrequencies.at(1));
    ASSERT_EQ(80320000u, useCaseList.at(0).modulationFrequencies.at(2));
    ASSERT_EQ(80320000u, useCaseList.at(0).modulationFrequencies.at(3));
    ASSERT_EQ(80320000u, useCaseList.at(0).modulationFrequencies.at(4));
    ASSERT_EQ(60240000u, useCaseList.at(0).modulationFrequencies.at(5));
    ASSERT_EQ(60240000u, useCaseList.at(0).modulationFrequencies.at(6));
    ASSERT_EQ(60240000u, useCaseList.at(0).modulationFrequencies.at(7));
    ASSERT_EQ(60240000u, useCaseList.at(0).modulationFrequencies.at(8));
    // This test data doesn't have a corresponding flashConfig
    ASSERT_TRUE(useCaseList.at(0).sequentialRegisterHeader.empty());
    ASSERT_EQ(uint32_t {0}, useCaseList.at(0).sequentialRegisterHeader.flashConfigAddress);
    ASSERT_EQ(size_t{ 0 }, useCaseList.at(0).sequentialRegisterHeader.flashConfigSize);
    
    const auto uc11RegMap = useCaseList.at(0).registerMap;
    ASSERT_EQ(0x0123, uc11RegMap.at(0).address);
    ASSERT_EQ(0x0124, uc11RegMap.at(1).address);
    ASSERT_EQ(0x0125, uc11RegMap.at(2).address);
    ASSERT_EQ(0x0126, uc11RegMap.at(3).address);
    ASSERT_EQ(0xAAAA, uc11RegMap.at(0).value);
    ASSERT_EQ(0xAAAA, uc11RegMap.at(1).value);
    ASSERT_EQ(0xAAAA, uc11RegMap.at(2).value);
    ASSERT_EQ(0xAAAA, uc11RegMap.at(3).value);
    ASSERT_EQ(0u, uc11RegMap.at(0).sleepTime);
    ASSERT_EQ(0u, uc11RegMap.at(1).sleepTime);
    ASSERT_EQ(0u, uc11RegMap.at(2).sleepTime);
    ASSERT_EQ(0u, uc11RegMap.at(3).sleepTime);

    ASSERT_EQ(parseImagerUseCaseIdentifierGuidString ("{020B6301-30B5-4E39-9F5F-92F7B4DF4A7D}"), useCaseList.at(1).guid);
    ASSERT_EQ("MyUseCase2", useCaseList.at(1).name);
    ASSERT_EQ(size_t{ 3 }, useCaseList.at(1).imageStreamBlockSizes.size());
    ASSERT_EQ(size_t{ 4 }, useCaseList.at(1).imageStreamBlockSizes.at(0));
    ASSERT_EQ(size_t{ 4 }, useCaseList.at(1).imageStreamBlockSizes.at(1));
    ASSERT_EQ(size_t{ 1 }, useCaseList.at(1).imageStreamBlockSizes.at(2));
    ASSERT_EQ(size_t{ 9 }, useCaseList.at(1).modulationFrequencies.size());
    ASSERT_EQ(80320000u, useCaseList.at(1).modulationFrequencies.at(0));
    ASSERT_EQ(80320000u, useCaseList.at(1).modulationFrequencies.at(1));
    ASSERT_EQ(80320000u, useCaseList.at(1).modulationFrequencies.at(2));
    ASSERT_EQ(80320000u, useCaseList.at(1).modulationFrequencies.at(3));
    ASSERT_EQ(60240000u, useCaseList.at(1).modulationFrequencies.at(4));
    ASSERT_EQ(60240000u, useCaseList.at(1).modulationFrequencies.at(5));
    ASSERT_EQ(60240000u, useCaseList.at(1).modulationFrequencies.at(6));
    ASSERT_EQ(60240000u, useCaseList.at(1).modulationFrequencies.at(7));
    ASSERT_EQ(60240000u, useCaseList.at(1).modulationFrequencies.at(8));

    const auto uc12RegMap = useCaseList.at(1).registerMap;
    ASSERT_EQ(0x0123, uc12RegMap.at(0).address);
    ASSERT_EQ(0x0124, uc12RegMap.at(1).address);
    ASSERT_EQ(0xAAAA, uc12RegMap.at(0).value);
    ASSERT_EQ(0xAAAA, uc12RegMap.at(1).value);
    ASSERT_EQ(0u, uc12RegMap.at(0).sleepTime);
    ASSERT_EQ(0u, uc12RegMap.at(1).sleepTime);
}

TEST_F(TestImagerLenaReader, ReadImagerRealFile)
{
    const auto reader = ImagerLenaReader::fromFile(testM2452_B1xFileName);

    // scope for the ilr temporary
    {
        const auto ilr = dynamic_cast<ImagerLenaReader*> (reader.get());
        ASSERT_NE(nullptr, ilr);
        ASSERT_EQ(uint32_t{ 1003 }, ilr->getVersion());
    }

    const auto initMap = reader->getInitializationMap();
    const auto startMap = reader->getStartMap();
    const auto stopMap = reader->getStopMap();
    const auto useCaseList = reader->getUseCaseList();

    ASSERT_EQ(size_t{ 2041 }, initMap.size());
    ASSERT_EQ(size_t{ 3 }, startMap.size());
    ASSERT_EQ(size_t{ 1 }, stopMap.size());
    ASSERT_EQ(size_t{ 1 }, useCaseList.size());

    const auto useCase = useCaseList.at(0);
    ASSERT_EQ(std::string{"SomeUseCaseJustForTest"}, useCase.name);
    ASSERT_EQ(parseImagerUseCaseIdentifierGuidString ("{40864D9B-F844-4CCB-9975-8CF820292B03}"), useCase.guid);
    ASSERT_EQ(size_t{ 1 }, useCase.imageStreamBlockSizes.size());
    ASSERT_EQ(size_t{ 9 }, useCase.imageStreamBlockSizes.at(0));

    // This test data doesn't have a corresponding flashConfig
    ASSERT_TRUE(useCaseList.at(0).sequentialRegisterHeader.empty());
    ASSERT_EQ(uint32_t{ 0 }, useCaseList.at(0).sequentialRegisterHeader.flashConfigAddress);
    ASSERT_EQ(size_t{ 0 }, useCaseList.at(0).sequentialRegisterHeader.flashConfigSize);

    ASSERT_EQ(size_t{ 41 }, useCase.registerMap.size());
}
