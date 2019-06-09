#include <gtest/gtest.h>
#include <royale/Status.hpp>

using namespace royale;

TEST (TestStatus, GetErrorMessage)
{
    EXPECT_STREQ (royale::getStatusString (CameraStatus::SUCCESS).c_str(), "No error");
    EXPECT_STREQ (royale::getStatusString (CameraStatus::UNKNOWN).c_str(), "An unknown error occurred");
    EXPECT_STREQ (royale::getStatusString (CameraStatus::COULD_NOT_OPEN).c_str(), "Cannot open resource");
}
