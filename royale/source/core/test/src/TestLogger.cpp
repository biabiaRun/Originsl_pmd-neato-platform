/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <common/exceptions/Exception.hpp>
#include <common/RoyaleLogger.hpp>
#include <common/MakeUnique.hpp>

#include <regex>
#include <thread>

using ::testing::AtLeast;
using ::testing::_;
namespace
{
    void logFromThread (int tid)
    {
        LOG (DEBUG) << "Thread " << tid;
    }

    /**
    * If the option is true, throws. If it's false, returns normally (this isn't used, it's just to
    * avoid compiler warnings that the function can never return normally).
    */
    int throwsInsteadOfReturning (bool doThrow)
    {
        if (doThrow)
        {
            throw royale::common::Exception ("testing throwing an exception during a log");
        }
        return 0;
    }
}

TEST (TestLogger, Threads)
{
    // Save the pointer to the current cout buffer
    std::streambuf *coutbuf = std::cout.rdbuf();

    // Redirect log output to our own buffer
    std::stringstream newOutStream;
    std::cout.rdbuf (newOutStream.rdbuf());

    LOG (DEBUG) << "Test output";
    LOG (DEBUG) << 56u;

    // This test only makes sense if logging is enabled
    if (newOutStream.str().empty())
    {
        // Reset the cout buffer
        std::cout.rdbuf (coutbuf);

        return;
    }
    newOutStream.str (std::string());
    newOutStream.clear();

    const auto numThreads = 9u;
    std::thread threadArray[numThreads];

    // Launch a number of threads that all output some log
    for (auto i = 0u; i < numThreads; ++i)
    {
        threadArray[i] = std::thread (logFromThread, i);

    }

    for (auto i = 0u; i < numThreads; ++i)
    {
        threadArray[i].join();
    }

    auto outputString = newOutStream.str();

    // Parse the log output so that every log message
    // is in a separate string
    std::size_t pos;
    std::vector<std::string> messages;
    while ( (pos = outputString.find ('\n')) != std::string::npos)
    {
        auto curMsg = outputString.substr (0u, pos);
        outputString.erase (0u, pos + 1u);
        messages.push_back (curMsg);
    }

    EXPECT_EQ (numThreads, messages.size());

    // The threads' logging might happen in any order, although it's often in the same order as the
    // threads were created. The next loop loops over the logs in the order that they were received,
    // the countLogs vector is indexed by which thread did the logging.
    auto countLogs = std::vector<int> (numThreads, 0);
    auto i = 0u;
    for (auto curMsg : messages)
    {
        pos = curMsg.find (']');
        ASSERT_NE (std::string::npos, pos);
        auto logText = curMsg.substr (pos + 1, curMsg.length () - (pos + 1) - 1);
        EXPECT_STREQ (" DEBUG Thread ", logText.c_str ());
        auto logNumText = curMsg.substr (curMsg.length() - 1, 1);
        auto logNum = atoi (logNumText.c_str());
        ASSERT_GE (logNum, 0);
        ASSERT_LE (static_cast<unsigned> (logNum), numThreads);
        ++countLogs.at (logNum);
        ++i;
    }

    // Check that we received exactly one log from each thread
    auto expectedCounts = std::vector<int> (numThreads, 1);
    EXPECT_EQ (expectedCounts, countLogs);

    // Reset the cout buffer
    std::cout.rdbuf (coutbuf);
}

/**
* Check that an exception can be thrown during logging. Remember that Royale exceptions do a LOG in
* their constructor, so needs either a recursive lock or to detect the problem in the LOG
* implementation.
*
* NOTE: this test can deadlock when it fails.
*/
TEST (TestLogger, ExceptionDuringLog)
{
    try
    {
        LOG (WARN) << "Test " << throwsInsteadOfReturning (true);
    }
    catch (...)
    {

    }
}

/**
* Ensures that a later LOG succeeds, even after an exception is thrown during a LOG.
*
* NOTE: this test can deadlock when it fails.
*/
TEST (TestLogger, LogAfterException)
{
    try
    {
        LOG (WARN) << "Test " << throwsInsteadOfReturning (true);
    }
    catch (...)
    {

    }
    LOG (WARN) << "Test";
}

TEST (TestLogger, LogBackend)
{
    std::unique_ptr<IlogBackend> backend;
    std::vector<uint16_t> logLevels{ 0, 1, 2, 4, 8 };

    ASSERT_EQ (backend->logLevToString (logLevels.at (0)), "NONE");
    ASSERT_EQ (backend->logLevToString (logLevels.at (1)), "INFO");
    ASSERT_EQ (backend->logLevToString (logLevels.at (2)), "DEBUG");
    ASSERT_EQ (backend->logLevToString (logLevels.at (3)), "WARN");
    ASSERT_EQ (backend->logLevToString (logLevels.at (4)), "ERROR");
    ASSERT_EQ (backend->logLevToString (5), "UNKNOWN");

}

TEST (TestLogger, CommandLogger)
{
    std::unique_ptr<IlogBackend> backend = royale::common::makeUnique<CommandLog>();
    std::streambuf *coutbuf = std::cout.rdbuf();
    std::stringstream newOutStream;
    std::cout.rdbuf (newOutStream.rdbuf());

    backend->printLogLine (2, "test started");
    std::regex logline ("(\\[.*?\\]\\s+DEBUG\\s+test\\s+started\n)");

    ASSERT_TRUE (std::regex_match (newOutStream.str(), logline));

    std::cout.rdbuf (coutbuf);
}

class MockLogBackend : public IlogBackend
{
public:
    MOCK_METHOD2 (printLogLine, void (uint16_t loglevel, const std::string &logLine));
};

TEST (TestLogger, LogSettings)
{
    std::unique_ptr<MockLogBackend> backend = royale::common::makeUnique<MockLogBackend>();
    royale::common::LogSettings *logsettings = royale::common::LogSettings::getInstance();

    EXPECT_CALL (*backend, printLogLine (_, "test started"))
    .Times (AtLeast (1));

    std::unique_ptr<IlogBackend> defaultBackend = logsettings->swapLogBackend (std::move (backend));
    logsettings->pushLogLine (2, "test started");
    logsettings->swapLogBackend (std::move (defaultBackend));
}
