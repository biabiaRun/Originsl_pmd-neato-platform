/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <config/FlashMemoryConfig.hpp>
#include <factory/ModuleConfigFactoryByStorageId.hpp>
#include <factory/NonVolatileStorageFactory.hpp>

#include <memory>

using namespace royale::common;
using namespace royale::config;
using namespace royale::factory;
using namespace royale::pal;

ModuleConfigFactoryByStorageId::ModuleConfigFactoryByStorageId (const std::shared_ptr<ISensorRoutingConfig> &route,
        const FlashMemoryConfig &memoryConfig,
        const royale::Vector<royale::Pair<const royale::Vector<uint8_t>, const royale::config::ModuleConfig>> configs,
        const royale::Vector<uint8_t> &defaultId) :
    ModuleConfigFactoryByStorageIdBase (configs, defaultId),
    m_route (route),
    m_memoryConfig (memoryConfig)
{
}

std::shared_ptr<const royale::config::ModuleConfig>
ModuleConfigFactoryByStorageId::probeAndCreate (royale::factory::IBridgeFactory &bf) const
{
    try
    {
        auto storage = NonVolatileStorageFactory::createFlash (bf, m_memoryConfig, m_route.get(),
                       NonVolatileStorageFactoryConstraint::CONTROLLED_LIFETIME);
        if (storage)
        {
            auto modConfig = readAndCreate (* (storage), m_memoryConfig.useCaching);
            if (modConfig != nullptr)
            {
                auto extConfFile = modConfig->imagerConfig.externalConfigFileConfig;
                if (extConfFile != decltype (extConfFile) ::empty())
                {
                    auto extConfig = NonVolatileStorageFactory::createExternalConfig (extConfFile);
                    modConfig->imagerConfig.externalImagerConfig = std::move (extConfig.imagerExternalConfig);
                    if (! extConfig.royaleUseCaseList.empty())
                    {
                        modConfig->coreConfigData.useCases = std::move (extConfig.royaleUseCaseList);
                    }
                    // extConfig->calibration is ignored.  If calibration is provided by the device,
                    // then it's in `storage` rather than `extConfig`, and will be handled by the
                    // superclass.  Developers can already use calibration on the filesystem with
                    // ICameraDevice::setCalibrationData().
                }
                return modConfig;
            }
        }
    }
    catch (...)
    {
        // Fall through to the return nullptr line
    }

    // We found no identifier or it is set to zero
    return nullptr;

}
