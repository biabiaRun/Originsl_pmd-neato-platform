/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usecase/UseCaseGrayScale.hpp>
#include <stdint.h>

using namespace royale;
using namespace royale::usecase;

UseCaseGrayScale::UseCaseGrayScale (Pair<uint32_t, uint32_t> exposureLimits, uint32_t exposureGray, bool expoOnForGray) :
    UseCaseDefinition (45)
{
    m_typeName = "GrayScale";
    m_targetRate = 45;

    m_imageColumns = 176;
    m_imageRows = 120;

    royale::Vector<RawFrameSet> rawFrameSets;

    auto exposureGroupIdx = createExposureGroup ("gray", exposureLimits, exposureGray);
    auto expoOn = expoOnForGray ? ExposureGray::On : ExposureGray::Off;
    rawFrameSets.push_back (createGrayRFS (exposureGroupIdx, expoOn));

    constructNonMixedUseCase (std::move (rawFrameSets));
    verifyClassInvariants();
}
