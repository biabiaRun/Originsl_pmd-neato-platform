/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/


#include <modules/CharacterizationHelper.hpp>
#include <memory>
#include <royale/CallbackData.hpp>
#include <royale/ProcessingFlag.hpp>
#include <usecase/UseCaseFourPhase.hpp>
#include <usecase/UseCase.hpp>
#include <royale/CameraAccessLevel.hpp>
#include <imager/M2452_B1x/ConstantsAIOFirmware.hpp>
using namespace royale::config;
namespace royale
{

    namespace config
    {

        static const royale::ProcessingParameterMap ProcessingParams45fps
        {
            royale::parameter::stdConsistencyTolerance,
            royale::parameter::stdFlyingPixelsF0,
            royale::parameter::stdFlyingPixelsF1,
            royale::parameter::stdFlyingPixelsFarDist,
            royale::parameter::stdFlyingPixelsNearDist,
            royale::parameter::stdLowerSaturationThreshold,
            royale::parameter::stdUpperSaturationThreshold,
            royale::parameter::stdMPIAmpThreshold,
            royale::parameter::stdMPIDistThreshold,
            royale::parameter::stdMPINoiseDistance,
            royale::parameter::stdNoiseThreshold,
            royale::parameter::stdAdaptiveNoiseFilterType,
            royale::parameter::stdAutoExposureRefValue,
            { royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseAutoExposure_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) }
        };

        royale::usecase::UseCase CharacterizationHelper::createCharacterizationUseCase (double frequency)
        {

            const double ssc_freq = 10000.;
            const double ssc_kspread = 0.5;
            const double ssc_delta_80320kHz = 0.0125;

            uint32_t modFreq = static_cast<uint32_t> (frequency * 1000000.0);
            std::stringstream ss;
            ss << "CH_" << frequency << "MHz";
            return { ss.str().c_str(),
                     std::make_shared<royale::usecase::UseCaseFourPhase> (
                         5u, modFreq, royale::Pair<uint32_t, uint32_t> { royale::imager::M2452_B1x::MIN_EXPO_NR_WITH_NTC, 100u }, 100u, 0u,
                         royale::usecase::ExposureGray::Off,
                         royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                         false, ssc_freq, ssc_kspread, ssc_delta_80320kHz),
            { ProcessingParams45fps },
            royale::CallbackData::Raw,
            royale::CameraAccessLevel::L3 };
        }

    }
}
