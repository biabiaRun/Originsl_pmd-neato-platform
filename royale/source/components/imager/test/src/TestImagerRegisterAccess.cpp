/****************************************************************************\
* Copyright (C) 2019 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <common/exceptions/InvalidValue.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/exceptions/Timeout.hpp>
#include <common/NarrowCast.hpp>
#include <imager/ImagerRegisterAccess.hpp>

using namespace royale::common;
using namespace royale::imager;
using namespace testing;

namespace
{
    class MockBridgeImager : public royale::hal::IBridgeImager
    {
    public:
        MOCK_METHOD1 (setImagerReset, void (bool));
        MOCK_METHOD2 (readImagerRegister, void (uint16_t, uint16_t &));
        MOCK_METHOD2 (writeImagerRegister, void (uint16_t, uint16_t));
        MOCK_METHOD2 (readImagerBurst, void (uint16_t, std::vector<uint16_t> &));
        MOCK_METHOD2 (writeImagerBurst, void (uint16_t, const std::vector<uint16_t> &));
        MOCK_METHOD1 (sleepFor, void (std::chrono::microseconds));
    };
}

/**
 * Merely creating an ImagerRegisterAccess should not cause any accesses to the imager.
 */
TEST (TestImagerRegisterAccess, DoesNotAccessImagerWithoutCalls)
{
    auto bridge = std::make_shared<MockBridgeImager> ();
    EXPECT_CALL (*bridge, setImagerReset (_)).Times (0);
    EXPECT_CALL (*bridge, sleepFor (_)).Times (0);
    EXPECT_CALL (*bridge, readImagerRegister (_, _)).Times (0);
    EXPECT_CALL (*bridge, writeImagerRegister (_, _)).Times (0);
    EXPECT_CALL (*bridge, readImagerBurst (_, _)).Times (0);
    EXPECT_CALL (*bridge, writeImagerBurst (_, _)).Times (0);

    ImagerRegisterAccess access {bridge};
}

TEST (TestImagerRegisterAccess, CombinesWrites)
{
    auto bridge = std::make_shared<MockBridgeImager> ();
    EXPECT_CALL (*bridge, setImagerReset (_)).Times (0);
    EXPECT_CALL (*bridge, sleepFor (_)).Times (0);
    EXPECT_CALL (*bridge, readImagerRegister (_, _)).Times (0);
    EXPECT_CALL (*bridge, writeImagerRegister (_, _)).Times (0);
    EXPECT_CALL (*bridge, readImagerBurst (_, _)).Times (0);
    EXPECT_CALL (*bridge, writeImagerBurst (_, _)).Times (1);

    ImagerRegisterAccess access {bridge};
    std::vector<uint16_t> addresses = {0xA000, 0xA001};
    std::vector<uint16_t> values = {0, 0};
    access.writeRegisters (addresses, values);
}

TEST (TestImagerRegisterAccess, SplitsWrites)
{
    auto bridge = std::make_shared<MockBridgeImager> ();
    EXPECT_CALL (*bridge, setImagerReset (_)).Times (0);
    EXPECT_CALL (*bridge, sleepFor (_)).Times (0);
    EXPECT_CALL (*bridge, readImagerRegister (_, _)).Times (0);
    EXPECT_CALL (*bridge, writeImagerRegister (_, _)).Times (2);
    EXPECT_CALL (*bridge, readImagerBurst (_, _)).Times (0);
    EXPECT_CALL (*bridge, writeImagerBurst (_, _)).Times (0);

    ImagerRegisterAccess access {bridge};
    std::vector<uint16_t> addresses = {0xA000, 0xA002};
    std::vector<uint16_t> values = {0, 0};
    access.writeRegisters (addresses, values);
}

