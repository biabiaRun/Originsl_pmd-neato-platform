/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <LenaTestUtils.hpp>

royale::config::ExternalConfigFileConfig royale::test::utils::getNvsfGetterForLenaFile()
{
#ifndef IMAGER_TEST_FILE_PATH
#error The test environment needs to define IMAGER_TEST_FILE_PATH via CMake
#endif
    return royale::config::ExternalConfigFileConfig::fromLenaFile (IMAGER_TEST_FILE_PATH "/C2_M2452_B1x.lena");
}
