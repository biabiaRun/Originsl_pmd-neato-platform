/****************************************************************************\
* Copyright (C) 2015 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usecase/UseCaseCalibration.hpp>
#include <stdint.h>

using namespace royale;
using namespace royale::usecase;

//! generate definition for the two different sequence modes 1+4+4+1+1
UseCaseCalibration::UseCaseCalibration (uint16_t frameRate,
                                        uint32_t modulationFrequency1,
                                        uint32_t modulationFrequency2,
                                        Pair<uint32_t, uint32_t> exposureSettings,
                                        uint32_t exposureModulation1,
                                        uint32_t exposureModulation2,
                                        uint32_t exposureGray1,
                                        uint32_t exposureGray2,
                                        bool enableSSC,
                                        double ssc_freq_mod1,
                                        double ssc_kspread_mod1,
                                        double ssc_delta_mod1,
                                        double ssc_freq_mod2,
                                        double ssc_kspread_mod2,
                                        double ssc_delta_mod2) : UseCaseDefinition (frameRate)
{
    m_typeName = "TenPhase";
    m_targetRate = frameRate;
    m_enableSSC = enableSSC;
    m_imageColumns = 176;
    m_imageRows = 120;

    royale::Vector<RawFrameSet> rawFrameSets;

    // first raw frame set
    {
        auto exposureGroupIdx = createExposureGroup ("mod1", exposureSettings, exposureModulation1);
        RawFrameSet set
        {
            modulationFrequency1,
            RawFrameSet::PhaseDefinition::MODULATED_4PH_CW,
            RawFrameSet::DutyCycle::DC_AUTO,
            exposureGroupIdx,
            RawFrameSet::Alignment::START_ALIGNED, 0.,
            ssc_freq_mod1, ssc_kspread_mod1, ssc_delta_mod1
        };
        rawFrameSets.push_back (std::move (set));
    }

    // second raw frame set
    {
        auto exposureGroupIdx = createExposureGroup ("mod2", exposureSettings, exposureModulation2);
        RawFrameSet set
        {
            modulationFrequency2,
            RawFrameSet::PhaseDefinition::MODULATED_4PH_CW,
            RawFrameSet::DutyCycle::DC_AUTO,
            exposureGroupIdx,
            RawFrameSet::Alignment::START_ALIGNED, 0.,
            ssc_freq_mod2, ssc_kspread_mod2, ssc_delta_mod2
        };
        rawFrameSets.push_back (std::move (set));
    }

    // intensity 1
    if (exposureGray1 > 0)
    {
        auto exposureGroupIdx1 = createExposureGroup ("gray1", exposureSettings, exposureGray1);
        rawFrameSets.insert (rawFrameSets.begin(), createGrayRFS (exposureGroupIdx1, ExposureGray::Off));

        // For the ten phase case (calibration), the gray image is added twice, so that the ninth
        // and tenth phase remain comparable Pre mixed-mode, these would have two sequence numbers,
        // so in mixed mode they're two separate RawFrameSets with the same configuration.
        rawFrameSets.push_back (createGrayRFS (createExposureGroup ("gray1a", exposureSettings, exposureGray1), ExposureGray::Off, modulationFrequency2));
    }

    // intensity 2
    if (exposureGray2 > 0)
    {
        rawFrameSets.push_back (createGrayRFS (createExposureGroup ("gray2", exposureSettings, exposureGray2), ExposureGray::On, modulationFrequency2));
    }

    constructNonMixedUseCase (std::move (rawFrameSets));
    verifyClassInvariants();
}
