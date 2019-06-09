/****************************************************************************\
 * Copyright (C) 2016 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

// This file uses the CyAPI library from Cypress which is licensed under the Cypress
// Software License Agreement (see cypress_license.txt)
 
#include <usb/enumerator/BusEnumeratorEnclustraCyApi.hpp>

#include <usb/config/UsbProbeData.hpp>
#include <usb/factory/BridgeFactoryEnclustraCyApi.hpp>

#include <common/exceptions/InvalidValue.hpp>
#include <common/MakeUnique.hpp>
#include <common/RoyaleLogger.hpp>

using namespace royale::common;
using namespace royale::usb::enumerator;
using namespace royale::usb::config;
using namespace royale::config;

BusEnumeratorEnclustraCyApi::BusEnumeratorEnclustraCyApi (const UsbProbeDataList &probeData)
    : BusEnumeratorCyApi (probeData)
{
    for (const auto &pd : probeData)
    {
        if (pd.bridgeType != BridgeType::ENCLUSTRA)
        {
            throw InvalidValue ("Non-Enclustra device in Enclustra probe data");
        }
    }
}

BusEnumeratorEnclustraCyApi::~BusEnumeratorEnclustraCyApi() = default;

std::unique_ptr<royale::factory::IBridgeFactory> BusEnumeratorEnclustraCyApi::createBridgeFactory (
    std::unique_ptr<CCyUSBDevice> device) const
{
    return common::makeUnique<royale::factory::BridgeFactoryEnclustraCyApi> (std::move (device));
}
