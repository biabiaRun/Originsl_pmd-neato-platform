/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/enumerator/BusEnumeratorUvcDirectShow.hpp>

#include <usb/bridge/WrappedComPtr.hpp>

#include <usb/config/UsbProbeData.hpp>
#include <usb/factory/BridgeFactoryUvcDirectShow.hpp>

#include <common/exceptions/InvalidValue.hpp>
#include <common/MakeUnique.hpp>
#include <common/RoyaleLogger.hpp>
#include <royale/IEvent.hpp>

#include <mfidl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <codecvt>
#include <memory>

#include <dshow.h>
#include <vidcap.h>
#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>


using namespace royale::usb::descriptor;
using namespace royale::common;
using namespace royale::usb::enumerator;
using namespace royale::usb::config;
using namespace royale::config;
using namespace royale::usb::bridge;
using std::size_t;


BusEnumeratorUvcDirectShow::BusEnumeratorUvcDirectShow (const UsbProbeDataList &probeData)
    : m_probeData (probeData)
{
    for (const auto &pd : m_probeData)
    {
        if (pd.bridgeType != BridgeType::UVC)
        {
            throw InvalidValue ("Non-UVC device in UVC probe data");
        }
    }
}

BusEnumeratorUvcDirectShow::~BusEnumeratorUvcDirectShow ()
{
}

namespace
{
    /**
     * Returns true if the USB vendorId and productId match.
     * Returns false if the VID and PID don't match.
     */
    bool tryUsbMatch (uint16_t vid, uint16_t pid, IPropertyBag *pPropBag)
    {
        bool match = false;
        VARIANT devPath;
        VariantInit (&devPath);

        // Extract the VID/PID information from device path
        auto hr = pPropBag->Read (L"DevicePath", &devPath, NULL);
        if (SUCCEEDED (hr))
        {
            if ( (devPath.vt == VT_BSTR) && (SysStringLen (devPath.bstrVal) >= 0x30))
            {
                // Convert ascii hex values from the multi-byte string.  The 0x20 converts A-F to a-f,
                // the 0x30 converts 0-9 to the right value, and the 0x27 converts a-f to the right value.
                WORD cVID = 0, cPID = 0;
                for (BYTE i = 0; i < 8; i += 2)
                {
                    cVID += ( ( (devPath.pbVal[0x1E - i] | 0x20) - 0x30) % 0x27) << (2 * i);
                    cPID += ( ( (devPath.pbVal[0x30 - i] | 0x20) - 0x30) % 0x27) << (2 * i);
                }

                if (vid == cVID && pid == cPID)
                {
                    match = true;
                }
            }
            VariantClear (&devPath);
        }
        return match;
    }

}

void BusEnumeratorUvcDirectShow::enumerateDevices (
    std::function<void (const royale::usb::config::UsbProbeData &,
                        std::unique_ptr<royale::factory::IBridgeFactory>) > callback)
{
    enumerateDevicesWithInfo (callback, nullptr);
}

void BusEnumeratorUvcDirectShow::enumerateDevicesWithInfo (
    std::function<void (const royale::usb::config::UsbProbeData &,
                        std::unique_ptr<royale::factory::IBridgeFactory>) > callback,
    royale::device::ProbeResultInfo *probeResultInfo)
{
    WrappedComPtr<ICreateDevEnum> devEnum;
    auto hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, devEnum.getRef());
    if (hr == CO_E_NOTINITIALIZED)
    {
        return;
    }
    else if (SUCCEEDED (hr))
    {
        WrappedComPtr<IEnumMoniker> enumCat;
        hr = devEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, enumCat.getTypeRef(), 0);
        if (hr == S_OK) // hr==S_FALSE means there were no devices, and enumCat is a nullptr
        {

            // The next loop might leak all but the last moniker.  The MSDN documentation for
            // IEnumMoniker does the same, it releases only the last one.
            WrappedComPtr<IMoniker> moniker;
            while (enumCat->Next (1, moniker.getTypeRef(), NULL) == S_OK)
            {
                WrappedComPtr<IPropertyBag> propBag;
                hr = moniker->BindToStorage (NULL, NULL, IID_IPropertyBag, propBag.getRef());
                if (FAILED (hr))
                {
                    continue;
                }

                HRESULT hr;
                VARIANT name;
                VariantInit (&name);
                hr = propBag->Read (L"FriendlyName", &name, NULL);
                if (SUCCEEDED (hr))
                {
                    if (probeResultInfo != nullptr)
                    {
                        std::wstring probedDeviceNameWide (name.bstrVal,
                                                           SysStringLen (name.bstrVal));

                        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                        royale::String probedDeviceName = converter.to_bytes (probedDeviceNameWide);
                        royale::device::ProbedDeviceInfo probedDeviceInfo;

                        probedDeviceInfo.setDeviceName (probedDeviceName);
                        probeResultInfo->addDeviceInfo (probedDeviceInfo);
                    }
                }
                VariantClear (&name);

                for (const auto &pd : m_probeData)
                {
                    if (tryUsbMatch (pd.vid, pd.pid, propBag.get()))
                    {
                        auto desc = common::makeUnique<CameraDescriptorDirectShow> (moniker);
                        auto bridgeFactory = common::makeUnique<factory::BridgeFactoryUvcDirectShow> (std::move (desc));
                        callback (pd, std::move (bridgeFactory));
                        // each device should match at most one route
                        break;
                    }
                }
            }
        }
    }
}
