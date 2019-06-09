/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#include <gmock/gmock.h>
#include <vector>
#include <common/WeightedAverage.hpp>

using namespace royale::common;
using testing::_;
using testing::AtLeast;
using testing::Return;
using testing::Eq;
using testing::FloatEq;

TEST (AWeightedAverage, calculatesWeightedAverage)
{
    auto alpha = 0.03f;
    auto beta = 1.0f - alpha;
    auto wmAverage = WeightedAverage{alpha};
    EXPECT_THAT (wmAverage.calc (5.0f), FloatEq (5.0f));
    EXPECT_THAT (wmAverage.calc (6.0f), FloatEq (alpha * 6.0f + beta * 5.0f));
    EXPECT_THAT (wmAverage.calc (7.0f), FloatEq (alpha * 7.0f + beta * (alpha * 6.0f + beta * 5.0f)));
    EXPECT_THAT (wmAverage.calc (8.0f), FloatEq (alpha * 8.0f + beta * (alpha * 7.0f + beta * (alpha * 6.0f + beta * 5.0f))));
}
