/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <factory/ICameraCoreBuilder.hpp>

namespace royale
{
    namespace factory
    {
        // Helper class for ICameraCoreBuilder implementations.
        // Contains implementations for the getters and setters.
        class CameraCoreBuilderImpl : public ICameraCoreBuilder
        {
        protected:
            CameraCoreBuilderImpl();
            virtual ~CameraCoreBuilderImpl() = default;

            void setBridgeFactory (std::unique_ptr<royale::factory::IBridgeFactory> bridgeFactory) override;
            void setConfig (const std::shared_ptr<const royale::config::ICoreConfig> &coreConfig,
                            const std::shared_ptr<const royale::config::ImagerConfig> &imagerConfig,
                            const royale::config::IlluminationConfig &illuminationConfig) override;
            void setEssentialSensors (const royale::common::SensorMap &essentialSensors) override;
            void setTemperatureSensorConfig (const royale::config::TemperatureSensorConfig &temperatureSensorConfig) override;
            void setFlashMemoryConfig (const royale::config::FlashMemoryConfig &flashMemoryConfig) override;
            void setAccessLevel (CameraAccessLevel accessLevel) override;
            std::shared_ptr<const royale::config::ICoreConfig> getICoreConfig() const override;
            royale::config::ImagerType getImagerType() const override;
            royale::factory::IBridgeFactory &getBridgeFactory() override;

            // The user still needs to override this one:
            virtual std::unique_ptr<royale::device::CameraCore> createCameraCore() override = 0;

        protected:
            std::unique_ptr<royale::factory::IBridgeFactory> m_bridgeFactory;
            std::shared_ptr<const royale::config::ICoreConfig> m_config;
            std::shared_ptr<const royale::config::ImagerConfig> m_imagerConfig;
            royale::config::IlluminationConfig m_illuminationConfig;
            royale::common::SensorMap m_essentialSensors;
            royale::config::TemperatureSensorConfig m_temperatureSensorConfig;
            royale::config::FlashMemoryConfig m_flashMemoryConfig;
            CameraAccessLevel m_accessLevel;
        };

    }
}
