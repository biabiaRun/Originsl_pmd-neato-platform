/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <factory/NonVolatileStorageFactory.hpp>
#include <storage/IBridgeWithPagedFlash.hpp>

#include <storage/NonVolatileStoragePersistent.hpp>
#include <storage/StorageFormatPicoLegacy.hpp>
#include <storage/StorageFormatPolar.hpp>
#include <storage/StorageFormatZwetschge.hpp>

#include <common/SensorRoutingConfigI2c.hpp>
#include <common/SensorRoutingConfigSpi.hpp>
#include <config/SensorRoutingFilename.hpp>
#include <config/SensorRoutingImagerAsBridge.hpp>

#include <pal/ISpiBusAccess.hpp>

#if defined(ROYALE_BRIDGE_EXTENSION_ARCTIC)
#include <usb/pal/SpiBusAccessArctic.hpp>
#include <usb/bridge/StorageSpiFlashArctic.hpp>
#endif

#include <pal/II2cBusAccess.hpp>
#include <pal/Access2I2cDeviceAdapter.hpp>
#include <storage/StorageI2cEeprom.hpp>

#include <storage/StorageFile.hpp>

#include <hal/IBridgeImager.hpp>
#include <storage/SpiImagerFactory.hpp>
#include <imager/ImagerLenaReader.hpp>

#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/NotImplemented.hpp>
#include <common/exceptions/ImagerConfigNotFoundError.hpp>
#include <common/FileSystem.hpp>

using namespace royale::config;
using namespace royale::common;
using namespace royale::factory;
using namespace royale::imager;
using namespace royale::storage;

std::shared_ptr<royale::hal::INonVolatileStorage> NonVolatileStorageFactory::createFlash (
    royale::factory::IBridgeFactory &bridgeFactory,
    const royale::config::FlashMemoryConfig  &flashMemoryConfig,
    const royale::common::ISensorRoutingConfig *sensorRouting,
    NonVolatileStorageFactoryConstraint constraint)
{
    std::shared_ptr<royale::hal::INonVolatileStorage> flash;

    try
    {
        switch (flashMemoryConfig.type)
        {
            case FlashMemoryConfig::FlashMemoryType::PICO_PAGED:
                if (bridgeFactory.supports<IBridgeWithPagedFlash>())
                {
                    // IBridgeWithPagedFlash doesn't need a route in the m_essentialSensors map
                    flash.reset (new StorageFormatPicoLegacy (bridgeFactory.create<IBridgeWithPagedFlash>(), flashMemoryConfig));
                }
                else
                {
                    throw NotImplemented ("StorageFormatPicoLegacy only supports IBridgeWithPagedFlash");
                }
                break;
            case FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM:
                {
                    auto storage = createStorageReadRandom (bridgeFactory, flashMemoryConfig, sensorRouting, constraint);
                    flash.reset (new StorageFormatPolar (std::move (storage), flashMemoryConfig.identifierIsMandatory));
                    break;
                }
            case FlashMemoryConfig::FlashMemoryType::FIXED:
                {
                    flash = flashMemoryConfig.nonVolatileStorageFixed;
                    break;
                }
            case FlashMemoryConfig::FlashMemoryType::JUST_CALIBRATION:
                {
                    auto fileRoute = dynamic_cast<const SensorRoutingFilename *> (sensorRouting);
                    if (fileRoute)
                    {
                        flash = std::make_shared<NonVolatileStoragePersistent> (fileRoute->getFilename());
                    }
                    else
                    {
                        // A mostly-stubbed implementation similar to NonVolatileStoragePersistent
                        // could be supported on top of IStorageReadRandom; it's currently
                        // unimplemented because we don't have a need for it.
                        throw InvalidValue ("Unsupported storage configuration for JUST_CALIBRATION");
                    }
                    break;
                }
            case FlashMemoryConfig::FlashMemoryType::NONE:
                // no flash, will return nullptr.
                break;
            case FlashMemoryConfig::FlashMemoryType::ZWETSCHGE:
                {
                    // We could support this, and simply return the INonVolatileStorage part
                    throw NotImplemented ("StorageFormatZwetschge is only supported when returning ExternalConfig");
                }
            default:
                throw LogicError ("Flash memory type not supported");
        }
    }
    catch (...)
    {
        // log and rethrow error
        LOG (ERROR) << "Failed to initialize storage.";
        throw;
    }

    return flash;
}

