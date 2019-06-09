/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usecase/UseCaseSlave.hpp>
#include <stdint.h>

using namespace royale;
using namespace royale::usecase;

UseCaseSlave::UseCaseSlave (const UseCaseDefinition &masterDefinition)
    : UseCaseDefinition (static_cast<uint16_t> (masterDefinition.getTargetRate() + 1u))
{
    m_typeName = masterDefinition.getTypeName() + "_Slave";

    // Raise the target rate by 1 to help synchronization
    m_targetRate = static_cast<uint16_t> (masterDefinition.getTargetRate() + 1u);
    m_rawFrameSet = masterDefinition.getRawFrameSets();
    masterDefinition.getImage (m_imageColumns, m_imageRows);
    m_enableSSC = masterDefinition.getSSCEnabled();
    auto streamIds = masterDefinition.getStreamIds();
    for (auto curId : streamIds)
    {
        m_streams.emplace_back (masterDefinition.getStream (curId));
    }
    m_exposureGroups = masterDefinition.getExposureGroups();

    // Adapt the exposure times for every modulated raw frame set according to
    // new_max_exposure = old_max_exposure * old_framerate / new_framerate
    float ratio = static_cast<float> (masterDefinition.getTargetRate()) /
                  static_cast<float> (m_targetRate);
    for (auto curRFS : m_rawFrameSet)
    {
        if (curRFS.isModulated())
        {
            auto &expoGroup = m_exposureGroups[curRFS.exposureGroupIdx];
            expoGroup.m_exposureLimits.second = static_cast<uint32_t> (static_cast<float> (expoGroup.m_exposureLimits.second) *
                                                ratio);
            expoGroup.m_exposureTime = static_cast<uint32_t> (static_cast<float> (expoGroup.m_exposureTime) *
                                       ratio);
        }
    }

    verifyClassInvariants();
}
