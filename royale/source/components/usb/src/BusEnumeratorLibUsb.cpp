/****************************************************************************\
 * Copyright (C) 2017 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include <usb/enumerator/BusEnumeratorLibUsb.hpp>

#include <usb/descriptor/CameraDescriptorLibUsb.hpp>
#include <usb/config/UsbProbeData.hpp>

#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <common/RoyaleLogger.hpp>

#include <algorithm>
#include <iomanip>

using namespace royale::common;
using namespace royale::usb::enumerator;
using namespace royale::usb::descriptor;
using namespace royale::usb::config;
using namespace royale::config;
using std::size_t;

namespace
{
    /**
     * Deleter function for the shared_ptr<libusb_context*>
     */
    void exit_usb_context (libusb_context **context_pointer)
    {
        if (context_pointer != nullptr)
        {
            libusb_exit (*context_pointer);
            delete context_pointer;
        }
    }
}

BusEnumeratorLibUsb::BusEnumeratorLibUsb (const UsbProbeDataList &probeData)
    : m_usbContext {nullptr, exit_usb_context},
      m_probeData (probeData)
{
}

BusEnumeratorLibUsb::~BusEnumeratorLibUsb()
{
}

#if defined(TARGET_PLATFORM_ANDROID)
void BusEnumeratorLibUsb::enumerateDevices (std::function<void (const UsbProbeData &, std::unique_ptr<royale::factory::IBridgeFactory>) > callback, uint32_t androidUsbDeviceFD,
        uint32_t androidUsbDeviceVid,
        uint32_t androidUsbDevicePid)
#else
void BusEnumeratorLibUsb::enumerateDevices (std::function<void (const UsbProbeData &, std::unique_ptr<royale::factory::IBridgeFactory>) > callback)
#endif
{
    libusb_context **context = new libusb_context*;
    libusb_init (context);
    m_usbContext.reset (context, exit_usb_context);

#if defined(TARGET_PLATFORM_ANDROID)
    LOG (DEBUG) << "BusEnumeratorLibUsb::enumerateDevices (fd=" << androidUsbDeviceFD << ", PID=" << std::hex << androidUsbDevicePid;
    for (auto &pd : m_probeData)
    {
        if ( (androidUsbDeviceVid == pd.vid) && (androidUsbDevicePid == pd.pid))
        {
            LOG (DEBUG) << "Making a callback for a found USB device";
            std::unique_ptr<CameraDescriptorLibUsb> device (new CameraDescriptorLibUsb (m_usbContext, NULL, androidUsbDeviceFD));
            auto bridgeFactory = createBridgeFactory (std::move (device));
            callback (pd, std::move (bridgeFactory));
        }
    }
#else

    libusb_device **list = nullptr;
    ssize_t numdev = libusb_get_device_list (*m_usbContext, &list);
    if (numdev < 0)
    {
        LOG (ERROR) << "Error in enumerating devices";
        libusb_free_device_list (list, 1);
        throw CouldNotOpen ("Error in enumerating devices");
    }

    struct libusb_device_descriptor usbinfo;
    for (size_t i = 0; i < static_cast<size_t> (numdev); ++i)
    {
#if defined (ROYALE_LOGGING_VERBOSE_BRIDGE)
        LOG (DEBUG) << "BusEnumeratorLibUsb iterating over list " << i;
#endif
        for (auto &pd : m_probeData)
        {
            libusb_get_device_descriptor (list[i], &usbinfo);
            if ( (usbinfo.idVendor == pd.vid) && (usbinfo.idProduct == pd.pid))
            {
                LOG (DEBUG) << "Found a device, USB ID "
                            << std::setfill ('0') << std::setw (4) << std::hex << pd.vid << ":"
                            << std::setfill ('0') << std::setw (4) << std::hex << pd.pid;
                std::unique_ptr<CameraDescriptorLibUsb> device (new CameraDescriptorLibUsb (m_usbContext, list[i]));
                auto bridgeFactory = createBridgeFactory (std::move (device));
                callback (pd, std::move (bridgeFactory));
            }
            else
            {
#if defined (ROYALE_LOGGING_VERBOSE_BRIDGE)
                LOG (DEBUG) << "... no match with USB ID "
                            << std::setfill ('0') << std::setw (4) << std::hex << pd.vid << ":"
                            << std::setfill ('0') << std::setw (4) << std::hex << pd.pid;
#endif
            }
        }
    }
    // The devices are ref-counted, and the CameraDescriptorLibUsb increments the refcount.  We can
    // delete the list immediately without affecting the probed data.
    libusb_free_device_list (list, 1);

#endif

}
