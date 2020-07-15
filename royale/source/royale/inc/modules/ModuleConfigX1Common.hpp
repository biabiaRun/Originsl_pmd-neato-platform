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
#include <imager/M2452_B1x/ConstantsAIOFirmware.hpp>
#include <imager/M2452_B1x/ImagerBaseConfig.hpp>
#include <usecase/UseCaseFourPhase.hpp>
#include <usecase/UseCaseEightPhase.hpp>
#include <usecase/UseCaseCalibration.hpp>
#include <modules/CommonProcessingParameters.hpp>
#include <common/MakeUnique.hpp>

namespace royale
{
    namespace x1
    {
        static const royale::config::ImagerConfig imagerConfig = royale::config::ImagerConfig
        {
            royale::config::ImagerType::M2452_B1x_AIO,
            24000000,
            royale::imager::M2452_B1x::BaseConfig,
            0.0,
            royale::config::ImageDataTransferType::MIPI_2LANE
        };
        static const royale::config::IlluminationConfig illuConfig = royale::config::IlluminationConfig
        {
            royale::usecase::RawFrameSet::DutyCycle::DC_37_5,
            90000000,
            royale::config::IlluminationPad::SE_P
        };

        static royale::config::FlashMemoryConfig flashConfig = royale::config::FlashMemoryConfig
        {
            royale::config::FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM
        }
        .setImageSize (128 * 1024)
        .setPageSize (256)
        .setWriteTime (std::chrono::microseconds{ 5000 });

        static const bool   ssc_enable = true;
        static const double ssc_freq = 10000.;
        static const double ssc_kspread = 0.5;
        static const double ssc_delta_80320kHz = 0.0125;
        static const double ssc_delta_60240kHz = 0.0166;

        const auto MIN_EXPO_NR = royale::imager::M2452_B1x::MIN_EXPO_NR;
    }

    namespace x1_1
    {
        static const royale::config::TemperatureSensorConfig tempsensorConfig = royale::config::TemperatureSensorConfig
        {
            royale::config::TemperatureSensorConfig::TemperatureSensorType::TMP102
        };

        static const royale::common::SensorMap sensorMap = royale::common::SensorMap
        {
            { royale::common::SensorRole::MAIN_IMAGER, std::make_shared<royale::common::SensorRoutingConfigI2c> (0x3D) },
            { royale::common::SensorRole::TEMP_ILLUMINATION, std::make_shared<royale::common::SensorRoutingConfigI2c> (0x48) },
            { royale::common::SensorRole::STORAGE_CALIBRATION, std::make_shared<royale::common::SensorRoutingConfigI2c> (0x56) }
        };
    }

    namespace x1_2
    {
        static const royale::config::TemperatureSensorConfig tempsensorConfig = royale::config::TemperatureSensorConfig
        {
            royale::config::TemperatureSensorConfig::TemperatureSensorType::TMP103
        };

        static const royale::common::SensorMap sensorMap = royale::common::SensorMap
        {
            { royale::common::SensorRole::MAIN_IMAGER, std::make_shared<royale::common::SensorRoutingConfigI2c> (0x3D) },
            { royale::common::SensorRole::TEMP_ILLUMINATION, std::make_shared<royale::common::SensorRoutingConfigI2c> (0x71) },
            { royale::common::SensorRole::STORAGE_CALIBRATION, std::make_shared<royale::common::SensorRoutingConfigI2c> (0x56) }
        };
    }
}

