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
#include <modules/ModuleConfigData.hpp>
#include <royale/ProcessingFlag.hpp>
#include <usecase/UseCaseCalibration.hpp>
#include <usecase/UseCaseEightPhase.hpp>
#include <usecase/UseCaseFourPhase.hpp>

#include <modules/ModuleConfigEuropaCommon.hpp>

#include <modules/CommonProcessingParameters.hpp>

const royale::config::ModuleConfig royale::config::moduleconfig::Europa2Full
{
    royale::config::CoreConfig
    {
        { 224, 84 },
        { 448, 168 },
        royale::usecase::UseCaseList{
            royale::usecase::UseCaseList{
                royale::usecase::UseCase{
                    "MODE_CALIBRATION",
                    std::make_shared<royale::europa::NameIsIdentifierUseCaseCalibration> ("{4cdd2641-c71c-4782-a7d4-d33367685903}",
                    5u, royale::Pair<uint32_t, uint32_t> { 1u, 1500u }, 1200u),
                    { royale::moduleconfig::CommonProcessingParams2Frequencies },
                    royale::CallbackData::Raw,
                    royale::CameraAccessLevel::L3
                },

            },
        },
        royale::config::BandwidthRequirementCategory::USB3_THROTTLING,
        royale::config::FrameTransmissionMode::SUPERFRAME,
        "Europa2Full",
        65.0f,
        60.0f,
        true
    },
    royale::config::ImagerConfig
    {
        royale::config::ImagerType::M2455_A14,
        24000000,
        {},
        0.0,
        royale::config::ImageDataTransferType::MIPI_2LANE,
        royale::config::ImagerConfig::Trigger::I2C, royale::config::ImConnectedTemperatureSensor::NTC,
        royale::config::ExternalConfigFileConfig::empty(),
        false
    },
    royale::config::IlluminationConfig{ royale::usecase::RawFrameSet::DutyCycle::DC_25, 90000000, royale::config::IlluminationPad::SE_P },
    royale::config::TemperatureSensorConfig{
        royale::config::TemperatureSensorConfig::TemperatureSensorType::PSD_NTC,
        std::make_shared<royale::config::NTCTemperatureSensorConfig> (6800.0f, 100000.0f, 25.0f, 4200.0f),
        hal::IPsdTemperatureSensor::PseudoDataPhaseSync::FIRST },
    royale::config::FlashMemoryConfig{ royale::config::FlashMemoryConfig::FlashMemoryType::FIXED },
    royale::common::SensorMap
    {
        { royale::common::SensorRole::MAIN_IMAGER, std::make_shared<royale::common::SensorRoutingConfigI2c> (0x3D) }
    }
};
