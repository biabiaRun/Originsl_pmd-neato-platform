/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <storage/SpiBusMasterM2453.hpp>
#include <storage/SpiBusMasterM2455.hpp>
#include <storage/SpiGenericFlash.hpp>
#include <storage/SpiImagerFactory.hpp>
#include <storage/StorageSpiImagerM2452.hpp>

#include <common/exceptions/InvalidValue.hpp>

using namespace royale::common;
using namespace royale::config;
using namespace royale::storage;

std::shared_ptr<royale::pal::IStorageReadRandom> SpiImagerFactory::createStorageImager (
    const royale::config::FlashMemoryConfig &flashMemoryConfig,
    std::shared_ptr<royale::hal::IBridgeImager> bridgeImager,
    const SensorRoutingImagerAsBridge &imagerRoute)
{
    std::shared_ptr<royale::pal::IStorageReadRandom> storage;
    switch (imagerRoute.getImagerType())
    {
        case royale::config::ImagerAsBridgeType::M2452_SPI:
            storage = std::make_shared<StorageSpiImagerM2452> (flashMemoryConfig, bridgeImager, imagerRoute.getImagerType());
            break;
        case royale::config::ImagerAsBridgeType::M2453_SPI:
            {
                auto imager = std::make_shared<SpiBusMasterM2453> (bridgeImager, imagerRoute.getImagerType());
                storage = std::make_shared<SpiGenericFlash> (flashMemoryConfig, imager, SensorRoutingConfigSpi {ONLY_DEVICE_ON_IMAGERS_SPI});
                break;
            }
        case royale::config::ImagerAsBridgeType::M2455_SPI:
            {
                auto imager = std::make_shared<SpiBusMasterM2455> (bridgeImager, imagerRoute.getImagerType());
                storage = std::make_shared<SpiGenericFlash> (flashMemoryConfig, imager, SensorRoutingConfigSpi {ONLY_DEVICE_ON_IMAGERS_SPI});
                break;
            }
        default:
            throw InvalidValue ("Using this imager as storage access is not supported");
    }

    return storage;
}
