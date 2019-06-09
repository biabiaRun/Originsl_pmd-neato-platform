/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <factory/CameraCoreBuilderImpl.hpp>
#include <common/MakeUnique.hpp>

#include <common/exceptions/LogicError.hpp>

using namespace royale::factory;
using namespace royale::config;
using namespace royale::common;


CameraCoreBuilderImpl::CameraCoreBuilderImpl()
    : m_illuminationConfig(),
      m_temperatureSensorConfig(),
      m_flashMemoryConfig(),
      m_accessLevel (CameraAccessLevel::L1)
{
}

void CameraCoreBuilderImpl::setBridgeFactory (std::unique_ptr<royale::factory::IBridgeFactory> bridgeFactory)
{
    m_bridgeFactory = std::move (bridgeFactory);
}

void CameraCoreBuilderImpl::setConfig (const std::shared_ptr<const royale::config::ICoreConfig> &coreConfig,
                                       const std::shared_ptr<const royale::config::ImagerConfig> &imagerConfig,
                                       const royale::config::IlluminationConfig &illuminationConfig)
{
    m_config = coreConfig;
    m_imagerConfig = imagerConfig;
    m_illuminationConfig = illuminationConfig;
}

void CameraCoreBuilderImpl::setEssentialSensors (const royale::common::SensorMap &essentialSensors)
{
    m_essentialSensors = essentialSensors;
}

void CameraCoreBuilderImpl::setTemperatureSensorConfig (const royale::config::TemperatureSensorConfig &temperatureSensorConfig)
{
    m_temperatureSensorConfig = temperatureSensorConfig;
}

void CameraCoreBuilderImpl::setFlashMemoryConfig (const royale::config::FlashMemoryConfig &flashMemoryConfig)
{
    m_flashMemoryConfig = flashMemoryConfig;
}

std::shared_ptr<const royale::config::ICoreConfig> CameraCoreBuilderImpl::getICoreConfig() const
{
    return m_config;
}

royale::config::ImagerType CameraCoreBuilderImpl::getImagerType() const
{
    return m_imagerConfig->imagerType;
}

royale::factory::IBridgeFactory &CameraCoreBuilderImpl::getBridgeFactory()
{
    if (m_bridgeFactory != nullptr)
    {
        return *m_bridgeFactory;
    }
    else
    {
        throw LogicError ("No bridge factory in CameraCoreBuilder!");
    }
}

void CameraCoreBuilderImpl::setAccessLevel (royale::CameraAccessLevel accessLevel)
{
    m_accessLevel = accessLevel;
}
