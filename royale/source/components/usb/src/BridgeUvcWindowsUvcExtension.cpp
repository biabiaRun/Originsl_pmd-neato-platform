/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#include <usb/bridge/BridgeUvcWindowsUvcExtension.hpp>

#include <common/exceptions/CouldNotOpen.hpp>
#include <common/exceptions/Disconnected.hpp>
#include <common/exceptions/LogicError.hpp>
#include <common/exceptions/PossiblyUsbStallError.hpp>

#include <common/events/EventCaptureStream.hpp>

#include <common/NarrowCast.hpp>
#include <common/RoyaleLogger.hpp>

#include <royale/Vector.hpp>

#include <initguid.h>

using namespace royale::common;
using namespace royale::usb::bridge;
using namespace royale::usb::config;
using std::size_t;


// This namespace is the vendor extensions that IFX have written for the FX3 and CX3
namespace
{
    /** The GUID used for the Infineon vendor extension (BridgeImager...) */
    // Arctic protocol {559596A5-9D86-4F62-BF52-ACBFC251F6CC}
    DEFINE_GUID (GUID_IFX_VENDOR_EXTENSION_ARCTIC,
                 0xa5969555, 0x869d, 0x624f, 0xbf, 0x52, 0xac, 0xbf, 0xc2, 0x51, 0xf6, 0xcc);

    /**
     * This driver searches for the vendor extension protocol by GUID.
     *
     * The data could be in the UsbProbeData struct, but this requires Windows-specific
     * data to handle.  The GUID is for the protocol, not the device, and so only a few
     * GUIDs will cover all devices.
     */
    const royale::Vector<royale::Pair<UvcExtensionType, GUID> > ALL_SUPPORTED_PROTOCOL_GUIDS
    {
        {UvcExtensionType::Arctic, GUID_IFX_VENDOR_EXTENSION_ARCTIC}
    };

    /**
     * Find the UVC extension which is the tunnel that I2C commands will be sent through.  This will return S_OK if a vendor extension is found.
     * Otherwise it will return a failed status, with vendorExtension empty and UvcExtensionType::None.
     *
     * \param mediaSource the UVC device itself, both DirectShow and MediaFoundation expose the same device
     * \param vendorExtension on return, access to the control channel
     * \param vendorExtensionNode on return, the node to access on the vendor extension control
     * \param vendorExtensionType type (None if no protocol was found)
     */
    HRESULT searchForVendorExtension (IUnknown *mediaSource,
                                      WrappedComPtr<IKsControl> &vendorExtension,
                                      KSP_NODE &vendorExtensionNode,
                                      UvcExtensionType &vendorExtensionType)
    {
        // Clear the arguments in case no extension is found
        vendorExtension.reset();
        std::memset (&vendorExtensionNode, 0, sizeof (vendorExtensionNode));
        vendorExtensionType = UvcExtensionType::None;

        WrappedComPtr<IKsTopologyInfo> uvcTopology;
        auto hr = mediaSource->QueryInterface (__uuidof (IKsTopologyInfo), uvcTopology.getRef());
        if (FAILED (hr))
        {
            return hr;
        }

        // The UVC extensions are a device-specific type of node, so first find a
        // device specifc node, and then check if it's the right one.
        DWORD numNodes;
        hr = uvcTopology->get_NumNodes (&numNodes);
        if (FAILED (hr))
        {
            return hr;
        }

        // Extension unit IDs, as accessed though this API, start at one not zero
        for (DWORD i = 1; i <= numNodes ; i++)
        {
            GUID nodeType;
            hr = uvcTopology->get_NodeType (i, &nodeType);
            if (SUCCEEDED (hr) && IsEqualGUID (KSNODETYPE_DEV_SPECIFIC, nodeType))
            {
                WrappedComPtr<IKsControl> control;
                hr = uvcTopology->CreateNodeInstance (i, __uuidof (IKsControl), control.getRef());
                if (SUCCEEDED (hr))
                {
                    for (const auto &protocol : ALL_SUPPORTED_PROTOCOL_GUIDS)
                    {
                        // now find out if this node has an interface with the property that we want
                        // try to read 0 bytes from it, see if the error is ERROR_MORE_DATA
                        KSP_NODE match = {0};
                        match.Property.Set = protocol.second;
                        match.Property.Id = KSPROPERTY_EXTENSION_UNIT_INFO;
                        match.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
                        match.NodeId = i;

                        ULONG bytesAvailable;
                        hr = control->KsProperty (reinterpret_cast<PKSPROPERTY> (&match), sizeof (match), NULL, 0, &bytesAvailable);
                        if (hr == HRESULT_FROM_WIN32 (ERROR_MORE_DATA))
                        {
                            LOG (DEBUG) << "Found the correct UVC vendor extension";
                            mediaSource->QueryInterface (__uuidof (IKsControl), vendorExtension.getRef());
                            vendorExtensionNode = match;
                            vendorExtensionType = protocol.first;
                            return S_OK;
                        }
                    }
                }
            }
        }
        return E_FAIL;
    }
}

