/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royale/Definitions.hpp>

#include <hal/IBridgeImager.hpp>
#include <pal/IStorageReadRandom.hpp>

#include <config/FlashMemoryConfig.hpp>
#include <config/SensorRoutingImagerAsBridge.hpp>

#include <memory>

namespace royale
{
    namespace storage
    {
        class SpiImagerFactory
        {
        public:
            /**
             * Some imagers can be used as SPI masters, and storage can be accessed via these
             * devices.
             *
             * This factory creates IStorageReadRandom instances assuming that the device is in a
             * state where the imager will be accessed exclusively by these instances, and that
             * instance is destroyed before a normal software imager class is used. The caller must
             * ensure this assumption is correct.
             */
            ROYALE_API static std::shared_ptr<royale::pal::IStorageReadRandom> createStorageImager (
                const royale::config::FlashMemoryConfig &config,
                std::shared_ptr<royale::hal::IBridgeImager> access,
                const royale::config::SensorRoutingImagerAsBridge &imagerRoute);
        };
    }
}
