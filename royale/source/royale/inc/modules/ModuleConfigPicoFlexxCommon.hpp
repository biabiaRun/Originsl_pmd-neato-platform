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

#include <modules/ModuleConfigData.hpp>
#include <royale/ProcessingFlag.hpp>
#include <common/SensorRoutingConfigI2c.hpp>
#include <modules/imager/ImagerBaseConfigPicoFlexx.hpp>
#include <imager/M2450_A12/ConstantsAIOFirmware.hpp>
#include <common/MakeUnique.hpp>

namespace royale
{
    namespace picoflexx
    {
        static const royale::ProcessingParameterMap standardParametersPicoFlexx
        {
            { royale::ProcessingFlag::ConsistencyTolerance_Float, royale::Variant (1.2f, 0.2f, 1.5f) },
            { royale::ProcessingFlag::FlyingPixelsF0_Float, royale::Variant (0.018f, 0.01f, 0.04f) },
            { royale::ProcessingFlag::FlyingPixelsF1_Float, royale::Variant (0.14f, 0.01f, 0.5f) },
            { royale::ProcessingFlag::FlyingPixelsFarDist_Float, royale::Variant (4.5f, 0.01f, 1000.0f) },
            { royale::ProcessingFlag::FlyingPixelsNearDist_Float, royale::Variant (1.0f, 0.01f, 1000.0f) },
            { royale::ProcessingFlag::LowerSaturationThreshold_Int, royale::Variant (400, 0, 600) },
            { royale::ProcessingFlag::UpperSaturationThreshold_Int, royale::Variant (3750, 3500, 4095) },
            { royale::ProcessingFlag::MPIAmpThreshold_Float, royale::Variant (0.3f, 0.1f, 0.5f) },
            { royale::ProcessingFlag::MPIDistThreshold_Float, royale::Variant (0.1f, 0.05f, 0.2f) },
            { royale::ProcessingFlag::MPINoiseDistance_Float, royale::Variant (3.0f, 2.0f, 5.0f) },
            { royale::ProcessingFlag::NoiseThreshold_Float, royale::Variant (0.07f, 0.03f, 0.2f) },
            { royale::ProcessingFlag::AdaptiveNoiseFilterType_Int, royale::Variant (1, 1, 2) },
            { royale::ProcessingFlag::AutoExposureRefValue_Float, 1000.0f },
            { royale::ProcessingFlag::UseAdaptiveNoiseFilter_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveFlyingPixel_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseAutoExposure_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseRemoveStrayLight_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::GlobalBinning_Int, royale::Variant (1) },
            { royale::ProcessingFlag::UseAdaptiveBinning_Bool, royale::Variant (false) }
        };

        static const royale::ProcessingParameterMap ProcessingParams5fps
        {
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (true) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (true) }
        };

        static const royale::ProcessingParameterMap ProcessingParams5fpsMixed
        {
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (true) }
        };

        static const royale::ProcessingParameterMap ProcessingParams45fps
        {
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (false) }
        };

        static const royale::ProcessingParameterMap ProcessingParams15MHz
        {
            { royale::ProcessingFlag::NoiseThreshold_Float, royale::Variant (0.5f) },
            { royale::ProcessingFlag::UseMPIFlagAverage_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Amp_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseMPIFlag_Dist_Bool, royale::Variant (false) },
            { royale::ProcessingFlag::UseFilter2Freq_Bool, royale::Variant (false) }
        };

        static const bool   ssc_enable = true;
        static const double ssc_freq = 10000.;
        static const double ssc_kspread = 0.5;
        static const double ssc_delta_80320kHz = 0.0125;
        static const double ssc_delta_60240kHz = 0.0166;

        static const royale::config::ImagerConfig imagerConfig = royale::config::ImagerConfig
        {
            royale::config::ImagerType::M2450_A12_AIO,
            26000000,
            royale::imager::M2450_A12::BaseConfigForPicoFlexx,
            0.0,
            royale::config::ImageDataTransferType::PIF
        };
        static const royale::config::IlluminationConfig illuConfig =
            royale::config::IlluminationConfig{ royale::usecase::RawFrameSet::DutyCycle::DC_25_DEPRECATED, 90000000, royale::config::IlluminationPad::SE_P };
        static const royale::config::TemperatureSensorConfig tempsensorConfig =
            royale::config::TemperatureSensorConfig{ royale::config::TemperatureSensorConfig::TemperatureSensorType::TMP102 };
        static const royale::config::FlashMemoryConfig flashConfig =
            royale::config::FlashMemoryConfig{ royale::config::FlashMemoryConfig::FlashMemoryType::PICO_PAGED }
            .setImageSize (2000000)
            .setPageSize (256)
            .setSectorSize (1024 * 64);
        static const royale::common::SensorMap sensorMap = royale::common::SensorMap
        {
            { royale::common::SensorRole::MAIN_IMAGER, std::make_shared<royale::common::SensorRoutingConfigI2c> (0x3D) },
            { royale::common::SensorRole::TEMP_ILLUMINATION, std::make_shared<royale::common::SensorRoutingConfigI2c> (0x49) },
            { royale::common::SensorRole::TEMP_IMAGER, std::make_shared<royale::common::SensorRoutingConfigI2c> (0x48) }
        };

    }
}
