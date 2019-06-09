/****************************************************************************\
* Copyright (C) 2018 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/MakeUnique.hpp>
#include <common/SensorRoutingConfigI2c.hpp>
#include <imager/M2452_B1x/ConstantsAIOFirmware.hpp>
#include <imager/M2452_B1x/ImagerBaseConfig.hpp>
#include <usecase/UseCaseFourPhase.hpp>
#include <usecase/UseCaseEightPhase.hpp>
#include <usecase/UseCaseCalibration.hpp>
#include <modules/CommonProcessingParameters.hpp>
#include <modules/ModuleConfigData.hpp>
#include <royale/ProcessingFlag.hpp>

using namespace royale::common;
using namespace royale::config;

namespace
{
    namespace x1
    {
        static const ImagerConfig imagerConfig = ImagerConfig
        {
            royale::config::ImagerType::M2452_B1x_AIO,
            24000000,
            royale::imager::M2452_B1x::BaseConfig,
            0.0,
            royale::config::ImageDataTransferType::MIPI_2LANE
        };
        static const IlluminationConfig illuConfig = IlluminationConfig  { royale::usecase::RawFrameSet::DutyCycle::DC_37_5, 90000000, royale::config::IlluminationPad::SE_P };
        static const TemperatureSensorConfig tempsensorConfig = TemperatureSensorConfig{ TemperatureSensorConfig::TemperatureSensorType::TMP102 };

        static FlashMemoryConfig flashConfig = FlashMemoryConfig{ FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM }
                                               .setImageSize (128 * 1024)
                                               .setPageSize (256)
                                               .setWriteTime (std::chrono::microseconds {5000});

        static const bool   ssc_enable = true;
        static const double ssc_freq = 10000.;
        static const double ssc_kspread = 0.5;
        static const double ssc_delta_80320kHz = 0.0125;
        static const double ssc_delta_60240kHz = 0.0166;

        static const SensorMap sensorMap = SensorMap
        {
            { SensorRole::MAIN_IMAGER, std::make_shared<SensorRoutingConfigI2c> (0x3D) },
            { SensorRole::TEMP_ILLUMINATION, std::make_shared<SensorRoutingConfigI2c> (0x48) },
            { SensorRole::STORAGE_CALIBRATION, std::make_shared<SensorRoutingConfigI2c> (0x56) }
        };

        const auto MIN_EXPO_NR = royale::imager::M2452_B1x::MIN_EXPO_NR;
    }

} // anonymous namespace

using namespace x1;

const ModuleConfig royale::config::moduleconfig::X18502W
{
    royale::config::CoreConfig
    {
        { 112, 86 },
        { 224, 172 },
        royale::usecase::UseCaseList{
            royale::usecase::UseCase{
                "MODE_9_5FPS",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                5u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 2000u }, 2000u, 2000u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId2Frequencies,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_9_1FPS",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                1u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 2250u }, 2250u, 2250u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId2Frequencies,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_9_2FPS",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                2u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 2250u }, 2250u, 2250u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId2Frequencies,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_9_3FPS",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                3u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 2250u }, 2250u, 2250u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId2Frequencies,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_9_10FPS",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                10u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 1000u }, 1000u, 1000u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId2Frequencies,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_9_15FPS",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                15u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 650u }, 650u, 650u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId2Frequencies,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_9_25FPS",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                25u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 400u }, 400u, 400u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId2Frequencies,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_5_5FPS",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                5u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 3500u }, 3500u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId1Frequency,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_5_10FPS",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                10u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 1750u }, 1750u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId1Frequency,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_5_15FPS",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                15u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 1150u }, 1150u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId1Frequency,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_5_30FPS",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                30u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 550u }, 550u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId1Frequency,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_5_35FPS",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                35u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 500u }, 500u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId1Frequency,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_5_45FPS",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                45u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 350u }, 350u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId1Frequency,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_5_60FPS",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                60u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 250u }, 250u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId1Frequency,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_11_5FPS",
                std::make_shared<royale::usecase::UseCaseCalibration> (
                5u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 1800u }, 1800u, 1800u, 100u, 100u,
                ssc_enable, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_freq,
                ssc_kspread, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId2Frequencies,
                royale::CallbackData::Raw,
                royale::CameraAccessLevel::L3
            },
            royale::usecase::UseCase{
                "MODE_11_10FPS",
                std::make_shared<royale::usecase::UseCaseCalibration> (
                10u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 900u }, 900u, 900u, 100u, 100u,
                ssc_enable, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_freq,
                ssc_kspread, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId2Frequencies,
                royale::CallbackData::Raw,
                royale::CameraAccessLevel::L3
            }
        },
        royale::config::BandwidthRequirementCategory::NO_THROTTLING,
        royale::config::FrameTransmissionMode::SUPERFRAME,
        "X1_850nm_2W",
        65.0f,
        60.0f,
        true
    },
    imagerConfig,
    illuConfig,
    tempsensorConfig,
    flashConfig,
    sensorMap
};
