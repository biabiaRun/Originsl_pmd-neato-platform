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

#include <device/CameraCore.hpp>
#include <usb/factory/IBridgeFactory.hpp>
#include <config/ICoreConfig.hpp>
#include <config/ImagerConfig.hpp>
#include <config/IlluminationConfig.hpp>
#include <royale/ICameraDevice.hpp>

#include <common/SensorMap.hpp>
#include <config/TemperatureSensorConfig.hpp>
#include <config/FlashMemoryConfig.hpp>

#include <memory>

namespace royale
{
    namespace factory
    {
        /**
         * Factory for step-wise construction of a CameraCore.
         *
         * This is intended to be used for creating a single instance of CameraCore only.
         * Before calling createCameraCore(), all necessary setters should be called.
         *
         * In addition to the CameraCore itself, implementations of this interface will also
         * need to instantiate the external components required for creating the CameraCore
         * (e.g. temperature sensor, imager, calibration storage).
         *
         */
        class ICameraCoreBuilder
        {
        public:
            virtual ~ICameraCoreBuilder() = default;
            virtual void setBridgeFactory (std::unique_ptr<royale::factory::IBridgeFactory> bridgeFactory) = 0;
            virtual void setConfig (const std::shared_ptr<const royale::config::ICoreConfig> &coreConfig,
                                    const std::shared_ptr<const royale::config::ImagerConfig> &imagerConfig,
                                    const royale::config::IlluminationConfig &illuminationConfig) = 0;
            virtual void setEssentialSensors (const royale::common::SensorMap &essentialSensors) = 0;
            virtual void setTemperatureSensorConfig (const royale::config::TemperatureSensorConfig &temperatureSensorConfig) = 0;
            virtual void setFlashMemoryConfig (const royale::config::FlashMemoryConfig &flashMemoryConfig) = 0;
            virtual void setAccessLevel (CameraAccessLevel accessLevel) = 0;
            virtual std::shared_ptr<const royale::config::ICoreConfig> getICoreConfig() const = 0;
            virtual royale::config::ImagerType getImagerType() const = 0;
            virtual royale::factory::IBridgeFactory &getBridgeFactory() = 0;
            virtual std::unique_ptr<royale::device::CameraCore> createCameraCore () = 0;

        };
    }
}
