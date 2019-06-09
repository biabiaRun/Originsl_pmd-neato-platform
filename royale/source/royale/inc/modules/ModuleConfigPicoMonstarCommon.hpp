/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#pragma once

#include <modules/CommonProcessingParameters.hpp>
#include <modules/ModuleConfigData.hpp>
#include <royale/ProcessingFlag.hpp>
#include <common/SensorRoutingConfigI2c.hpp>
#include <modules/imager/ImagerBaseConfigPicoMonstar.hpp>
#include <imager/M2450_A12/ConstantsAIOFirmware.hpp>
#include <common/SensorRoutingConfigSpi.hpp>
#include <common/MakeUnique.hpp>

namespace royale
{
    namespace picomonstar
    {
        static const royale::ProcessingParameterMap ProcessingParams2Frequencies
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
            { royale::ProcessingFlag::NoiseThreshold_Float, royale::Variant (0.03f, 0.01f, 0.2f) },
            royale::parameter::stdAdaptiveNoiseFilterType,
            royale::parameter::stdAutoExposureRefValue,
            { royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseAutoExposure_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveStrayLight_Bool, royale::Variant (false) }
        };

        static const royale::ProcessingParameterMap ProcessingParams10fps
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
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveStrayLight_Bool, royale::Variant (false) }
        };

        static const royale::ProcessingParameterMap ProcessingParams15fps
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
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveStrayLight_Bool, royale::Variant (false) }
        };

        static const royale::ProcessingParameterMap ProcessingParams25fps
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
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveStrayLight_Bool, royale::Variant (false) }
        };

        static const royale::ProcessingParameterMap ProcessingParams1Frequency
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
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveStrayLight_Bool, royale::Variant (false) }
        };

        static const royale::processing::ProcessingParameterId ProcessingParamsMonstarGlass5fps
        {
            royale::processing::ProcessingParameterId{"ProcessingParamsMonstarGlass5fps"}
        };

        static const royale::ProcessingParameterMap ProcessingParams5fpsGlass
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
            { royale::ProcessingFlag::NoiseThreshold_Float, royale::Variant (0.02f, 0.01f, 0.2f) },
            royale::parameter::stdAdaptiveNoiseFilterType,
            royale::parameter::stdAutoExposureRefValue,
            { royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseAutoExposure_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveStrayLight_Bool, royale::Variant (false) }
        };

        static const royale::processing::ProcessingParameterId ProcessingParamsMonstarGlass10fps
        {
            royale::processing::ProcessingParameterId{ "ProcessingParamsMonstarGlass10fps" }
        };

        static const royale::ProcessingParameterMap ProcessingParams10fpsGlass
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
            { royale::ProcessingFlag::NoiseThreshold_Float, royale::Variant (0.05f, 0.01f, 0.2f) },
            royale::parameter::stdAdaptiveNoiseFilterType,
            royale::parameter::stdAutoExposureRefValue,
            { royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseAutoExposure_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveStrayLight_Bool, royale::Variant (false) }
        };

        static const double ssc_freq = 10000.;
        static const double ssc_kspread = 0.5;
        static const double ssc_delta_80320kHz = 0.0125;
        static const double ssc_delta_60240kHz = 0.0166;

        static const royale::config::ImagerConfig imagerConfig = royale::config::ImagerConfig
        {
            royale::config::ImagerType::M2450_A12_AIO,
            19200000,
            royale::access::imager::M2450_A12::BaseConfigPicoMonstar,
            0.0f,
            royale::config::ImageDataTransferType::MIPI_2LANE,
            royale::config::ImagerConfig::Trigger::GPIO14
        };
        static const royale::config::IlluminationConfig illuConfig =
            royale::config::IlluminationConfig{ royale::usecase::RawFrameSet::DutyCycle::DC_37_5, 90000000, royale::config::IlluminationPad::SE_P };
        static const royale::config::TemperatureSensorConfig tempsensorConfig =
            royale::config::TemperatureSensorConfig{ royale::config::TemperatureSensorConfig::TemperatureSensorType::TMP102 };
        static const royale::config::FlashMemoryConfig flashConfig =
            royale::config::FlashMemoryConfig{ royale::config::FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM }
            .setPageSize (256)
            .setSectorSize (0x10000)
            .setAccessOffset (0x20000);
        static const royale::common::SensorMap sensorMap = royale::common::SensorMap
        {
            { royale::common::SensorRole::MAIN_IMAGER, std::make_shared<royale::common::SensorRoutingConfigI2c> (0x3D) },
            { royale::common::SensorRole::TEMP_ILLUMINATION, std::make_shared<royale::common::SensorRoutingConfigI2c> (0x49) },
            { royale::common::SensorRole::STORAGE_CALIBRATION, std::make_shared<royale::common::SensorRoutingConfigSpi> (0x0) }
        };
    }
}

