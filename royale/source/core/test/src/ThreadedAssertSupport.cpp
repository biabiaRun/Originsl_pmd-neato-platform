/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <ThreadedAssertSupport.hpp>
#include <common/exceptions/Exception.hpp>
#include <gtest/gtest.h>

using namespace royaletest;
using namespace royale::common;

void ThreadedAssertSupport::checkForThreadedAssert()
{
    // Comparing to the empty string will show the assert reason in the GTest error
    std::unique_lock<std::mutex> lock {m_mutex};
    std::string empty;
    ASSERT_EQ (empty, m_assertReason);
}

void ThreadedAssertSupport::fail (const std::string &reason)
{
    std::unique_lock<std::mutex> lock {m_mutex};
    if (m_assertReason.empty())
    {
        m_assertReason = reason;
    }
    throw Exception (reason);
}
