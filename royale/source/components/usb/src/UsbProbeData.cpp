/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/config/UsbProbeData.hpp>

#include <tuple>
#include <algorithm>

using royale::usb::config::UsbProbeData;
using royale::usb::config::UsbProbeDataList;

UsbProbeDataList royale::usb::config::filterUsbProbeDataByBridgeType (const UsbProbeDataList &data, BridgeType bridgeType)
{
    UsbProbeDataList ret;

    std::copy_if (data.begin(), data.end(), std::inserter (ret, ret.end()),
                  [bridgeType] (const UsbProbeData & pd) -> bool { return pd.bridgeType == bridgeType; }
                 );
    return ret;
}

