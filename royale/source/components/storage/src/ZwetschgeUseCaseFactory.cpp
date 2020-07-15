/****************************************************************************\
* Copyright (C) 2020 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <storage/ZwetschgeUseCaseFactory.hpp>

#include <usecase/UseCaseDefFactoryProcessingOnly.hpp>
#include <usecase/UseCaseFaceID.hpp>

using namespace royale;
using namespace royale::storage;
using namespace royale::usecase;

std::shared_ptr<UseCaseDefinition> ZwetschgeUseCaseFactory::createUcd (
    UseCaseIdentifier identifier,
    Pair<uint16_t, uint16_t> imageSize,
    Pair<uint16_t, uint16_t> frameRateLimits,
    uint16_t targetFrameRate,
    const Vector<ExposureGroup> &expoGroups,
    const Vector<RawFrameSet> &rfs
)
{
    if (rfs.size() == 4u &&
            rfs.at (0).countRawFrames() == 1u &&
            rfs.at (1).countRawFrames() == 1u &&
            rfs.at (2).countRawFrames() == 1u &&
            rfs.at (3).countRawFrames() == 4u)
    {
        // This gets a special treatment as we currently can't distinguish
        // mixed modes in the Zwetschge file
        return std::make_shared<UseCaseFaceID> (
                   identifier,
                   targetFrameRate,
                   rfs[0].modulationFrequency,
                   rfs[3].modulationFrequency,
                   rfs[2].modulationFrequency,
                   expoGroups[0].m_exposureLimits,
                   expoGroups[3].m_exposureLimits,
                   expoGroups[2].m_exposureLimits,
                   expoGroups[0].m_exposureTime,
                   expoGroups[3].m_exposureTime,
                   expoGroups[2].m_exposureTime);
    }
    else
    {
        return UseCaseDefFactoryProcessingOnly::createUcd (
                   identifier,
                   imageSize,
                   frameRateLimits,
                   targetFrameRate,
                   expoGroups,
                   rfs);
    }
}

