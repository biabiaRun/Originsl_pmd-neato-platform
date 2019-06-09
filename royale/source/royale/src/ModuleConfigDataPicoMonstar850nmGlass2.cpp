/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <modules/ModuleConfigPicoMonstarCommon.hpp>
#include <usecase/UseCaseFourPhase.hpp>
#include <usecase/UseCaseEightPhase.hpp>
#include <usecase/UseCaseCalibration.hpp>
#include <usecase/UseCaseMixedIrregularXHt.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::config;
using namespace royale::imager::M2450_A12;

const ModuleConfig royale::config::moduleconfig::PicoMonstar850nmGlass2
{
    royale::config::CoreConfig{
        { 176, 143 },  // lens center should be constant
        { 352, 287 },  // resolution
        royale::usecase::UseCaseList{
            royale::usecase::UseCase{
                "MODE_9_5FPS_1520",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                5u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 1520u }, 1520u, 1520u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, picomonstar::ssc_freq, picomonstar::ssc_kspread, picomonstar::ssc_delta_80320kHz, picomonstar::ssc_delta_60240kHz
                ),
                picomonstar::ProcessingParamsMonstarGlass5fps,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_9_10FPS_720",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                10u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 720u }, 720u, 720u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, picomonstar::ssc_freq, picomonstar::ssc_kspread, picomonstar::ssc_delta_80320kHz, picomonstar::ssc_delta_60240kHz
                ),
                picomonstar::ProcessingParamsMonstarGlass10fps,
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_9_15FPS_480",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                15u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 480u }, 480u, 480u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, picomonstar::ssc_freq, picomonstar::ssc_kspread, picomonstar::ssc_delta_80320kHz, picomonstar::ssc_delta_60240kHz
                ),
                { picomonstar::ProcessingParams15fps },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_9_25FPS_240",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                25u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 240u }, 240u, 240u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, picomonstar::ssc_freq, picomonstar::ssc_kspread, picomonstar::ssc_delta_80320kHz, picomonstar::ssc_delta_60240kHz
                ),
                { picomonstar::ProcessingParams25fps },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_5_35FPS_400",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                35u, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 400u }, 400u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, picomonstar::ssc_freq, picomonstar::ssc_kspread, picomonstar::ssc_delta_60240kHz
                ),
                { picomonstar::ProcessingParams1Frequency },
                royale::CallbackData::Depth },
            royale::usecase::UseCase{
                "MODE_5_45FPS_320",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                45u, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 320u }, 320u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, picomonstar::ssc_freq, picomonstar::ssc_kspread, picomonstar::ssc_delta_60240kHz
                ),
                { picomonstar::ProcessingParams1Frequency },
                royale::CallbackData::Depth },

            royale::usecase::UseCase{
                "MODE_5_60FPS_240",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                60u, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 240u }, 240u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, picomonstar::ssc_freq, picomonstar::ssc_kspread, picomonstar::ssc_delta_60240kHz
                ),
                { picomonstar::ProcessingParams1Frequency },
                royale::CallbackData::Depth },

            royale::usecase::UseCase{
                "MODE_10_5FPS_1600",
                std::make_shared<royale::usecase::UseCaseCalibration> (
                5u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 1600u }, 1000u, 1000u, 1000u, 1000u,
                true, picomonstar::ssc_freq, picomonstar::ssc_kspread, picomonstar::ssc_delta_80320kHz, picomonstar::ssc_freq,
                picomonstar::ssc_kspread, picomonstar::ssc_delta_60240kHz
                ),
                { picomonstar::ProcessingParams2Frequencies },
                royale::CallbackData::Raw,
                royale::CameraAccessLevel::L3
            },
            royale::usecase::UseCase{
                "MODE_MIXED_30_5",
                std::make_shared<royale::usecase::UseCaseMixedIrregularXHt> (
                    30u, 6u, 60240000, 80320000, 60240000,
                royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_MX, 216u },
                royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_MX, 776u },
                216u, 776u, 776u, 200u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, picomonstar::ssc_freq, picomonstar::ssc_kspread, picomonstar::ssc_delta_60240kHz,
                picomonstar::ssc_delta_80320kHz, picomonstar::ssc_delta_60240kHz
                ),
                { picomonstar::ProcessingParams1Frequency, picomonstar::ProcessingParams2Frequencies },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_MIXED_50_5",
                std::make_shared<royale::usecase::UseCaseMixedIrregularXHt> (
                    50u, 10u, 60240000, 80320000, 60240000,
                royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_MX, 168u },
                royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_MX, 680u },
                168u, 680u, 680u, 150u, 150u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, picomonstar::ssc_freq, picomonstar::ssc_kspread, picomonstar::ssc_delta_60240kHz,
                picomonstar::ssc_delta_80320kHz, picomonstar::ssc_delta_60240kHz
                ),
                { picomonstar::ProcessingParams1Frequency, picomonstar::ProcessingParams2Frequencies },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "Low_Noise_Extended",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                5u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 1520u }, 1520u, 1520u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, picomonstar::ssc_freq, picomonstar::ssc_kspread, picomonstar::ssc_delta_80320kHz, picomonstar::ssc_delta_60240kHz
                ),
                { royale::moduleconfig::CommonIdLowNoiseExtended },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "Fast_Acquisition",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                45u, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 320u }, 320u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, picomonstar::ssc_freq, picomonstar::ssc_kspread, picomonstar::ssc_delta_60240kHz
                ),
                { royale::moduleconfig::CommonIdFastAcquisition },
                royale::CallbackData::Depth },

            royale::usecase::UseCase{
                "Very_Fast_Acquisition",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                60u, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 240u }, 240u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                true, picomonstar::ssc_freq, picomonstar::ssc_kspread, picomonstar::ssc_delta_60240kHz
                ),
                { royale::moduleconfig::CommonIdVeryFastAcquisition },
                royale::CallbackData::Depth },
        },
        royale::config::BandwidthRequirementCategory::USB3_THROTTLING,
        royale::config::FrameTransmissionMode::INDIVIDUAL,
        "PICOMONSTAR850GLASS2",
        65.0f,
        60.0f,
        true
    },
    picomonstar::imagerConfig,
    picomonstar::illuConfig,
    picomonstar::tempsensorConfig,
    picomonstar::flashConfig,
    picomonstar::sensorMap
};
