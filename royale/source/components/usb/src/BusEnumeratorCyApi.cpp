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

#include <usb/enumerator/BusEnumeratorEnclustraCyApi.hpp>

#include <usb/config/UsbProbeData.hpp>
#include <usb/factory/BridgeFactoryEnclustraCyApi.hpp>

#include <common/exceptions/InvalidValue.hpp>
#include <common/MakeUnique.hpp>
#include <common/RoyaleLogger.hpp>

#include <guiddef.h>
#include <initguid.h>
#include <Windows.h>
#include <CyAPI.h>

#include <algorithm>
#include <set>

using namespace royale::common;
using namespace royale::usb::enumerator;
using namespace royale::usb::config;
using namespace royale::config;
using std::size_t;

namespace
{
    /** For sanity-checking that we have the correct device. */
    const UCHAR CY_ENDPOINT_COUNT = 4;

    // In addition to the GUIDs defined here, there's also CYUSBDRV_GUID, from CyAPI.h

    // {59158dd2-ecf1-4018-b1e6-334eb625e50f}
    DEFINE_GUID (GUID_DEV_CLASS_PMD_DEVICES,
                 0x59158dd2, 0xecf1, 0x4018, 0xb1, 0xe6, 0x33, 0x4e, 0xb6, 0x25, 0xe5, 0x0f);

    // {77B88EFB-2D85-492B-98D1-B6E33A132524}, this is not the PicoS or PicoFlexx
    DEFINE_GUID (GUID_DEVINTERFACE_CAMBOARDPICO,
                 0x77b88efb, 0x2d85, 0x492b, 0x98, 0xd1, 0xb6, 0xe3, 0x3a, 0x13, 0x25, 0x24);

    // {2C2B212B-BC84-4319-B465-367604F4F8FF}, used for both PicoS and PicoFlexx
    DEFINE_GUID (GUID_DEVINTERFACE_PMDVISION_CAMERA,
                 0x2c2b212b, 0xbc84, 0x4319, 0xb4, 0x65, 0x36, 0x76, 0x4, 0xf4, 0xf8, 0xff);

    // {F0152A21-83C9-456d-B307-0C852C2D7199}
    DEFINE_GUID (GUID_DEVINTERFACE_IFXEVALBOARD,
                 0xF0152A21, 0x83C9, 0x456d, 0xB3, 0x07, 0x0C, 0x85, 0x2C, 0x2D, 0x71, 0x99);
    /**
     * This probe searches for CyAPI devices by GUID, these are the GUIDS that it tries.
     *
     * The data could be in the UsbProbeData struct, but this requires Windows-specific
     * data to handle.  The GUID is for the driver, not the device, and so the same GUID
     * is used for all Pico devices.
     */
    const std::vector<GUID> ALL_CYAPI_ENCLUSTRA_GUIDS =
    {
        CYUSBDRV_GUID,
        GUID_DEV_CLASS_PMD_DEVICES,
        GUID_DEVINTERFACE_CAMBOARDPICO,
        GUID_DEVINTERFACE_PMDVISION_CAMERA,
        GUID_DEVINTERFACE_IFXEVALBOARD
    };


}

BusEnumeratorCyApi::BusEnumeratorCyApi (const UsbProbeDataList &probeData)
    : m_probeData (probeData)
{
}

BusEnumeratorCyApi::~BusEnumeratorCyApi() = default;

void BusEnumeratorCyApi::enumerateDevices (std::function<void (const UsbProbeData &, std::unique_ptr<royale::factory::IBridgeFactory>) > callback)
{
    for (auto &guid : ALL_CYAPI_ENCLUSTRA_GUIDS)
    {
        // We can only find one camera per CCyUSBDevice, calling Open() again will close the current
        // camera. Each pass through the searchThisGuidAgain loop opens an CCyUSBDevice, having
        // passed ownership of the first one to the callback.
        //
        // The CyAPI does not document whether the order of the device list will be the same for the
        // second CCyUSBDevice, which is why all devices are searched again.
        //
        // CyAPI may allow an application to open a device with multiple CCyUSBDevice instances,
        // this can depend on the device.  Logic is needed here to ensure that this function only
        // returns each device once, and doesn't become an infinite loop, which is why the device
        // paths are copied to the alreadyFound list.
        bool searchThisGuidAgain = true;
        std::set<std::string> alreadyFound;

        while (searchThisGuidAgain)
        {
            searchThisGuidAgain = false;

#if defined (ROYALE_LOGGING_VERBOSE_BRIDGE)
            LOG (DEBUG) << "Trying to connect GUID " << guid.Data1 << " ... ";
#endif
            auto device = common::makeUnique<CCyUSBDevice> (nullptr, guid);
            const UCHAR count = device->DeviceCount ();
            for (UCHAR i = 0; i < count; i++)
            {
#if defined (ROYALE_LOGGING_VERBOSE_BRIDGE)
                LOG (DEBUG) << "Trying to open device";
#endif
                if (! device->Open (i))
                {
                    // If a device fails here, it may be open in another application. If you've
                    // just written firmware to a device and the opening sequence fails here,
                    // please ensure that you've closed Cypress USB Control Center (the tool for
                    // writing firmware), as it may be holding exclusive access.
                    // \todo ROYAL-2520 report this to the application
                    continue;
                }

                const UsbProbeData *route = nullptr;
                for (auto &pd : m_probeData)
                {
                    if ( (pd.vid == device->VendorID) && (pd.pid == device->ProductID))
                    {
                        route = &pd;
                        break;
                    }
                }
                if (route == nullptr)
                {
                    continue;
                }

#if defined (ROYALE_LOGGING_VERBOSE_BRIDGE)
                LOG (DEBUG) << "Found a matching USB device";
#endif

                // DevPath is a fixed size array, char[256]. It's probably always a nul-terminated
                // string, but as that's not documented, sanity check it.
                //
                // The DevPath can't be treated as a 256-byte binary object. If the trailing bytes
                // after the nul are uninitialized then comparing the device paths as 256-byte
                // binary objects might open a single device multiple times, each time thinking the
                // name was different.
                if (strnlen (device->DevPath, sizeof (device->DevPath)) == sizeof (device->DevPath))
                {
                    LOG (ERROR) << "Found a device with a name longer than supported";
                    continue;
                }

                if (! alreadyFound.emplace (device->DevPath).second)
                {
                    // emplace() did not insert anything, this device was already in alreadyFound
                    continue;
                }

                auto bridgeFactory = createBridgeFactory (std::move (device));

                callback (*route, std::move (bridgeFactory));
                searchThisGuidAgain = true;
                break; // the for (i < device->DeviceCount) loop
            }
        }
    }
}
