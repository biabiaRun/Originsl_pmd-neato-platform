/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <modules/ModuleConfigX1Common.hpp>

using namespace royale::common;
using namespace royale::config;
using namespace royale::x1;

using namespace royale::x1_1;

const ModuleConfig royale::config::moduleconfig::X1_18502W
{
    royale::config::CoreConfig
    {
        { 112, 86 },
        { 224, 172 },
        royale::usecase::UseCaseList{
            royale::usecase::UseCase{
                "MODE_9_5FPS",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                5u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 2200u }, 2200u, 2200u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId2Frequencies,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_9_10FPS",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                10u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 1100u }, 1100u, 1100u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId2Frequencies,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_9_15FPS",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                15u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 750u }, 750u, 750u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId2Frequencies,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_9_20FPS",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                20u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 560u }, 560u, 560u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId2Frequencies,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_9_30FPS",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                30u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 370u }, 370u, 370u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId2Frequencies,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_5_15FPS",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                15u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 1500u }, 1500u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId1Frequency,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_5_30FPS",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                30u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 750u }, 750u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId1Frequency,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_5_45FPS",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                45u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 500u }, 500u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, ssc_freq, ssc_kspread, ssc_delta_60240kHz),
                royale::moduleconfig::CommonId1Frequency,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_5_60FPS",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                60u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 370u }, 370u, 200u,
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
        "X1.1_850nm_2W",
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
