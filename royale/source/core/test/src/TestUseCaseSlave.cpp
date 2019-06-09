/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <gtest/gtest.h>
#include <royale/Status.hpp>
#include <usecase/UseCaseFourPhase.hpp>
#include <usecase/UseCaseSlave.hpp>
#include <memory>

using namespace royale::usecase;

TEST (TestUseCaseSlave, create)
{
    std::shared_ptr<UseCaseDefinition> simple = std::make_shared<UseCaseFourPhase> (45u, 30000000,
            royale::Pair<uint32_t, uint32_t> { 50u, 1000u }, 1000u, 0u);

    UseCaseSlave simpleSlave (*simple);

    // new_max_exposure = old_max_exposure * old_framerate / new_framerate
    uint32_t expectedExpoTime = static_cast<uint32_t> (1000.0f * (45.0f / 46.0f));

    ASSERT_EQ (46u, simpleSlave.getTargetRate ());

    auto streamIds = simpleSlave.getStreamIds();
    for (auto curId : streamIds)
    {
        ASSERT_EQ (expectedExpoTime, simpleSlave.getExposureLimits (curId).second);
    }
}