std::shared_ptr<royale::pal::IStorageReadRandom> NonVolatileStorageFactory::createStorageReadRandom (
    royale::factory::IBridgeFactory &bridgeFactory,
    const royale::config::FlashMemoryConfig  &flashMemoryConfig,
    const royale::common::ISensorRoutingConfig *sensorRouting,
    NonVolatileStorageFactoryConstraint constraint)
{
    std::shared_ptr<royale::pal::IStorageReadRandom> storage;

    if (sensorRouting == nullptr)
    {
        throw InvalidValue ("Need sensorRouting to create IStorageReadRandom");
    }

    auto spiRoute = dynamic_cast<const SensorRoutingConfigSpi *> (sensorRouting);
    auto i2cRoute = dynamic_cast<const SensorRoutingConfigI2c *> (sensorRouting);
    auto imagerRoute = dynamic_cast<const SensorRoutingImagerAsBridge *> (sensorRouting);
    auto fileRoute = dynamic_cast<const SensorRoutingFilename *> (sensorRouting);
    if (spiRoute != nullptr && bridgeFactory.supports<royale::pal::ISpiBusAccess>())
    {
#if defined(ROYALE_BRIDGE_EXTENSION_ARCTIC)
        auto spiAccess = bridgeFactory.create<royale::pal::ISpiBusAccess>();
        auto arcticAccess = std::dynamic_pointer_cast<royale::usb::pal::arctic::SpiBusAccessArctic> (spiAccess);
        if (arcticAccess != nullptr)
        {
            storage = std::make_shared<royale::usb::bridge::StorageSpiFlashArctic> (flashMemoryConfig, arcticAccess, spiRoute->getAddress());
        }
#endif
        if (storage == nullptr)
        {
            throw NotImplemented ("SPI storage is only supported on Arctic devices at the moment");
        }
    }
    else if (i2cRoute != nullptr && bridgeFactory.supports<royale::pal::II2cBusAccess>())
    {
        auto i2cAccess = bridgeFactory.create<royale::pal::II2cBusAccess>();
        storage = std::make_shared<StorageI2cEeprom> (flashMemoryConfig, i2cAccess, i2cRoute->getAddress());
    }
    else if (imagerRoute != nullptr && bridgeFactory.supports<royale::hal::IBridgeImager>())
    {
        if (constraint != NonVolatileStorageFactoryConstraint::CONTROLLED_LIFETIME)
        {
            // A LogicError instead of an InvalidValue, because the module should not have been
            // configured to reach this line.
            throw LogicError ("Can't create an StorageSpiImager without exclusive use of the imager");
        }

        auto bridgeImager = bridgeFactory.create<royale::hal::IBridgeImager>();
        storage = SpiImagerFactory::createStorageImager (flashMemoryConfig, bridgeImager, *imagerRoute);
    }
    else if (fileRoute != nullptr)
    {
        storage = std::make_shared<StorageFile> (flashMemoryConfig, fileRoute->getFilename());
    }

    if (!storage)
    {
        throw InvalidValue ("Sensor routing not supported by this bridge");
    }

    return storage;
}

royale::config::ExternalConfig NonVolatileStorageFactory::createExternalConfig (
    const royale::config::ExternalConfigFileConfig &storageConfig)
{
    royale::config::ExternalConfig externalConfig;

    if (!storageConfig.zwetschgeFile.empty())
    {
        auto storage = std::make_shared<StorageFile> (FlashMemoryConfig {}, storageConfig.zwetschgeFile);
        auto zwetschge = StorageFormatZwetschge {storage};
        auto config = zwetschge.getExternalConfig();
        externalConfig = std::move (config);
    }
    else if (!storageConfig.lenaString.empty())
    {
        externalConfig.imagerExternalConfig = ImagerLenaReader::fromString (storageConfig.lenaString);
    }
    else if (!storageConfig.lenaFile.empty())
    {
        // We have no external configuration as a string, so we have to load it from
        // a file
        if (royale::common::fileexists (royale::String (storageConfig.lenaFile)))
        {
            externalConfig.imagerExternalConfig = ImagerLenaReader::fromFile (storageConfig.lenaFile);
        }
        else
        {
            royale::common::ImagerConfigNotFoundError imagerConfigNotFoundError;

            LOG (ERROR) << ("External imager configuration file not found");
            imagerConfigNotFoundError.setConfigFileName (storageConfig.lenaFile);

            throw imagerConfigNotFoundError;
        }
    }
    else
    {
        // This shouldn't happen, we need the configuration
        LOG (WARN) << "NonVolatileStorageFactory : No or unsupported external configuration source";
        throw royale::common::RuntimeError ("No or unsupported external configuration source");
    }

    return externalConfig;
}

royale::config::ExternalConfig NonVolatileStorageFactory::createExternalConfig (
    royale::factory::IBridgeFactory &bridgeFactory,
    const royale::config::FlashMemoryConfig  &flashMemoryConfig,
    const royale::common::ISensorRoutingConfig *sensorRouting,
    NonVolatileStorageFactoryConstraint constraint)
{
    // Most of the code in createFlash could be moved here, and then createFlash could be
    // implemented by returning just the INonVolatileStorage. But I'm choosing to implement this for
    // Zwetschge-in-IStorageReadRandom only.
    if (flashMemoryConfig.type != FlashMemoryConfig::FlashMemoryType::ZWETSCHGE)
    {
        throw NotImplemented ();
    }

    auto storage = createStorageReadRandom (bridgeFactory, flashMemoryConfig, sensorRouting, constraint);
    auto zwetschge = StorageFormatZwetschge {storage};
    auto externalConfig = zwetschge.getExternalConfig();
    return externalConfig;
}
