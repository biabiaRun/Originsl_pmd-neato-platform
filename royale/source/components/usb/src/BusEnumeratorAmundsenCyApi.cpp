/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

// This file uses the CyAPI library from Cypress which is licensed under the Cypress
// Software License Agreement (see cypress_license.txt)

#include <usb/enumerator/BusEnumeratorAmundsenCyApi.hpp>

#include <usb/config/UsbProbeData.hpp>
#include <usb/factory/BridgeFactoryAmundsenCyApi.hpp>

#include <common/exceptions/InvalidValue.hpp>
#include <common/MakeUnique.hpp>
#include <common/RoyaleLogger.hpp>

using namespace royale::common;
using namespace royale::usb::enumerator;
using namespace royale::usb::config;
using namespace royale::config;

BusEnumeratorAmundsenCyApi::BusEnumeratorAmundsenCyApi (const UsbProbeDataList &probeData) :
    BusEnumeratorCyApi (probeData)
{
    for (const auto &pd : probeData)
    {
        if (pd.bridgeType == BridgeType::AMUNDSEN)
        {
            continue;
        }
#ifdef ROYALE_BRIDGE_UVC_AMUNDSEN
        if (pd.bridgeType == BridgeType::UVC)
        {
            continue;
        }
#endif
        throw InvalidValue ("Non-Amundsen device in Amundsen probe data");
    }
}

BusEnumeratorAmundsenCyApi::~BusEnumeratorAmundsenCyApi() = default;

std::unique_ptr<royale::factory::IBridgeFactory> BusEnumeratorAmundsenCyApi::createBridgeFactory (
    std::unique_ptr<CCyUSBDevice> device) const
{
    return common::makeUnique<royale::factory::BridgeFactoryAmundsenCyApi> (std::move (device));
}
