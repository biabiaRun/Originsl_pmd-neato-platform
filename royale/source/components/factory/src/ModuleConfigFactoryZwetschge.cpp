/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/exceptions/WrongDataFormatFound.hpp>
#include <common/FileSystem.hpp>
#include <factory/ImagerFactory.hpp>
#include <factory/ModuleConfigFactoryZwetschge.hpp>
#include <factory/NonVolatileStorageFactory.hpp>
#include <storage/StorageFile.hpp>
#include <storage/StorageFormatZwetschge.hpp>

using namespace royale::common;
using namespace royale::config;
using namespace royale::factory;
using namespace royale::imager;
using namespace royale::storage;

ModuleConfigFactoryZwetschge::ModuleConfigFactoryZwetschge (
    const std::shared_ptr<ISensorRoutingConfig> &route,
    const FlashMemoryConfig &memoryConfig,
    const royale::Vector<royale::Pair<const royale::Vector<uint8_t>, const royale::config::ModuleConfig>> configs,
    const royale::Vector<uint8_t> &defaultId) :
    ModuleConfigFactoryByStorageIdBase (configs, defaultId),
    m_route (route),
    m_memoryConfig (memoryConfig)
{
}

std::shared_ptr<const royale::config::ModuleConfig>
ModuleConfigFactoryZwetschge::probeAndCreate (royale::factory::IBridgeFactory &bf) const
{
    try
    {
        auto storageFlash = NonVolatileStorageFactory::createStorageReadRandom (bf, m_memoryConfig, m_route.get(),
                            NonVolatileStorageFactoryConstraint::CONTROLLED_LIFETIME);
        auto zwetschgeFlash = StorageFormatZwetschge{ storageFlash };

        ExternalConfig externalConfig;

        royale::String zwetschgeBackupFile = ZWETSCHGE_BACKUP_FILE;

        std::shared_ptr<StorageFormatZwetschge> zwetschgeFile;

        bool fileFound = false;
        bool flashFound = false;
        bool flashCalibFound = false;
        bool convertSeqRegMaps = true;

        try
        {
            externalConfig = zwetschgeFlash.getExternalConfig();
            flashFound = true;
        }
        catch (...)
        {
            // We can't use the information from flash
        }

        bool useCaching = m_memoryConfig.useCaching;
        if (flashFound)
        {
            auto calib = externalConfig.calibration;
            auto calibCRC = calib->getCalibrationDataChecksum();
            if (calibCRC == 0u)
            {
                // The CRC is zero, that's why we assume that there is no calibration data
                // in the Zwetschge on the flash. Try to load a Zwetschge with calibration
                // data from the file system.
                // This workaround has to be used for some modules with the wrong flash capacity.
                auto serialFile = calib->getModuleSerialNumber() + ".zwetschge";

                if (fileexists (serialFile))
                {
                    zwetschgeBackupFile = serialFile;
                    convertSeqRegMaps = false;
                }
            }
            else
            {
                flashCalibFound = true;
            }
        }

        // Check if a default file exists and if it really is a Zwetschge file
        if (fileexists (zwetschgeBackupFile))
        {
            try
            {
                auto storageFile = std::make_shared<StorageFile> (FlashMemoryConfig{}, zwetschgeBackupFile);
                zwetschgeFile = std::make_shared<StorageFormatZwetschge> (storageFile);
                fileFound = true;
            }
            catch (...)
            {
                // We can't use the information from the file
            }
        }

        if (fileFound &&
                (!flashFound || (flashFound && !flashCalibFound)))
        {
            LOG (DEBUG) << "Using Zwetschge from file : " << zwetschgeBackupFile;
            // We don't have any information on the flash -> use the information from the file.
            // If the config contains sequential register maps -> convert them to timed register maps.
            externalConfig = zwetschgeFile->getExternalConfig (convertSeqRegMaps, false);
            useCaching = false;
        }
        else
        {
            try
            {
                // We don't have a file -> use the information from the flash
                externalConfig = zwetschgeFlash.getExternalConfig();
            }
            catch (...)
            {
                throw DataNotFound ("No flash information available");
            }
        }

        auto modConfig = readAndCreate (* (externalConfig.calibration), useCaching);
        if (modConfig != nullptr)
        {
            modConfig->imagerConfig.externalImagerConfig = std::move (externalConfig.imagerExternalConfig);
            if (!externalConfig.royaleUseCaseList.empty())
            {
                modConfig->coreConfigData.useCases = std::move (externalConfig.royaleUseCaseList);
            }
            // externalConfig->calibration is handled by the superclass.  It will be cached if the
            // ModuleConfig uses FlashMemoryType::FIXED with nullptr, and in every case the
            // CONTROLLED_LIFETIME INonVolatileStorage won't be put in to the ModuleConfig.
            return modConfig;
        }
    }
    catch (DataNotFound &)
    {
        static_assert (std::is_base_of<DataNotFound, WrongDataFormatFound>::value,
                       "This should catch WrongDataFormatFound too");
        return createWithoutStorage();
    }
    catch (...)
    {
        // Fall through to the return nullptr line
    }

    return nullptr;
}
