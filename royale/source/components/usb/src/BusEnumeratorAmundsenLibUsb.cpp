/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <usb/enumerator/BusEnumeratorAmundsenLibUsb.hpp>

#include <usb/descriptor/CameraDescriptorLibUsb.hpp>
#include <usb/config/UsbProbeData.hpp>
#include <usb/factory/BridgeFactoryAmundsenLibUsb.hpp>

#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/MakeUnique.hpp>
#include <common/RoyaleLogger.hpp>

#include <algorithm>

using namespace royale::common;
using namespace royale::usb::enumerator;
using namespace royale::usb::descriptor;
using namespace royale::usb::config;
using namespace royale::config;
using namespace royale::usb::bridge;
using std::size_t;

BusEnumeratorAmundsenLibUsb::BusEnumeratorAmundsenLibUsb (const UsbProbeDataList &probeData) :
    BusEnumeratorLibUsb (probeData)
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

BusEnumeratorAmundsenLibUsb::~BusEnumeratorAmundsenLibUsb() = default;

std::unique_ptr<royale::factory::IBridgeFactory> BusEnumeratorAmundsenLibUsb::createBridgeFactory (
    std::unique_ptr<CameraDescriptorLibUsb> device) const
{
    return common::makeUnique<factory::BridgeFactoryAmundsenLibUsb> (std::move (device));
}
