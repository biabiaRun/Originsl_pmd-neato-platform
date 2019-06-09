/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/bridge/BridgeUvcCommon.hpp>

using namespace royale::common;
using namespace royale::usb::bridge;
using namespace royale::usb::pal;

BridgeUvcCommon::BridgeUvcCommon() = default;
BridgeUvcCommon::~BridgeUvcCommon() = default;

void BridgeUvcCommon::setUvcBridgeInfo (royale::Vector<royale::Pair<royale::String, royale::String>> &&info)
{
    m_uvcBridgeInfo = info;
}

royale::Vector<royale::Pair<royale::String, royale::String>> BridgeUvcCommon::getBridgeInfo()
{
    auto info = BridgeCopyAndNormalize::getBridgeInfo();
    for (const auto &x : m_uvcBridgeInfo)
    {
        info.emplace_back (x);
    }
    info.emplace_back ("BRIDGE_TYPE", "UVC");

    return info;
}