TEST (TestImagerRegisterAccess, CombinesReads)
{
    auto bridge = std::make_shared<MockBridgeImager> ();
    EXPECT_CALL (*bridge, setImagerReset (_)).Times (0);
    EXPECT_CALL (*bridge, sleepFor (_)).Times (0);
    EXPECT_CALL (*bridge, readImagerRegister (_, _)).Times (0);
    EXPECT_CALL (*bridge, writeImagerRegister (_, _)).Times (0);
    EXPECT_CALL (*bridge, readImagerBurst (_, _)).Times (1);
    EXPECT_CALL (*bridge, writeImagerBurst (_, _)).Times (0);

    ImagerRegisterAccess access {bridge};
    std::vector<uint16_t> addresses = {0xA000, 0xA001};
    std::vector<uint16_t> values = {0, 0};
    access.readRegisters (addresses, values);
}

TEST (TestImagerRegisterAccess, SplitsReads)
{
    auto bridge = std::make_shared<MockBridgeImager> ();
    EXPECT_CALL (*bridge, setImagerReset (_)).Times (0);
    EXPECT_CALL (*bridge, sleepFor (_)).Times (0);
    EXPECT_CALL (*bridge, readImagerRegister (_, _)).Times (2);
    EXPECT_CALL (*bridge, writeImagerRegister (_, _)).Times (0);
    EXPECT_CALL (*bridge, readImagerBurst (_, _)).Times (0);
    EXPECT_CALL (*bridge, writeImagerBurst (_, _)).Times (0);

    ImagerRegisterAccess access {bridge};
    std::vector<uint16_t> addresses = {0xA000, 0xA002};
    std::vector<uint16_t> values = {0, 0};
    access.readRegisters (addresses, values);
}

TEST (TestImagerRegisterAccess, TransferRegisterMapAuto)
{
    const auto sleepTime = std::chrono::microseconds {3};
    const auto regs = TimedRegisterList
    {
        {
            {0xA000, 0, 0},
            {0xA001, 0, 0},
            {0xA003, 0, 0},
            {0xA005, 0, 0},
            {0xA006, 0, static_cast<uint32_t> (sleepTime.count()) },
            {0xA007, 0, 0}
        }
    };

    // In this sequence, the writes to 0xA003 and 0xA007 could use either writeImagerBurst or
    // writeImagerRegister; the current implementation uses writeImagerBurst.
    InSequence inSequence;
    auto bridge = std::make_shared<MockBridgeImager> ();
    EXPECT_CALL (*bridge, writeImagerBurst (0xA000, _)).Times (1);
    EXPECT_CALL (*bridge, writeImagerBurst (0xA003, _)).Times (1);
    EXPECT_CALL (*bridge, writeImagerBurst (0xA005, _)).Times (1);
    EXPECT_CALL (*bridge, sleepFor (sleepTime)).Times (1);
    EXPECT_CALL (*bridge, writeImagerBurst (0xA007, _)).Times (1);

    ImagerRegisterAccess access {bridge};
    access.transferRegisterMapAuto (regs);
}

TEST (TestImagerRegisterAccess, PollSucceedsFirstTime)
{
    const auto firstSleep = std::chrono::microseconds {3};
    const auto pollSleep = std::chrono::microseconds {7};
    const auto valFinished = uint16_t {27};
    auto bridge = std::make_shared<MockBridgeImager> ();
    EXPECT_CALL (*bridge, setImagerReset (_)).Times (0);
    EXPECT_CALL (*bridge, sleepFor (_)).Times (0);
    EXPECT_CALL (*bridge, readImagerRegister (_, _)).Times (0);
    EXPECT_CALL (*bridge, writeImagerRegister (_, _)).Times (0);
    EXPECT_CALL (*bridge, readImagerBurst (_, _)).Times (0);
    EXPECT_CALL (*bridge, writeImagerBurst (_, _)).Times (0);

    InSequence inSequence;
    EXPECT_CALL (*bridge, sleepFor (firstSleep)).Times (1);
    EXPECT_CALL (*bridge, readImagerRegister (0xA000, _)).Times (1).WillRepeatedly (SetArgReferee<1> (valFinished));

    ImagerRegisterAccess access {bridge};
    access.pollUntil (0xA000, valFinished, firstSleep, pollSleep);
}

