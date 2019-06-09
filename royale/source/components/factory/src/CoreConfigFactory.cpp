/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <factory/CoreConfigFactory.hpp>
#include <common/exceptions/LogicError.hpp>
#include <config/CoreConfigAdapter.hpp>

#include <algorithm>

using namespace royale::config;
using namespace royale::factory;

CoreConfigFactory::CoreConfigFactory (const royale::config::CoreConfig &data,
                                      std::shared_ptr<royale::factory::IProcessingParameterMapFactory> factory) :
    m_data (data),
    m_paramFactory (std::move (factory))
{
    if (!m_paramFactory)
    {
        // This check could be deferred until operator() is called for a config that uses a
        // ProcessingParameterId. However, checking it here means that a missing factory will be
        // detected when tested with any hardware, not just with a specific module.
        throw common::LogicError ("CoreConfigFactory constructed without an IProcessingParameterMapFactory");
    }
}

CoreConfigFactory::CoreConfigFactory (const royale::config::ModuleConfig &data,
                                      std::shared_ptr<royale::factory::IProcessingParameterMapFactory> factory) :
    CoreConfigFactory (data.coreConfigData, std::move (factory))
{
}

std::unique_ptr<royale::config::ICoreConfig> CoreConfigFactory::operator() ()
{
    // make a copy of m_data
    auto config = common::makeUnique<royale::config::CoreConfig> (m_data);

    auto maxImageWidth = config->maxImageSize.first;
    auto maxImageHeight = config->maxImageSize.second;

    for (auto &useCase : config->useCases)
    {
        // set correct image size
        useCase.getDefinition()->setImage (maxImageWidth, maxImageHeight);

        // fill parameter maps
        Vector<ProcessingParameterMap> params;
        const auto paramId = useCase.getProcessingParameterId();
        if (paramId.isSentinel())
        {
            // The config must provide the map, via the combination of per-use-case and per-device
            // settings.
            params = useCase.getProcessingParameters();
            for (auto &curParams : params)
            {
                curParams = combineProcessingMaps (config->standardParameters, curParams);
            }
        }
        else
        {
            // \todo ROYAL-3354 CoreConfigFactory does not know the productId, and so it does not
            // yet support productId-specific overrides
            params = m_paramFactory->getParameterMaps ({}, useCase.getProcessingParameterId());
        }
        useCase.setProcessingParameters (params);
    }

    // check that the names are unique
    const auto ucEnd = config->useCases.end();
    auto ucIter = config->useCases.begin();
    while (ucIter != ucEnd)
    {
        const auto name = ucIter->getName();
        auto matchesName = [&name] (const royale::usecase::UseCase & x)
        {
            return x.getName() == name;
        };
        ucIter++;
        if (std::count_if (ucIter, ucEnd, matchesName))
        {
            LOG (ERROR) << "Multiple use cases called \"" << name << "\"";
            throw common::LogicError ("Multiple use cases with the same name");
        }
    }

    return common::makeUnique<CoreConfigAdapter> (std::move (config));
}
