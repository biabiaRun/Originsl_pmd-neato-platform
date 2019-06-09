/****************************************************************************\
 * Copyright (C) 2018 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#pragma once

#include <common/MakeUnique.hpp>
#include <common/SensorRoutingConfigI2c.hpp>
#include <modules/ModuleConfigData.hpp>
#include <royale/ProcessingFlag.hpp>
#include <usecase/UseCaseCalibration.hpp>
#include <usecase/UseCaseEightPhase.hpp>
#include <usecase/UseCaseFourPhase.hpp>

using namespace royale;
using namespace royale::common;
using namespace royale::config;

namespace royale
{
    namespace selene
    {

        static const double ssc_freq = 10000.;
        static const double ssc_kspread = 0.5;
        static const double ssc_delta_80320kHz = 0.0125;
        static const double ssc_delta_60240kHz = 0.0166;


        static const ImagerConfig imagerConfig = ImagerConfig
        {
            royale::config::ImagerType::M2453_B11,
            24000000,
            {},
            0.0,
            royale::config::ImageDataTransferType::MIPI_2LANE,
            ImagerConfig::Trigger::I2C, ImConnectedTemperatureSensor::NTC,
            royale::config::ExternalConfigFileConfig::empty (),
            false
        };
        static const IlluminationConfig illuConfig = IlluminationConfig{ royale::usecase::RawFrameSet::DutyCycle::DC_25, 90000000, royale::config::IlluminationPad::SE_P };
        static const TemperatureSensorConfig tempsensorConfig = TemperatureSensorConfig{ TemperatureSensorConfig::TemperatureSensorType::PSEUDODATA,
                                                                                         std::make_shared<royale::config::NTCTemperatureSensorConfig> (6800.0f, 100000.0f, 25.0f, 4200.0f),
                                                                                         hal::IPsdTemperatureSensor::PseudoDataPhaseSync::SECOND };

        static FlashMemoryConfig flashConfig = FlashMemoryConfig{ FlashMemoryConfig::FlashMemoryType::FIXED };

        static const SensorMap sensorMap = SensorMap
        {
            { SensorRole::MAIN_IMAGER, std::make_shared<SensorRoutingConfigI2c> (0x3D) }
        };

        class NameIsIdentifierUseCaseFourPhase : public royale::usecase::UseCaseFourPhase
        {
        public:
            NameIsIdentifierUseCaseFourPhase (const royale::String &name,
                                              const uint16_t fps,
                                              royale::Pair<uint32_t, uint32_t> exposureLimits,
                                              uint32_t exposureModulation) :
                UseCaseFourPhase (
                    fps, 60240000, exposureLimits, exposureModulation, 100u,
                    royale::usecase::ExposureGray::Off,
                    royale::usecase::IntensityPhaseOrder::IntensityFirstPhase)
            {
                m_typeName = name;
                m_identifier = royale::usecase::UseCaseIdentifier::parseRfc4122 (name);
            }
        };

        class NameIsIdentifierUseCaseEightPhase : public royale::usecase::UseCaseEightPhase
        {
        public:
            NameIsIdentifierUseCaseEightPhase (const royale::String &name,
                                               const uint16_t fps,
                                               royale::Pair<uint32_t, uint32_t> exposureLimits,
                                               uint32_t exposureModulation) :
                UseCaseEightPhase (
                    fps, 80320000, 60240000, exposureLimits, exposureModulation, exposureModulation, 100u,
                    royale::usecase::ExposureGray::Off,
                    royale::usecase::IntensityPhaseOrder::IntensityFirstPhase)
            {
                m_typeName = name;
                m_identifier = royale::usecase::UseCaseIdentifier::parseRfc4122 (name);
            }
        };

        class NameIsIdentifierUseCaseCalibration : public royale::usecase::UseCaseCalibration
        {
        public:
            NameIsIdentifierUseCaseCalibration (const royale::String &name,
                                                const uint16_t fps,
                                                royale::Pair<uint32_t, uint32_t> exposureLimits,
                                                uint32_t exposureModulation) :
                UseCaseCalibration (
                    fps, 80320000, 60240000, exposureLimits, exposureModulation, exposureModulation, 100u, 100u,
                    true, selene::ssc_freq, selene::ssc_kspread, selene::ssc_delta_80320kHz, selene::ssc_freq,
                    selene::ssc_kspread, selene::ssc_delta_60240kHz)
            {
                m_typeName = name;
                m_identifier = royale::usecase::UseCaseIdentifier::parseRfc4122 (name);
            }
        };
    }
}

