/****************************************************************************\
* Copyright (C) 2019 pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <ModuleConfigCustom.hpp>
#include <config/ModuleConfig.hpp>
#include <modules/CommonProcessingParameters.hpp>
#include <modules/ModuleConfigData.hpp>
#include <royale/ProcessingFlag.hpp>
#include <common/SensorRoutingConfigI2c.hpp>
#include <usecase/UseCaseDefinition.hpp>
#include <usecase/UseCaseGrayScale.hpp>
#include <usecase/UseCaseFourPhase.hpp>
#include <usecase/UseCaseEightPhase.hpp>
#include <usecase/UseCaseCalibration.hpp>
#include <usecase/UseCaseMixedIrregularXHt.hpp>
#include <imager/M2452_B1x/ConstantsAIOFirmware.hpp>
#include <imager/M2452_B1x/ImagerBaseConfig.hpp>
#include <common/MakeUnique.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::config;

namespace {
    namespace x1 {
        static const ImagerConfig imagerConfig = ImagerConfig {
            royale::config::ImagerType::M2452_B1x_AIO,
            24000000,
            royale::imager::M2452_B1x::BaseConfig,
            0.0,
            royale::config::ImageDataTransferType::MIPI_2LANE
        };
        static const IlluminationConfig illuConfig = IlluminationConfig  { royale::usecase::RawFrameSet::DutyCycle::DC_37_5, 90000000, royale::config::IlluminationPad::SE_P };
        static const TemperatureSensorConfig tempsensorConfig = TemperatureSensorConfig{ TemperatureSensorConfig::TemperatureSensorType::TMP103 };
        static FlashMemoryConfig flashConfig = FlashMemoryConfig{ FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM }
                                               .setImageSize (128 * 1024)
                                               .setPageSize (256)
                                               .setWriteTime (std::chrono::microseconds {5000});
        static const bool   ssc_enable = true;
        static const double ssc_freq = 10000.;
        static const double ssc_kspread = 0.5;
        static const double ssc_delta_80320kHz = 0.0125;
        static const double ssc_delta_60240kHz = 0.0166;

        static const SensorMap sensorMap = SensorMap {
            { SensorRole::MAIN_IMAGER, std::make_shared<SensorRoutingConfigI2c> (0x3D) },
            { SensorRole::TEMP_ILLUMINATION, std::make_shared<SensorRoutingConfigI2c> (0x71) },
            { SensorRole::STORAGE_CALIBRATION, std::make_shared<SensorRoutingConfigI2c> (0x56) }
        };

        const auto MIN_EXPO_NR = royale::imager::M2452_B1x::MIN_EXPO_NR;
    }
} // anonymous namespace

using namespace x1;

const ModuleConfig moduleConfigCustom {
  royale::config::CoreConfig {
    { 112, 86 },
    { 224, 172 },
    royale::usecase::UseCaseList {
      royale::usecase::UseCase {
        "MODE_9_5FPS",
        std::make_shared<royale::usecase::UseCaseEightPhase> (
        5u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 1880u }, 1880u, 1880u, 200u,
        royale::usecase::ExposureGray::Off,
        royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
        true, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_delta_60240kHz),
        royale::moduleconfig::CommonId2Frequencies,
        royale::CallbackData::Depth
      },
      royale::usecase::UseCase {
        "MODE_9_10FPS",
        std::make_shared<royale::usecase::UseCaseEightPhase> (
        10u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 1070u }, 1070u, 1070u, 200u,
        royale::usecase::ExposureGray::Off,
        royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
        true, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_delta_60240kHz),
        royale::moduleconfig::CommonId2Frequencies,
        royale::CallbackData::Depth
      },
      royale::usecase::UseCase {
        "MODE_9_15FPS",
        std::make_shared<royale::usecase::UseCaseEightPhase> (
        15u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 710u }, 710u, 710u, 200u,
        royale::usecase::ExposureGray::Off,
        royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
        true, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_delta_60240kHz),
        royale::moduleconfig::CommonId2Frequencies,
        royale::CallbackData::Depth
      },
      royale::usecase::UseCase {
        "MODE_9_20FPS",
        std::make_shared<royale::usecase::UseCaseEightPhase> (
        20u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 530u }, 530u, 530u, 200u,
        royale::usecase::ExposureGray::Off,
        royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
        true, ssc_freq, ssc_kspread, ssc_delta_80320kHz, ssc_delta_60240kHz),
        royale::moduleconfig::CommonId2Frequencies,
        royale::CallbackData::Depth
      },
      royale::usecase::UseCase {
        "MODE_9_30FPS",
        std::make_shared<royale::usecase::UseCaseEightPhase> (
        30u, 80320000, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 350u }, 350u, 350u, 200u,
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
      royale::usecase::UseCase {
          "MODE_5_15FPS",
          std::make_shared<royale::usecase::UseCaseFourPhase> (
          15u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 1430u }, 1430u, 200u,
          royale::usecase::ExposureGray::Off,
          royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
          true, ssc_freq, ssc_kspread, ssc_delta_60240kHz),
          royale::moduleconfig::CommonId1Frequency,
          royale::CallbackData::Depth
      },
      royale::usecase::UseCase {
          "MODE_5_30FPS",
          std::make_shared<royale::usecase::UseCaseFourPhase> (
          30u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 710u }, 710u, 200u,
          royale::usecase::ExposureGray::Off,
          royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
          true, ssc_freq, ssc_kspread, ssc_delta_60240kHz),
          royale::moduleconfig::CommonId1Frequency,
          royale::CallbackData::Depth
      },
      royale::usecase::UseCase {
          "MODE_5_45FPS",
          std::make_shared<royale::usecase::UseCaseFourPhase> (
          45u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 470u }, 470u, 200u,
          royale::usecase::ExposureGray::Off,
          royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
          true, ssc_freq, ssc_kspread, ssc_delta_60240kHz),
          royale::moduleconfig::CommonId1Frequency,
          royale::CallbackData::Depth
      },
      royale::usecase::UseCase {
          "MODE_5_60FPS",
          std::make_shared<royale::usecase::UseCaseFourPhase> (
          60u, 60240000, royale::Pair<uint32_t, uint32_t> { MIN_EXPO_NR, 350u }, 350u, 200u,
          royale::usecase::ExposureGray::Off,
          royale::usecase::IntensityPhaseOrder::IntensityFirstPhase,
          true, ssc_freq, ssc_kspread, ssc_delta_60240kHz),
          royale::moduleconfig::CommonId1Frequency,
          royale::CallbackData::Depth
      },
    },
    royale::config::BandwidthRequirementCategory::NO_THROTTLING,
    royale::config::FrameTransmissionMode::SUPERFRAME,
    "A66.1",
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

std::shared_ptr<royale::config::ModuleConfig> platform::getModuleConfigCustom()
{
    auto config = std::make_shared<ModuleConfig> (moduleConfigCustom);
    return config;
}