BridgeUvcWindowsUvcExtension::BridgeUvcWindowsUvcExtension (WrappedComPtr<IUnknown> device) :
    m_device {device},
    m_vendorExtension {},
    m_vendorExtensionNode {}
{
}

BridgeUvcWindowsUvcExtension::~BridgeUvcWindowsUvcExtension()
{
    try
    {
        closeConnection();
    }
    catch (...)
    {
    }
}

void BridgeUvcWindowsUvcExtension::openConnection()
{
    std::unique_lock<std::recursive_mutex> lock (m_controlLock);
    if (m_vendorExtension)
    {
        throw LogicError ("The UVC extension should only be opened once");
    }
    // Get the tunnel that I2C commands will be sent through
    auto hr = searchForVendorExtension (m_device.get(), m_vendorExtension, m_vendorExtensionNode, m_vendorExtensionType);
    if (FAILED (hr) || ! m_vendorExtension)
    {
        throw CouldNotOpen ("Device found, but UVC vendor extension not found");
    }
}

void BridgeUvcWindowsUvcExtension::closeConnection()
{
    auto lock = lockVendorExtension();
    m_vendorExtension.reset();
}

royale::usb::config::UvcExtensionType BridgeUvcWindowsUvcExtension::getVendorExtensionType() const
{
    return m_vendorExtensionType;
}

std::unique_lock<std::recursive_mutex> BridgeUvcWindowsUvcExtension::lockVendorExtension()
{
    std::unique_lock<std::recursive_mutex> lock (m_controlLock);
    return lock;
}

bool BridgeUvcWindowsUvcExtension::onlySupportsFixedSize() const
{
    return false;
}

void BridgeUvcWindowsUvcExtension::vendorExtensionGet (uint16_t dataId, std::vector<uint8_t> &data)
{
    auto lock = lockVendorExtension();
    if (! m_vendorExtension)
    {
        throw Disconnected ();
    }

    ULONG bytesReturned;
    m_vendorExtensionNode.Property.Id = dataId;
    m_vendorExtensionNode.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
    auto hr = m_vendorExtension->KsProperty (&m_vendorExtensionNode.Property,
              narrow_cast<ULONG> (sizeof (m_vendorExtensionNode)),
              data.data (),
              narrow_cast<ULONG> (data.size ()),
              &bytesReturned);
    if (hr == HRESULT_FROM_WIN32 (ERROR_INVALID_FUNCTION))
    {
        // This is reached when the firmware detects an error and sends a USB STALL packet, but
        // there may be other causes of this (including not being able to talk to the firmware).
        throw PossiblyUsbStallError();
    }
    else if (hr == HRESULT_FROM_WIN32 (ERROR_GEN_FAILURE) || hr == HRESULT_FROM_WIN32 (ERROR_DEVICE_NOT_CONNECTED))
    {
        closeConnection();
        throw Disconnected ("Data transfer failed (disconnected?)");
    }
    else if (FAILED (hr))
    {
        throw RuntimeError ("Data transfer failed (status)");
    }
    else if (bytesReturned != data.size ())
    {
        throw RuntimeError ("Data transfer was not the expected size");
    }
}

void BridgeUvcWindowsUvcExtension::vendorExtensionSet (uint16_t id, const std::vector<uint8_t> &data)
{
    auto lock = lockVendorExtension();
    if (! m_vendorExtension)
    {
        throw Disconnected ();
    }

    ULONG bytesReturned;
    m_vendorExtensionNode.Property.Id = id;
    m_vendorExtensionNode.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
    auto hr = m_vendorExtension->KsProperty (&m_vendorExtensionNode.Property,
              narrow_cast<ULONG> (sizeof (m_vendorExtensionNode)),
              const_cast<uint8_t *> (data.data ()),
              narrow_cast<ULONG> (data.size ()),
              &bytesReturned);
    if (hr == HRESULT_FROM_WIN32 (ERROR_GEN_FAILURE) || hr == HRESULT_FROM_WIN32 (ERROR_DEVICE_NOT_CONNECTED))
    {
        closeConnection();
        throw Disconnected ("Data transfer failed (disconnected?)");
    }
    else if (FAILED (hr))
    {
        throw RuntimeError ("Data transfer failed (status)");
    }
    else if (bytesReturned != 0u)
    {
        throw RuntimeError ("Unexpected data transfer");
    }
}
