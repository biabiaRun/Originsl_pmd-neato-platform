/****************************************************************************\
* Copyright (C) 2017 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <modules/ModuleConfigPicoFlexxCommon.hpp>
#include <modules/CommonProcessingParameters.hpp>
#include <usecase/UseCaseFourPhase.hpp>
#include <usecase/UseCaseEightPhase.hpp>
#include <usecase/UseCaseCalibration.hpp>
#include <usecase/UseCaseMixedIrregularXHt.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::config;

// Standard pico flexx
// Product ID : 0x00000000

const ModuleConfig royale::config::moduleconfig::PicoFlexxU6
{
    royale::config::CoreConfig{
        { 176, 151 },
        { 224, 171 },
        royale::usecase::UseCaseList{
            royale::usecase::UseCase {
                "MODE_9_5FPS_2000",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                5u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 2000u }, 2000u, 2000u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                picoflexx::ssc_enable, picoflexx::ssc_freq, picoflexx::ssc_kspread, picoflexx::ssc_delta_80320kHz, picoflexx::ssc_delta_60240kHz
                ),
                { royale::moduleconfig::CommonProcessingParams2Frequencies },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase {
                "MODE_9_10FPS_1000",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                10u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 1000u }, 1000u, 1000u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                picoflexx::ssc_enable, picoflexx::ssc_freq, picoflexx::ssc_kspread, picoflexx::ssc_delta_80320kHz, picoflexx::ssc_delta_60240kHz
                ),
                { royale::moduleconfig::CommonProcessingParams2Frequencies },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase {
                "MODE_9_15FPS_700",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                15u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 700u }, 700u, 700u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                picoflexx::ssc_enable, picoflexx::ssc_freq, picoflexx::ssc_kspread, picoflexx::ssc_delta_80320kHz, picoflexx::ssc_delta_60240kHz
                ),
                { royale::moduleconfig::CommonProcessingParams2Frequencies },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase {
                "MODE_9_25FPS_450",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                25u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 450u }, 450u, 450u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                picoflexx::ssc_enable, picoflexx::ssc_freq, picoflexx::ssc_kspread, picoflexx::ssc_delta_80320kHz, picoflexx::ssc_delta_60240kHz
                ),
                { royale::moduleconfig::CommonProcessingParams2Frequencies },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase {
                "MODE_5_35FPS_600",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                35u, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 600u }, 600u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                picoflexx::ssc_enable, picoflexx::ssc_freq, picoflexx::ssc_kspread, picoflexx::ssc_delta_60240kHz
                ),
                { royale::moduleconfig::CommonProcessingParams1Frequency },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase {
                "MODE_5_45FPS_500",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                45u, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 500u }, 500u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                picoflexx::ssc_enable, picoflexx::ssc_freq, picoflexx::ssc_kspread, picoflexx::ssc_delta_60240kHz
                ),
                { royale::moduleconfig::CommonProcessingParams1Frequency },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase {
                "MODE_10_5FPS_2000",
                std::make_shared<royale::usecase::UseCaseCalibration> (
                5u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 2000u }, 2000u, 2000u, 200u, 200u,
                picoflexx::ssc_enable, picoflexx::ssc_freq, picoflexx::ssc_kspread, picoflexx::ssc_delta_80320kHz, picoflexx::ssc_freq,
                picoflexx::ssc_kspread, picoflexx::ssc_delta_60240kHz),
                { royale::moduleconfig::CommonProcessingParams2Frequencies },
                royale::CallbackData::Raw,
                royale::CameraAccessLevel::L3
            },
            royale::usecase::UseCase{
                "MODE_MIXED_30_5",
                std::make_shared<royale::usecase::UseCaseMixedIrregularXHt> (
                    30u, 6u, 60240000, 80320000, 60240000,
                royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_MX, 300u },
                royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_MX, 1300u },
                300u, 1300u, 1300u, 200u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                picoflexx::ssc_enable, picoflexx::ssc_freq, picoflexx::ssc_kspread, picoflexx::ssc_delta_60240kHz,
                picoflexx::ssc_delta_80320kHz, picoflexx::ssc_delta_60240kHz
                ),
                { royale::moduleconfig::CommonProcessingParams1Frequency, picoflexx::ProcessingParams5fpsMixed },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_MIXED_50_5",
                std::make_shared<royale::usecase::UseCaseMixedIrregularXHt> (
                    50u, 10u, 60240000, 80320000, 60240000,
                royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_MX, 250u },
                royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_MX, 1000u },
                250u, 1000u, 1000u, 200u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                picoflexx::ssc_enable, picoflexx::ssc_freq, picoflexx::ssc_kspread, picoflexx::ssc_delta_60240kHz,
                picoflexx::ssc_delta_80320kHz, picoflexx::ssc_delta_60240kHz
                ),
                { royale::moduleconfig::CommonProcessingParams1Frequency, picoflexx::ProcessingParams5fpsMixed },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "Low_Noise_Extended",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                5u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 2000u }, 2000u, 2000u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                picoflexx::ssc_enable, picoflexx::ssc_freq, picoflexx::ssc_kspread, picoflexx::ssc_delta_80320kHz, picoflexx::ssc_delta_60240kHz
                ),
                { royale::moduleconfig::CommonIdLowNoiseExtended },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "Fast_Acquisition",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                45u, 60240000, royale::Pair<uint32_t, uint32_t> { royale::imager::M2450_A12::MIN_EXPO_NR, 500u }, 500u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
                picoflexx::ssc_enable, picoflexx::ssc_freq, picoflexx::ssc_kspread, picoflexx::ssc_delta_60240kHz
                ),
                { royale::moduleconfig::CommonIdFastAcquisition },
                royale::CallbackData::Depth
            },
        },
        royale::config::BandwidthRequirementCategory::USB2_THROTTLING,
        royale::config::FrameTransmissionMode::INDIVIDUAL,
        "PICOFLEXX",
        65.0f,
        60.0f,
        true,
        picoflexx::standardParametersPicoFlexx
    },
    picoflexx::imagerConfig,
    picoflexx::illuConfig,
    picoflexx::tempsensorConfig,
    picoflexx::flashConfig,
    picoflexx::sensorMap
};
