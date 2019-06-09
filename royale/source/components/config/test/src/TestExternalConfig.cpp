/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <config/ExternalConfigFileConfig.hpp>

#include <gtest/gtest.h>

using namespace royale::config;
using namespace ::testing;

TEST (TestExternalConfig, StorageConfigBooleans)
{
    ASSERT_FALSE (ExternalConfigFileConfig::empty());
    ASSERT_TRUE (ExternalConfigFileConfig::fromLenaFile ("a"));
    ASSERT_TRUE (ExternalConfigFileConfig::fromLenaString ("a"));
    ASSERT_TRUE (ExternalConfigFileConfig::fromZwetschgeFile ("a"));
}
