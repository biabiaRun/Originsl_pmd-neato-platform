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
    /**
     * Common data for devices using PMD's USB dongles with IRS238xC and IRS277x imagers (M2453 and M2455 respectively).
     */
    namespace pmdModule
    {
        static const double ssc_freq = 10000.;
        static const double ssc_kspread = 0.5;
        static const double ssc_delta_80320kHz = 0.0125;
        static const double ssc_delta_60240kHz = 0.0166;

        static const royale::config::IlluminationConfig illuConfig = royale::config::IlluminationConfig{ royale::usecase::RawFrameSet::DutyCycle::DC_25, 90000000, royale::config::IlluminationPad::SE_P };
        static const royale::config::TemperatureSensorConfig tempsensorConfig = royale::config::TemperatureSensorConfig{ royale::config::TemperatureSensorConfig::TemperatureSensorType::PSEUDODATA,
                                                             std::make_shared<royale::config::NTCTemperatureSensorConfig> (6800.0f, 100000.0f, 25.0f, 4200.0f),
                                                             hal::IPsdTemperatureSensor::PseudoDataPhaseSync::SECOND };

        static royale::config::FlashMemoryConfig flashConfig = royale::config::FlashMemoryConfig{ royale::config::FlashMemoryConfig::FlashMemoryType::FIXED };

        static const royale::common::SensorMap sensorMap = royale::common::SensorMap
        {
            { royale::common::SensorRole::MAIN_IMAGER, std::make_shared<royale::common::SensorRoutingConfigI2c> (0x3D) }
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
                    true, pmdModule::ssc_freq, pmdModule::ssc_kspread, pmdModule::ssc_delta_80320kHz, pmdModule::ssc_freq,
                    pmdModule::ssc_kspread, pmdModule::ssc_delta_60240kHz)
            {
                m_typeName = name;
                m_identifier = royale::usecase::UseCaseIdentifier::parseRfc4122 (name);
            }
        };
    }

    /**
     * Data for devices using PMD's USB dongles with IRS238xC imagers (M2453 imagers).
     */
    namespace pmdModule238x
    {
        static const royale::config::ImagerConfig imagerConfig = royale::config::ImagerConfig
        {
            royale::config::ImagerType::M2453_B11,
            24000000,
            {},
            0.0,
            royale::config::ImageDataTransferType::MIPI_2LANE,
            royale::config::ImagerConfig::Trigger::I2C, royale::config::ImConnectedTemperatureSensor::NTC,
            royale::config::ExternalConfigFileConfig::empty (),
            false
        };
    }

    /**
     * Data for devices using PMD's USB dongles with IRS277xC imagers (M2455 imagers).
     */
    namespace pmdModule277x
    {
        static const royale::config::ImagerConfig imagerConfig = royale::config::ImagerConfig
        {
            royale::config::ImagerType::M2455_A11,
            24000000,
            {},
            0.0,
            royale::config::ImageDataTransferType::MIPI_2LANE,
            royale::config::ImagerConfig::Trigger::I2C, royale::config::ImConnectedTemperatureSensor::NTC,
            royale::config::ExternalConfigFileConfig::empty (),
            false
        };
    }
}
