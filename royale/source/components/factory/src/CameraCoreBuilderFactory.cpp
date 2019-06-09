/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <factory/CameraCoreBuilderFactory.hpp>
#include <factory/CameraCoreBuilderMira.hpp>

#include <common/MakeUnique.hpp>
#include <common/exceptions/LogicError.hpp>

using namespace royale::factory;
using namespace royale::usb::config;
using namespace royale::config;
using namespace royale::common;


CameraCoreBuilderFactory::CameraCoreBuilderFactory (royale::usb::config::BridgeType type,
        const royale::config::ModuleConfig &config)
    : m_type (type),
      m_config (config)
{
}

std::unique_ptr<royale::factory::ICameraCoreBuilder> CameraCoreBuilderFactory::operator() ()
{
    std::unique_ptr<royale::factory::ICameraCoreBuilder> builder;

    switch (m_type)
    {
        case BridgeType::ENCLUSTRA:
        case BridgeType::UVC:
        case BridgeType::AMUNDSEN:
            builder = makeUnique<CameraCoreBuilderMira>();
            break;
        default:
            throw LogicError ("Can't create ICameraBuilder, unknown bridge type!");
    }

    builder->setEssentialSensors (m_config.essentialSensors);
    builder->setFlashMemoryConfig (m_config.flashMemoryConfig);
    builder->setTemperatureSensorConfig (m_config.temperatureSensorConfig);
    return builder;
}
