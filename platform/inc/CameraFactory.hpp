/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies & pmdtechnologies ag
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <hal/IImager.hpp>
#include <hal/IBridgeDataReceiver.hpp>
#include <hal/INonVolatileStorage.hpp>
#include <hal/IBufferCaptureReleaser.hpp>
#include <pal/II2cBusAccess.hpp>
#include <royale/ICameraDevice.hpp>
#include <royale/Vector.hpp>
#include <config/ModuleConfig.hpp>
#include <hal/IBridgeImager.hpp>
#include <royale/Definitions.hpp>

namespace platform
{
    class CameraFactory
    {
    public:
        ROYALE_API CameraFactory();

        /**
         * This method creates an instance of the ICameraDevice.
         *
         * It requires an implementation of three interfaces:
         *
         * - IBridgeImager
         * - IBridgeDataReceiver
         * - II2CAccess
         *
         * and a configuration of the module itself
         */
        ROYALE_API std::unique_ptr<royale::ICameraDevice> createDevice (
            std::shared_ptr<royale::config::ModuleConfig> config,
            std::shared_ptr<royale::hal::IBridgeImager> bridgeImager,
            std::shared_ptr<royale::hal::IBridgeDataReceiver> bridgeDataReceiver,
            std::shared_ptr<royale::pal::II2cBusAccess> i2cAccess);

        ROYALE_API std::unique_ptr<royale::ICameraDevice> createCamera();

#ifdef ROYALE_FACTORY_PLAYBACK
        ROYALE_API std::unique_ptr<royale::ICameraDevice> createPlaybackDevice (const char *fileName);
#endif

        ROYALE_API virtual ~CameraFactory() = default;

    };
}
