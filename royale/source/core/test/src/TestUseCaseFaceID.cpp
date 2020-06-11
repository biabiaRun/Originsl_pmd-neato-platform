/****************************************************************************\
* Copyright (C) 2020 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <gtest/gtest.h>
#include <royale/Status.hpp>
#include <usecase/UseCaseFaceID.hpp>

using namespace royale::usecase;

TEST (TestUseCaseFaceID, create)
{
    UseCaseFaceID simple (UseCaseIdentifier ("FaceID"), 45u, 80320000u, 80320000u, 80320000u,
    { 1u, 1000u }, { 1u, 1000u }, { 1u, 1000u }, 1000u, 1000u, 1000u);
    ASSERT_EQ (4u, simple.getRawFrameSets().size());
    ASSERT_EQ (7u, simple.getRawFrameCount());
    ASSERT_EQ (2u, simple.getStreamIds().size());
}
