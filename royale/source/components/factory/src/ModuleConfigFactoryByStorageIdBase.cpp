/****************************************************************************\
* Copyright (C) 2018 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <common/Crc32.hpp>
#include <common/exceptions/DataNotFound.hpp>
#include <common/exceptions/RuntimeError.hpp>
#include <common/RoyaleLogger.hpp>
#include <config/FlashMemoryConfig.hpp>
#include <factory/ModuleConfigFactoryByStorageIdBase.hpp>
#include <storage/NonVolatileStorageShadow.hpp>

#include <memory>

using namespace royale::common;
using namespace royale::config;
using namespace royale::factory;
using namespace royale::hal;
using namespace royale::storage;

ModuleConfigFactoryByStorageIdBase::ModuleConfigFactoryByStorageIdBase (
    const royale::Vector<royale::Pair<const royale::Vector<uint8_t>, const royale::config::ModuleConfig>> configs,
    const royale::Vector<uint8_t> &defaultId) :
    m_moduleConfigs (configs),
    m_defaultId (defaultId)
{
}

std::shared_ptr<royale::config::ModuleConfig>
ModuleConfigFactoryByStorageIdBase::findConfig (const royale::Vector<uint8_t> &id) const
{
    LOG (DEBUG) << "Searching for module identifier with CRC32 \"" << calculateCRC32 (id.data(), id.size()) << "\"";
    for (const auto &x : m_moduleConfigs)
    {
        if (id == x.first)
        {
            return std::make_shared<ModuleConfig> (x.second);
        }
    }

    return nullptr;
}

std::shared_ptr<royale::config::ModuleConfig>
ModuleConfigFactoryByStorageIdBase::readAndCreate (royale::hal::INonVolatileStorage &storage, bool cacheOnDisk) const
{
    std::shared_ptr<royale::config::ModuleConfig> config = nullptr;

    try
    {
        config = findConfig (storage.getModuleIdentifier());
    }
    catch (...)
    {
        LOG (DEBUG) << "Retrieving the module identifier failed";
    }
    if (config == nullptr && !m_defaultId.empty())
    {
        LOG (DEBUG) << "Falling back to default module config";
        config = findConfig (m_defaultId);
    }

    if (config)
    {
        if (config->flashMemoryConfig.type == FlashMemoryConfig::FlashMemoryType::FIXED
                && config->flashMemoryConfig.nonVolatileStorageFixed == nullptr)
        {
            LOG (DEBUG) << "Caching calibration data";
            try
            {
                config->flashMemoryConfig.nonVolatileStorageFixed = std::make_shared<NonVolatileStorageShadow> (storage,
                        cacheOnDisk);
            }
            catch (const DataNotFound &)
            {
                // The module has no calibration data.  If the user tries to use depth data this
                // will fail at a later point, but if the user only wants to use raw data
                // the calibration data is not needed.
                LOG (DEBUG) << "DataNotFound for the calibration data";
                LOG (DEBUG) << "Setting FlashMemoryType to NONE";
                config->flashMemoryConfig.type = FlashMemoryConfig::FlashMemoryType::NONE;
            }
            catch (const RuntimeError &)
            {
                // Generic problem reading the calibration data.  If the user tries to use depth
                // data this will fail at a later point, but if the user only wants to use raw data
                // the calibration data is not needed.
                LOG (DEBUG) << "Error reading out calibration data";
                LOG (DEBUG) << "Setting FlashMemoryType to NONE";
                config->flashMemoryConfig.type = FlashMemoryConfig::FlashMemoryType::NONE;
            }
        }

        return config;
    }

    LOG (WARN) << "No matches for module identifier found";
    return nullptr;
}

std::shared_ptr<royale::config::ModuleConfig>
ModuleConfigFactoryByStorageIdBase::createWithoutStorage () const
{
    if (m_defaultId.empty())
    {
        return nullptr;
    }

    auto config = findConfig (m_defaultId);
    if (config)
    {
        config->flashMemoryConfig.type = FlashMemoryConfig::FlashMemoryType::NONE;
    }
    return config;
}

royale::Vector<std::shared_ptr<const royale::config::ModuleConfig>> ModuleConfigFactoryByStorageIdBase::enumerateConfigs() const
{
    royale::Vector<std::shared_ptr<const royale::config::ModuleConfig>> ret;
    for (const auto &x : m_moduleConfigs)
    {
        ret.emplace_back (std::make_shared<ModuleConfig> (x.second));
    }

    return ret;
}
