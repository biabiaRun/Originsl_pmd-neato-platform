/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <modules/ModuleConfigData.hpp>
#include <royale/ProcessingFlag.hpp>
#include <common/SensorRoutingConfigI2c.hpp>
#include <usecase/UseCaseFourPhase.hpp>
#include <usecase/UseCaseEightPhase.hpp>
#include <usecase/UseCaseCalibration.hpp>
#include <imager/M2452_B1x/ConstantsAIOFirmware.hpp>
#include <imager/M2452_B1x/ImagerBaseConfig.hpp>
#include <common/MakeUnique.hpp>

using namespace royale::common;
using namespace royale::config;

namespace
{
    const auto MIN_EXPO_NR = royale::imager::M2452_B1x::MIN_EXPO_NR;

    namespace params // royale::Processing parameters
    {

        static const royale::ProcessingParameterMap ProcessingParams5fps
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
            { royale::ProcessingFlag::UseValidateImage_Bool, royale::Variant (true) }
        };

        static const royale::ProcessingParameterMap ProcessingParams35fps
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
    }
} // anonymous namespace


const ModuleConfig royale::config::moduleconfig::AnimatorDefault
{
    royale::config::CoreConfig
    {
        { 112, 86 },
        { 224, 172 },
        royale::usecase::UseCaseList{
            royale::usecase::UseCase{
                "MODE_9_5FPS",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                5u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 600u }, 600u, 600u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase),
                { params::ProcessingParams5fps },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_9_10FPS",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                10u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 300u }, 300u, 300u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase),
                { params::ProcessingParams5fps },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_9_15FPS",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                15u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 200u }, 200u, 200u, 200u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase),
                { params::ProcessingParams5fps },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_9_25FPS",
                std::make_shared<royale::usecase::UseCaseEightPhase> (
                25u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 100u }, 100u, 100u, 100u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase),
                { params::ProcessingParams5fps },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_5_35FPS",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                35u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 180u }, 180u, 150u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase),
                { params::ProcessingParams35fps },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_5_45FPS",
                std::make_shared<royale::usecase::UseCaseFourPhase> (
                45u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 130u }, 130u, 130u,
                royale::usecase::ExposureGray::Off,
                royale::usecase::IntensityPhaseOrder::IntensityFirstPhase),
                { params::ProcessingParams45fps },
                royale::CallbackData::Depth
            },
            royale::usecase::UseCase{
                "MODE_10_5FPS",
                std::make_shared<royale::usecase::UseCaseCalibration> (
                5u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 550u }, 550u, 550u, 200u, 200u),
                { params::ProcessingParams5fps },
                royale::CallbackData::Raw,
                royale::CameraAccessLevel::L3
            }
        },
        royale::config::BandwidthRequirementCategory::NO_THROTTLING,
        royale::config::FrameTransmissionMode::SUPERFRAME,
        "Animator (reference config)",
        65.0f,
        60.0f,
        true
    },
    royale::config::ImagerConfig
    {
        royale::config::ImagerType::M2452_B1x_AIO,
        24000000,
        royale::imager::M2452_B1x::BaseConfig,
        0.0,
        royale::config::ImageDataTransferType::MIPI_2LANE
    },
    royale::config::IlluminationConfig  { royale::usecase::RawFrameSet::DutyCycle::DC_25, 90000000, royale::config::IlluminationPad::SE_P },
    TemperatureSensorConfig{ TemperatureSensorConfig::TemperatureSensorType::TMP102 },
    FlashMemoryConfig{ FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM }
    .setImageSize (128 * 1024)
    .setPageSize (256)
    .setWriteTime (std::chrono::microseconds {5000}),
    SensorMap
    {
        { SensorRole::MAIN_IMAGER, std::make_shared<SensorRoutingConfigI2c> (0x3D) },
        { SensorRole::TEMP_ILLUMINATION, std::make_shared<SensorRoutingConfigI2c> (0x48) },
        { SensorRole::STORAGE_CALIBRATION, std::make_shared<SensorRoutingConfigI2c> (0x50) }
    }
};
