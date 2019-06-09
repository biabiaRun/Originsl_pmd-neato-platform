/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usecase/UseCaseMultiplier.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/NarrowCast.hpp>
#include <stdint.h>

using namespace royale;
using namespace royale::common;
using namespace royale::usecase;
using PhaseDefVec = royale::Vector<RawFrameSet::PhaseDefinition>;

UseCaseMultiplier::UseCaseMultiplier (std::size_t multiplier, const UseCaseDefinition &base) :
    UseCaseDefinition (base)
{
    m_typeName = String::fromInt (narrow_cast<int> (multiplier)) + String ("x") + m_typeName;

    if (multiplier < 2u)
    {
        throw LogicError ("Multiplying by one or zero isn't logical");
    }

    // UseCaseDefinition::UseCaseDefinition (base) has already copied base's streams, which is why
    // this loop starts at i=1.
    for (std::size_t i = 1u; i < multiplier; i++)
    {
        for (auto streamId : base.getStreamIds())
        {
            auto stream = createStream();
            for (std::size_t groupIdx = 0; groupIdx < base.getFrameGroupCount (streamId); groupIdx++)
            {
                auto frameGroup = stream->createFrameGroup();
                for (auto rfsIdx : base.getRawFrameSetIndices (streamId, groupIdx))
                {
                    frameGroup->addFrameSet (rfsIdx);
                }
            }
        }
    }

    verifyClassInvariants();
}

