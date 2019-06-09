/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <gtest/gtest.h>

#include <imager/ImagerUseCaseDefinition.hpp>
#include <factory/ImagerUseCaseDefinitionAdapter.hpp>

#include <usecase/UseCaseEightPhase.hpp>

using namespace royale::factory;
using namespace royale::usecase;


TEST (TestImagerUseCaseDefinition, tailTime)
{
    {
        UseCaseEightPhase ucd (5u, 30000000, 20200000, { 200u, 1000u }, 1000u, 1000u, 1000u);
        ImagerUseCaseDefinitionAdapter iucd (ucd, 0, 0, 0);
        EXPECT_EQ (std::chrono::milliseconds (191u), iucd.getTailTime());
    }
    {
        UseCaseEightPhase ucd (5u, 30000000, 20200000, { 200u, 2000u }, 2000u, 2000u, 2000u);
        ImagerUseCaseDefinitionAdapter iucd (ucd, 0, 0, 0);
        EXPECT_EQ (std::chrono::milliseconds (182u), iucd.getTailTime());
    }
    {
        UseCaseEightPhase ucd (10u, 30000000, 20200000, { 200u, 1000u }, 1000u, 1000u, 1000u);
        ImagerUseCaseDefinitionAdapter iucd (ucd, 0, 0, 0);
        EXPECT_EQ (std::chrono::milliseconds (91u), iucd.getTailTime());
    }
}
