/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#include <modules/ModuleConfigFactorySalome.hpp>

#include <modules/ModuleConfigData.hpp>
#include <config/SensorRoutingImagerAsBridge.hpp>

using namespace royale::factory;
using namespace royale::common;
using namespace royale::config;

namespace
{
    const auto salomeModules = royale::Vector<ModuleConfigFactoryByStorageId::value_type>
                               {
    {
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        moduleconfig::SalomeDefault
    },
    {
        // Salome (K9 / 60x45 / 945nm / 202 / PO)
        // Module ID hash : 0x1376DA3C
        { 0x01, 0x02, 0x23, 0x00, 0x00, 0x60, 0x62, 0x00, 0x20, 0x00, 0x03, 0x01, 0x04, 0x00, 0x00, 0x00 },
        moduleconfig::Salome940nm
    },
    {
        // Salome rev 2(K9 / 60x45 / 945nm / 202 / PO / ECM)
        // Module Code : 0102-2600-0060-6300-2000-0302-0400-0000
        { 0x01, 0x02, 0x26, 0x00, 0x00, 0x60, 0x63, 0x00, 0x20, 0x00, 0x03, 0x02, 0x04, 0x00, 0x00, 0x00 },
        moduleconfig::Salome2Rev940nm
    },
    {
        // Salome rev 2(K9 / 60x45 / 945nm / 202 / PO / ICM)
        // Module Code : 0102-2600-1060-6300-2000-0302-0400-0000
        { 0x01, 0x02, 0x26, 0x00, 0x10, 0x60, 0x63, 0x00, 0x20, 0x00, 0x03, 0x02, 0x04, 0x00, 0x00, 0x00 },
        moduleconfig::Salome2Rev940nm
    },
    {
        // Salome rev 2(Leica / 60x45 / 945nm / 202 / PO / ECM)
        // Module Code : 0102-2600-0100-6300-2000-0302-0400-0000
        { 0x01, 0x02, 0x26, 0x00, 0x01, 0x00, 0x63, 0x00, 0x20, 0x00, 0x03, 0x02, 0x04, 0x00, 0x00, 0x00 },
        moduleconfig::Salome2Rev940nm
    },
    {
        // Salome rev 2(Leica / 60x45 / 945nm / 202 / PO / ICM)
        // Module Code : 0102-2600-1100-6300-2000-0302-0400-0000
        { 0x01, 0x02, 0x26, 0x00, 0x11, 0x00, 0x63, 0x00, 0x20, 0x00, 0x03, 0x02, 0x04, 0x00, 0x00, 0x00},
        moduleconfig::Salome2Rev940nm
    },

                               };

    const auto salomeRouting = std::make_shared<SensorRoutingImagerAsBridge> (ImagerAsBridgeType::M2453_SPI);

    const auto salomeConfig = FlashMemoryConfig
                              {
                                  FlashMemoryConfig::FlashMemoryType::POLAR_RANDOM
                              }
                              .setImageSize (128 * 1024 * 2)
                              .setPageSize (256)
                              .setAccessOffset (0x2000u)
                              .setUseCaching (true);

    const auto salomeDefault = royale::Vector<uint8_t>
                               {
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                               };
}

ModuleConfigFactorySalome::ModuleConfigFactorySalome() :
    ModuleConfigFactoryByStorageId (salomeRouting, salomeConfig, salomeModules, salomeDefault)
{
}

