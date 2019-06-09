/****************************************************************************\
* Copyright (C) 2015 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usecase/UseCaseTwoPhase.hpp>
#include <stdint.h>

using namespace royale;
using namespace royale::usecase;

//! generate definition for the two gray scale sequences 1+1
UseCaseTwoPhase::UseCaseTwoPhase (uint16_t frameRate,
                                  Pair<uint32_t, uint32_t> exposureSettings,
                                  uint32_t exposureGray1,
                                  uint32_t exposureGray2,
                                  ExposureGray expoOnForGray1,
                                  ExposureGray expoOnForGray2) : UseCaseDefinition (frameRate)
{
    m_typeName = "TwoPhase";
    m_targetRate = frameRate;

    m_imageColumns = 176;
    m_imageRows = 120;

    royale::Vector<RawFrameSet> rawFrameSets;

    // intensity 1
    if (exposureGray1 > 0)
    {
        auto exposureGroupIdx = createExposureGroup ("gray1", royale::Pair<uint32_t, uint32_t> (exposureGray1, exposureGray1), exposureGray1);
        rawFrameSets.push_back (createGrayRFS (exposureGroupIdx, expoOnForGray1));
    }

    // intensity 2
    if (exposureGray2 > 0)
    {
        auto exposureGroupIdx = createExposureGroup ("gray2", royale::Pair<uint32_t, uint32_t> (exposureGray2, exposureGray2), exposureGray2);
        rawFrameSets.push_back (createGrayRFS (exposureGroupIdx, expoOnForGray2));
    }

    constructNonMixedUseCase (std::move (rawFrameSets));

    verifyClassInvariants();
}