TEST (TestImagerRegisterAccess, PollSucceedsSecondTime)
{
    const auto firstSleep = std::chrono::microseconds (3);
    const auto pollSleep = std::chrono::microseconds (7);
    const auto valFinished = uint16_t {31};
    auto bridge = std::make_shared<MockBridgeImager> ();
    EXPECT_CALL (*bridge, setImagerReset (_)).Times (0);
    EXPECT_CALL (*bridge, sleepFor (_)).Times (0);
    EXPECT_CALL (*bridge, readImagerRegister (_, _)).Times (0);
    EXPECT_CALL (*bridge, writeImagerRegister (_, _)).Times (0);
    EXPECT_CALL (*bridge, readImagerBurst (_, _)).Times (0);
    EXPECT_CALL (*bridge, writeImagerBurst (_, _)).Times (0);

    InSequence inSequence;
    EXPECT_CALL (*bridge, sleepFor (firstSleep)).Times (1);
    EXPECT_CALL (*bridge, readImagerRegister (0xA000, _)).WillOnce (SetArgReferee<1> (uint16_t{1}));
    EXPECT_CALL (*bridge, sleepFor (pollSleep)).Times (1);
    EXPECT_CALL (*bridge, readImagerRegister (0xA000, _)).WillRepeatedly (SetArgReferee<1> (valFinished));

    ImagerRegisterAccess access {bridge};
    access.pollUntil (0xA000, valFinished, firstSleep, pollSleep);
}

TEST (TestImagerRegisterAccess, PollTimesOut)
{
    const auto firstSleep = std::chrono::microseconds (3);
    const auto pollSleep = std::chrono::microseconds (7);
    const auto valFinished = uint16_t {0};
    auto bridge = std::make_shared<MockBridgeImager> ();
    EXPECT_CALL (*bridge, setImagerReset (_)).Times (0);
    EXPECT_CALL (*bridge, writeImagerRegister (_, _)).Times (0);
    EXPECT_CALL (*bridge, readImagerBurst (_, _)).Times (0);
    EXPECT_CALL (*bridge, writeImagerBurst (_, _)).Times (0);

    Sequence pollSequence;
    EXPECT_CALL (*bridge, sleepFor (firstSleep)).InSequence (pollSequence);
    EXPECT_CALL (*bridge, readImagerRegister (0xA000, _)).InSequence (pollSequence).WillOnce (SetArgReferee<1> (uint16_t{1}));
    EXPECT_CALL (*bridge, sleepFor (pollSleep)).InSequence (pollSequence);
    EXPECT_CALL (*bridge, readImagerRegister (0xA000, _)).InSequence (pollSequence).WillOnce (SetArgReferee<1> (uint16_t{1}));
    EXPECT_CALL (*bridge, sleepFor (pollSleep)).InSequence (pollSequence);
    EXPECT_CALL (*bridge, readImagerRegister (0xA000, _)).InSequence (pollSequence).WillOnce (SetArgReferee<1> (uint16_t{1}));
    EXPECT_CALL (*bridge, sleepFor (pollSleep)).InSequence (pollSequence);
    EXPECT_CALL (*bridge, readImagerRegister (0xA000, _)).InSequence (pollSequence).WillOnce (SetArgReferee<1> (uint16_t{1}));
    EXPECT_CALL (*bridge, sleepFor (pollSleep)).InSequence (pollSequence);
    EXPECT_CALL (*bridge, readImagerRegister (0xA000, _)).InSequence (pollSequence).WillOnce (SetArgReferee<1> (uint16_t{1}));
    // This hardcodes the exact number of retries, and will break if ImagerRegisterAccess's
    // MAX_POLL_RETRIES changes.

    ImagerRegisterAccess access {bridge};
    EXPECT_THROW (access.pollUntil (0xA000, valFinished, firstSleep, pollSleep), Timeout);
}
